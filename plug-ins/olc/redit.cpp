/* $Id$
 *
 * ruffina, 2004
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include <config.h>

#include <npcharacter.h>
#include <pcharacter.h>
#include <commandmanager.h>
#include "logstream.h"
#include <object.h>
#include <affect.h>
#include "room.h"

#include "comm.h"
#include "merc.h"
#include "act.h"
#include "interp.h"
#include "clanreference.h"
#include "profession.h"
#include "handler.h"
#include "act_move.h"
#include "update_areas.h"

#include "mercdb.h"

#include "redit.h"
#include "tedit.h"
#include "eeedit.h"
#include "olc.h"
#include "security.h"

#include "def.h"

#define EDIT_ROOM(Ch, Room)     ( Room = Ch->in_room )

CLAN(none);
LIQ(none);
LIQ(water);

OLC_STATE(OLCStateRoom);

void
OLCStateRoom::attach(PCharacter *ch, Room *pRoom)
{
    originalRoom.setValue( ch->in_room->vnum );
    
    transfer_char( ch, ch, pRoom );

    OLCState::attach(ch);
}

void
OLCStateRoom::detach(PCharacter *ch)
{
    Room *pRoom;

    pRoom = get_room_index(originalRoom.getValue( ));

    if(!pRoom)
        return;

    transfer_char( ch, ch, pRoom );
    
    OLCState::detach(ch);
}

void 
OLCStateRoom::commit( )
{
    /*we are stateliess - no commit*/
}

void
OLCStateRoom::statePrompt( Descriptor *d )
{
    d->send( "Editing room> " );
}

void
OLCStateRoom::changed( PCharacter *ch )
{
    SET_BIT(ch->in_room->area->area_flag, AREA_CHANGED);
}

/*-------------------------------------------------------------------------
 * state level commands
 *-------------------------------------------------------------------------*/
REDIT(flag)
{
    bitstring_t value;
    Room *pRoom;

    EDIT_ROOM(ch, pRoom);
    
    if(!*argument) {
        stc("Usage flag <room_flag>\n\r", ch);
        return false;
    }
    
    if ((value = room_flags.bitstring( argument )) != NO_FLAG) {
        TOGGLE_BIT(pRoom->room_flags, value);
        stc("Room flag toggled.\n\r", ch);
        return true;
    }

    stc("No such room flag. See `olchelp room' for possible values.\n\r", ch);
    return false;
}

REDIT(sector)
{
    int value;
    Room *pRoom;
    
    EDIT_ROOM(ch, pRoom);
    
    if(!*argument) {
        stc("Usage: sector <sector_type>\n\r", ch);
        return false;
    }
    
    if ((value = sector_table.value( argument )) != NO_FLAG) {
        pRoom->sector_type = value;
        stc("Sector type set.\n\r", ch);
        if (IS_WATER(pRoom) && pRoom->liquid == liq_none)
            pRoom->liquid = liq_water;
        return true;
    }

    stc("No such sector type. See `olchelp sector' for possible values.\n\r", ch);
    return false;
}

REDIT(rlist)
{
    Room *pRoomIndex;
    AREA_DATA *pArea;
    ostringstream buf;
    char arg[MAX_INPUT_LENGTH];
    int col = 0;

    one_argument(argument, arg);

    pArea = ch->in_room->area;

    if (pArea->rooms.empty( )) {
        stc("Комната(комнаты) в этой арии не обнаружены.\n\r", ch);
        return false;
    }

    for (map<int, Room *>::iterator i = pArea->rooms.begin( ); i != pArea->rooms.end( ); i++) {
        pRoomIndex = i->second;

        buf << fmt( 0, "[%7d] %-17.17s",
                       pRoomIndex->vnum,
                       pRoomIndex->name );

        if (++col % 3 == 0)
            buf << endl;
    }

    if (col % 3 != 0)
        buf << endl;

    page_to_char(buf.str( ).c_str( ), ch);
    return false;
}

REDIT(mlist)
{
    MOB_INDEX_DATA *pMobIndex;
    AREA_DATA *pArea;
    ostringstream buf1;
    char arg[MAX_INPUT_LENGTH];
    bool fAll, found;
    int vnum;
    int col = 0;

    if(!*argument) {
        stc("Синтаксис:  mlist <all/имя>\n\r", ch);
        return false;
    }

    one_argument(argument, arg);
    pArea = ch->in_room->area;
    fAll = !str_cmp(arg, "all");
    found = false;

    for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
        if ((pMobIndex = get_mob_index(vnum)) != NULL) {
            if (fAll || is_name(arg, pMobIndex->player_name)) {
                found = true;
                buf1 << fmt( 0, "[%5d] ", pMobIndex->vnum );
                buf1 << fmt( 0,  "%-17.17s{x ", russian_case(pMobIndex->short_descr, '1').c_str( ));
                if (++col % 3 == 0)
                    buf1 << endl;
            }
        }
    }

    if (!found) {
        stc("Монстр(ы) в этой арии не найдены.\n\r", ch);
        return false;
    }

    if (col % 3 != 0)
        buf1 << endl;

    page_to_char(buf1.str( ).c_str( ), ch);
    return false;
}

