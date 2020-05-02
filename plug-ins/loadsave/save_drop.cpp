/* $Id: save_drop.cpp,v 1.1.2.2.6.5 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/

#include "fileformatexception.h"
#include "logstream.h"

#include "skillreference.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "dreamland.h"
#include "save.h"
#include "loadsave.h"
#include "fread_utils.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

GSN(doppelganger);

bool create_obj_dropped = false;

/*
 * dropped objects loading
 */
void load_drops( )
{
        char dirn[MAX_STRING_LENGTH];
        
        LogStream::sendNotice( ) <<  "Drops loading...." << endl;
        dreamland->removeOption( DL_SAVE_OBJS );


        for ( int i = 0; i < 16; i++ )
        {
                sprintf( dirn, "%d", i );

                LogStream::sendNotice() << "Folder " << dirn << endl;

                try {
                    load_single_objects_folder( dirn, false );
                } catch (FileFormatException e) {
                    LogStream::sendError( ) << e.what( ) << endl;
                }
        }

        dreamland->resetOption( DL_SAVE_OBJS );

        LogStream::sendNotice( ) <<  "Drops loaded." << endl;
}

void load_single_objects_folder( char * subdir, bool remove_after )
{
    dirent* dp;
    DIR  *dirp;
    char dirn[MAX_STRING_LENGTH];

    Room *room;

    sprintf( dirn, "%s/objects/%s", dreamland->getSavedDir( ).getPath( ).c_str( ), subdir );
    if( !( dirp = opendir( dirn ) ) )
    {
            throw FileFormatException( "Load_rooms_date: unable to open saved rooms directory." );
    }

    for( dp = readdir( dirp ); dp; dp = readdir( dirp ) )
    {
        try {
            if ( NAMLEN( dp ) > 0
                    && dp->d_name[0] != '.' )
            {
                    room = get_room_index( atoi( dp->d_name ) );
                    
                    try {
                        load_room_objects( room, dirn, remove_after );
                    } catch (const FileFormatException &e) {
                        LogStream::sendError( ) << e.what( ) << endl;
                    }
            }
        } catch (const Exception &ex) {
            LogStream::sendError() << ex.what() << endl;
        }
    }

    CLOSEDIR( dirp );
}

/*
 * Read room's with objects
 */
void        load_room_objects( Room *room, char * path, bool remove_after )
{
    FILE        *fp;
    char        fname[MAX_INPUT_LENGTH];

    bool  has_objects = false;
    bool  buggy = false;

    if ( !room )
            return;
    
    sprintf( fname, "%s/%d", path, room->vnum );

    if ( (fp = fopen(fname,"r")) == NULL )
    {
            sprintf (fname,"Load_room: fopen vnum %d", room->vnum);
            bug(fname,0);
            return;
    }

    create_obj_dropped = true;

    fseek( fp, 0L, SEEK_END );
        
    if ( ftell( fp ) > 0 )
    {
        fseek( fp, 0L, SEEK_SET );

        for(;;)
        {
            try {
                char letter;

                const char *word = feof( fp ) ? "#END" : fread_word( fp );

                letter = word[0];

                if ( letter == '*' )
                {
                        fread_to_eol( fp );
                        continue;
                }

                if ( letter != '#' )
                {
                        bug( "Load_room_obj: # not found.", 0 );
                        buggy = true;
                        break;
                }

                word++;

                if ( !str_cmp( word, "OBJECT" ) )
                {
                        fread_obj ( NULL, room, fp );
                        has_objects = true;
                }
                else if ( !str_cmp( word, "O"      ) )
                {
                        fread_obj  ( NULL, room, fp );
                        has_objects = true;
                }
                else if ( !str_cmp( word, "END"      ) ) break;
                else
                {
                        bug( "Load_room_obj: bad section.", 0 );
                        buggy = true;
                        break;
                }
                
            } catch (FileFormatException e) {
                LogStream::sendError( ) << "Load_room_obj: " << e.what( ) << endl;
                buggy = true;
                break;
            }
        }
    }

    create_obj_dropped = false;

    fclose( fp );

    if ( !has_objects || remove_after || buggy )
            unlink( fname );
}

/*
 * dropped mobiles loading
 */
void load_dropped_mobs( )
{
        char dirn[MAX_STRING_LENGTH];

        LogStream::sendNotice( ) <<  "Mobiles loading...." << endl;
        dreamland->removeOption( DL_SAVE_MOBS );

        for ( int i = 0; i < 16; i++ )
        {
                sprintf( dirn, "%d", i );

                LogStream::sendNotice() << "Folder " << dirn << endl;

                try {
                    load_single_mobiles_folder( dirn, false );
                } catch (const FileFormatException &e) {
                    LogStream::sendError( ) << e.what( ) << endl;
                }
        }

        dreamland->resetOption( DL_SAVE_MOBS );
        
        LogStream::sendNotice( ) <<  "Mobiles loaded." << endl;
}


void load_single_mobiles_folder( char * subdir, bool remove_after )
{
        dirent* dp;
        DIR  *dirp;
        char dirn[MAX_STRING_LENGTH];

        Room *room;

        sprintf( dirn, "%s/mobiles/%s", dreamland->getSavedDir( ).getPath( ).c_str( ), subdir );
        if( !( dirp = opendir( dirn ) ) )
        {
                throw FileFormatException( "Load_rooms_date: unable to open saved rooms directory." );
        }

        for( dp = readdir( dirp ); dp; dp = readdir( dirp ) )
        {
            try {
                if ( NAMLEN( dp ) > 0
                        && dp->d_name[0] != '.' )
                {
                        room = get_room_index( atoi( dp->d_name ) );

                        if ( !room )
                        {
                                bug("load_room_mobiles: NULL room %d!", atoi(dp->d_name));
                                continue;
                        }
                        
                        try {
                            load_room_mobiles( room, dirn, remove_after );
                        } catch (const FileFormatException &e) {
                            LogStream::sendError( ) << e.what( ) << endl;
                        }
                }
            } catch (const Exception &e) {
                LogStream::sendError() << e.what() << endl;
            }
        }

        CLOSEDIR( dirp );
}

