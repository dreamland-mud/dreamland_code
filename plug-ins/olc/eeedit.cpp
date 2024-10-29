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

#include "grammar_entities_impl.h"
#include <character.h>
#include <pcharacter.h>
#include <commandmanager.h>
#include <object.h>
#include <affect.h>
#include "room.h"

#include "eeedit.h"
#include "merc.h"
#include "interp.h"
#include "websocketrpc.h"
#include "string_utils.h"

#include "olc.h"
#include "security.h"

#include "def.h"

OLC_STATE(OLCStateExtraExit);

OLCStateExtraExit::OLCStateExtraExit( )
                     : max_size_pass(SIZE_GARGANTUAN, &size_table)
{
}

OLCStateExtraExit::OLCStateExtraExit( RoomIndexData *pRoom, const DLString &name )
                         : OLCStateExtraExit()
{
    room.setValue(pRoom->vnum);
    keyword.fromMixedString(name);
    
    EXTRA_EXIT_DATA *eexit = pRoom->extra_exits.find(name);
    
    if(!eexit)
        return;

    if(eexit->u1.vnum)
        to_room.setValue( eexit->u1.vnum );
    
    info.setValue( eexit->exit_info_default );
    key.setValue( eexit->key );

    max_size_pass.setValue( eexit->max_size_pass );
    msgLeaveRoom = eexit->msgLeaveRoom;
    msgLeaveSelf = eexit->msgLeaveSelf;
    msgEntryRoom = eexit->msgEntryRoom;
    msgEntrySelf = eexit->msgEntrySelf;
    
    keyword = eexit->keyword;
    short_desc_from = eexit->short_desc_from;
    short_desc_to = eexit->short_desc_to;
    description = eexit->description;
    room_description = eexit->room_description;
}

OLCStateExtraExit::~OLCStateExtraExit( )
{
}

void
OLCStateExtraExit::commit( )
{
    RoomIndexData *pRoom = get_room_index(room.getValue( ));
    
    if(!pRoom)
        return;

    pRoom->areaIndex->changed = true;

    EXTRA_EXIT_DATA *eexit = pRoom->extra_exits.find(String::toString(keyword));
    
    if(!eexit) {
        eexit = new EXTRA_EXIT_DATA;
        pRoom->extra_exits.push_front(eexit);
    }

    eexit->u1.vnum = to_room;
    
    eexit->exit_info ^= info.getValue( ) ^ eexit->exit_info_default;
    eexit->exit_info_default = info.getValue( );
    eexit->key = key.getValue( );
    eexit->max_size_pass = max_size_pass.getValue( );


    eexit->keyword = keyword;
    eexit->short_desc_from = short_desc_from;
    eexit->short_desc_to = short_desc_to;
    eexit->description = description;
    eexit->room_description = room_description;
    eexit->msgEntryRoom = msgEntryRoom;
    eexit->msgEntrySelf = msgEntrySelf;
    eexit->msgLeaveRoom = msgLeaveRoom;
    eexit->msgLeaveSelf = msgLeaveSelf;

    // FIXME: need to update all instances
    pRoom->room->extra_exits.findAndDestroy(String::toString(keyword));
    EXTRA_EXIT_DATA *eexit0 = eexit->create();
    eexit0->resolve();
    pRoom->room->extra_exits.push_front(eexit0);
}

void
OLCStateExtraExit::changed( PCharacter *ch )
{
    /*ignore*/
}

void 
OLCStateExtraExit::statePrompt( Descriptor *d )
{
    d->send("Extra exit> ");
}

