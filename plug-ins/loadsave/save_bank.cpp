/* Object bank cell store. Re-keyed sibling of save_drop.cpp (ruffina, 2004);
 * same #O..End record format and serializer, keyed by owner instead of room.
 *
 * Deposit/withdraw follow player quit/login semantics, NOT destruction: the
 * object leaves object_list but its Fenia wrapper (guts + DB entry) is kept and
 * relinked by Id on the way back, and the proto count is left untouched so a
 * banked limited item can't repop under it. */

#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fileformatexception.h"
#include "logstream.h"

#include "object.h"
#include "affect.h"
#include "character.h"
#include "area.h"
#include "dl_ctype.h"
#include "dreamland.h"
#include "save.h"
#include "save_bank.h"
#include "loadsave.h"
#include "fread_utils.h"
#include "merc.h"

#include "def.h"

/*-------------------------------------------------------------------------
 * cell paths
 *------------------------------------------------------------------------*/
static void bank_cell_path( char *out, size_t n, const DLString &kind, const DLString &key )
{
    snprintf( out, n, "%s/bank/%s/%s",
              dreamland->getSavedDir( ).getPath( ).c_str( ), kind.c_str( ), key.c_str( ) );
}

/* fopen("a") will not create parent dirs, so ensure bank/<kind>/ exists. A
 * failed mkdir just makes the later fopen fail, and deposit bails before it
 * mutates or extracts anything -- the safe failure ordering. */
static void bank_ensure_dir( const DLString &kind )
{
    char path[MAX_INPUT_LENGTH];

    snprintf( path, sizeof( path ), "%s/bank",
              dreamland->getSavedDir( ).getPath( ).c_str( ) );
    ::mkdir( path, 0775 );

    snprintf( path, sizeof( path ), "%s/bank/%s",
              dreamland->getSavedDir( ).getPath( ).c_str( ), kind.c_str( ) );
    ::mkdir( path, 0775 );
}

/*-------------------------------------------------------------------------
 * temp-affect stripping
 *------------------------------------------------------------------------*/
void bank_strip_temp_affects( Object *obj )
{
    if ( obj == 0 )
        return;

    if ( !obj->affected.empty( ) ) {
        // Clone so affect_remove_obj can mutate the real list under us: this is
        // the same snapshot-then-remove idiom the affect-expiry loops use, and
        // clone() keeps the same Affect* pointers so remove() matches by identity.
        AffectList affects = obj->affected.clone( );
        for ( auto paf_iter = affects.cbegin( ); paf_iter != affects.cend( ); paf_iter++ ) {
            Affect *paf = *paf_iter;
            if ( paf->duration >= 0 )                 // temporary -> strip
                affect_remove_obj( obj, paf, false );
            // duration < 0 -> permanent, keep
        }
    }

    for ( Object *content = obj->contains; content != 0; content = content->next_content )
        bank_strip_temp_affects( content );
}

/*-------------------------------------------------------------------------
 * deposit
 *------------------------------------------------------------------------*/

/* fwrite_obj_0 silently writes nothing for a NOSAVEDROP area/item (save.cpp
 * 617-621). Depositing such an object would serialize nothing and then extract
 * it -- a silent destruction. Refuse the whole subtree if any node is affected. */
static bool bank_subtree_saveable( Object *obj )
{
    if ( IS_SET( obj->pIndexData->area->area_flag, AREA_NOSAVEDROP ) )
        return false;
    if ( IS_SET( obj->extra_flags, ITEM_NOSAVEDROP ) )
        return false;

    for ( Object *content = obj->contains; content != 0; content = content->next_content )
        if ( !bank_subtree_saveable( content ) )
            return false;

    return true;
}