REDIT(olist)
{
    OBJ_INDEX_DATA *pObjIndex;
    AREA_DATA *pArea;
    ostringstream buf1;
    char arg[MAX_INPUT_LENGTH];
    bool fAll, found;
    int vnum;
    int col = 0;

    if(!*argument) {
        stc("Синтаксис:  olist <all/имя/тип_предмета>\n\r", ch);
        return false;
    }

    one_argument(argument, arg);
    pArea = ch->in_room->area;
    fAll = !str_cmp(arg, "all");
    found = false;

    for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
        if ((pObjIndex = get_obj_index(vnum))) {
            if (fAll 
                || is_name(arg, pObjIndex->name)
                || item_table.value( arg ) == pObjIndex->item_type) 
            {
                found = true;
                buf1 << fmt( 0, "[%5d] ", pObjIndex->vnum);
                buf1 << fmt( 0, "%-30.30s{x ", russian_case(pObjIndex->short_descr, '1').c_str( ));
                if (++col % 2 == 0)
                    buf1 << endl;
            }
        }
    }

    if (!found) {
        stc("Предмет(предметы) в этой арии не найдены.\n\r", ch);
        return false;
    }

    if (col % 3 != 0)
        buf1 << endl;

    page_to_char(buf1.str( ).c_str( ), ch);
    return false;
}

void
OLCStateRoom::show(PCharacter *ch, Room *pRoom)
{
    Object *obj;
    Character *rch;
    int door;
    bool fcnt;

    ptc(ch, "Description:\n\r%s", pRoom->description);
    ptc(ch, "Name:       [{W%s{x]\n\rArea:       [{W%5d{x] %s\n\r",
              pRoom->name, pRoom->area->vnum, pRoom->area->name);
    ptc(ch, "Vnum:       [{W%u{x]\n\r", pRoom->vnum);
    if(pRoom->clan != clan_none)
        ptc(ch, "Clan:       [{W%s{x]\n\r", pRoom->clan->getName( ).c_str( ));
    if (!pRoom->guilds.empty()) 
        ptc(ch, "Guilds:     [{W%s{x]\n\r", pRoom->guilds.toString().c_str());
    if(pRoom->liquid != liq_none)
        ptc(ch, "Liquid: [{W%s{x]\n\r", pRoom->liquid->getName( ).c_str( ));
    ptc(ch, "Sector:     [{W%s{x]\n\r",
              sector_table.name(pRoom->sector_type).c_str());
        
    ptc(ch, "Room flags: [{W%s{x]\n\r",
              room_flags.names(pRoom->room_flags).c_str());
    ptc(ch, "Health recovery:[{W%d{x]\n\rMana recovery  :[{W%d{x]\n\r",
              pRoom->heal_rate_default, pRoom->mana_rate_default);
    
    if (!pRoom->properties.empty( )) {
        ptc(ch, "Properties:\n\r");
        for (Properties::const_iterator p = pRoom->properties.begin( ); p != pRoom->properties.end( ); p++)
            ptc(ch, "%20s: %s\n\r", p->first.c_str( ), p->second.c_str( ));
    }

    if (pRoom->extra_descr) {
        EXTRA_DESCR_DATA *ed;

        stc("Desc Kwds:  [{W", ch);
        for (ed = pRoom->extra_descr; ed; ed = ed->next) {
            stc(ed->keyword, ch);
            if (ed->next)
                stc(" ", ch);
        }
        stc("{x]\n\r", ch);
    }

    if (pRoom->extra_exit) {
        EXTRA_EXIT_DATA *eed;

        stc("Extra exits:[{W", ch);
        for(eed = pRoom->extra_exit; eed; eed = eed->next) {
            stc(eed->keyword, ch);
            if(eed->next)
                stc(" ", ch);
        }
        stc("{x]\n\r", ch);
    }
    
    /* XXX onDive */
    
    stc("Characters: [{W", ch);
    fcnt = false;
    for (rch = pRoom->people; rch; rch = rch->next_in_room) {
        DLString names = rch->getNameP();
        ptc(ch, "%s ", names.getOneArgument().c_str());
        fcnt = true;
    }

    if (fcnt)
        stc("{x]\n\r", ch);
    else
        stc("none{x]\n\r", ch);

    stc("Objects:    [{W", ch);
    fcnt = false;
    for (obj = pRoom->contents; obj; obj = obj->next_content) {
        ptc(ch, "%s ", obj->getFirstName().c_str());
        fcnt = true;
    }

    if (fcnt)
        stc("{x]\n\r", ch);
    else
        stc("none{x]\n\r", ch);

    for (door = 0; door < DIR_SOMEWHERE; door++) {
        EXIT_DATA *pexit;

        if ((pexit = pRoom->exit[door])) {
            ptc(ch, "-{G%-5s{x -> [{W%7u{x]:\n\r",
                      DLString(dirs[door].name).capitalize( ).c_str( ),
                      pexit->u1.to_room ? pexit->u1.to_room->vnum : 0);

            if(pexit->key > 0)
                ptc(ch, "        Key: [{W%7u{x]\n\r", pexit->key);
            
            ptc(ch, "        Exit flags: [{W%s{x]\n\r",
                      exit_flags.names(pexit->exit_info).c_str());

            ptc(ch, "        Default exit flags: [{W%s{x]\n\r", 
                      exit_flags.names(pexit->exit_info_default).c_str());

            if (pexit->keyword && pexit->keyword[0] != '\0') {
                ptc(ch, "        Kwds: [{W%s{x]\n\r", pexit->keyword);
            }

            if (pexit->short_descr && pexit->short_descr[0] != '\0') {
                ptc(ch, "  Short desc: [{W%s{x]\n\r", pexit->short_descr);
            }

            if (pexit->description && pexit->description[0] != '\0') {
                stc(pexit->description, ch);
            }
        }
    }

    if (pRoom->behavior) {
        try {
            std::basic_ostringstream<char> ostr;
            pRoom->behavior.toStream( ostr );
            ptc(ch, "Behavior:\r\n%s\r\n", ostr.str( ).c_str( ));
            
        } catch (const ExceptionXMLError &e) {
            ptc(ch, "Behavior is BUGGY.\r\n");
        }
    }
}

