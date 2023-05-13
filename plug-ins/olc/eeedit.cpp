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
#include "mercdb.h"

#include "olc.h"
#include "security.h"

#include "def.h"

OLC_STATE(OLCStateExtraExit);

OLCStateExtraExit::OLCStateExtraExit( )
{
}

OLCStateExtraExit::OLCStateExtraExit( RoomIndexData *pRoom, const DLString &name )
{
    room.setValue(pRoom->vnum);
    keyword.setValue(name); 
    
    EXTRA_EXIT_DATA *eexit = pRoom->extra_exits.find(name);
    
    if(!eexit)
        return;

    if(eexit->u1.vnum)
        to_room.setValue( eexit->u1.vnum );
    
    info.setValue( eexit->exit_info_default );
    key.setValue( eexit->key );

    max_size_pass.setValue( eexit->max_size_pass );
    gender_from.setValue(eexit->gender_from.toString());
    gender_to.setValue(eexit->gender_to.toString());
    msgLeaveRoom.setValue(eexit->msgLeaveRoom);
    msgLeaveSelf.setValue(eexit->msgLeaveSelf);
    msgEntryRoom.setValue(eexit->msgEntryRoom);
    msgEntrySelf.setValue(eexit->msgEntrySelf);
    
    if(eexit->keyword)
        keyword.setValue( eexit->keyword );
    if(eexit->short_desc_from)
        short_desc_from.setValue( eexit->short_desc_from );
    if(eexit->short_desc_to)
        short_desc_to.setValue( eexit->short_desc_to );
    if(eexit->description)
        description.setValue( eexit->description );
    if(eexit->room_description)
        room_description.setValue( eexit->room_description );
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

    EXTRA_EXIT_DATA *eexit = pRoom->extra_exits.find(keyword);
    
    if(!eexit) {
        eexit = new EXTRA_EXIT_DATA;
        pRoom->extra_exits.push_front(eexit);
    }

    eexit->u1.vnum = to_room;
    
    eexit->exit_info ^= info.getValue( ) ^ eexit->exit_info_default;
    eexit->exit_info_default = info.getValue( );
    eexit->key = key.getValue( );
    eexit->max_size_pass = max_size_pass.getValue( );

    free_string(eexit->keyword);
    free_string(eexit->short_desc_from);
    free_string(eexit->short_desc_to);
    free_string(eexit->description);
    free_string(eexit->room_description);

    eexit->keyword = str_dup( keyword.getValue( ).c_str( ) );
    eexit->short_desc_from = str_dup( short_desc_from.getValue( ).c_str( ) );
    eexit->short_desc_to = str_dup( short_desc_to.getValue( ).c_str( ) );
    eexit->description = str_dup( description.getValue( ).c_str( ) );
    eexit->room_description = str_dup( room_description.getValue( ).c_str( ) );
    eexit->msgEntryRoom = msgEntryRoom;
    eexit->msgEntrySelf = msgEntrySelf;
    eexit->msgLeaveRoom = msgLeaveRoom;
    eexit->msgLeaveSelf = msgLeaveSelf;
    eexit->gender_from = Grammar::MultiGender(gender_from.getValue().c_str());
    eexit->gender_to = Grammar::MultiGender(gender_to.getValue().c_str());

    // FIXME: need to update all instances
    pRoom->room->extra_exits.findAndDestroy(keyword);
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
    d->send("Editing extra exit> ");
}

EEEDIT(flags)
{
    return flagBitsEdit(exit_flags, info);
}

EEEDIT(size)
{
    int value;
    
    value = size_table.value( argument);

    if (value != NO_FLAG) {
        max_size_pass.setValue( value );
        stc("Size set.\n\r", ch);
        return true;
    }

    stc("unknow size\r\n", ch);
    return false;
}

EEEDIT(show)
{
    show(ch);
    return false;
}

void OLCStateExtraExit::show(PCharacter *ch)
{
    OBJ_INDEX_DATA *obj;
    RoomIndexData *r;

    ptc(ch, "{CDescription:{x %s {D(desc help){x\n%s\n", 
        web_edit_button(ch, "desc", "web").c_str(), description.getValue( ).c_str( ));
    ptc(ch, "{CRoom desc:{x %s {D(rdesc help){x\n%s\n", 
        web_edit_button(ch, "rdesc", "web").c_str(), room_description.getValue( ).c_str( ));
    ptc(ch, "Name:       [{W%s{x]\n", keyword.getValue( ).c_str( ));
    ptc(ch, "Flags:      [{W%s{x] {D(? exit_flags){x\n", 
            exit_flags.names(info.getValue( )).c_str());
    
    if(key.getValue( ) > 0 && (obj = get_obj_index(key.getValue( ))))
        ptc(ch, "Key:        [{W%d{x] ({G%s{x)\n", 
                key.getValue( ), 
                russian_case(obj->short_descr, '1').c_str( ));
    else
        ptc(ch, "Key:        []\n");

    r = get_room_index(to_room.getValue( ));
    if (r)
        ptc(ch, "Target:     [{W%d{x] ({G%s{x)\n", 
                r->vnum, r->name);
    else
        ptc(ch, "Target:     none\n");

    ptc(ch, "Size:       [{W%s{x] {D(? size){x\n", 
            size_table.name(max_size_pass.getValue( )).c_str());

    ptc(ch, "Short desc leave: %s  %s{D(shortleave help){x Gender: %s {D(genderleave){x\n",
              short_desc_from.c_str(), web_edit_button(ch, "shortleave", "web").c_str(),
              gender_from.c_str());

    ptc(ch, "Short desc entry: %s  %s{D(shortentry help){x Gender: %s {D(genderentry){x\n",
              short_desc_to.c_str(), web_edit_button(ch, "shortentry", "web").c_str(),
              gender_to.c_str());

    ptc(ch, "{GLeave room:{x %s  %s{D(leaveroom help){x\n", 
            msgLeaveRoom.c_str(), web_edit_button(ch, "leaveroom", "web").c_str());

    ptc(ch, "{GLeave self:{x %s  %s{D(leaveself help){x\n", 
            msgLeaveSelf.c_str(), web_edit_button(ch, "leaveself", "web").c_str());

    ptc(ch, "{GEntry room:{x %s  %s{D(entryroom help){x\n", 
            msgEntryRoom.c_str(), web_edit_button(ch, "entryroom", "web").c_str());

    ptc(ch, "{GEntry self:{x %s  %s{D(entryself help){x\n", 
            msgEntrySelf.c_str(), web_edit_button(ch, "entryself", "web").c_str());

}

EEEDIT(desc)
{
    return editor(argument, description);      
}

EEEDIT(rdesc)
{
    return editor(argument, room_description);
}

EEEDIT(name)
{
    keyword = argument;
    stc("Keyword set.\n\r", ch);
    return true;
}

EEEDIT(shortleave)
{
    return editor(argument, short_desc_from, (editor_flags)(ED_NO_NEWLINE));      
}

EEEDIT(shortentry)
{
    return editor(argument, short_desc_to, (editor_flags)(ED_NO_NEWLINE));      
}

EEEDIT(leaveroom)
{
    return editor(argument, msgLeaveRoom, (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));      
}

EEEDIT(leaveself)
{
    return editor(argument, msgLeaveSelf, (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));      
}

EEEDIT(entryroom)
{
    return editor(argument, msgEntryRoom, (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));      
}

EEEDIT(entryself)
{
    return editor(argument, msgEntrySelf, (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));      
}

EEEDIT(genderleave)
{
    return genderEdit(gender_from);
}

EEEDIT(genderentry)
{
    return genderEdit(gender_to);
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