void OLCStateExtraExit::show(PCharacter *ch)
{
    OBJ_INDEX_DATA *pKey = key > 0 ? get_obj_index(key) : 0;
    RoomIndexData *target = get_room_index(to_room);

    ptc(ch, "Name EN:    [{W%s{x] %s {D(name help){x\n",   keyword.get(EN).c_str(), web_edit_button(ch, "name", "web").c_str());
    ptc(ch, "Name UA:    [{W%s{x] %s {D(uaname help){x\n", keyword.get(UA).c_str(), web_edit_button(ch, "uaname", "web").c_str());
    ptc(ch, "Name RU:    [{W%s{x] %s {D(runame help){x\n", keyword.get(RU).c_str(), web_edit_button(ch, "runame", "web").c_str());
    ptc(ch, "Flags:      [{W%s{x] {D(? exit_flags){x\n", exit_flags.names(info.getValue( )).c_str());
    ptc(ch, "Size:       [{W%s{x] {D(? size){x\n", size_table.name(max_size_pass.getValue( )).c_str());

    if (pKey)
        ch->pecho("Key:        [{W%d{x] ({G%N1{x)", pKey->vnum, pKey->getShortDescr(LANG_DEFAULT));
    else
        ptc(ch, "Key:        []\n");

    if (target)
        ptc(ch, "Target:     [{W%d{x] ({G%s{x)\n", target->vnum, target->name.get(LANG_DEFAULT).c_str());
    else
        ptc(ch, "Target:     none\n");

    ptc(ch, "Desc EN: %s {D(desc help){x\n%s\n",   web_edit_button(ch, "desc", "web").c_str(), description[EN].c_str());
    ptc(ch, "Desc UA: %s {D(uadesc help){x\n%s\n", web_edit_button(ch, "uadesc", "web").c_str(), description[UA].c_str());
    ptc(ch, "Desc RU: %s {D(rudesc help){x\n%s\n", web_edit_button(ch, "rudesc", "web").c_str(), description[RU].c_str());

    ptc(ch, "Room EN: %s {D(room help){x\n%s\n",   web_edit_button(ch, "room", "web").c_str(), room_description[EN].c_str());
    ptc(ch, "Room UA: %s {D(uaroom help){x\n%s\n", web_edit_button(ch, "uaroom", "web").c_str(), room_description[UA].c_str());
    ptc(ch, "Room RU: %s {D(ruroom help){x\n%s\n", web_edit_button(ch, "ruroom", "web").c_str(), room_description[RU].c_str());

    ptc(ch, "Short from EN: %s  %s{D(from help){x\n",   short_desc_from[EN].c_str(), web_edit_button(ch, "from", "web").c_str());
    ptc(ch, "Short from UA: %s  %s{D(uafrom help){x\n", short_desc_from[UA].c_str(), web_edit_button(ch, "uafrom", "web").c_str());
    ptc(ch, "Short from RU: %s  %s{D(rufrom help){x\n", short_desc_from[RU].c_str(), web_edit_button(ch, "rufrom", "web").c_str());

    ptc(ch, "Short to EN:   %s  %s{D(to help){x\n",   short_desc_to[EN].c_str(), web_edit_button(ch, "to", "web").c_str());
    ptc(ch, "Short to UA:   %s  %s{D(uato help){x\n", short_desc_to[UA].c_str(), web_edit_button(ch, "uato", "web").c_str());
    ptc(ch, "Short to RU:   %s  %s{D(ruto help){x\n", short_desc_to[RU].c_str(), web_edit_button(ch, "ruto", "web").c_str());

    ptc(ch, "Leave room EN: %s  %s{D(leaveroom help){x\n", msgLeaveRoom[EN].c_str(), web_edit_button(ch, "leaveroom", "web").c_str());
    ptc(ch, "Leave room UA: %s  %s{D(ruleaveroom help){x\n", msgLeaveRoom[UA].c_str(), web_edit_button(ch, "ualeaveroom", "web").c_str());
    ptc(ch, "Leave room RU: %s  %s{D(ualeaveroom help){x\n", msgLeaveRoom[RU].c_str(), web_edit_button(ch, "ruleaveroom", "web").c_str());

    ptc(ch, "Leave self EN: %s  %s{D(leaveself help){x\n", msgLeaveSelf[EN].c_str(), web_edit_button(ch, "leaveself", "web").c_str());
    ptc(ch, "Leave self UA: %s  %s{D(ualeaveself help){x\n", msgLeaveSelf[UA].c_str(), web_edit_button(ch, "ualeaveself", "web").c_str());
    ptc(ch, "Leave self RU: %s  %s{D(ruleaveself help){x\n", msgLeaveSelf[RU].c_str(), web_edit_button(ch, "ruleaveself", "web").c_str());

    ptc(ch, "Entry room EN: %s  %s{D(entryroom help){x\n", msgEntryRoom[EN].c_str(), web_edit_button(ch, "entryroom", "web").c_str());
    ptc(ch, "Entry room UA: %s  %s{D(uaentryroom help){x\n", msgEntryRoom[UA].c_str(), web_edit_button(ch, "uaentryroom", "web").c_str());
    ptc(ch, "Entry room RU: %s  %s{D(ruentryroom help){x\n", msgEntryRoom[RU].c_str(), web_edit_button(ch, "ruentryroom", "web").c_str());

    ptc(ch, "Entry self EN: %s  %s{D(entryself help){x\n", msgEntrySelf[EN].c_str(), web_edit_button(ch, "entryself", "web").c_str());
    ptc(ch, "Entry self UA: %s  %s{D(uaentryself help){x\n", msgEntrySelf[UA].c_str(), web_edit_button(ch, "uaentryself", "web").c_str());
    ptc(ch, "Entry self RU: %s  %s{D(ruentryself help){x\n", msgEntrySelf[RU].c_str(), web_edit_button(ch, "ruentryself", "web").c_str());
}