REDIT(show)
{
    Room *pRoom;
    EDIT_ROOM(ch, pRoom);

    show(ch, pRoom);

    return false;
}

bool 
OLCStateRoom::change_exit(PCharacter * ch, char *argument, int door)
{
    Room *pRoom, *pToRoom;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    bitstring_t value;

    EDIT_ROOM(ch, pRoom);

    // Set the exit flags, needs full argument.
    // ----------------------------------------
    if ((value = exit_flags.bitstring( argument )) != NO_FLAG) {
        //Room *pToRoom;
        //int rev;

        if (!pRoom->exit[door]) {
            stc("Здесь нет двери.\n\r", ch);
            return false;
        }

        // This room.
        TOGGLE_BIT(pRoom->exit[door]->exit_info_default, value);
        // Don't toggle exit_info because it can be changed by players.
        pRoom->exit[door]->exit_info = pRoom->exit[door]->exit_info_default;

        /*
        // Connected room.
        pToRoom = pRoom->exit[door]->u1.to_room;
        rev = dirs[door].rev;

        if (pToRoom->exit[rev] != NULL) {
            TOGGLE_BIT(pToRoom->exit[rev]->exit_info_default, value);
            TOGGLE_BIT(pToRoom->exit[rev]->exit_info, value);
        }
        */

        stc("Exit flag toggled.\n\r", ch);
        return true;
    }

    // Now parse the arguments.
    argument = one_argument(argument, command);
    one_argument(argument, arg);

    if (command[0] == '\0' && argument[0] == '\0') {        /* Move command. */
        move_char(ch, door );
        return false;
    }

    if (command[0] == '?') {
//        do_help(ch, "EXIT");
        return false;
    }

    if (!str_cmp(command, "delete")) {
        int rev;

        if (!pRoom->exit[door]) {
            stc("REdit:  Cannot delete a null exit.\n\r", ch);
            return false;
        }

        // Remove ToRoom Exit.
        rev = dirs[door].rev;
        pToRoom = pRoom->exit[door]->u1.to_room;

        if (pToRoom->exit[rev] && pToRoom->exit[rev]->u1.to_room == pRoom) {
            free_exit(pToRoom->exit[rev]);
            pToRoom->exit[rev] = NULL;
            
            if(pRoom->area != pToRoom->area)
                SET_BIT(pToRoom->area->area_flag, AREA_CHANGED);
            stc("Exit unlinked from remote side.\n\r", ch);
        }

        // Remove this exit.
        free_exit(pRoom->exit[door]);
        pRoom->exit[door] = NULL;

        stc("Exit unlinked.\n\r", ch);
        return true;
    }

    if (!str_cmp(command, "unlink")) {
        if (!pRoom->exit[door]) {
            stc("REdit:  Cannot delete a null exit.\n\r", ch);
            return false;
        }

        // Remove this exit.
        free_exit(pRoom->exit[door]);
        pRoom->exit[door] = NULL;

        stc("Exit unlinked.\n\r", ch);
        return true;
    }

    if (!str_cmp(command, "link")) {
        EXIT_DATA *pExit;

        if (arg[0] == '\0' || !is_number(arg)) {
            stc("Syntax:  [direction] link [vnum]\n\r", ch);
            return false;
        }

        value = atoi(arg);

        if (!get_room_index(value)) {
            stc("REdit:  Cannot link to non-existant room.\n\r", ch);
            return false;
        }

        if (!OLCState::can_edit(ch, value)) {
            stc("REdit:  Cannot link to that area.\n\r", ch);
            return false;
        }

        if (get_room_index(value)->exit[dirs[door].rev]) {
            stc("REdit:  Remote side's exit already exists.\n\r", ch);
            return false;
        }

        if (!pRoom->exit[door]) {
            pRoom->exit[door] = new_exit();
        }

        pToRoom = pRoom->exit[door]->u1.to_room = get_room_index(value);
        pRoom->exit[door]->orig_door = door;

        door = dirs[door].rev;
        pExit = new_exit();
        pExit->u1.to_room = pRoom;
        pExit->orig_door = door;
        pToRoom->exit[door] = pExit;

        if(pRoom->area != pToRoom->area)
            SET_BIT(pToRoom->area->area_flag, AREA_CHANGED);

        stc("Two-way link established.\n\r", ch);
        return true;
    }

    if (!str_cmp(command, "dig")) {
        char buf[MAX_STRING_LENGTH];

        if (arg[0] == '\0') {
            stc("Syntax: [direction] dig (<vnum> | next)\n\r", ch);
            return false;
        }

        Room *newRoom = redit_create(ch, arg);
        if(!newRoom)
            return false;
    
        sprintf(buf, "link %d", newRoom->vnum);
        change_exit(ch, buf, door);
        return true;
    }

    if (!str_cmp(command, "room")) {
        if (arg[0] == '\0' || !is_number(arg)) {
            stc("Syntax:  [direction] room [vnum]\n\r", ch);
            return false;
        }

        value = atoi(arg);

        if (!get_room_index(value)) {
            stc("REdit:  Cannot link to non-existant room.\n\r", ch);
            return false;
        }

        if (!pRoom->exit[door])
            pRoom->exit[door] = new_exit();

        pRoom->exit[door]->u1.to_room = get_room_index(value);
        pRoom->exit[door]->orig_door = door;

        stc("One-way link established.\n\r", ch);
        return true;
    }

    if (!str_cmp(command, "key")) {
        if (arg[0] == '\0' || !is_number(arg)) {
            stc("Syntax:  [direction] key [vnum]\n\r", ch);
            return false;
        }

        if (!pRoom->exit[door]) {
            stc("REdit:  Door doesn't exist.\n\r", ch);
            return false;
        }

        value = atoi(arg);

        if (!get_obj_index(value)) {
            stc("REdit:  Item doesn't exist.\n\r", ch);
            return false;
        }

        if (get_obj_index(atoi(argument))->item_type != ITEM_KEY) {
            stc("REdit:  Key doesn't exist.\n\r", ch);
            return false;
        }

        pRoom->exit[door]->key = value;

        stc("Exit key set.\n\r", ch);
        return true;
    }

    if (!str_cmp(command, "name")) {
        if (arg[0] == '\0') {
            stc("Syntax:  [direction] name [string]\n\r", ch);
            return false;
        }

        if (!pRoom->exit[door]) {
            stc("REdit:  Door doesn't exist.\n\r", ch);
            return false;
        }

        free_string(pRoom->exit[door]->keyword);
        pRoom->exit[door]->keyword = str_dup(arg);

        stc("Exit name set.\n\r", ch);
        return true;
    }

    if (!str_cmp(command, "short")) {
        if (arg[0] == '\0') {
            stc("Syntax:  [direction] short [string]\n\r", ch);
            return false;
        }

        if (!pRoom->exit[door]) {
            stc("REdit:  Door doesn't exist.\n\r", ch);
            return false;
        }

        free_string(pRoom->exit[door]->short_descr);
        pRoom->exit[door]->short_descr = str_dup(arg);

        stc("Exit short description set.\n\r", ch);
        return true;
    }

    if (!str_prefix(command, "description")) {
        if (arg[0] == '\0') {
            if (!pRoom->exit[door]) {
                stc("REdit:  Door doesn't exist.\n\r", ch);
                return false;
            }
            if(!sedit(pRoom->exit[door]->description))
                return false;

            stc("REdit:  exit description set.\n\r", ch);
            return true;
        }

        stc("Syntax:  [direction] desc\n\r", ch);
        return false;
    }
    return false;
}

