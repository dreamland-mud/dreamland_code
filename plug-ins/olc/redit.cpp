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
#include "fight_extract.h"
#include "comm.h"
#include "merc.h"
#include "act.h"
#include "interp.h"
#include "clanreference.h"
#include "profession.h"
#include "loadsave.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "update_areas.h"
#include "websocketrpc.h"
#include "string_utils.h"
#include "areautils.h"
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
    ch->in_room->areaIndex()->changed = true;
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
        if ((pRoom->sector_type == SECT_WATER_NOSWIM 
            || pRoom->sector_type == SECT_WATER_SWIM
            || pRoom->sector_type == SECT_UNDERWATER)
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
    fAll = arg_is_all(arg);
    found = false;

    for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
        if ((pMobIndex = get_mob_index(vnum)) != NULL) {
            if (fAll || mob_index_has_name(pMobIndex, arg)) {
                found = true;
                buf1 << fmt( 0, "[%5d] ", pMobIndex->vnum );
                buf1 << fmt( 0,  "%-17.17N1{x ", pMobIndex->getShortDescr(LANG_DEFAULT));
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
    fAll = arg_is_all(arg);
    found = false;

    for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
        if ((pObjIndex = get_obj_index(vnum))) {
            if (fAll 
                || obj_index_has_name(pObjIndex, arg)
                || item_table.value( arg ) == pObjIndex->item_type) 
            {
                found = true;
                buf1 << fmt( 0, "[%5d] ", pObjIndex->vnum);
                buf1 << fmt( 0, "%-30.30N1{x ", pObjIndex->getShortDescr(LANG_DEFAULT));
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
    int door;
    const size_t descSize = 300;

    auto &text = pRoom->description;
    DLString entext = String::ellipsis(text.get(EN), descSize);
    DLString uatext = String::ellipsis(text.get(UA), descSize);
    DLString rutext = String::ellipsis(text.get(RU), descSize);

    ptc(ch, "Vnum:    [{W%u{x]  Area: [{W%5d{x] %s\n\r", pRoom->vnum, pRoom->areaIndex->vnum, pRoom->areaIndex->getName().c_str());
    ptc(ch, "Desc EN: %s\n%s\n", web_edit_button(showWeb, ch, "desc", "web").c_str(), entext.c_str());
    ptc(ch, "Desc UA: %s\n%s\n", web_edit_button(showWeb, ch, "uadesc", "web").c_str(), uatext.c_str());
    ptc(ch, "Desc RU: %s\n%s\n", web_edit_button(showWeb, ch, "rudesc", "web").c_str(), rutext.c_str());
    ptc(ch, "Name EN: [{W%s{x] %s\n\r", pRoom->name.get(EN).c_str(), web_edit_button(showWeb, ch, "name", "web").c_str());   
    ptc(ch, "Name UA: [{W%s{x] %s\n\r", pRoom->name.get(UA).c_str(), web_edit_button(showWeb, ch, "uaname", "web").c_str());   
    ptc(ch, "Name RU: [{W%s{x] %s\n\r", pRoom->name.get(RU).c_str(), web_edit_button(showWeb, ch, "runame", "web").c_str());   
    ptc(ch, "Clan:    [{W%s{x]  Guilds: [{W%s{x]\n", pRoom->clan->getName( ).c_str( ), pRoom->guilds.toString().c_str());
    ptc(ch, "Sector:  [{W%s{x] {D(? sector_table){x  Liquid: [{W%s{x] {D(? liquid){x\n\r", 
            sector_table.name(pRoom->sector_type).c_str(), pRoom->liquid->getName().c_str());
    ptc(ch, "Smell:   [{W%s{x] %s [{W%s{x] %s [{W%s{x] %s\n\r", 
          String::stripEOL(pRoom->smell.get(EN)).c_str(), web_edit_button(showWeb, ch, "smell", "web").c_str(),   
          String::stripEOL(pRoom->smell.get(UA)).c_str(), web_edit_button(showWeb, ch, "uasmell", "web").c_str(),   
          String::stripEOL(pRoom->smell.get(RU)).c_str(), web_edit_button(showWeb, ch, "rusmell", "web").c_str());   
    ptc(ch, "Sound:   [{W%s{x] %s [{W%s{x] %s [{W%s{x] %s\n\r", 
          String::stripEOL(pRoom->sound.get(EN)).c_str(), web_edit_button(showWeb, ch, "sound", "web").c_str(),   
          String::stripEOL(pRoom->sound.get(UA)).c_str(), web_edit_button(showWeb, ch, "uasound", "web").c_str(),   
          String::stripEOL(pRoom->sound.get(RU)).c_str(), web_edit_button(showWeb, ch, "rusound", "web").c_str());   
    ptc(ch, "Flags:   [{W%s{x] {D(? room_flags){x\n\r", room_flags.names(pRoom->room_flags).c_str());
    ptc(ch, "Health:  [{W%d{x]%%  Mana: [{W%d{x]%%\r\n", pRoom->heal_rate,  pRoom->mana_rate);
    
    if (!pRoom->extraDescriptions.empty()) {
        ptc(ch, "Extra desc: {D(ed help){x\r\n");
        for (auto &ed: pRoom->extraDescriptions) {
            ptc(ch, "    EN [{W%s{x] %s \r\n", ed->keyword.c_str(), web_edit_button(showWeb, ch, "ed web", ed->keyword).c_str());
            ptc(ch, "    UA [{W%s{x] %s \r\n", ed->keyword.c_str(), web_edit_button(showWeb, ch, "uaed web", ed->keyword).c_str());
            ptc(ch, "    RU [{W%s{x] %s \r\n", ed->keyword.c_str(), web_edit_button(showWeb, ch, "rued web", ed->keyword).c_str());
        }
        
    } else {
        ptc(ch, "Extra desc: (none) {D(ed help){x\r\n");
    }

    if (!pRoom->extra_exits.empty()) {
        ptc(ch, "Extra exits: {D(eexit help){x\r\n");
        for(auto &eed: pRoom->extra_exits) {
            ptc(ch, "    [{W%s{x] %s\r\n", String::toString(eed->keyword).c_str(), web_edit_button(showWeb, ch, "eexit set", eed->keyword.get(EN)).c_str());
        }
        
    } else {
        ptc(ch, "Extra exits: (none) {D(eexit help){x\r\n");
    }

    stc("Exits:      {D(north help){x\r\n", ch);

    for (door = 0; door < DIR_SOMEWHERE; door++) {
        EXIT_DATA *pexit;

        if ((pexit = pRoom->exit[door])) {
            Room *to_room = get_room_instance(pexit->u1.vnum);

            ptc(ch, "-{G%-5s{x ->   [{W%5u{x] {g%s{x\n\r",
                      DLString(dirs[door].name).capitalize( ).c_str( ),
                      to_room ? to_room->vnum : 0,
                      to_room ? to_room->getName() : "");

            if(pexit->key > 0)
                ptc(ch, "            Key:        [{W%7u{x]\n\r", pexit->key);
            
            ptc(ch, "            Exit flags: [{W%s{x] {D(? exit_flags){x\n\r",
                      exit_flags.names(pexit->exit_info).c_str());

            ptc(ch, "            Keywords:   [{W%s{x] [{W%s{x] [{W%s{x]\n\r", 
                    pexit->keyword.get(EN).c_str(),
                    pexit->keyword.get(UA).c_str(),
                    pexit->keyword.get(RU).c_str());

            ptc(ch, "            Short desc: [{W%s{x] [{W%s{x] [{W%s{x]\n\r", 
                    pexit->short_descr.get(EN).c_str(),
                    pexit->short_descr.get(UA).c_str(),
                    pexit->short_descr.get(RU).c_str());

            if (pexit->description.emptyValues())
                ptc(ch, "            Description: [] [] []\n");
            else {
                DLString entext = String::stripEOL(pexit->description.get(EN));
                DLString uatext = String::stripEOL(pexit->description.get(UA));
                DLString rutext = String::stripEOL(pexit->description.get(RU));

                ptc(ch, "            Description EN: %s\n", entext.c_str());            
                ptc(ch, "            Description UA: %s\n", uatext.c_str());            
                ptc(ch, "            Description RU: %s\n", rutext.c_str());            
            }
        }
    }

    if (pRoom->behavior) {
        try {
            std::basic_ostringstream<char> ostr;
            pRoom->behavior->save( ostr );
            ptc(ch, "Legacy behavior: {D(oldbehavior{x)\r\n{W%s{x", ostr.str( ).c_str( ));
            
        } catch (const ExceptionXMLError &e) {
            ptc(ch, "Legacy behavior is BUGGY.\r\n");
        }
    }

    show_behaviors(ch, pRoom->behaviors, pRoom->props);

    /* FIXME: instance or prototype triggers? */
    feniaTriggers->showTriggers(ch, pRoom->room ? pRoom->room->getWrapper() : 0, "room");
}

REDIT(oldbehavior, "старповедение", "редактирование поведения (behavior)")
{
    RoomIndexData *pRoom;
    EDIT_ROOM(ch, pRoom);

    if (argument[0] == '\0') {
        if(!xmledit(pRoom->behavior))
            return false;

        stc("Behavior set.\r\n", ch);
        return true;
    }

    if (arg_is_clear(argument)) {        
        pRoom->behavior.clear( );
        stc("Поведение очищено.\r\n", ch);
        return true;
    }

    stc("Syntax:  oldbehavior       - line edit\n\r", ch);
    stc("Syntax:  oldbehavior clear\n\r", ch);
    return false;
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

REDIT(behaviors, "поведение", "редактирование поведений")
{
    RoomIndexData *pRoom;
    EDIT_ROOM(ch, pRoom);

    return editBehaviors(pRoom->behaviors, pRoom->props);
}

REDIT(props, "свойства", "редактирование свойств поведения")
{
    RoomIndexData *pRoom;
    EDIT_ROOM(ch, pRoom);

    return editProps(pRoom->behaviors, pRoom->props, argument);
}

void OLCStateRoom::delete_exit(RoomIndexData *pRoom, int door)
{
    // FIXME rework to delete all instances.
    if (pRoom->exit[door]) {
        pRoom->exit[door] = NULL;
    }

    if (pRoom->room->exit[door]) {
        pRoom->room->exit[door] = NULL;
    }
}

void OLCStateRoom::create_exit(RoomIndexData *sourceIndex, int door, RoomIndexData *destIndex)
{
    // FIXME rework to link all instances.
    if (!sourceIndex->exit[door]) {
        sourceIndex->exit[door] = new exit_data();
        sourceIndex->room->exit[door] = new exit_data();
    }

    sourceIndex->exit[door]->u1.vnum = destIndex->vnum;
    sourceIndex->exit[door]->orig_door = door;
    sourceIndex->room->exit[door]->u1.to_room = destIndex->room;
    sourceIndex->room->exit[door]->orig_door = door;
}

RoomIndexData* OLCStateRoom::getOriginal()
{
    PCharacter *ch = owner->character->getPC();
    return ch->in_room->pIndexData;
}

void OLCStateRoom::default_door_names(PCharacter *ch, int door)
{
    RoomIndexData *pRoom;
    EDIT_ROOM(ch, pRoom);

    // TODO externalize translations
    auto &kw = pRoom->exit[door]->keyword;
    if (kw.empty()) {
        kw[EN] = "door";
        kw[RU] = "дверь";
        kw[UA] = "двері";
    }

    auto &sd = pRoom->exit[door]->short_descr;
    if (sd.empty()) {
        sd[EN] = "door";
        sd[RU] = "двер|ь|и|и|ь|ью|и";
        sd[UA] = "двер|і|ей|ям|і|ями|ях";
    }
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
        ptc(ch, "Syntax:\r\n%s <флаги>          - установить флаги выхода ({y{hcolchelp exit{x)\r\n", name);
        ptc(ch, "%s delete           - удалить выход с обеих сторон\r\n", name);
        ptc(ch, "%s unlink           - удалить выход только с этой стороны\r\n", name);
        ptc(ch, "%s link <vnum>      - создать двусторонний выход в указанную комнату\r\n", name);
        ptc(ch, "%s room <vnum>      - создать односторонний выход в указанную комнату\r\n", name);
        ptc(ch, "%s dig <vnum>       - вырыть новую комнату %s, с внумом vnum\r\n", name, dirs[door].leave);
        ptc(ch, "%s dig next         - вырыть новую комнату %s, со следующим свободным внумом\r\n", name, dirs[door].leave);
        ptc(ch, "%s key <vnum>       - установить ключ, флаги и имена дверей по умолчанию\r\n", name);
        ptc(ch, "%s name <string>    - задать ключевые слова для двери, EN\r\n", name);        
        ptc(ch, "%s uaname <string>  - задать ключевые слова для двери, UA\r\n", name);        
        ptc(ch, "%s runame <string>  - задать ключевые слова для двери, RU\r\n", name);        
        ptc(ch, "%s short <string>   - задать название для двери, EN\r\n", name);        
        ptc(ch, "%s uashort <string> - задать название для двери с падежами, UA\r\n", name);        
        ptc(ch, "%s rushort <string> - задать название для двери с падежами, RU\r\n", name);        
        ptc(ch, "%s desc             - войти в редактор описания того, что видно по look <dir>, EN\r\n", name);        
        ptc(ch, "%s uadesc           - войти в редактор описания того, что видно по look <dir>, UA\r\n", name);        
        ptc(ch, "%s rudesc           - войти в редактор описания того, что видно по look <dir>, RU\r\n", name);
        ptc(ch, "%s copy             - скопировать флаги,имена,ключ на дверь с другой стороны\r\n", name);
        return false;
    }

    if (arg_is_strict(command, "delete")) {
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
                to_room->areaIndex()->changed = true;
            stc("Exit unlinked from remote side.\n\r", ch);
        }

        // Remove this exit.
        delete_exit(pRoom, door);

        stc("Exit unlinked.\n\r", ch);
        return true;
    }

    if (arg_is_strict(command, "copy")) {
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
        dst->keyword = src->keyword;
        dsti->keyword = srci->keyword;

        dst->short_descr = src->short_descr;
        dsti->short_descr = srci->short_descr;

        dst->key = dsti->key = src->key;

        ptc(ch, "У выхода на {g%s{x в комнате [{W%d{x] {W%s{x установлены:\r\n", dirs[rev].name, to_room->vnum, to_room->getName());
        ptc(ch, "    флаги выхода: {g%s{x\r\n", exit_flags.names(dst->exit_info_default).c_str());
        ptc(ch, "    имена: '{g%s{x', '{g%s{x'\r\n", 
                    String::toString(dst->keyword).c_str(), 
                    String::toString(dst->short_descr).c_str());
        ptc(ch, "    ключ: {W%d{x\r\n", dst->key);
        return true;
    }   

    if (arg_is_strict(command, "unlink")) {
        if (!pRoom->exit[door] || !pRoom->room->exit[door]) {
            stc("REdit:  Cannot delete a null exit.\n\r", ch);
            return false;
        }

        // Remove this exit.
        delete_exit(pRoom, door);

        stc("Exit unlinked.\n\r", ch);
        return true;
    }

    if (arg_is_strict(command, "link")) {
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
            pRemoteRoom->areaIndex->changed = true;

        stc("Two-way link established.\n\r", ch);
        return true;
    }

    if (arg_is_strict(command, "dig")) {
        if (arg[0] == '\0') {
            stc("Syntax: [direction] dig (<vnum> | next)\n\r", ch);
            return false;
        }

        RoomIndexData *newRoom = redit_create(ch, arg);
        if(!newRoom)
            return false;
    
        DLString cmd = fmt(0, "link %d", newRoom->vnum);
        change_exit(ch, cmd.c_str(), door);
        return true;
    }

    if (arg_is_strict(command, "room")) {
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

    if (arg_is_strict(command, "key")) {
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
            ch->pecho("Item %d [%N1] is not a key.", pKey->vnum, pKey->getShortDescr(LANG_DEFAULT));
            return false;
        }

        pRoom->exit[door]->key = value;
        pRoom->room->exit[door]->key = value;
        ch->pecho("Exit key set to {W%d{x [{W%N1{x].", pKey->vnum, pKey->getShortDescr(LANG_DEFAULT));

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

    DLString cmd = command;

    if (DLString("name").strSuffix(cmd)) {
        if (arg[0] == '\0') {
            stc("Syntax:  [direction] name [string]\n\r", ch);
            return false;
        }

        if (!pRoom->exit[door]) {
            stc("REdit:  Door doesn't exist.\n\r", ch);
            return false;
        }

        lang_t lang = cmd == "name" ? EN : (cmd == "uaname" ? UA : RU);
        pRoom->exit[door]->keyword[lang] = arg;
        pRoom->room->exit[door]->keyword[lang] = arg;

        ptc(ch, "Exit name set to {g%s{x.\n\r", arg);
        return true;
    }

    if (DLString("short").strSuffix(cmd)) {
        if (arg[0] == '\0') {
            stc("Syntax:  [direction] short [string]\n\r", ch);
            return false;
        }

        if (!pRoom->exit[door]) {
            stc("REdit:  Door doesn't exist.\n\r", ch);
            return false;
        }

        lang_t lang = cmd == "short" ? EN : (cmd == "uashort" ? UA : RU);

        pRoom->exit[door]->short_descr[lang] = arg;
        pRoom->room->exit[door]->short_descr[lang] = arg;

        ptc(ch, "Exit short description set to {g%s{x.\n\r", arg);
        return true;
    }

    if (DLString("desc").strSuffix(cmd)) {
        if (arg[0] == '\0') {
            if (!pRoom->exit[door]) {
                stc("REdit:  Door doesn't exist.\n\r", ch);
                return false;
            }

            lang_t lang = cmd == "desc" ? EN : (cmd == "uadesc" ? UA : RU);

            if(!sedit(pRoom->exit[door]->description[lang]))
                return false;

            pRoom->room->exit[door]->description[lang] = pRoom->exit[door]->description[lang];

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
    return extraDescrEdit(getOriginal()->extraDescriptions);
}

REDIT(uaed, "экстра", "редактор экстра-описаний (ed help)")
{
    return extraDescrEdit(getOriginal()->extraDescriptions);
}

REDIT(rued, "экстра", "редактор экстра-описаний (ed help)")
{
    return extraDescrEdit(getOriginal()->extraDescriptions);
}

RoomIndexData *
OLCStateRoom::redit_create(PCharacter *ch, char *argument)
{
    AreaIndexData *pArea;
    RoomIndexData *pRoom;
    int value;

    EDIT_ROOM(ch, pRoom);

    if(arg_is_strict(argument, "next")) {
        value = next_room(ch, pRoom);
        if (value < 0) {
            ch->pecho("Все внумы в этой зоне уже заняты!");
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
    RoomIndexData *pRoom = getOriginal();
    return editor(argument, pRoom->name[EN], (editor_flags)(ED_NO_NEWLINE|ED_UPPER_FIRST_CHAR));
}

REDIT(uaname, "укимя", "установить название комнаты")
{
    RoomIndexData *pRoom = getOriginal();
    return editor(argument, pRoom->name[UA], (editor_flags)(ED_NO_NEWLINE|ED_UPPER_FIRST_CHAR));
}

REDIT(runame, "руимя", "установить название комнаты")
{
    RoomIndexData *pRoom = getOriginal();
    return editor(argument, pRoom->name[RU], (editor_flags)(ED_NO_NEWLINE|ED_UPPER_FIRST_CHAR));
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

    if (arg_is_clear(argument)) {
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

REDIT(guilds, "гильдии", "установить гильдию для классов или clear")
{
    RoomIndexData *pRoom;
    Profession *prof;

    EDIT_ROOM(ch, pRoom);

    if (argument[0] == '\0') {
        stc("Syntax:  guild [name]\n\r", ch);
        stc("         guild clear\n\r", ch);
        return false;
    }
    
    if (arg_is_clear(argument)) {
        pRoom->guilds.clear();
        stc("All guilds cleared.\n\r", ch);
        return true;
    }

    prof = professionManager->findUnstrict( argument );
    if (!prof) {
        stc("Guild (class) not found.\n\r", ch);
        return false;
    }

    pRoom->guilds.set(*prof);
    stc("Guild set.\n\r", ch);
    return true;
}

REDIT(liquid, "жидкость", "установить жидкость для рек (? liquid)")
{
    RoomIndexData *pRoom;
    EDIT_ROOM(ch, pRoom);
    return globalReferenceEdit<LiquidManager, Liquid>(pRoom->liquid);
}

REDIT(eexit, "экстравыход", "редактор экстра-выходов (eexit help)")
{
    RoomIndexData *pRoom = getOriginal();
    DLString args = argument;
    DLString command = args.getOneArgument();

    if (command.empty() || arg_is_help(command)) {
        stc("Syntax:  eexit set <keyword>\n\r", ch);
        stc("         eexit delete <keyword>\n\r", ch);
        return false;
    }

    if (arg_is(command, "set")) {
        OLCStateExtraExit::Pointer eedp(NEW, pRoom, args);
        eedp->attach(ch);
        eedp->show(ch);
        return false;
    }

    if (arg_is(command, "delete")) {    
        if (!pRoom->extra_exits.findAndDestroy(args)) {
            stc("REdit:  Extra exit keyword not found.\n\r", ch);
            return false;
        }

        // FIXME: need to destroy exit in all instances.
        pRoom->room->extra_exits.findAndDestroy(args);
        stc("Extra exit deleted.\n\r", ch);
        return true;
    }

    findCommand(ch, "eexit")->entryPoint(ch, "");
    return false;
}

REDIT(desc, "описание", "войти в редактор описания комнаты (desc help)")
{
    RoomIndexData *pRoom = getOriginal();
    return editor(argument, pRoom->description[EN]);
}

REDIT(uadesc, "укописание", "войти в редактор описания комнаты (desc help)")
{
    RoomIndexData *pRoom = getOriginal();
    return editor(argument, pRoom->description[UA]);
}

REDIT(rudesc, "руописание", "войти в редактор описания комнаты (desc help)")
{
    RoomIndexData *pRoom = getOriginal();
    return editor(argument, pRoom->description[RU]);
}

REDIT(sound, "звук", "войти в редактор звука комнаты (sound help)")
{
    RoomIndexData *pRoom = getOriginal();
    return editor(argument, pRoom->sound[EN]);
}

REDIT(uasound, "укзвук", "войти в редактор звука комнаты (sound help)")
{
    RoomIndexData *pRoom = getOriginal();
    return editor(argument, pRoom->sound[UA]);
}

REDIT(rusound, "рузвук", "войти в редактор звука комнаты (sound help)")
{
    RoomIndexData *pRoom = getOriginal();
    return editor(argument, pRoom->sound[RU]);
}

REDIT(smell, "запах", "войти в редактор запаха комнаты (smell help)")
{
    RoomIndexData *pRoom = getOriginal();
    return editor(argument, pRoom->smell[EN]);
}

REDIT(uasmell, "укзапах", "войти в редактор запаха комнаты (smell help)")
{
    RoomIndexData *pRoom = getOriginal();
    return editor(argument, pRoom->smell[UA]);
}

REDIT(rusmell, "рузапах", "войти в редактор запаха комнаты (smell help)")
{
    RoomIndexData *pRoom = getOriginal();
    return editor(argument, pRoom->smell[RU]);
}

REDIT(heal, "здоровье", "установить скорость восстановления здоровья и шагов в комнате (100-400)")
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

static bool redit_purge_obj(PCharacter *ch, Object *obj) 
{
    if (!OLCState::can_edit(ch, obj->pIndexData->vnum)) {
        ch->pecho("У тебя недостаточно прав для уничтожения %O2.", obj);
        return false;
    }
    
    oldact("Ты уничтожаешь $o4.", ch, obj, 0, TO_CHAR);
    oldact("$c1 уничтожает $o4.", ch, obj, 0, TO_ROOM);
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
        ch->pecho("У тебя недостаточно прав для уничтожения %C2.", mob);
        return false;
    }

    oldact("Ты уничтожаешь $C4.", ch, 0, mob, TO_CHAR);
    oldact("$c1 уничтожает $C4.", ch, 0, mob, TO_ROOM);
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
    
    if (arg_is_all(arg)) {
        fObj = true;
        fMob = true;
    } else if (arg_is_strict(arg, "mob")) {
        fObj = false;
        fMob = true;
    } else if (arg_is_strict(arg, "obj")) {
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

REDIT(move, "переместить", "переместить комнату в новую зону")
{
    RoomIndexData *roomToMove = getOriginal();
    AreaIndexData *targetArea = AreaUtils::lookup(argument);

    if (!targetArea) {
        ptc(ch, "Зона '%s' не найдена.\r\n", argument);
        return false;
    }

    if (!OLCState::can_edit(ch, targetArea)) {
        ptc(ch, "У тебя недостаточно прав для редактирования зоны %d (%s).\n\r", targetArea->vnum, targetArea->getName().c_str());
        return false;
    }
    
    move(ch, roomToMove, targetArea);
    return true;
}

void OLCStateRoom::move(PCharacter* ch, RoomIndexData* roomToMove, AreaIndexData* targetArea)
{
    // 1. Check if target area has a free vnum, grab it (new util AreaUtils::xxx)
    // 2. Create new room index with this vnum and its first instance (new util in RoomUtils::create)
    // 3. Copy source room desc, eds, behavior, flags, props, sector, guilds, ... (externalise OLCStateRoom::copy into RoomUtils)
    // 4. Create same number of exits, duplicate source exit names, flags, vnums, keys (see if a new util can be added)
    // 5. For target rooms, re-point their exits to the newly created room
    // 6. Do the same for extra exits (again, create an util)
    // 7. Delete source room exits, extra exits, extra descriptions etc: from index and instance
    // 8. Transfer ch to the new room
    // 9. Remove instance from room_list and affected room lists
    // 10. Remove index from all maps/area/lists
    // 11. Delete the instance
    // 12. Delete the index

    
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

    if (arg_is_strict(arg1, "show")) {
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
    } else if (arg_is_strict(arg1, "move")) {
        // 'redit move <vnum> <area file or vnum>'
        DLString args = argument;
        DLString arg2 = args.getOneArgument();
        DLString arg3 = args.getOneArgument();
        Integer rvnum;
        AreaIndexData *pArea;

        if (arg2.empty() || arg3.empty()) {
            stc("Использование: redit move <vnum> <area file or vnum>\r\n", ch);
            return;
        }

        if (!Integer::tryParse(rvnum, arg2)) {
            stc("Укажи внум комнаты, которую нужно переместить.\r\n", ch);
            return;
        }        

        pRoom2 = get_room_index(rvnum);
        if (!pRoom2) {
            stc("Нет такой комнаты.\n\r", ch);
            return;
        }

        if (!OLCState::can_edit(ch, pRoom2->vnum)) {
            ptc(ch, "У тебя недостаточно прав для редактирования комнаты %d.\n\r", pRoom2->vnum);
            return;
        }

        pArea = AreaUtils::lookup(arg3);
        if (!pArea) {
            ptc(ch, "Зона '%s' не найдена.\r\n", arg3.c_str());
            return;
        }

        if (!OLCState::can_edit(ch, pArea)) {
            ptc(ch, "У тебя недостаточно прав для редактирования зоны %d (%s).\n\r", pArea->vnum, pArea->getName().c_str());
            return;
        }

        OLCStateRoom::move(ch, pRoom2, pArea);
        return;

    } else if (arg_is_strict(arg1, "reset")) {
// TODO call single method from inside and outside of redit. But conflicts with "reset" command
        if (!OLCState::can_edit(ch, pRoom->vnum)) {
            stc("У тебя недостаточно прав для редактирования комнат.\n\r", ch);
            return;
        }
        reset_room(pRoom->room, FRESET_ALWAYS);
        
        stc("Room reset.\n\r", ch);
        return;
        
    } else if (arg_is_strict(arg1, "goto")) {
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
        
    } else if (arg_is_strict(arg1, "purge")) {
        if (!OLCState::can_edit(ch, pRoom->vnum)) {
            stc("У тебя недостаточно прав для редактирования этой комнаты.\n\r", ch);
            return;
        }
        
        redit_purge(pRoom->room, ch, argument);
        return;

    } else if (arg_is_strict(arg1, "create")) {
// TODO call single method from inside and outside of redit
        if (argument[0] == '\0') {
            stc("Syntax: redit create <vnum>\n\r", ch);
            stc("        redit create next\n\r", ch);
            return;
        }

        pRoom = OLCStateRoom::redit_create(ch, argument);
        if(!pRoom)
            return;

        pRoom->areaIndex->changed = true;
        
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
        stc("       redit move <vnum> <area file or vnum>\r\n", ch);
        return;
    }
    
    if (!OLCState::can_edit(ch, pRoom->vnum)) {
        stc("У тебя недостаточно прав для редактирования комнат.\n\r", ch);
        return;
    }
    
    OLCStateRoom::Pointer sr(NEW);
    sr->attach(ch, pRoom);
}
