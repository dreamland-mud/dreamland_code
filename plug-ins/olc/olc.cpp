/* $Id$
 *
 * ruffina, 2004
 */
#include <algorithm>
#include "grammar_entities_impl.h"
#include "dlfilestream.h"
#include "util/regexp.h"
#include <xmldocument.h>
#include "stringset.h"
#include "wrapperbase.h"

#include <skill.h>
#include <spell.h>
#include "defaultspell.h"
#include "basicskill.h"
#include <skillmanager.h>
#include "skillcommand.h"
#include "skillgroup.h"
#include "feniamanager.h"
#include "areabehaviormanager.h"
#include <affect.h>
#include <object.h>
#include <pcharacter.h>
#include "pcharactermanager.h"
#include <npcharacter.h>
#include <commandmanager.h>
#include "profession.h"
#include "race.h"
#include "clanreference.h"
#include "room.h"

#include "olc.h"
#include "olcflags.h"
#include "olcstate.h"
#include "security.h"
#include "areahelp.h"

#include "damageflags.h"
#include "commandflags.h"
#include "occupations.h"
#include "update_areas.h"
#include "websocketrpc.h"
#include "interp.h"
#include "merc.h"
#include "handler.h"
#include "act.h"
#include "save.h"
#include "act_move.h"
#include "weapontier.h"
#include "../anatolia/handler.h"
#include "vnum.h"
#include "mercdb.h"
#include "comm.h"
#include "def.h"

void obj_update();
GSN(none);
CLAN(none);
GROUP(clan);
WEARLOC(sheath);

using namespace std;


enum {
    NDX_ROOM,
    NDX_OBJ,
    NDX_MOB,
};

static int next_index_data( Character *ch, RoomIndexData *r, int ndx_type )
{
    AreaIndexData *pArea;
    
    if (!r)
        return -1;

    pArea = r->areaIndex;
    if (!pArea)
        return -1;

    for (int i = pArea->min_vnum; i <= pArea->max_vnum; i++) {
        if (!OLCState::can_edit( ch, i ))
            continue;

        switch (ndx_type) {
        case NDX_ROOM:
            if (!get_room_index( i ))
                return i;
            break;
        case NDX_OBJ:
            if (!get_obj_index( i ))
                return i;
            break;
        case NDX_MOB:
            if (!get_mob_index( i ))
                return i;
            break;
        }
    }

    return -1;
}
    
int next_room( Character *ch, RoomIndexData *r )
{
    return next_index_data( ch, r, NDX_ROOM );
}

int next_obj_index( Character *ch, RoomIndexData *r )
{
    return next_index_data( ch, r, NDX_OBJ );
}

int next_mob_index( Character *ch, RoomIndexData *r )
{
    return next_index_data( ch, r, NDX_MOB );
}

const char *
get_skill_name( int sn, bool verbose )
{
    Skill *skill = SkillManager::getThis( )->find( sn );

    if (skill)
        return skill->getName( ).c_str( );
    else if (verbose)
        return "none";
    else
        return "";
}

void
ptc(Character *ch, const char *fmt, ...)
{
    va_list av;

    va_start(av, fmt);

    DLString rc = vfmt(ch, fmt, av);
    stc(rc.c_str( ), ch);

    va_end(av);
}


/** Find area with given vnum. */
AreaIndexData *get_area_data(int vnum)
{
    for(auto &pArea: areaIndexes) {
        if (pArea->vnum == vnum)
            return pArea;
    }
    return 0;
}

/** Get next available help ID to use. Can potentially result in duplicates if a plugin 
    containing some helps is unloaded when this function is being called.  */
int help_next_free_id()
{
    list<int> ids;
    for (auto &a: helpManager->getArticles())
        ids.push_back(a->getID());

    ids.sort();

    int latest = 1;

    for (auto &id: ids) {
        if (id <= 0)
            continue;
            
        if (id - latest > 1)
            return latest + 1;

        latest = id;
    }

    return helpManager->getLastID();
}


struct editor_table_entry {
    const char *arg, *cmd;
} editor_table[] = {
    {"area",   "aedit"},
    {"room",   "redit"},
    {"object", "oedit"},
    {"mobile", "medit"},
    {"help",   "hmedit"},
    {NULL, 0,}
};