REDIT(ed)
{
    Room *pRoom;
    EXTRA_DESCR_DATA *ed;
    char command[MAX_INPUT_LENGTH];

    EDIT_ROOM(ch, pRoom);

    argument = one_argument(argument, command);

    if (command[0] == '\0' || argument[0] == '\0') {
        stc("Syntax:  ed set [keyword]\n\r", ch);
        stc("         ed delete [keyword]\n\r", ch);
        return false;
    }

    if (is_name(command, "set")) {
        for (ed = pRoom->extra_descr; ed; ed = ed->next) {
            if (is_name(argument, ed->keyword))
                break;
        }

        char *desc = str_empty;

        if(ed)
            desc = ed->description;

        if(!sedit(desc))
            return false;

        if (!ed) {
            ed = new_extra_descr();
            ed->keyword = str_dup(argument);
            ed->next = pRoom->extra_descr;
            pRoom->extra_descr = ed;
        }
        
        ed->description = desc;
        
        stc("Extra description set.\n\r", ch);
        return true;
    }


    if (is_name(command, "delete")) {
        EXTRA_DESCR_DATA *ped = NULL;

        for (ed = pRoom->extra_descr; ed; ed = ed->next) {
            if (is_name(argument, ed->keyword))
                break;
            ped = ed;
        }

        if (!ed) {
            stc("REdit:  Extra description keyword not found.\n\r", ch);
            return false;
        }

        if (!ped)
            pRoom->extra_descr = ed->next;
        else
            ped->next = ed->next;

        free_extra_descr(ed);

        stc("Extra description deleted.\n\r", ch);
        return true;
    }

    findCommand(ch, "ed")->run(ch, "");
    return false;
}

