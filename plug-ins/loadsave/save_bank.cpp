/* Object bank cell store. Re-keyed sibling of save_drop.cpp (ruffina, 2004);
 * same #O..End record format and serializer, keyed by owner instead of room.
 *
 * One ENTRY is one file bank/<kind>/<key>/<Id> holding a top-level object and
 * its whole Nest subtree. Per-entry files make selective withdrawal a
 * read-one-file-and-unlink and browsing a header-peek per file, so neither ever
 * needs to splice records out of a shared cell.
 *
 * Deposit/withdraw follow player quit/login semantics, NOT destruction: the
 * object leaves object_list but its Fenia wrapper (guts + DB entry) is kept and
 * relinked by Id on the way back, and the proto count is left untouched so a
 * banked limited item can't repop under it. */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <algorithm>

#include "fileformatexception.h"
#include "logstream.h"

#include "object.h"
#include "affect.h"
#include "character.h"
#include "area.h"
#include "lang.h"
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
static void bank_owner_dir( char *out, size_t n, const DLString &kind, const DLString &key )
{
    snprintf( out, n, "%s/bank/%s/%s",
              dreamland->getSavedDir( ).getPath( ).c_str( ), kind.c_str( ), key.c_str( ) );
}

static void bank_entry_path( char *out, size_t n, const DLString &kind, const DLString &key, long long id )
{
    snprintf( out, n, "%s/bank/%s/%s/%lld",
              dreamland->getSavedDir( ).getPath( ).c_str( ), kind.c_str( ), key.c_str( ), id );
}

/* fopen will not create parent dirs, so ensure bank/<kind>/<key>/ exists. A
 * failed mkdir just makes the later fopen fail, and deposit bails before it
 * mutates or extracts anything -- the safe failure ordering. */
