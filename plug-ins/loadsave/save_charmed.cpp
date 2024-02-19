/* $Id$
 *
 * ruffina, 2004
 */
#include <errno.h>
#include <sys/stat.h>
#include <set>
#include <string.h>

#include "fileformatexception.h"
#include "logstream.h"

#include "npcharacter.h"
#include "room.h"

#include "dreamland.h"
#include "loadsave.h"
#include "save.h"
#include "fread_utils.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

const char * DIR_SAVED_CHARMED = "charmed";
const unsigned int HASH_SAVED_CHARMED = 16;

set<int> room_markers;

static long long make_fnum( NPCharacter *ch )
{
    return ch->getID( );
}

static int make_dirnum( NPCharacter *ch )
{
    return ch->getID( ) % HASH_SAVED_CHARMED;
}

static DLString make_fname( NPCharacter *ch )
{
    DLString fname;

    fname << dreamland->getSavedDir( ).getPath( ) << "/" 
          << DIR_SAVED_CHARMED << "/"
          << make_dirnum( ch ) << "/"
          << make_fnum( ch );

    return fname;
}

static DLString make_dirname( int dirnum )
{
    DLString dirname;

    dirname << dreamland->getSavedDir( ).getPath( ) << "/" 
            << DIR_SAVED_CHARMED << "/"
            << dirnum;

    return dirname;
}

int
mymkdir(const char *path, int perm) 
{
#ifdef __MINGW32__
    return mkdir(path);
#else
    return mkdir(path, perm);
#endif
}

void save_creature( NPCharacter *ch )
{
    FILE *fp;
    
    if (ch == 0) {
        LogStream::sendError( ) << "save_creature: mob is null during save" << endl;
        return;
    }

    if (ch->in_room == 0) {
        LogStream::sendError( ) << "save_creature: mob " << ch->pIndexData->vnum << " in null room during save" << endl;
        return;
    }

    if (( fp = fopen( make_fname( ch ).c_str( ), "w" ) ) == NULL) {
        if (errno == ENOENT) {
            if (mymkdir( make_dirname( make_dirnum( ch ) ).c_str( ), S_IRWXU )) {
                LogStream::sendError( ) << "save_creature: mkdir for " << make_fnum( ch ) << ":" << strerror( errno ) << endl;
                return;
            }
            else
                fp = fopen( make_fname( ch ).c_str( ), "w" );
        }
        
        if (fp == NULL) {
            LogStream::sendError( ) << "save_creature: fopen of " << make_fnum( ch ) << ":" << strerror( errno ) << endl;
            return;
        }
    }

    fwrite_mob( ch, fp );
    fprintf( fp,"\n#End\n" );
    fflush( fp );
    fclose( fp );
}

void unsave_creature( NPCharacter *ch )
{
    if (ch == 0) {
        LogStream::sendError( ) << "unsave_creature: mob is null during unsave" << endl;
        return;
    }

    if (unlink( make_fname( ch ).c_str( ) ) == -1)
        LogStream::sendError( ) << "unsave_creature: unlink of " << make_fnum( ch ) << ":" << strerror( errno ) << endl;
}

static void load_one_creature( const DLString &path )
{
    NPCharacter *ch = 0;
    FILE *fp;
    bool buggy = false;

    if (( fp = fopen( path.c_str( ), "r" ) ) == NULL ) {
        unlink( path.c_str( ) ); 
        throw FileFormatException( strerror( errno ) );
    }

    create_obj_dropped = true;

    for ( ; ; ) {
        try {
            char letter;
            char *word;

            letter = fread_letter( fp );
            if ( letter == '*' )
            {
                fread_to_eol( fp );
                continue;
            }

            if ( letter != '#' )
            {            
                LogStream::sendError( ) << "[" << path << "]: # not found." << endl;
                buggy = true;
                break;
            }

            word = fread_word( fp );
            if ( !str_cmp( word, "MOBILE" ) )
            {
                ch = 0;
                ch = fread_mob( fp );
                
                if (ch->in_room)
                    char_to_room( ch, ch->in_room );
                else {
                    LogStream::sendError( ) << "[" << path << "]:  null room" << endl;
                    buggy = true;
                    break;
                }
                
                continue;
            }
            
            if ( !str_cmp( word, "OBJECT" ) ) {
                fread_obj  ( ch, NULL, fp );
                continue;
            }
            
            if ( !str_cmp( word, "O"      ) ) {
                fread_obj  ( ch, NULL, fp );
                continue;
            }
            
            if ( !str_cmp( word, "End"    ) ) 
                break;
            
            LogStream::sendError( ) << "bad section: " << word << endl;
            buggy = true;
            break;
        
        } catch (const FileFormatException &e) {
            LogStream::sendError( ) << e.what( ) << endl;
            buggy = true;
            break;
        }
    }

    fclose( fp );

    create_obj_dropped = false;

    unlink( path.c_str( ) );

    if (ch && buggy)
        extract_mob_dropped( ch );
    else if (ch)
        room_markers.insert( ch->in_room->vnum );
}

void load_creatures( )
{
    LogStream::sendNotice( ) <<  "Creatures loading...." << endl;
    room_markers.clear( );
    dreamland->removeOption( DL_SAVE_MOBS );

    for (unsigned int i = 0; i < HASH_SAVED_CHARMED; i++) {
        dirent* dp;
        DIR *dirp;
        DLString dirname = make_dirname( i );

        LogStream::sendNotice() << "Folder " << i << endl;

        try {
            if (!( dirp = opendir( dirname.c_str( ) ) )) {
                if (errno == ENOENT)
                    continue;
                else
                    throw FileFormatException( "unable to open directory %d", i );
            }

            for (dp = readdir( dirp ); dp; dp = readdir( dirp )) {
                if (NAMLEN( dp ) <= 0 || dp->d_name[0] == '.')
                    continue;

                try {
                    load_one_creature( dirname + "/" + dp->d_name );
                } catch (const FileFormatException &e) {
                    LogStream::sendError( ) << e.what( ) << endl;
                }
            }

            CLOSEDIR( dirp );
        } 
        catch (const FileFormatException &e) {
            LogStream::sendError( ) << e.what( ) << endl;
        }
    }

    dreamland->resetOption( DL_SAVE_MOBS );

    for (set<int>::iterator m = room_markers.begin( ); m != room_markers.end( ); m++) {
        save_mobs( get_room_instance( *m ) );
    }
    
    LogStream::sendNotice( ) <<  "Creatures loaded" << endl;
}