Room *
OLCStateRoom::redit_create(PCharacter *ch, char *argument)
{
    AREA_DATA *pArea;
    Room *pRoom;
    int value;

    EDIT_ROOM(ch, pRoom);

    if(!str_cmp(argument, "next")) {
        value = next_room(ch, pRoom);
    } else
        value = atoi(argument);

    if (argument[0] == '\0' || value <= 0) {
        stc("Syntax:  create [vnum > 0]\n\r", ch);
        return 0;
    }

    pArea = get_vnum_area(value);
    if (!pArea) {
        stc("REdit:  That vnum is not assigned an area.\n\r", ch);
        return 0;
    }

    if (!can_edit(ch, value)) {
        stc("REdit:  Vnum in an area you cannot build in.\n\r", ch);
        return 0;
    }

    if (get_room_index(value)) {
        stc("REdit:  Room vnum already exists.\n\r", ch);
        return 0;
    }

    int iHash;

    pRoom = new_room_index();
    pRoom->vnum = value;
    pRoom->area = get_vnum_area(value);

    if (value > top_vnum_room)
        top_vnum_room = value;

    iHash = (int) value % MAX_KEY_HASH;
    pRoom->next = room_index_hash[iHash];
    room_index_hash[iHash] = pRoom;

    pRoom->rnext = room_list;
    room_list = pRoom;

    pRoom->area->rooms[value] = pRoom;

    stc("Room created.\n\r", ch);
    return pRoom;
}

REDIT(create)
{
    Room *pRoom;
    
    pRoom = redit_create(ch, argument);
    
    if(!pRoom) 
        return false;

    OLCStateRoom::Pointer sr(NEW);

    sr->attach(ch, pRoom);
    return true;
}

REDIT(name)
{
    Room *pRoom;

    EDIT_ROOM(ch, pRoom);

    if (argument[0] == '\0') {
        stc("Syntax:  name [name]\n\r", ch);
        return false;
    }

    free_string(pRoom->name);
    pRoom->name = str_dup(argument);

    stc("Name set.\n\r", ch);
    return true;
}

REDIT(clan)
{
    Room *pRoom;
    Clan *clan;

    EDIT_ROOM(ch, pRoom);

    if (argument[0] == '\0') {
        stc("Syntax:  clan [name]\n\r", ch);
        stc("         clan clear\n\r", ch);
        return false;
    }

    if (!str_cmp(argument, "clear")) {
        pRoom->clan.assign(clan_none);
        stc("Clan cleared.\n\r", ch);
        return true;
    }

    clan = ClanManager::getThis( )->findUnstrict( argument );
    if (!clan) {
        stc("Clan not found\n\r", ch);
        return false;
    }

    pRoom->clan.assign( *clan );

    stc("Clan set.\n\r", ch);
    return true;
}