CMD(edit, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
        "Online editor.")
{
    char command[MAX_INPUT_LENGTH];
    int cmd;

    argument = one_argument(argument, command);

    if (command[0] == '\0') {
//        do_help(ch, "olc");
        return;
    }

    for (cmd = 0; editor_table[cmd].arg != NULL; cmd++) {
        if (!str_prefix(command, editor_table[cmd].arg)) {
            interpret_raw(ch, editor_table[cmd].cmd, "%s", argument);
            return;
        }
    }

//    do_help(ch, "olc");
}

static bool area_cmp_filename(const AreaIndexData *a, const AreaIndexData *b)
{
    return strcmp(a->area_file->file_name, b->area_file->file_name) < 0;
}

static bool area_cmp_vnum(const AreaIndexData *a, const AreaIndexData *b)
{
    return a->min_vnum < b->min_vnum;
}

static bool area_cmp_name(const AreaIndexData *a, const AreaIndexData *b) 
{
    DLString name1 = DLString(a->name).colourStrip();
    DLString name2 = DLString(b->name).colourStrip();
    return name1.compareRussian(name2) < 0;
}

CMD(alist, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
        "List areas.")
{
    vector<AreaIndexData *> areas;
    DLString args(argument);
    DLString arg = args.getOneArgument();
    DLString flagName;

    for(auto &pArea: areaIndexes) {
        areas.push_back(pArea);
    }

    if (arg_has_oneof(arg, "vnum", "внум")) 
        sort(areas.begin(), areas.end(), area_cmp_vnum);
    else if (arg_has_oneof(arg, "name", "имя"))
        sort(areas.begin(), areas.end(), area_cmp_name);
    else if (arg_has_oneof(arg, "file", "файл"))
        sort(areas.begin(), areas.end(), area_cmp_filename);
    else if (arg_has_oneof(arg, "flag", "флаг") && !args.empty())
        flagName = args;
    else if (!arg.empty()) {
        ch->pecho("Формат:\r\nalist - список всех арий\r\nalist vnum|name|file - список арий, отсортированный по критерию");
        ch->pecho("alist flag <name> - список всех арий с флагом name");
        return;
    }

    const DLString lineFormat = 
            "[" + web_cmd(ch, "aedit $1", "%3d") 
            + "] {%s%-29s {%s(%5u-%5u) %17s %s{w\n\r";

    ptc(ch, "[%3s] %-29s   (%5s-%5s) %-17s %s\n\r",
      "Num", "Area Name", "lvnum", "uvnum", "Filename", "Help");

    for (auto &pArea: areas) {
        if (!flagName.empty()) {
            DLString areaflags = area_flags.names(pArea->area_flag);
            if (areaflags.find(flagName) == DLString::npos)
                continue;
        }
            
        DLString hedit = "";
        AreaHelp *ahelp = area_selfhelp(pArea);
        if (ahelp && ahelp->getID() > 0) {
            DLString id(ahelp->getID());
            // Mark zones without meaningful help articles with red asterix.
            DLString color = help_is_empty(*ahelp) ? " {R*{x": "";
            hedit = web_cmd(ch, "hedit " + id, "hedit " + id) + color;
        }

        // System areas are shown in grey colors.           
        const char *colorAreaName = IS_SET(pArea->area_flag, AREA_SYSTEM) ? "D" : "W";
        const char *colorAreaVnums = IS_SET(pArea->area_flag, AREA_SYSTEM) ? "D" : "w";

        ch->send_to(
            dlprintf(lineFormat.c_str(), 
                pArea->vnum, 
                colorAreaName,
                DLString(pArea->name).colourStrip().cutSize(29).c_str(),
                colorAreaVnums,
                pArea->min_vnum, pArea->max_vnum,
                pArea->area_file->file_name,
                hedit.c_str()));
    }
}

static DLString trim(const DLString& str, const string& chars = "\t\n\v\f\r ")
{
    DLString line = str;
    line.erase(line.find_last_not_of(chars) + 1);
    line.erase(0, line.find_first_not_of(chars));
    return line;
}