bool bank_deposit( Object *obj, const DLString &kind, const DLString &key )
{
    if ( obj == 0 )
        return false;

    // Refuse before any mutation: nothing NOSAVEDROP may be banked, or the
    // serializer drops it and we destroy it on extract.
    if ( !bank_subtree_saveable( obj ) )
        return false;

    bank_ensure_dir( kind );

    char fname[MAX_INPUT_LENGTH];
    bank_cell_path( fname, sizeof( fname ), kind, key );

    // Open BEFORE stripping affects: a failed open must not leave the item
    // mutated (temp affects already gone) but still in the world.
    FILE *fp = fopen( fname, "a" );
    if ( fp == 0 ) {
        bug( "bank_deposit: cannot open cell for writing.", 0 );
        return false;
    }

    bank_strip_temp_affects( obj );

    // ch == NULL: skip the player-ownership "crumble" checks in fwrite_obj_0.
    // fwrite_obj_0 emits the #O..End record and recurses into contained items.
    fwrite_obj_0( 0, obj, fp, 0 );
    fflush( fp );
    fclose( fp );

    // Logout semantics: nocount extract keeps the Fenia wrapper alive (guts + DB
    // entry survive, relinked by Id on withdrawal, exactly like player logout)
    // and leaves the proto count intact so a banked limited item can't repop
    // under it. A plain extract_obj (count=true) would clear the wrapper guts and
    // fire onExtract as "destroyed forever" -- that is the bug this avoids.
    extract_obj_nocount( obj );
    return true;
}

/*-------------------------------------------------------------------------
 * whole-cell withdrawal
 *------------------------------------------------------------------------*/
void bank_withdraw_all( Character *ch, const DLString &kind, const DLString &key )
{
    if ( ch == 0 )
        return;

    char fname[MAX_INPUT_LENGTH];
    bank_cell_path( fname, sizeof( fname ), kind, key );

    FILE *fp = fopen( fname, "r" );
    if ( fp == 0 )
        return;

    // Mirror the pfile object load (pcharacter.cpp:240-268): zero the nest
    // tracker so a truncated record can't misparent into a stale container from
    // an earlier load, and do NOT set create_obj_dropped -- like login,
    // withdrawal must not bump the proto count (nocount deposit never dropped it).
    for ( int iNest = 0; iNest < MAX_NEST; iNest++ )
        rgObjNest[iNest] = 0;

    bool clean = false;

    try {
        fseek( fp, 0L, SEEK_END );
        if ( ftell( fp ) <= 0 ) {
            clean = true;                          // empty cell: nothing to read
        }
        else {
            fseek( fp, 0L, SEEK_SET );

            for ( ; ; ) {
                // The cell has no explicit #END terminator (append-mode deposits),
                // and each record ends with a behavior blob whose trailing "~\n\n"
                // leaves feof() unset after the last record read -- a bare feof()
                // probe would then let fread_word() spin on EOF and throw
                // "word too long". Skip inter-record whitespace ourselves (same
                // dl_isspace fread_word uses) and treat a real EOF as the end.
                int c;
                do { c = getc( fp ); } while ( c != EOF && dl_isspace( (char)c ) );

                const char *word;
                if ( c == EOF ) {
                    word = "#END";
                }
                else {
                    ungetc( c, fp );
                    word = fread_word( fp );
                }

                char letter = word[0];

                if ( letter == '*' ) {
                    fread_to_eol( fp );
                    continue;
                }

                if ( letter != '#' ) {
                    bug( "bank_withdraw_all: # not found.", 0 );
                    break;                         // clean stays false
                }

                word++;

                if ( !strcmp( word, "OBJECT" ) || !strcmp( word, "O" ) ) {
                    fread_obj( ch, 0, fp );
                }
                else if ( !strcmp( word, "END" ) ) {
                    clean = true;
                    break;
                }
                else {
                    bug( "bank_withdraw_all: bad section.", 0 );
                    break;                         // clean stays false
                }
            }
        }
    }
    catch ( const Exception &e ) {                 // FileFormatException derives from this
        LogStream::sendError( ) << "bank_withdraw_all: " << e.what( ) << endl;
        clean = false;
    }

    fclose( fp );

    // Only delete the cell when it read cleanly end to end. A mid-cell failure
    // keeps the file for inspection instead of destroying the unread records.
    if ( clean )
        unlink( fname );
}