REDIT(guild)
{
    Room *pRoom;
    Profession *prof;

    EDIT_ROOM(ch, pRoom);

    if (argument[0] == '\0') {
        stc("Syntax:  guild [name]\n\r", ch);
        stc("         guild clear\n\r", ch);
        return false;
    }
    
    if (!str_cmp(argument, "clear")) {
        pRoom->guilds.clear();
        stc("All guilds cleared.\n\r", ch);
        return true;
    }

    prof = professionManager->findUnstrict( argument );
    if (!prof) {
        stc("Guild (profession) not found.\n\r", ch);
        return false;
    }

    pRoom->guilds.set(*prof);
    stc("Guild set.\n\r", ch);
    return true;
}

REDIT(waterliquid)
{
    Room *pRoom;
    Liquid *liq;

    EDIT_ROOM(ch, pRoom);

    if (argument[0] == '\0') {
        stc("Syntax:  waterliquid [name]\n\r", ch);
        return false;
    }
    
    liq = liquidManager->findUnstrict( argument );
    if (!liq) {
        stc("Liquid not found\n\r", ch);
        return false;
    }

    pRoom->liquid.assign( *liq );
    stc("Liquid set.\n\r", ch);
    return true;
}

REDIT(eexit)
{
    Room *pRoom;
    EXTRA_EXIT_DATA *eed;
    char command[MAX_INPUT_LENGTH];

    EDIT_ROOM(ch, pRoom);
    
    argument = one_argument(argument, command);

    if (command[0] == '\0' || argument[0] == '\0') {
        stc("Syntax:  eexit set <keyword>\n\r", ch);
        stc("         eexit delete <keyword>\n\r", ch);
        return false;
    }

    if (is_name(command, "set")) {
        OLCStateExtraExit::Pointer eedp(NEW, pRoom, argument);
        eedp->attach(ch);
        return false;
    }

    if (is_name(command, "delete")) {
        EXTRA_EXIT_DATA *pee = NULL;

        for (eed = pRoom->extra_exit; eed; eed = eed->next) {
            if (is_name(argument, eed->keyword))
                break;
            pee = eed;
        }

        if (!eed) {
            stc("REdit:  Extra exit keyword not found.\n\r", ch);
            return false;
        }

        LogStream::sendNotice() << "found eexit " << eed->keyword << endl;

        if (!pee)
            pRoom->extra_exit = eed->next;
        else
            pee->next = eed->next;

        LogStream::sendNotice() << "pee " << pee << endl;
        free_extra_exit(eed);

        stc("Extra exit deleted.\n\r", ch);
        return true;
    }

    findCommand(ch, "eexit")->run(ch, "");
    return false;
}

REDIT(trap)
{
    Room *pRoom;
    
    EDIT_ROOM(ch, pRoom);
    
    if (!*argument || !is_number(argument)) {
        stc("Syntax:  trap <index>\n\r", ch);
        return false;
    }

    OLCStateTrap::Pointer tedp(NEW, pRoom, atoi(argument));
    tedp->attach(ch);
    return false;
}

REDIT(desc)
{
    Room *pRoom;
    char command[MAX_INPUT_LENGTH];

    EDIT_ROOM(ch, pRoom);

    argument = one_argument(argument, command);

    if (command[0] == '\0') {
        if(!sedit(pRoom->description))
            return false;
        stc("Description set\n\r", ch);
        return true;
    }

    if (is_name(command, "copy")) {
        DLString str = pRoom->description;
        ch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")->regs[0].split(str);
        ptc(ch, "Описание скопировано в буфер.\r\n");
        return true;
    }

    if (is_name(command, "paste")) {
        DLString str = ch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")->regs[0].dump( );
        free_string(pRoom->description);
        pRoom->description = str_dup(str.c_str( ));
        ptc(ch, "Описание вставлено из буфера.\r\n");
        return true;
    }

    stc("Syntax:\n\r", ch);
    stc("    desc      : войти в редактор описаний\n\r", ch);
    stc("    desc copy : скопировать описание в буфер\n\r", ch);
    stc("    desc paste: заменить описание на то, что в буфере\n\r", ch);
    return false;
}

REDIT(heal)
{
    Room *pRoom;

    EDIT_ROOM(ch, pRoom);

    if (is_number(argument)) {
        int i = atoi(argument);
        pRoom->heal_rate += i - pRoom->heal_rate_default; 
        pRoom->heal_rate_default = i;
        stc("Heal rate set.\n\r", ch);
        return true;
    }

    stc("Syntax : heal <#xnumber>\n\r", ch);
    return false;
}