void load_room_mobiles( Room *room, char *path, bool remove_after )
{
    char strsave[MAX_INPUT_LENGTH];
    NPCharacter *ch = 0;
    FILE *fp;
    bool buggy = false;

    sprintf( strsave, "%s/%d",path,room->vnum );

    create_obj_dropped = true;
                    
    if ( ( fp = fopen( strsave, "r" ) ) )
    {
            for ( ; ; )
            {
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
                            LogStream::sendError( ) 
                                << "Room [" << room->vnum << "] " 
                                << "load_room_mobiles: # not found." << endl;
                            buggy = true;
                            break;
                    }

                    word = fread_word( fp );
                    if ( !str_cmp( word, "MOBILE" ) )
                    {
                            ch = 0;
                            ch = fread_mob  ( fp );
                            char_to_room( ch, room );
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
                    
                    if ( !str_cmp( word, "End"    ) ) {
                            break;
                    }
                    
                    bug( "Load_char_obj: bad section.", 0 );
                    buggy = true;
                    break;
                
                } catch (FileFormatException e) {
                    LogStream::sendError( ) << "Load_room_mobiles: " << e.what( ) << endl;
                    buggy = true;
                    break;
                }
                
                    
            }
            fclose( fp );
    }

    create_obj_dropped = false;


    if ( ch == 0 || remove_after || buggy )
    {
            unlink( strsave );
    }

    if (ch && buggy)
        extract_mob_dropped( ch );

    return;
}


/*
 * Save rooms with objects
 */

void save_room_objects( Room *room )
{
    char        fname[MAX_INPUT_LENGTH];
    FILE        *fp;
    Object *obj;

    if ( !room )
    {
        bug("Save_room_obj: null room",0);
        return;
    }

    sprintf( fname, "%s/%s/%d/%d", dreamland->getSavedDir( ).getPath( ).c_str( ),"objects", room->vnum % 16, room->vnum );

    if ( (fp = fopen(fname,"w")) == NULL )
    {
            sprintf (fname,"Save_room: fopen vnum %d", room->vnum);
            bug(fname,0);
            return;
    }

    try
    {
        // Save all items, being prepared for the fact that extract_obj and obj_from_room
        // can be called from fwrite_obj, disrupting links between objects in the room.
        list<Object *> items;
        list<Object *>::iterator i;
        for (obj = room->contents; obj; obj = obj->next_content)
            items.push_back( obj );

        for (i = items.begin( ); i != items.end( ); i++)
            fwrite_obj_0( NULL, *i, fp, 0 );
    }
    catch(Exception e)
    {
            sprintf (fname,"{RSave_room: filling {Cvnum %d{R FAILED!!!!!!!!", room->vnum);
            bug(fname,0);
    }

    fprintf(fp,"\n#END\n");
    fflush(fp);
    fclose(fp);
}


void save_items ( Room *room )
{
        if ( room != 0 && dreamland->hasOption( DL_SAVE_OBJS ))
        {
                save_room_objects( room );
        }

        return;
}

/*
 * Save rooms with mobiles 
 */

void save_room_mobiles( Room *room )
{
    char        fname[MAX_INPUT_LENGTH];
    FILE        *fp;
    Character *ch;

    if ( !room )
    {
            bug("Save_room_mobiles: null room",0);
            return;
    }

    sprintf( fname, "%s/%s/%d/%d", dreamland->getSavedDir( ).getPath( ).c_str( ),"mobiles", room->vnum % 16, room->vnum );

    if ( (fp = fopen(fname,"w")) == NULL )
    {
            sprintf (fname,"Save_room_mobiles: fopen vnum %d", room->vnum);
            bug(fname,0);
            return;
    }

    bool found = false;

    for ( ch = room->people; ch; ch = ch->next_in_room )
            if ( ch->is_npc( )
                    && !IS_CHARMED(ch)
                    && !ch->isAffected(gsn_doppelganger ) )
            {
                    if ( found )
                            fprintf( fp, "\n" );
                    
                    fwrite_mob( ch->getNPC(), fp );

                    found = true;
            }

    fprintf(fp,"\n#End\n");
    fflush(fp);
    fclose(fp);
}

void save_mobs ( Room *room )
{
    if ( room != 0 && dreamland->hasOption( DL_SAVE_MOBS ) )
    {
            save_room_mobiles( room );
    }

    return;
}

void save_mobs_at( Character *ch )
{
    if (!ch->is_npc())
        return;

    if (IS_CHARMED(ch))
        return;
            
    if (!ch->in_room)
        return;

    save_mobs( ch->in_room );
}


void save_items_at_holder( Object * obj )
{
    // Prepare for saving objects

    Object *u_obj = obj;

    while ( u_obj->in_obj )
            u_obj = u_obj->in_obj;

    if ( u_obj->in_room != 0 )
    {
            save_items( u_obj->in_room );
    }
    else
    if ( u_obj->carried_by != 0
            && u_obj->carried_by->is_npc( )        )
    {
            save_mobs( u_obj->carried_by->in_room );
    }
}


