/* $Id$
 *
 * ruffina, 2004
 */

#include <list>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "fileformatexception.h"
#include "logstream.h"
#include "dlfileop.h"

#include "npcharacter.h"
#include "object.h"
#include "room.h"

#include "dreamland.h"
#include "directions.h"
#include "fread_utils.h"
#include "arg_utils.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

static void skip_section( FILE *fp )
{
    for ( ; ; ) {
        char letter = fread_letter(fp);
        if (letter == '#') {
            ungetc(letter, fp);
            return;
        }
        fread_to_eol( fp );
    }
}

static DLString trim(const DLString& str, const string& chars = "\t\n\v\f\r ")
{
    DLString line = str;
    line.erase(line.find_last_not_of(chars) + 1);
    line.erase(0, line.find_first_not_of(chars));
    return line;
}

static DLString trimText(const DLString& str, const string& chars = "\t\n\v\f\r ")
{
    istringstream is(str);
    char buf[1024];
    ostringstream os;

    while (is.getline(buf, sizeof(buf))) {
        string line = buf;
        os << trim(line) << endl;
    }
    return os.str();
}
 
static void c7i_area_header( FILE *fp, AREA_DATA *ourArea )
{
    LogStream::sendNotice() << "Updating area " << ourArea->name << endl;

    for ( ; ; ) {
        char *word = fread_word(fp);
        DLString name = trim(fread_dlstring_to_eol(fp));
        if (!str_cmp( word, "rus" )) {
            LogStream::sendNotice() << "Setting area name to " << name << endl;
            ourArea->name = str_dup(name.ruscase('1').c_str());
            break;
        } 
    }

    skip_section( fp );

}

// Check for 'look <direction>' descriptions.
// Return true if a direction has been matched.
static bool c7i_room_exit( FILE *fp, Room *room, char *word )
{
    for (int door = 0; door < DIR_SOMEWHERE; door++) {
        if (!str_cmp(word, dirs[door].name)) {
            // Read comments section.
            fread_to_eol(fp);
            // Look ahead at the next line.
            char next = fread_letter(fp);
            ungetc(next, fp);
            if (isalpha(next) && islower(next)) {
                // No description, next keyword begins.
                return true;
            }
            
            notice("Updating %s description", dirs[door].name);
            if (!room->exit[door]) {
                warn("Exit in that direction doesn't exist");
                break;
            }

            if (room->exit[door]->description) {
                free_string(room->exit[door]->description);
            } else {
                warn("Setting new directional description");
            }
           
            DLString ed = trimText(fread_dlstring(fp));
            room->exit[door]->description = str_dup(ed.c_str());
            return true;
        }
    }

    return false;
}

static void c7i_room( FILE *fp, AREA_DATA *ourArea )
{
    int vnum = fread_number( fp );
    Room *room = get_room_index(vnum);
    if (!room) {
        LogStream::sendError() << "Native room #" << vnum << " not found, skipping." << endl;
        skip_section(fp);
        return;
    }

    if (room->area != ourArea) {
        LogStream::sendError() << "Native room #" << vnum << " doesn't belong to same area, skipping." << endl;
        skip_section(fp);
        return;
    }

    LogStream::sendNotice() << "Updating room #" << vnum << endl;

    for ( ; ; ) {
        char *word = fread_word( fp );
        
        if (!str_cmp(word, "endsection")) {
            break;
        }

        if (!str_cmp(word, "name")) {
            char *name = fread_string_eol(fp);
            notice("Room name %s -> %s", room->name, name);
            free_string(room->name);
            room->name = name;
            continue;
        } 

        if (!str_cmp(word, "desc")) {
            DLString description = trimText(fread_dlstring(fp));
            notice("Room description %s -> %s", room->description, description.c_str());
            free_string(room->description);
            room->description = str_dup(description.c_str());
            continue;
        } 
        
        if (!str_cmp(word, "extra")) {
            bool found = false;
            DLString keywords = trim(fread_dlstring_to_eol(fp));
            DLString ed = trimText(fread_dlstring(fp));

            for (EXTRA_DESCR_DATA *pEd = room->extra_descr; pEd; pEd= pEd->next) {
                if (arg_contains_someof(pEd->keyword, keywords.c_str())) {
                    notice("Updating room extra descr %s.", pEd->keyword);
                    free_string(pEd->keyword);
                    free_string(pEd->description);
                    pEd->keyword = str_dup(keywords.c_str());
                    pEd->description = str_dup(ed.c_str());
                    found = true;
                    break;
                }
            }

            if (!found)
                warn("No extra descr found for %s.", keywords.c_str());
            continue;
        } 
     
        if (c7i_room_exit(fp, room, word))
            continue;

        // Nothing matched.
        fread_to_eol(fp);
    }
}

