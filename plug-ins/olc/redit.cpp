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
#include "../anatolia/handler.h"
#include "act_move.h"
#include "update_areas.h"
#include "websocketrpc.h"

#include "mercdb.h"

#include "redit.h"
#include "eeedit.h"
#include "olc.h"
#include "security.h"
#include "feniatriggers.h"

#include "def.h"

#define EDIT_ROOM(Ch, Room)     ( Room = Ch->in_room->pIndexData )

CLAN(none);
LIQ(none);
LIQ(water);

OLC_STATE(OLCStateRoom);

void
OLCStateRoom::attach(PCharacter *ch, RoomIndexData *pRoom)
{
    originalRoom.setValue( ch->in_room->vnum );
    room.setValue(pRoom->vnum);
    
    // FIXME transfer to default instance
    if (ch->in_room->pIndexData != pRoom)
        transfer_char( ch, ch, pRoom->room );

    OLCState::attach(ch);
}

void
OLCStateRoom::detach(PCharacter *ch)
{
    Room *pRoom;

    pRoom = get_room_instance(originalRoom.getValue( ));

    if(!pRoom)
        return;

    transfer_char( ch, ch, pRoom );
    
    OLCState::detach(ch);
}

void 
OLCStateRoom::commit( )
{
    RoomIndexData *pRoom = get_room_index(room);
    // FIXME: update all instances.
    if (pRoom && pRoom->room) {
        pRoom->room->room_flags = pRoom->room_flags;
    }
}

void
OLCStateRoom::statePrompt( Descriptor *d )
{
    d->send( "Editing room> " );
}

void
OLCStateRoom::changed( PCharacter *ch )
{
    SET_BIT(ch->in_room->areaIndex()->area_flag, AREA_CHANGED);
}

/*-------------------------------------------------------------------------
 * state level commands
 *-------------------------------------------------------------------------*/
REDIT(flags, "флаги", "установить или сбросить флаги комнаты (? room_flags)")
{
    RoomIndexData *pRoom;
    EDIT_ROOM(ch, pRoom);
    return flagBitsEdit(room_flags, pRoom->room_flags);
}

REDIT(sector, "местность", "установить тип местности (? sector_table)")
{
    RoomIndexData *pRoom;
    EDIT_ROOM(ch, pRoom);

    if (flagValueEdit(sector_table, pRoom->sector_type)) {
        if ((pRoom->sector_type == SECT_WATER_NOSWIM || pRoom->sector_type == SECT_WATER_SWIM)
            && pRoom->liquid == liq_none)
            pRoom->liquid = liq_water;
        return true;
    }

    return false;
}

