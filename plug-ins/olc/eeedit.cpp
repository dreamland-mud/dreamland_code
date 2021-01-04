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
    moving_from.setValue( eexit->moving_from );
    moving_mode_from.setValue( eexit->moving_mode_from );
    moving_to.setValue( eexit->moving_to );
    moving_mode_to.setValue( eexit->moving_mode_to );
    max_size_pass.setValue( eexit->max_size_pass );
    
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

    SET_BIT(pRoom->areaIndex->area_flag, AREA_CHANGED);

    EXTRA_EXIT_DATA *eexit = pRoom->extra_exits.find(keyword);
    
    if(!eexit) {
        eexit = new EXTRA_EXIT_DATA;
        pRoom->extra_exits.push_front(eexit);
    }

    eexit->u1.vnum = to_room;
    
    eexit->exit_info ^= info.getValue( ) ^ eexit->exit_info_default;
    eexit->exit_info_default = info.getValue( );
    eexit->key = key.getValue( );
    eexit->moving_from = moving_from.getValue( );
    eexit->moving_mode_from = moving_mode_from.getValue( );
    eexit->moving_to = moving_to.getValue( );
    eexit->moving_mode_to = moving_mode_to.getValue( );
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

extern const char * extra_move_ru [];
extern const char * extra_move_rp [];
extern const char * extra_move_rt [];


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

    ptc(ch, "From:       Type: [{W%d{x] ({G%s{x) Mode: [{W%d{x] ({G%s{x) {W%s{x\n", 
            moving_from.getValue( ), extra_move_ru[moving_from.getValue( )],
            moving_mode_from.getValue( ), extra_move_rt[moving_mode_from.getValue( )],
            short_desc_from.c_str( )
            );
    ptc(ch, "{D            (from <0..11> <0..11> <short descr>)\n");
    
    ptc(ch, "To:         Type: [{W%d{x] ({G%s{x) Mode: [{W%d{x] ({G%s{x) {W%s{x\n", 
            moving_to.getValue( ), extra_move_rp[moving_to.getValue( )],
            moving_mode_to.getValue( ), extra_move_rt[moving_mode_to.getValue( )],
            short_desc_to.c_str()
            );
    ptc(ch, "{D            (to <0..11> <0..11> <short descr>)\n");            
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

static bool
eeedit_dir_usage(PCharacter *ch) 
{
    stc("Usage: from|to <#type> <#mode> <short>\n\r", ch);
    return false;
}

EEEDIT(from)
{
    char buf[MAX_STRING_LENGTH];
    int type, mode;

    if(!*argument)
        return eeedit_dir_usage(ch);

    argument = one_argument(argument, buf);
    if(!*buf || !is_number(buf)) 
        return eeedit_dir_usage(ch);
    type = atoi(buf);
    
    argument = one_argument(argument, buf);
    if(!*buf || !is_number(buf)) 
        return eeedit_dir_usage(ch);
    
    mode = atoi(buf);

    if(!*argument)
        return eeedit_dir_usage(ch);
    
    if(type < 0 || type > 11) {
        stc("Type must be in range 0..11\n\r", ch);
        return eeedit_dir_usage(ch);
    }
    
    if(mode < 0 || mode > 11) {
        stc("Mode must be in range 0..11\n\r", ch);
        return eeedit_dir_usage(ch);
    }
    
    moving_from = type;
    moving_mode_from = mode;
    
    short_desc_from = argument;
    
    stc("From message set.\n\r", ch);
    return true;
}

EEEDIT(to)
{
    char buf[MAX_STRING_LENGTH];
    int type, mode;

    if(!*argument)
        return eeedit_dir_usage(ch);

    argument = one_argument(argument, buf);
    if(!*buf || !is_number(buf)) 
        return eeedit_dir_usage(ch);
    type = atoi(buf);
    
    argument = one_argument(argument, buf);
    if(!*buf || !is_number(buf)) 
        return eeedit_dir_usage(ch);
    
    mode = atoi(buf);

    if(!*argument)
        return eeedit_dir_usage(ch);
    
    if(type < 0 || type > 11) {
        stc("Type must be in range 0..11\n\r", ch);
        return eeedit_dir_usage(ch);
    }
    
    if(mode < 0 || mode > 11) {
        stc("Mode must be in range 0..11\n\r", ch);
        return eeedit_dir_usage(ch);
    }
    
    moving_to = type;
    moving_mode_to = mode;
    
    short_desc_to = argument;

    stc("To message set.\n\r", ch);
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