static void c7i_obj( FILE *fp, AREA_DATA *ourArea )
{
    int vnum = fread_number( fp );
    OBJ_INDEX_DATA *pObj = get_obj_index(vnum);
    if (!pObj) {
        LogStream::sendError() << "Native obj #" << vnum << " not found, skipping." << endl;
        skip_section(fp);
        return;
    }

    if (pObj->area != ourArea) {
        LogStream::sendError() << "Native obj #" << vnum << " doesn't belong to same area, skipping." << endl;
        skip_section(fp);
        return;
    }

    LogStream::sendNotice() << "Updating obj #" << vnum << endl;

    for ( ; ; ) {
        char *word = fread_word( fp );
        
        if (!str_cmp(word, "endsection")) {
            break;
        }

        if (!str_cmp(word, "name")) {
            DLString shortDescr = trim(fread_dlstring_to_eol(fp));
            notice("Obj short [%s]", shortDescr.c_str());
            DLString names = DLString(pObj->name) + " " + shortDescr.ruscase('1');
            notice("Obj name [%s]", names.c_str());

            free_string(pObj->short_descr);
            pObj->short_descr = str_dup(shortDescr.c_str());
            free_string(pObj->name);
            pObj->name = str_dup(names.c_str());

        } else if (!str_cmp(word, "long")) {
            char *description = fread_string_eol(fp);
            free_string(pObj->description);
            pObj->description = description;

        } else if (!str_cmp(word, "desc")) {
            bool found = false;
            DLString ed = trimText(fread_dlstring(fp));

            for (EXTRA_DESCR_DATA *pEd = pObj->extra_descr; pEd; pEd = pEd->next) {
                if (arg_contains_someof( pEd->keyword, pObj->name )) {
                    LogStream::sendNotice() << "Updating ed '" << pEd->keyword << "'" << endl;
                    DLString keyword = pObj->name;
                    free_string(pEd->keyword);
                    free_string(pEd->description);
                    pEd->keyword = str_dup(keyword.c_str());
                    pEd->description = str_dup(ed.c_str());
                    found = true;
                    break;
                }
            }

            if (!found)
                LogStream::sendWarning() << "No matching ed found for '" << pObj->name << "'" << endl;

        } else {
            fread_to_eol(fp);
        }
    }
}

static void c7i_mob( FILE *fp, AREA_DATA *ourArea )
{
    int vnum = fread_number( fp );
    MOB_INDEX_DATA *pMob = get_mob_index(vnum);
    if (!pMob) {
        LogStream::sendError() << "Native mob #" << vnum << " not found, skipping." << endl;
        skip_section(fp);
        return;
    }

    if (pMob->area != ourArea) {
        LogStream::sendError() << "Native mob #" << vnum << " doesn't belong to same area, skipping." << endl;
        skip_section(fp);
        return;
    }

    LogStream::sendNotice() << "Updating mob #" << vnum << endl;

    for ( ; ; ) {
        char *word = fread_word( fp );
        
        if (!str_cmp(word, "endsection")) {
            break;
        }

        if (!str_cmp(word, "name")) {
            DLString shortDescr = trim(fread_dlstring_to_eol(fp));
            DLString names = shortDescr.ruscase('1') + " " + pMob->player_name;
            notice("Mob short %s -> %s", pMob->short_descr, shortDescr.c_str());
            notice("Mob names %s -> %s", pMob->player_name, names.c_str());
            free_string(pMob->short_descr);
            free_string(pMob->player_name);
            pMob->short_descr = str_dup(shortDescr.c_str());
            pMob->player_name = str_dup(names.c_str());

        } else if (!str_cmp(word, "long")) {
            DLString longDescr = trim(fread_dlstring_to_eol(fp)) + "\n";
            notice("Mob long %s -> %s", pMob->long_descr, longDescr.c_str());
            free_string(pMob->long_descr);
            pMob->long_descr = str_dup(longDescr.c_str());

        } else if (!str_cmp(word, "desc")) {
            DLString description = trimText(fread_dlstring(fp));
            notice("Mob description %s -> %s", pMob->description, description.c_str());
            free_string(pMob->description);
            pMob->description = str_dup(description.c_str());

        } else {
            fread_to_eol(fp);
        }
    }
}

void load_c7i_area( const DLString &areaPath, AREA_DATA *ourArea )
{
    FILE *file;

    DLFileRead areaFile(areaPath);
    if (!areaFile.open( )) {
        LogStream::sendSystem( ) << "load_c7i_area: cannot open " << areaPath << endl;
        return;
    }

    LogStream::sendNotice( ) << "Loading translations from " << areaPath << endl;
    file = areaFile.getFP( );

    for ( ; ; ) {
            char *word;
            char letter = fread_letter( file );

            if (letter != '#') {
                LogStream::sendError( ) << "load_c7i_area: # not found, read " << letter << endl;
                return;
            }

            word = fread_word( file );

            if (word[0] == '$')  
                break;
            else if (!str_cmp( word, "AREAC7I" )) c7i_area_header(file, ourArea);
            else if (!str_cmp( word, "MOB" )) c7i_mob(file, ourArea);
            else if (!str_cmp( word, "OBJ" )) c7i_obj(file, ourArea);
            else if (!str_cmp( word, "ROOM" )) c7i_room(file, ourArea);
            else if (!str_cmp( word, "END" )) break;
            else {
                LogStream::sendError( ) << "load_c7i_area: unknown section name " << word << endl;
                return;
            }
    }
}