CMD(abc, 50, "", POS_DEAD, 106, LOG_ALWAYS, "")
{
    DLString args = argument;
    DLString arg = args.getOneArgument();

    if (arg == "eexit") {
        ostringstream abuf, cbuf, mbuf;
        abuf << endl << "Экстравыходы везде:" << endl;
        mbuf << endl << "Экстравыходы в особняках и пригородах:" << endl;
        cbuf << endl << "Экстравыходы в кланах:" << endl;

        const DLString lineFormat = "[" + web_cmd(ch, "goto $1", "%5d") + "] %-35s{x [{C%s{x]";
        for (auto &room: roomInstances) {
            ostringstream *buf;
            if (IS_SET(room->room_flags, ROOM_MANSION) 
                    || !str_prefix("ht", room->areaIndex()->area_file->file_name))
                buf = &mbuf;
            else if (room->pIndexData->clan != clan_none)
                buf = &cbuf;
            else
                buf = &abuf;
            for (auto &eexit: room->extra_exits) {
                (*buf) << dlprintf(lineFormat.c_str(), room->vnum, room->getName(), eexit->keyword) << endl;
            }
        }
        
        page_to_char( mbuf.str( ).c_str( ), ch );
        page_to_char( cbuf.str( ).c_str( ), ch );
        page_to_char( abuf.str( ).c_str( ), ch );

        return;
    }

    if (!ch->isCoder( ))
        return;

    if (arg == "objname") {
        int cnt = 0, hcnt = 0, rcnt = 0;
        ostringstream buf, hbuf, rbuf;

        for (int i = 0; i < MAX_KEY_HASH; i++)
        for (OBJ_INDEX_DATA *pObj = obj_index_hash[i]; pObj; pObj = pObj->next) {
            DLString longd = pObj->description;
            longd.colourstrip( );

            static RegExp pattern_rus("[а-я]");
            static RegExp pattern_longd("^.*\\(([-a-z ]+)\\).*$");
            
            if (!pattern_rus.match( longd )) 
                continue;

            if (IS_SET(pObj->area->area_flag, AREA_NOQUEST|AREA_WIZLOCK|AREA_HIDDEN))
                continue;
            
            {
                DLString names = DLString( pObj->name );
                RussianString rshortd(pObj->short_descr, pObj->gram_gender );
                DLString shortd = rshortd.decline( '7' ).colourStrip( );
                if (!arg_contains_someof( shortd, pObj->name )) {
                    rcnt ++;
                    rbuf << pObj->vnum << ": [" << rshortd.decline( '1' ) << "] [" << pObj->name << "]" <<  endl;
                }
            }
            if (!pattern_longd.match( longd )) {
                buf << pObj->vnum << ": [" << longd << "] [" << pObj->name << "]" <<  endl;
                cnt++;
            } else {
                RegExp::MatchVector matches = pattern_longd.subexpr( longd.c_str( ) );
                if (matches.size( ) < 1) {
                    buf << pObj->vnum << ": [" << longd << "] [" << pObj->short_descr << "]" <<  endl;
                    cnt++;
                } else {
                    
                    DLString hint = matches.front( );
                    if (!is_name( hint.c_str( ), pObj->name )) {
                        hbuf << dlprintf( "%6d : [%35s] hint [{G%10s{x] [{W%s{x]\r\n",
                                pObj->vnum, longd.c_str( ), hint.c_str( ), pObj->name );
                        hcnt++;
                    }
                }
            }
        }

//        ch->printf("Найдено %d длинных имен предметов без подсказок (пустых).\r\n", cnt);
//        page_to_char( buf.str( ).c_str( ), ch );
//        ch->printf("Найдено %d несоответствий подсказок в длинном имени предметов.\r\n", hcnt);
//        page_to_char( hbuf.str( ).c_str( ), ch );
        ch->printf("Найдено %d предметов где в short descr не встречаются имена.\r\n", rcnt);
        page_to_char( rbuf.str( ).c_str( ), ch );
        return;
    }

    if (arg == "mobname") {
        int cnt = 0, hcnt = 0, rcnt = 0;
        ostringstream buf, hbuf, rbuf;

        for (int i = 0; i < MAX_KEY_HASH; i++)
        for (MOB_INDEX_DATA *pMob = mob_index_hash[i]; pMob; pMob = pMob->next) {
            DLString names = DLString(pMob->player_name).toLower();
            DLString longd = trim(pMob->long_descr).toLower();
            longd.colourstrip( );

            static RegExp pattern_rus("[а-я]");
            static RegExp pattern_longd("^.*\\(([-a-z ]+)\\).*$");
            
            if (!pattern_rus.match( longd )) 
                continue;

            if (IS_SET(pMob->area->area_flag, AREA_WIZLOCK|AREA_HIDDEN))
                continue;
            
            {
                RussianString rshortd(pMob->short_descr, MultiGender(pMob->sex, pMob->gram_number));
                DLString shortd = rshortd.decline( '7' ).colourStrip( ).toLower();
                if (!arg_contains_someof( shortd, pMob->player_name )) {
                    rcnt ++;
                    rbuf << pMob->vnum << ": [" << rshortd.decline( '1' ) << "] [" << pMob->player_name << "]" <<  endl;
                }
            }

            RegExp::MatchVector matches = pattern_longd.subexpr( longd.c_str( ) );
            if (matches.size( ) < 1) {
                buf << pMob->vnum << ": [" << longd << "] [" << pMob->short_descr << "]" <<  endl;
                cnt++;
            } else {
                
                DLString hint = matches.front( );
                if (!is_name(hint.c_str(), names.c_str())) {
                    hbuf << dlprintf( "%6d : [%35s] hint [{G%10s{x] [{W%s{x]\r\n",
                            pMob->vnum, longd.c_str( ), hint.c_str( ), pMob->player_name );
                    hcnt++;
                }
            }
        }

        ch->printf("\r\n{RНайдено %d несоответствий подсказок в длинном имени моба.{x\r\n", hcnt);
        page_to_char( hbuf.str( ).c_str( ), ch );
        return;
    }

    if (arg == "maxhelp") {
        ch->printf("Max help ID is %d, next free id is %d.\r\n", 
                    helpManager->getLastID(), help_next_free_id());
        return;
    }

    if (arg == "readroom") {
        Integer vnum;
        Room *room;

        if (args.empty() || !Integer::tryParse(vnum, args)) {
            ch->pecho("abc readroom <vnum>");
            return;
        }

        room = get_room_instance(vnum);
        if (!room) {
            ch->printf("Room vnum [%d] not found.\r\n", vnum.getValue());
            return;
        }
        
        ch->printf("Loading room objects for '%s' [%d], check logs for details.\r\n", 
                    room->getName(), room->vnum);
        load_room_objects(room, const_cast<char *>("/tmp"), false);
        return;
    }

    if (arg == "update") {
        obj_update();
        ch->pecho("Forced obj update.");
        return;
    }

    if (arg == "badresets") {
        ostringstream buf;

        buf << "List of all resets with invalid wearlocations: " << endl;
        for (auto &r: roomIndexMap) {
            MOB_INDEX_DATA *pMob = 0;

            for (auto &pReset: r.second->resets) {
                if (pReset->command == 'O' || pReset->command == 'R') {
                    pMob = 0;
                    continue;
                }

                if (pReset->command == 'M') {
                    pMob = get_mob_index(pReset->arg1);
                    continue;
                }

                if (pReset->command == 'E') {
                    if (!pMob) {
                        buf << "Bad mob reset " << pReset->arg1 << " in room " << r.first << endl;
                        continue;
                    }

                    Wearlocation *wloc = wearlocationManager->find(pReset->arg3);
                    if (!wloc) {
                        buf << "Bad wearloc for mob " << pMob->vnum << " in room " << r.first << endl;
                        continue;
                    }

                    if (wloc->getIndex() == wear_sheath)
                        continue;

                    Race *race = raceManager->find(pMob->race);
                    if (!race->getWearloc().isSet(*wloc)) {
                        buf << "[" << r.first << "] mob [" << pMob->vnum << "] race '" 
                            << race->getName() <<"' equipped at " << wloc->getName() << endl;
                    }
                }
            }
        }

        page_to_char(buf.str().c_str(), ch);
        return;
    }

    const int maxlines = 40;

    if (arg == "part") {
        ostringstream buf;
        int cnt = 0;
        buf << endl;

        for (int i = 0; i < MAX_KEY_HASH && cnt < maxlines; i++)
        for (MOB_INDEX_DATA *pMob = mob_index_hash[i]; pMob && cnt < maxlines; pMob = pMob->next) {
            Race *race = raceManager->find(pMob->race);
            if (!race || !race->isValid()) {
                buf << "[" << pMob->vnum << "] invalid race " << pMob->race << endl;
                continue;
            }
            if (pMob->properties.count("hidden") > 0)
                continue;

            bitstring_t raceParts = race->getParts();
            bitstring_t mobParts = pMob->parts;

            if (raceParts == mobParts)
                continue;

            DLString vnum = pMob->vnum;
            bitstring_t adds = mobParts & ~raceParts;
            bitstring_t dels = ~mobParts & raceParts;
            DLString line = 
                "[" + web_cmd(ch, "medit $1", "%5d") + "] "
                + "%-18.18s {g" 
                + web_cmd(ch, "raceedit $1", "%-10.10s") + "{x "
                + (adds ? "[{G%s{x " : "%s")
                + (adds ? web_cmd(ch, "abc reset part " + vnum + " add", "reset") + "]{x" : "")
                + (dels ? "[{r%s{x " : "%s")
                + (dels ? web_cmd(ch, "abc reset part " + vnum + " del", "reset") + "]{x" : "")
                + "   [" + web_cmd(ch, "abc hide part " + vnum, "hide") + "]"
                + "\n\r";
            buf << fmt(0, line.c_str(),
                pMob->vnum, russian_case(pMob->short_descr, '1').c_str(), pMob->race,
                part_flags.names(adds).c_str(), part_flags.names(dels).c_str());

            cnt++;
        }

        page_to_char(buf.str().c_str(), ch);
        return;
    }

    if (arg == "form") {
        ostringstream buf;
        int cnt = 0;
        buf << endl;
        
        for (int i = 0; i < MAX_KEY_HASH && cnt < maxlines; i++)
        for (MOB_INDEX_DATA *pMob = mob_index_hash[i]; pMob && cnt < maxlines; pMob = pMob->next) {
            Race *race = raceManager->find(pMob->race);
            if (!race || !race->isValid()) {
                buf << "[" << pMob->vnum << "] invalid race " << pMob->race << endl;
                continue;
            }
            if (pMob->properties.count("hidden") > 0)
                continue;

            bitstring_t raceForm = race->getForm();
            bitstring_t mobForm = pMob->form;

            if (raceForm == mobForm)
                continue;

            DLString vnum = pMob->vnum;
            bitstring_t adds = mobForm & ~raceForm;
            bitstring_t dels = ~mobForm & raceForm;
            DLString line = 
                "[" + web_cmd(ch, "medit $1", "%5d") + "] "
                + "%-18.18s {g" 
                + web_cmd(ch, "raceedit $1", "%-10.10s") + "{x "
                + (adds ? "[{G%s{x " : "%s")
                + (adds ? web_cmd(ch, "abc reset form " + vnum + " add", "reset") + "]{x" : "")
                + (dels ? "[{r%s{x " : "%s")
                + (dels ? web_cmd(ch, "abc reset form " + vnum + " del", "reset") + "]{x" : "")
                + "   [" + web_cmd(ch, "abc hide form " + vnum, "hide") + "]"
                + "\n\r";
            buf << fmt(0, line.c_str(),
                pMob->vnum, russian_case(pMob->short_descr, '1').c_str(), pMob->race,
                form_flags.names(adds).c_str(), 
                form_flags.names(dels).c_str());

            cnt++;
        }

        page_to_char(buf.str().c_str(), ch);
        return;
    }

    DLString arg2 = args.getOneArgument();
    DLString arg3 = args.getOneArgument();
    MOB_INDEX_DATA *pMob;
    Integer vnum;
    if (!Integer::tryParse(vnum, arg3))
        return;
    
    pMob = get_mob_index(vnum);
    if (!pMob)
        return;

    Race *race = raceManager->find(pMob->race);
    if (!race || !race->isValid())
        return;

    if (arg == "hide") {
        pMob->properties["hidden"] = "true";
        pMob->area->changed = true;
        ch->pecho("Mob %d is now hidden from output.", vnum.getValue());
        __do_abc(ch, const_cast<char *>(arg2.c_str()));
        return;
    }

    if (arg == "reset") {

        if (arg2 == "form") {
            bitstring_t raceForm = race->getForm();
            bitstring_t mobForm = pMob->form;            
            bitstring_t dels = ~mobForm & raceForm;

            if (args == "del")
                pMob->form = dels | pMob->form;
            else if (args == "add")
                pMob->form = raceForm & mobForm;
            else
                return;

            ch->pecho("Mob %d has forms [%s].", vnum.getValue(), form_flags.names(pMob->form).c_str());
        }
        else if (arg2 == "part") {
            bitstring_t racePart = race->getParts();
            bitstring_t mobPart = pMob->parts;
            bitstring_t dels = ~mobPart & racePart;

            if (args == "del")
                pMob->parts = dels | pMob->parts;
            else if (args == "add")
                pMob->parts = racePart & mobPart;
            else
                return;

            ch->pecho("Mob %d has parts [%s].", vnum.getValue(), part_flags.names(pMob->parts).c_str());
        }
        else
            return;

        __do_abc(ch, const_cast<char *>(arg2.c_str()));
        pMob->area->changed = true;
        return;
    }
}