REDIT(rlist, "ксписок", "список всех комнат в данной арии")
{
    AreaIndexData *pArea;
    ostringstream buf;
    char arg[MAX_INPUT_LENGTH];
    int col = 0;

    one_argument(argument, arg);

    pArea = ch->in_room->areaIndex();

    if (pArea->roomIndexes.empty( )) {
        stc("Комната(комнаты) в этой арии не обнаружены.\n\r", ch);
        return false;
    }

    for (auto &i: pArea->roomIndexes) {
        RoomIndexData *pRoomIndex = i.second;

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

REDIT(mlist, "мсписок", "список всех мобов в данной арии, по имени или all")
{
    MOB_INDEX_DATA *pMobIndex;
    AreaIndexData *pArea;
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
    pArea = ch->in_room->areaIndex();
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

REDIT(olist, "псписок", "список всех предметов в данной арии, по имени, типу или all")
{
    OBJ_INDEX_DATA *pObjIndex;
    AreaIndexData *pArea;
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
    pArea = ch->in_room->areaIndex();
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
OLCStateRoom::show(PCharacter *ch, RoomIndexData *pRoom, bool showWeb)
{
    Object *obj;
    Character *rch;
    int door;
    bool fcnt;

    ptc(ch, "Description: %s\n\r%s", 
        web_edit_button(showWeb, ch, "desc", "web").c_str(), pRoom->description);

    ptc(ch, "Name:       [{W%s{x] %s\n\rArea:       [{W%5d{x] %s\n\r",
              pRoom->name, web_edit_button(showWeb, ch, "name", "web").c_str(),
              pRoom->areaIndex->vnum, pRoom->areaIndex->name);
    ptc(ch, "Vnum:       [{W%u{x]\n\r", pRoom->vnum);
    ptc(ch, "Clan:       [{W%s{x] ", pRoom->clan->getName( ).c_str( ));
    ptc(ch, "Guilds: [{W%s{x]\n\r", pRoom->guilds.toString().c_str());
    ptc(ch, "Sector:     [{W%s{x] {D(? sector_table){x ",
              sector_table.name(pRoom->sector_type).c_str());
    ptc(ch, "Liquid: [{W%s{x] {D(? liquid){x\n\r", pRoom->liquid->getName( ).c_str( ));
        
    ptc(ch, "Flags:      [{W%s{x] {D(? room_flags){x\n\r",
              room_flags.names(pRoom->room_flags).c_str());
    ptc(ch, "Health:     [{W%d{x]%%\n\rMana:       [{W%d{x]%%\n\r",
              pRoom->heal_rate, pRoom->mana_rate);
    
    if (!pRoom->properties.empty( )) {
        ptc(ch, "Properties: {D(property){x\n\r");
        for (Properties::const_iterator p = pRoom->properties.begin( ); p != pRoom->properties.end( ); p++)
            ptc(ch, "%20s: %s\n\r", p->first.c_str( ), p->second.c_str( ));
    }

    if (pRoom->extra_descr) {
        EXTRA_DESCR_DATA *ed;

        stc("Extra desc: ", ch);
        for (ed = pRoom->extra_descr; ed; ed = ed->next) {
            ptc(ch, "[%s] %s ", ed->keyword, web_edit_button(showWeb, ch, "ed web", ed->keyword).c_str());
        }
        stc("{D(ed help){x\n\r", ch);
    }

    if (!pRoom->extra_exits.empty()) {
        stc("Extra exits: {D(eexit help){x\r\n            ", ch);
        for(auto &eed: pRoom->extra_exits) {
            ptc(ch, "[%s] %s ", eed->keyword, web_edit_button(showWeb, ch, "eexit set", eed->keyword).c_str());
        }
        stc("{x\n\r", ch);
    }
    
    stc("Characters: [{W", ch);
    fcnt = false;
    for (rch = pRoom->room->people; rch; rch = rch->next_in_room) {
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
    for (obj = pRoom->room->contents; obj; obj = obj->next_content) {
        ptc(ch, "%s ", obj->getFirstName().c_str());
        fcnt = true;
    }

    if (fcnt)
        stc("{x]\n\r", ch);
    else
        stc("none{x]\n\r", ch);

    stc("Exits:      {D(north help){x\r\n", ch);

    for (door = 0; door < DIR_SOMEWHERE; door++) {
        EXIT_DATA *pexit;

        if ((pexit = pRoom->exit[door])) {
            Room *to_room = get_room_instance(pexit->u1.vnum);

            ptc(ch, "-{G%-5s{x ->   [{W%5u{x] %s\n\r",
                      DLString(dirs[door].name).capitalize( ).c_str( ),
                      to_room ? to_room->vnum : 0,
                      to_room ? to_room->getName() : "");

            if(pexit->key > 0)
                ptc(ch, "            Key: [{W%7u{x]\n\r", pexit->key);
            
            ptc(ch, "            Exit flags:    [{W%s{x] {D(? exit_flags){x\n\r",
                      exit_flags.names(pexit->exit_info).c_str());

            ptc(ch, "            Default flags: [{W%s{x]\n\r", 
                      exit_flags.names(pexit->exit_info_default).c_str());

            if (pexit->keyword && pexit->keyword[0] != '\0') {
                ptc(ch, "            Keywords:      [{W%s{x]\n\r", pexit->keyword);
            }

            if (pexit->short_descr && pexit->short_descr[0] != '\0') {
                ptc(ch, "            Short desc:    [{W%s{x]\n\r", pexit->short_descr);
            }

            if (pexit->description && pexit->description[0] != '\0') {
                stc(pexit->description, ch);
            }
        }
    }

    if (pRoom->behavior) {
        try {
            std::basic_ostringstream<char> ostr;
            pRoom->behavior->save( ostr );
            ptc(ch, "Behavior:\r\n{W%s{x\r\n", ostr.str( ).c_str( ));
            
        } catch (const ExceptionXMLError &e) {
            ptc(ch, "Behavior is BUGGY.\r\n");
        }
    }

    show_fenia_triggers(ch, pRoom->wrapper);
    feniaTriggers->showAvailableTriggers(ch, "room");
}

REDIT(show, "показать", "показать все поля")
{
    RoomIndexData *pRoom;
    EDIT_ROOM(ch, pRoom);

    show(ch, pRoom, true);

    return false;
}

REDIT(fenia, "феня", "редактирование триггеров")
{
    RoomIndexData *pRoom;
    EDIT_ROOM(ch, pRoom);
    XMLRoomIndexData room(pRoom);

    feniaTriggers->openEditor(ch, room, argument);
    return false;
}

void OLCStateRoom::delete_exit(RoomIndexData *pRoom, int door)
{
    // FIXME rework to delete all instances.
    if (pRoom->exit[door]) {
        free_exit(pRoom->exit[door]);
        pRoom->exit[door] = NULL;
    }

    if (pRoom->room->exit[door]) {
        free_exit(pRoom->room->exit[door]);
        pRoom->room->exit[door] = NULL;
    }
}

void OLCStateRoom::create_exit(RoomIndexData *sourceIndex, int door, RoomIndexData *destIndex)
{
    // FIXME rework to link all instances.
    if (!sourceIndex->exit[door]) {
        sourceIndex->exit[door] = new_exit();
        sourceIndex->room->exit[door] = new_exit();
    }

    sourceIndex->exit[door]->u1.vnum = destIndex->vnum;
    sourceIndex->exit[door]->orig_door = door;
    sourceIndex->room->exit[door]->u1.to_room = destIndex->room;
    sourceIndex->room->exit[door]->orig_door = door;
}

void OLCStateRoom::default_door_names(PCharacter *ch, int door)
{
    RoomIndexData *pRoom;

    EDIT_ROOM(ch, pRoom);

    if (!pRoom->exit[door]->keyword || !pRoom->exit[door]->keyword[0])
        change_exit(ch, "name 'door дверь'", door);   

    if (!pRoom->exit[door]->short_descr || !pRoom->exit[door]->short_descr[0])
        change_exit(ch, "short двер|ь|и|и|ь|ью|и", door);
}

bool 
OLCStateRoom::change_exit(PCharacter * ch, const char *cargument, int door)
{
    // FIXME think of approach to change index data and instance exits at the same time.
    RoomIndexData *pRoom;
    Room *to_room;
    char command[MAX_INPUT_LENGTH];
    char argumentBuf[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char *argument;
    bitstring_t value;

    EDIT_ROOM(ch, pRoom);
    strcpy(argumentBuf, cargument);
    argument = argumentBuf;

    // Set the exit flags, needs full argument.
    // ----------------------------------------
    if ((value = exit_flags.bitstring( argument )) != NO_FLAG) {
        if (!pRoom->exit[door]) {
            stc("Здесь нет двери.\n\r", ch);
            return false;
        }

        TOGGLE_BIT(pRoom->exit[door]->exit_info_default, value);
        TOGGLE_BIT(pRoom->room->exit[door]->exit_info_default, value);

        // Don't toggle exit_info because it can be changed by players.
        pRoom->exit[door]->exit_info = pRoom->exit[door]->exit_info_default;
        pRoom->room->exit[door]->exit_info = pRoom->room->exit[door]->exit_info_default;

        ptc(ch, "Exit flags set to {g%s{x.\n\r", exit_flags.names(pRoom->exit[door]->exit_info_default).c_str());

        if (IS_SET(pRoom->exit[door]->exit_info_default, EX_ISDOOR))
            default_door_names(ch, door);

        return true;
    }

    // Now parse the arguments.
    argument = one_argument(argument, command);
    one_argument(argument, arg);

    if (command[0] == '\0' && argument[0] == '\0') {        /* Move command. */
        move_char(ch, door );
        return false;
    }

    if (arg_is_help(command)) {
        const char *name = dirs[door].name;
        ptc(ch, "Syntax:\r\n%s <флаги>        - установить флаги выхода ({y{hcolchelp exit{x)\r\n", name);
        ptc(ch, "%s delete         - удалить выход с обеих сторон\r\n", name);
        ptc(ch, "%s unlink         - удалить выход только с этой стороны\r\n", name);
        ptc(ch, "%s link <vnum>    - создать двусторонний выход в указанную комнату\r\n", name);
        ptc(ch, "%s room <vnum>    - создать односторонний выход в указанную комнату\r\n", name);
        ptc(ch, "%s dig <vnum>     - вырыть новую комнату %s, с внумом vnum\r\n", name, dirs[door].leave);
        ptc(ch, "%s dig next       - вырыть новую комнату %s, со следующим свободным внумом\r\n", name, dirs[door].leave);
        ptc(ch, "%s key <vnum>     - установить ключ, флаги и имена дверей по умолчанию\r\n", name);
        ptc(ch, "%s name <string>  - задать ключевые слова для двери\r\n", name);        
        ptc(ch, "%s short <string> - задать название для двери с падежами\r\n", name);        
        ptc(ch, "%s desc           - войти в редактор описания того, что видно по look <dir>\r\n", name);        
        ptc(ch, "%s copy           - скопировать флаги,имена,ключ на дверь с другой стороны\r\n", name);
        return false;
    }

    if (!str_cmp(command, "delete")) {
        int rev;

        if (!pRoom->exit[door] || !pRoom->room->exit[door]) {
            stc("REdit:  Cannot delete a null exit.\n\r", ch);
            return false;
        }

        // Remove ToRoom Exit.
        rev = dirs[door].rev;
        to_room = pRoom->room->exit[door]->u1.to_room;

        if (to_room->exit[rev] && to_room->exit[rev]->u1.to_room == pRoom->room) {
            delete_exit(to_room->pIndexData, rev);
            
            if (pRoom->areaIndex != to_room->areaIndex()) // FIXME instances
                SET_BIT(to_room->areaIndex()->area_flag, AREA_CHANGED);
            stc("Exit unlinked from remote side.\n\r", ch);
        }

        // Remove this exit.
        delete_exit(pRoom, door);

        stc("Exit unlinked.\n\r", ch);
        return true;
    }

    if (!str_cmp(command, "copy")) {
        int rev;

        if (!pRoom->exit[door]) {
            stc("REdit: Cannot copy a null exit.\n\r", ch);
            return false;
        }

        rev = dirs[door].rev;
        to_room = pRoom->room->exit[door]->u1.to_room;
        if (!to_room || !to_room->exit[rev] || to_room->exit[rev]->u1.to_room != pRoom->room) {
            stc("Выхода с другой стороны не существует, либо он не ведет обратно в эту комнату.\r\n", ch);
            return false;
        }

        EXIT_DATA *src = pRoom->exit[door];
        EXIT_DATA *srci = pRoom->room->exit[door];
        EXIT_DATA *dst = to_room->pIndexData->exit[rev];
        EXIT_DATA *dsti = to_room->exit[rev];

        dst->exit_info_default = src->exit_info_default;
        dst->exit_info = dst->exit_info_default;
        dsti->exit_info_default = srci->exit_info_default;
        dsti->exit_info = dsti->exit_info_default;

        free_string(dst->keyword);
        dst->keyword = str_dup(src->keyword);
        free_string(dsti->keyword);
        dsti->keyword = str_dup(srci->keyword);

        free_string(dst->short_descr);
        dst->short_descr = str_dup(src->short_descr);
        free_string(dsti->short_descr);
        dsti->short_descr = str_dup(srci->short_descr);

        dst->key = dsti->key = src->key;

        ptc(ch, "У выхода на {g%s{x в комнате [{W%d{x] {W%s{x установлены:\r\n", dirs[rev].name, to_room->vnum, to_room->getName());
        ptc(ch, "    флаги выхода: {g%s{x\r\n", exit_flags.names(dst->exit_info_default).c_str());
        ptc(ch, "    имена: '{g%s{x', '{g%s{x'\r\n", dst->keyword, dst->short_descr);
        ptc(ch, "    ключ: {W%d{x\r\n", dst->key);
        return true;
    }   

    if (!str_cmp(command, "unlink")) {
        if (!pRoom->exit[door] || !pRoom->room->exit[door]) {
            stc("REdit:  Cannot delete a null exit.\n\r", ch);
            return false;
        }

        // Remove this exit.
        delete_exit(pRoom, door);

        stc("Exit unlinked.\n\r", ch);
        return true;
    }

    if (!str_cmp(command, "link")) {
        RoomIndexData *pRemoteRoom;

        if (arg[0] == '\0' || !is_number(arg)) {
            stc("Syntax:  [direction] link [vnum]\n\r", ch);
            return false;
        }

        value = atoi(arg);
        pRemoteRoom = get_room_index(value);

        if (!pRemoteRoom) {
            stc("REdit:  Cannot link to non-existent room.\n\r", ch);
            return false;
        }

        if (!OLCState::can_edit(ch, value)) {
            stc("REdit:  Cannot link to that area.\n\r", ch);
            return false;
        }

        if (pRemoteRoom->exit[dirs[door].rev]) {
            stc("REdit:  Remote side's exit already exists.\n\r", ch);
            return false;
        }

        create_exit(pRoom, door, pRemoteRoom);
        door = dirs[door].rev;
        create_exit(pRemoteRoom, door, pRoom);

        if (pRoom->areaIndex != pRemoteRoom->areaIndex) // FIXME instances
            SET_BIT(pRemoteRoom->areaIndex->area_flag, AREA_CHANGED);

        stc("Two-way link established.\n\r", ch);
        return true;
    }

    if (!str_cmp(command, "dig")) {
        char buf[MAX_STRING_LENGTH];

        if (arg[0] == '\0') {
            stc("Syntax: [direction] dig (<vnum> | next)\n\r", ch);
            return false;
        }

        RoomIndexData *newRoom = redit_create(ch, arg);
        if(!newRoom)
            return false;
    
        sprintf(buf, "link %d", newRoom->vnum);
        change_exit(ch, buf, door);
        return true;
    }

    if (!str_cmp(command, "room")) {
        RoomIndexData *pRemoteRoom;

        if (arg[0] == '\0' || !is_number(arg)) {
            stc("Syntax:  [direction] room [vnum]\n\r", ch);
            return false;
        }

        value = atoi(arg);
        pRemoteRoom = get_room_index(value);

        if (!pRemoteRoom) {
            stc("REdit:  Cannot link to non-existent room.\n\r", ch);
            return false;
        }

        create_exit(pRoom, door, pRemoteRoom);

        stc("One-way link established.\n\r", ch);
        return true;
    }

    if (!str_cmp(command, "key")) {
        if (arg[0] == '\0' || (!is_number(arg) && !arg_is_clear(arg))) {
            stc("Syntax:  [direction] key [vnum]\n\r", ch);
            stc("         [direction] key clear\n\r", ch);
            return false;
        }

        if (!pRoom->exit[door]) {
            stc("REdit:  Door doesn't exist.\n\r", ch);
            return false;
        }

        if (arg_is_clear(arg)) {
            if (pRoom->exit[door]->key <= 0) {
                stc("У этой двери и так нет ключа.\r\n", ch);
                return false;
            }

            pRoom->exit[door]->key = 0;
            pRoom->room->exit[door]->key = 0;
            stc("Ключ удален.\r\n", ch);
            return true;
        }

        value = atoi(arg);
        OBJ_INDEX_DATA *pKey = get_obj_index(value);

        if (!pKey) {
            stc("REdit:  Item doesn't exist.\n\r", ch);
            return false;
        }

        if (pKey->item_type != ITEM_KEY) {
            ptc(ch, "Item %d [%s] is not a key.\r\n", pKey->vnum, russian_case(pKey->short_descr, '1').c_str());
            return false;
        }

        pRoom->exit[door]->key = value;
        pRoom->room->exit[door]->key = value;
        ptc(ch, "Exit key set to {W%d{x [{W%s{x].\r\n",
            pKey->vnum, russian_case(pKey->short_descr, '1').c_str());

        bitstring_t newflags = 0;

        if (!IS_SET(pRoom->exit[door]->exit_info_default, EX_ISDOOR))
            SET_BIT(newflags, EX_ISDOOR);
        
        if (!IS_SET(pRoom->exit[door]->exit_info_default, EX_CLOSED))
            SET_BIT(newflags, EX_CLOSED);
        
        if (!IS_SET(pRoom->exit[door]->exit_info_default, EX_LOCKED))
            SET_BIT(newflags, EX_LOCKED);

        if (newflags != 0)
            change_exit(ch, exit_flags.names(newflags).c_str(), door);

        default_door_names(ch, door);
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
        free_string(pRoom->room->exit[door]->keyword);
        pRoom->room->exit[door]->keyword = str_dup(arg);

        ptc(ch, "Exit name set to {g%s{x.\n\r", arg);
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
        free_string(pRoom->room->exit[door]->short_descr);
        pRoom->room->exit[door]->short_descr = str_dup(arg);

        ptc(ch, "Exit short description set to {g%s{x.\n\r", arg);
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

            free_string(pRoom->room->exit[door]->description);
            pRoom->room->exit[door]->description = str_dup(pRoom->exit[door]->description);

            stc("REdit:  exit description set.\n\r", ch);
            return true;
        }

        stc("Syntax:  [direction] desc\n\r", ch);
        return false;
    }
    return false;
}

REDIT(ed, "экстра", "редактор экстра-описаний (ed help)")
{
    RoomIndexData *pRoom;

    EDIT_ROOM(ch, pRoom);

    return extraDescrEdit(pRoom->extra_descr);
}

RoomIndexData *
OLCStateRoom::redit_create(PCharacter *ch, char *argument)
{
    AreaIndexData *pArea;
    RoomIndexData *pRoom;
    int value;

    EDIT_ROOM(ch, pRoom);

    if(!str_cmp(argument, "next")) {
        value = next_room(ch, pRoom);
        if (value < 0) {
            ch->println("Все внумы в этой зоне уже заняты!");
            return 0;
        }
    } else
        value = atoi(argument);

    if (argument[0] == '\0' || value <= 0) {
        stc("Syntax:  create <vnum>\n\r", ch);
        stc("         create next\n\r", ch);
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

    pRoom = new RoomIndexData;
    pRoom->vnum = value;
    pRoom->areaIndex = get_vnum_area(value);

    if (value > top_vnum_room)
        top_vnum_room = value;

    roomIndexMap[value] = pRoom;
    pRoom->areaIndex->roomIndexes[value] = pRoom;

    pRoom->create();

    stc("Room created.\n\r", ch);
    return pRoom;
}

REDIT(create, "создать", "создать комнату с указанным внумом или next")
{
    RoomIndexData *pRoom;
    
    pRoom = redit_create(ch, argument);
    
    if(!pRoom) 
        return false;

    OLCStateRoom::Pointer sr(NEW);

    sr->attach(ch, pRoom);
    return true;
}

REDIT(name, "имя", "установить название комнаты")
{
    RoomIndexData *pRoom;

    EDIT_ROOM(ch, pRoom);

    return editor(argument, pRoom->name, (editor_flags)(ED_NO_NEWLINE|ED_UPPER_FIRST_CHAR));
}

REDIT(clan, "клан", "установить клановую принадлежность или clear")
{
    RoomIndexData *pRoom;
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

REDIT(guilds, "гильдии", "установить гильдию для профессий или clear")
{
    RoomIndexData *pRoom;
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

REDIT(liquid, "жидкость", "установить жидкость для рек (? liquid)")
{
    RoomIndexData *pRoom;
    Liquid *liq;

    EDIT_ROOM(ch, pRoom);

    if (argument[0] == '\0') {
        stc("Syntax:  liquid [name]\n\r", ch);
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

REDIT(eexit, "экстравыход", "редактор экстра-выходов (eexit help)")
{
    RoomIndexData *pRoom;
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
        eedp->show(ch);
        return false;
    }

    if (is_name(command, "delete")) {    
        if (!pRoom->extra_exits.findAndDestroy(argument)) {
            stc("REdit:  Extra exit keyword not found.\n\r", ch);
            return false;
        }

        // FIXME: need to destroy exit in all instances.
        pRoom->room->extra_exits.findAndDestroy(argument);
        stc("Extra exit deleted.\n\r", ch);
        return true;
    }

    findCommand(ch, "eexit")->run(ch, "");
    return false;
}

REDIT(desc, "описание", "войти в редактор описания комнаты (desc help)")
{
    RoomIndexData *pRoom;

    EDIT_ROOM(ch, pRoom);

    return editor(argument, pRoom->description);
}

REDIT(heal, "здоровье", "установить скорость восстановления здоровья в комнате (100-400)")
{
    RoomIndexData *pRoom;

    EDIT_ROOM(ch, pRoom);

    if (is_number(argument)) {
        int i = atoi(argument);
        pRoom->heal_rate = i;
        stc("Heal rate set.\n\r", ch);
        return true;
    }

    stc("Syntax : heal <#xnumber>\n\r", ch);
    return false;
}

REDIT(mana, "мана", "установить скорость восстановления маны в комнате (100-400)")
{
    RoomIndexData *pRoom;

    EDIT_ROOM(ch, pRoom);

    if (is_number(argument)) {
        int i = atoi(argument);
        pRoom->mana_rate = i;
        stc("Mana rate set.\n\r", ch);
        return true;
    }

    stc("Syntax : mana <#xnumber>\n\r", ch);
    return false;
}

REDIT(property, "свойства", "редактор свойств комнаты")
{
    RoomIndexData *pRoom;

    EDIT_ROOM(ch, pRoom);
    DLString args = DLString( argument );
    return mapEdit( pRoom->properties, args );
}

static bool redit_purge_obj(PCharacter *ch, Object *obj) 
{
    if (!OLCState::can_edit(ch, obj->pIndexData->vnum)) {
        ptc(ch, "У тебя недостаточно прав для уничтожения %s.\n\r", obj->getShortDescr('2').c_str());
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
        ptc(ch, "У тебя недостаточно прав для уничтожения %s.\n\r", mob->getNameP('2').c_str());
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
    if (!arg[0] || arg_is_help(arg)) {
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

REDIT(purge, "уничтожить", "очистить комнату, уничтожив сущности (purge help)")
{
    RoomIndexData *pRoom;

    EDIT_ROOM(ch, pRoom);
    redit_purge(pRoom->room, ch, argument);
    return false;
}

REDIT(commands, "команды", "показать список встроенных команд redit")
{
    do_commands(ch);
    return false;
}

REDIT(done, "готово", "выйти из редактора (не забывайте про asave changed)")
{
    commit();
    detach(ch);
    return true;
}

/*-------------------------------------------------------------------------
 * movements. last declared - first priority
 *-------------------------------------------------------------------------*/
REDIT(north, "север", "редактор дверей (north help)")
{
    if (change_exit(ch, argument, DIR_NORTH))
        return true;
    return false;
}

REDIT(south, "юг", "редактор дверей")
{
    if (change_exit(ch, argument, DIR_SOUTH))
        return true;
    return false;
}

REDIT(east, "восток", "редактор дверей")
{
    if (change_exit(ch, argument, DIR_EAST))
        return true;
    return false;
}

REDIT(west, "запад", "редактор дверей")
{
    if (change_exit(ch, argument, DIR_WEST))
        return true;
    return false;
}

REDIT(up, "вверх", "редактор дверей")
{
    if (change_exit(ch, argument, DIR_UP))
        return true;
    return false;
}

REDIT(down, "вниз", "редактор дверей")
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
    RoomIndexData *pRoom, *pRoom2;
    char arg1[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg1);

    pRoom = ch->in_room->pIndexData;

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
        
        OLCStateRoom::show(ch, pRoom2, false);
        return;
        
    } else if (!str_cmp(arg1, "reset")) {
// TODO call single method from inside and outside of redit. But conflicts with "reset" command
        if (!OLCState::can_edit(ch, pRoom->vnum)) {
            stc("У тебя недостаточно прав для редактирования комнат.\n\r", ch);
            return;
        }
        reset_room(pRoom->room, FRESET_ALWAYS);
        
        stc("Room reset.\n\r", ch);
        return;
        
    } else if (!str_cmp(arg1, "goto")) {
// TODO call single method from inside and outside of redit
        if (!is_number(argument)) {
            stc("Syntax: redit goto <vnum>\n\r", ch);
            return;
        }

        Room *r = get_room_instance(atoi(argument));
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
        
        redit_purge(pRoom->room, ch, argument);
        return;

    } else if (!str_cmp(arg1, "create")) {
// TODO call single method from inside and outside of redit
        if (argument[0] == '\0') {
            stc("Syntax: redit create <vnum>\n\r", ch);
            stc("        redit create next\n\r", ch);
            return;
        }

        pRoom = OLCStateRoom::redit_create(ch, argument);
        if(!pRoom)
            return;

        SET_BIT(pRoom->areaIndex->area_flag, AREA_CHANGED);
        
    } else if(is_number(arg1)) {
        pRoom = get_room_index(atoi(arg1));

        if(!pRoom) {
            stc("Нет такой комнаты.\r\n", ch);
            return;
        }
    } else if(!*arg1) {
        /*nothing*/
    } else {
        stc("Usage: redit show [vnum]\r\n", ch);
        stc("       redit reset\r\n", ch);
        stc("       redit purge mobs|objs|all\r\n", ch);
        stc("       redit create next|<vnum>\r\n", ch);
        stc("       redit [vnum]\r\n", ch);
        stc("       redit goto <vnum>\r\n", ch);
        return;
    }
    
    if (!OLCState::can_edit(ch, pRoom->vnum)) {
        stc("У тебя недостаточно прав для редактирования комнат.\n\r", ch);
        return;
    }
    
    OLCStateRoom::Pointer sr(NEW);
    sr->attach(ch, pRoom);
}