REDIT(mana)
{
    Room *pRoom;

    EDIT_ROOM(ch, pRoom);

    if (is_number(argument)) {
        int i = atoi(argument);
        pRoom->mana_rate += i - pRoom->mana_rate_default; 
        pRoom->mana_rate_default = i;
        stc("Mana rate set.\n\r", ch);
        return true;
    }

    stc("Syntax : mana <#xnumber>\n\r", ch);
    return false;
}

REDIT(property)
{
    Room *pRoom;

    EDIT_ROOM(ch, pRoom);
    DLString args = DLString( argument );
    return mapEdit( pRoom->properties, args );
}

static bool redit_purge_obj(PCharacter *ch, Object *obj) 
{
    if (!OLCState::can_edit(ch, obj->pIndexData->vnum)) {
        ptc(ch, "У тебя недостаточно прав для редактирования %s.\n\r", obj->getShortDescr('2').c_str());
        return false;
    }
    
    act("Ты уничтожаешь $o4.", ch, obj, 0, TO_CHAR);
    act("$c1 уничтожает $o4.", ch, obj, 0, TO_ROOM);
    extract_obj(obj);
    return true;
}

static bool redit_purge_mob(PCharacter *ch, Character *vch) 
{
    NPCharacter *mob;

    if (!vch->is_npc()) 
        return false;
    
    mob = vch->getNPC();
    if (!OLCState::can_edit(ch, mob->pIndexData->vnum)) {
        ptc(ch, "У тебя недостаточно прав для редактирования %s.\n\r", mob->getNameP('2').c_str());
        return false;
    }

    act("Ты уничтожаешь $C4.", ch, 0, mob, TO_CHAR);
    act("$c1 уничтожает $C4.", ch, 0, mob, TO_ROOM);
    extract_char(mob);
    return true;
}

static bool redit_purge(Room *pRoom, PCharacter *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    bool fObj, fMob;
    Character *vch;
    Object *obj;

    one_argument(argument, arg);
    if (!arg[0]) {
        stc("Syntax: \n\rpurge all - destroy all objects and mobiles in the room\n\r"
            "purge mobs - destroy all mobiles in the room\n\r"
            "purge objs - destroy all objects in the room\n\r"
            "purge <name> - destroy obj or mob with given name\n\r", ch);
        return false;
    }
    
    if (!str_cmp(arg, "all")) {
        fObj = true;
        fMob = true;
    } else if (!str_cmp(arg, "mobs")) {
        fObj = false;
        fMob = true;
    } else if (!str_cmp(arg, "objs")) {
        fObj = true;
        fMob = false;
    } else {
        if (( obj = get_obj_room(ch, arg) ))
            return redit_purge_obj(ch, obj);

        if (( vch = get_char_room(ch, arg) )) 
            return redit_purge_mob(ch, vch);

        stc("Жертва не найдена. Используй \"purge all|mobs|objs|<name>\"\n\r", ch);
        return false;
    }
    
    if (fObj) {
        Object *obj_next;

        for (obj = ch->in_room->contents; obj; obj = obj_next) {
            obj_next = obj->next_content;
            redit_purge_obj(ch, obj);
        }
    }
    
    if (fMob) {
        Character *vch_next;

        for (vch = ch->in_room->people; vch; vch = vch_next) {
            vch_next = vch->next_in_room;
            redit_purge_mob(ch, vch);
        }
    }

    return true;
}

REDIT(commands)
{
    do_commands(ch);
    return false;
}

REDIT(done)
{
    commit();
    detach(ch);
    return true;
}

REDIT(cancel)
{
    stc("Rollback not supported by room editor\n\r", ch);
    detach(ch);
    return false;
}

REDIT(behavior)
{
    Room *pRoom;

    EDIT_ROOM(ch, pRoom);

    if (!*argument) {
        XMLDocument::Pointer doc(NEW);

        if (!pRoom->behavior.isEmpty()) {
            XMLNode::Pointer node(NEW);
            pRoom->behavior.toXML(node);
            node->setName("behavior");
            doc->appendChild(node);
        }

        if(!xmledit(doc))
            return false;

        if(doc->getDocumentElement()) {
            pRoom->behavior.fromXML(doc->getDocumentElement());
            pRoom->behavior->setRoom(pRoom);
            stc("Поведение установлено.\r\n", ch);
        } else {
            stc("Пустое поведение? Используй behavior clear для очистки.\r\n", ch);
        }

        return true;
    }

    if (!str_cmp( argument, "clear" )) {
        pRoom->behavior.clear( );
        stc("Поведение очищено.\r\n", ch);
        return true;
    }

    stc("Синтаксис:\r\n", ch);
    stc("behavior       - перейти в текстовый редактор поведенияr\r\n", ch);
    stc("behavior clear - очистить поведение\r\n", ch);
    return false;
}