EEEDIT(show)
{
    show(ch);
    return false;
}

EEEDIT(flags)
{
    return flagBitsEdit(exit_flags, info);
}

EEEDIT(size)
{
    return flagValueEdit(max_size_pass);
}

EEEDIT(desc)
{
    return editor(argument, description[EN]);      
}

EEEDIT(uadesc)
{
    return editor(argument, description[UA]);      
}

EEEDIT(rudesc)
{
    return editor(argument, description[RU]);      
}

EEEDIT(room)
{
    return editor(argument, room_description[EN]);
}

EEEDIT(uaroom)
{
    return editor(argument, room_description[UA]);
}

EEEDIT(ruroom)
{
    return editor(argument, room_description[RU]);
}

EEEDIT(name)
{
    return editor(argument, keyword[EN], (editor_flags)(ED_NO_NEWLINE));     
}

EEEDIT(uaname)
{
    return editor(argument, keyword[UA], (editor_flags)(ED_NO_NEWLINE));     
}

EEEDIT(runame)
{
    return editor(argument, keyword[RU], (editor_flags)(ED_NO_NEWLINE));     
}

EEEDIT(from)
{
    return editor(argument, short_desc_from[EN], (editor_flags)(ED_NO_NEWLINE));      
}

EEEDIT(uafrom)
{
    return editor(argument, short_desc_from[UA], (editor_flags)(ED_NO_NEWLINE));      
}

EEEDIT(rufrom)
{
    return editor(argument, short_desc_from[RU], (editor_flags)(ED_NO_NEWLINE));      
}

EEEDIT(to)
{
    return editor(argument, short_desc_to[EN], (editor_flags)(ED_NO_NEWLINE));      
}

EEEDIT(uato)
{
    return editor(argument, short_desc_to[UA], (editor_flags)(ED_NO_NEWLINE));      
}

EEEDIT(ruto)
{
    return editor(argument, short_desc_to[RU], (editor_flags)(ED_NO_NEWLINE));      
}

EEEDIT(leaveroom)
{
    return editor(argument, msgLeaveRoom[EN], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));      
}

EEEDIT(ualeaveroom)
{
    return editor(argument, msgLeaveRoom[UA], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));      
}

EEEDIT(ruleaveroom)
{
    return editor(argument, msgLeaveRoom[RU], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));      
}

EEEDIT(leaveself)
{
    return editor(argument, msgLeaveSelf[EN], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));      
}

EEEDIT(ualeaveself)
{
    return editor(argument, msgLeaveSelf[UA], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));      
}

EEEDIT(ruleaveself)
{
    return editor(argument, msgLeaveSelf[RU], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));      
}

EEEDIT(entryroom)
{
    return editor(argument, msgEntryRoom[EN], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));      
}

EEEDIT(uaentryroom)
{
    return editor(argument, msgEntryRoom[UA], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));      
}

EEEDIT(ruentryroom)
{
    return editor(argument, msgEntryRoom[RU], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));      
}

EEEDIT(entryself)
{
    return editor(argument, msgEntrySelf[EN], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));      
}

EEEDIT(uaentryself)
{
    return editor(argument, msgEntrySelf[UA], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));      
}

EEEDIT(ruentryself)
{
    return editor(argument, msgEntrySelf[RU], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));      
}

EEEDIT(key)
{
    OBJ_INDEX_DATA *k;

    if(!*argument || !is_number(argument) || 
            !(k = get_obj_index(atoi(argument)))) {
        stc("Usage: key <key_vnum>\n\r", ch);
        return false;
    }
    
    key = k->vnum;

    stc("Key set.\n\r", ch);
    return true;
}

EEEDIT(target)
{
    RoomIndexData *r;
    
    if(!*argument || !is_number(argument) || 
            !(r = get_room_index(atoi(argument)))) {
        stc("Usage: target <room_vnum>\n\r", ch);
        return false;
    }
    
    to_room = r->vnum;

    stc("Target room set.\n\r", ch);
    return true;
}

EEEDIT(commands)
{
    do_commands(ch);
    return false;
}

EEEDIT(done)
{
    commit();
    detach(ch);
    return true;
}

EEEDIT(cancel)
{
    detach(ch);
    return false;
}

EEEDIT(dump)
{
    ostringstream os;
    XMLStreamable<OLCState> xs( "OLCState" );
    
    xs.setPointer( this);
    xs.toStream(os);

    stc(os.str() + "\r\n", ch);
    return false;
}