static void bank_ensure_dir( const DLString &kind, const DLString &key )
{
    char path[MAX_INPUT_LENGTH];

    snprintf( path, sizeof( path ), "%s/bank",
              dreamland->getSavedDir( ).getPath( ).c_str( ) );
    ::mkdir( path, 0775 );

    snprintf( path, sizeof( path ), "%s/bank/%s",
              dreamland->getSavedDir( ).getPath( ).c_str( ), kind.c_str( ) );
    ::mkdir( path, 0775 );

    bank_owner_dir( path, sizeof( path ), kind, key );
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

    bank_ensure_dir( kind, key );

    char fname[MAX_INPUT_LENGTH];
    bank_entry_path( fname, sizeof( fname ), kind, key, obj->getID( ) );

    // One file = one entry, so truncate: the Id names the file and the object is
    // currently in-world (not already banked), so nothing valuable is here. Open
    // BEFORE stripping affects -- a failed open must not leave the item mutated
    // (temp affects already gone) but still in the world.
    FILE *fp = fopen( fname, "w" );
    if ( fp == 0 ) {
        bug( "bank_deposit: cannot open entry file for writing.", 0 );
        return false;
    }

    bank_strip_temp_affects( obj );

    // ch == NULL: skip the player-ownership "crumble" checks in fwrite_obj_0.
    // fwrite_obj_0 emits the #O..End record and recurses into contained items,
    // so a bag writes its whole subtree into this one file.
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
 * browse -- header peek, no object materialized
 *------------------------------------------------------------------------*/

/* Read just enough of an entry file to fill a BankEntry: the first record's
 * Vnum / ShortDesc override / Ityp / Lev, plus a count of #O blocks for a bag's
 * contents. Line-based on purpose -- Vnum and ShortDesc are written before the
 * possibly multi-line Description, so the display name is always read right;
 * Ityp/Lev come after it and might be missed on a pathological Description, but
 * that only falls back to the prototype's type/level (cosmetic). Never trusted
 * by withdrawal. */
static bool bank_entry_peek( const char *fname, BankEntry &be )
{
    FILE *fp = fopen( fname, "r" );
    if ( fp == 0 )
        return false;

    char line[MAX_STRING_LENGTH];
    int  objCount = 0;
    bool inFirst = false;
    bool firstDone = false;

    while ( fgets( line, sizeof( line ), fp ) != 0 ) {
        size_t len = strlen( line );
        while ( len > 0 && ( line[len - 1] == '\n' || line[len - 1] == '\r' ) )
            line[--len] = '\0';

        if ( !strcmp( line, "#O" ) ) {
            objCount++;
            inFirst = ( objCount == 1 );
            continue;
        }

        if ( !inFirst || firstDone )
            continue;

        if ( !strncmp( line, "Vnum ", 5 ) ) {
            be.vnum = atoi( line + 5 );
        }
        else if ( !strncmp( line, "ShortDesc ", 10 ) ) {
            // "ShortDesc <attr> <value>~"
            char *p = line + 10;
            char *sp = strchr( p, ' ' );
            if ( sp != 0 ) {
                *sp = '\0';
                DLString attr = p;
                char *val = sp + 1;
                char *tilde = strrchr( val, '~' );
                if ( tilde != 0 )
                    *tilde = '\0';
                be.shortDescr[ attr2lang( attr ) ] = val;
            }
        }
        else if ( !strncmp( line, "Ityp ", 5 ) ) {
            be.itemType = atoi( line + 5 );
        }
        else if ( !strncmp( line, "Lev ", 4 ) ) {
            be.level = atoi( line + 4 );
        }
        else if ( !strcmp( line, "End" ) ) {
            firstDone = true;
            inFirst = false;
        }
    }

    fclose( fp );

    be.contents = ( objCount > 0 ) ? objCount - 1 : 0;
    return objCount > 0;
}

void bank_browse( const DLString &kind, const DLString &key, std::vector<BankEntry> &out )
{
    char dir[MAX_INPUT_LENGTH];
    bank_owner_dir( dir, sizeof( dir ), kind, key );

    DIR *d = opendir( dir );
    if ( d == 0 )
        return;

    struct dirent *ent;
    while ( ( ent = readdir( d ) ) != 0 ) {
        const char *name = ent->d_name;

        // Entry files are named by decimal Id: skip ".", "..", and anything that
        // is not all digits so a stray file never becomes a phantom entry.
        if ( name[0] == '\0' || !isdigit( (unsigned char)name[0] ) )
            continue;
        bool allDigits = true;
        for ( const char *p = name; *p != '\0'; p++ )
            if ( !isdigit( (unsigned char)*p ) ) { allDigits = false; break; }
        if ( !allDigits )
            continue;

        long long id = atoll( name );

        char path[MAX_INPUT_LENGTH];
        bank_entry_path( path, sizeof( path ), kind, key, id );

        BankEntry be;
        be.id = id;
        if ( bank_entry_peek( path, be ) )
            out.push_back( be );
    }

    closedir( d );

    std::sort( out.begin( ), out.end( ),
               []( const BankEntry &a, const BankEntry &b ) { return a.id < b.id; } );
}

/*-------------------------------------------------------------------------
 * withdrawal -- one entry file
 *------------------------------------------------------------------------*/
static bool bank_read_entry_file( Character *ch, const char *fname )
{
    FILE *fp = fopen( fname, "r" );
    if ( fp == 0 )
        return false;

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
            clean = true;                          // empty file: nothing to read
        }
        else {
            fseek( fp, 0L, SEEK_SET );

            for ( ; ; ) {
                // The file has no explicit #END terminator, and each record ends
                // with a behavior blob whose trailing "~\n\n" leaves feof() unset
                // after the last record read -- a bare feof() probe would then let
                // fread_word() spin on EOF and throw "word too long". Skip
                // inter-record whitespace ourselves (same dl_isspace fread_word
                // uses) and treat a real EOF as the end.
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
                    bug( "bank_read_entry_file: # not found.", 0 );
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
                    bug( "bank_read_entry_file: bad section.", 0 );
                    break;                         // clean stays false
                }
            }
        }
    }
    catch ( const Exception &e ) {                 // FileFormatException derives from this
        LogStream::sendError( ) << "bank_read_entry_file: " << e.what( ) << endl;
        clean = false;
    }

    fclose( fp );
    return clean;
}

bool bank_withdraw_entry( Character *ch, const DLString &kind, const DLString &key, long long id )
{
    if ( ch == 0 )
        return false;

    char fname[MAX_INPUT_LENGTH];
    bank_entry_path( fname, sizeof( fname ), kind, key, id );

    bool clean = bank_read_entry_file( ch, fname );

    // Only unlink when the file read cleanly end to end. A mid-file failure keeps
    // the file for inspection instead of destroying the unread records.
    if ( clean )
        unlink( fname );

    return clean;
}