/*-------------------------------------------------------------------------
 * movements. last declared - first priority
 *-------------------------------------------------------------------------*/
REDIT(north)
{
    if (change_exit(ch, argument, DIR_NORTH))
        return true;
    return false;
}

REDIT(south)
{
    if (change_exit(ch, argument, DIR_SOUTH))
        return true;
    return false;
}

REDIT(east)
{
    if (change_exit(ch, argument, DIR_EAST))
        return true;
    return false;
}

REDIT(west)
{
    if (change_exit(ch, argument, DIR_WEST))
        return true;
    return false;
}

REDIT(up)
{
    if (change_exit(ch, argument, DIR_UP))
        return true;
    return false;
}

REDIT(down)
{
    if (change_exit(ch, argument, DIR_DOWN))
        return true;
    return false;
}

/*-------------------------------------------------------------------------
 * top level commands
 *-------------------------------------------------------------------------*/
CMD(redit, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
        "Online room editor.")
{
    Room *pRoom, *pRoom2;
    char arg1[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg1);

    pRoom = ch->in_room;

    if (!str_cmp(arg1, "show")) {
        if(*argument && is_number(argument))
            pRoom2 = get_room_index(atoi(argument));
        else
            pRoom2 = pRoom;

        if(!pRoom2) {
            stc("Нет такой комнаты.\n\r", ch);
            return;
        }
        if (!OLCState::can_edit(ch, pRoom2->vnum)) {
            stc("У тебя недостаточно прав для редактирования комнат.\n\r", ch);
            return;
        }
        
        OLCStateRoom::show(ch, pRoom2);
        return;
        
    } else if (!str_cmp(arg1, "reset")) {
        if (!OLCState::can_edit(ch, pRoom->vnum)) {
            stc("У тебя недостаточно прав для редактирования комнат.\n\r", ch);
            return;
        }
        reset_room(pRoom);
        
        stc("Room reset.\n\r", ch);
        return;
        
    } else if (!str_cmp(arg1, "goto")) {
        if (!is_number(argument)) {
            stc("Syntax: redit goto <vnum>\n\r", ch);
            return;
        }

        Room *r = get_room_index(atoi(argument));
        if (!r) {
            stc("Комната с таким номером не найдена.\n\r", ch);
            return;
        }

        if (!OLCState::can_edit(ch, r->vnum)) {
            stc("У тебя недостаточно прав, чтобы попасть в эту комнату.\n\r", ch);
            return;
        }

        transfer_char( ch, ch, r, 
                       "%1$^C1 взмахивает лопатой и исчезает.", NULL,
                       "%1$^C1 внезапно вырастает из-под земли." );
        return;
        
    } else if (!str_cmp(arg1, "purge")) {
        if (!OLCState::can_edit(ch, pRoom->vnum)) {
            stc("У тебя недостаточно прав для редактирования этой комнаты.\n\r", ch);
            return;
        }
        
        redit_purge(pRoom, ch, argument);
        return;

    } else if (!str_cmp(arg1, "create")) {
        if (argument[0] == '\0') {
            stc("Syntax:  edit room create (<vnum> | next)\n\r", ch);
            return;
        }

        pRoom = OLCStateRoom::redit_create(ch, argument);
        if(!pRoom)
            return;

        SET_BIT(pRoom->area->area_flag, AREA_CHANGED);
        
    } else if(is_number(arg1)) {
        pRoom = get_room_index(atoi(arg1));

        if(!pRoom) {
            stc("Нет такой комнаты.\r\n", ch);
            return;
        }
    } else if(!*arg1) {
        /*nothing*/
    } else {
        stc("Usage: edit room show [vnum]\r\n", ch);
        stc("       edit room reset\r\n", ch);
        stc("       edit room purge mobs|objs|all\r\n", ch);
        stc("       edit room create next|<vnum>\r\n", ch);
        stc("       edit room [vnum]\r\n", ch);
        stc("       edit room goto <vnum>\r\n", ch);
        return;
    }
    
    if (!OLCState::can_edit(ch, pRoom->vnum)) {
        stc("У тебя недостаточно прав для редактирования комнат.\n\r", ch);
        return;
    }
    
    OLCStateRoom::Pointer sr(NEW);
    sr->attach(ch, pRoom);
}
