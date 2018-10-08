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

#include "tedit.h"
#include "merc.h"
#include "act_move.h"
#include "interp.h"

#include "mercdb.h"

#include "olc.h"
#include "security.h"

#include "def.h"


OLC_STATE(OLCStateTrap);

OLCStateTrap::OLCStateTrap( ) : info(0, &trap_flags)
{
}

OLCStateTrap::OLCStateTrap(Room *r, int i) : info(0, &trap_flags)
{
    room.setValue(r->vnum);
    num.setValue( i );
#if 0
    TRAP_DATA *t;
    for(t = r->traps; t; t = t->next)
        if(i-- == 0)
            break;

    if(!t)
        return;

    if(t->u1.to_room)
        target.setValue( t->u1.to_room->vnum );

    surface_quality = t->surface_quality;
    diving_speed = t->diving_speed;
    optimal_move = t->optimal_move;
    info.setValue( t->info );

    if(t->short_desc)
        short_desc = t->short_desc;

    if(t->trap_on_o)
        trap_on_o = t->trap_on_o;
    if(t->trap_on_s)
        trap_on_s = t->trap_on_s;
    
    if(t->diving_on_o)
        diving_on_o = t->diving_on_o;
    if(t->diving_on_s)
        diving_on_s = t->diving_on_s;
    
    if(t->move_on_o)
        move_on_o = t->move_on_o;
    if(t->move_on_s)
        move_on_s = t->move_on_s;
#endif        
}

OLCStateTrap::~OLCStateTrap( )
{
}

void
OLCStateTrap::commit( )
{
#if 0    
    TRAP_DATA *t;
    Room *r = get_room_index(room.getValue( ));
    int i = num.getValue( );
    
    if(!r) {
        LogStream::sendError() << "tedit: commit: room disappeared" << endl;
        return;
    }
    
    for(t = r->traps; t; t = t->next)
        if(i-- == 0)
            break;

    if(!t) {
        t = new_trap( );
        t->next = r->traps;
        r->traps = t;
    }

    t->u1.to_room = get_room_index(target.getValue( ));
    t->surface_quality = surface_quality;
    t->diving_speed = diving_speed;
    t->optimal_move = optimal_move;
    t->info = info;

    free_string(t->short_desc);
    free_string(t->trap_on_s);
    free_string(t->trap_on_o);
    free_string(t->diving_on_s);
    free_string(t->diving_on_o);
    free_string(t->move_on_s);
    free_string(t->move_on_o);

    t->short_desc = str_dup(short_desc.getValue( ).c_str( ));
    t->trap_on_s = str_dup(trap_on_s.getValue( ).c_str( ));
    t->trap_on_o = str_dup(trap_on_o.getValue( ).c_str( ));
    t->diving_on_s = str_dup(diving_on_s.getValue( ).c_str( ));
    t->diving_on_o = str_dup(diving_on_o.getValue( ).c_str( ));
    t->move_on_s = str_dup(move_on_s.getValue( ).c_str( ));
    t->move_on_o = str_dup(move_on_o.getValue( ).c_str( ));
#endif    
}

void
OLCStateTrap::changed( PCharacter * )
{
    /*ignore*/
}

void 
OLCStateTrap::statePrompt( Descriptor *d )
{
    d->send("Editing trap> ");
}

#if 0
TEDIT(flags)
{
    bitstring_t value;
    
    value = trap_flags.bitstring( argument );
    
    if (value != NO_FLAG) {
        info.setValue(info.getValue( ) ^ value);
        stc("Trap info flag toggled.\r\n", ch);
        return true;
    }

    stc("No such trap flag\r\n", ch);
    return false;
}


TEDIT(show)
{
    Room *r;
    
    ptc(ch, "Info:            [{W%s{x]\n\r", 
            trap_flags.names(info.getValue( )).c_str());
    if(target.getValue( ) && (r = get_room_index(target.getValue( ))))
        ptc(ch, "Target:          [{W%d{x] ({G%s{x)\n\r", r->vnum, r->name);
    ptc(ch, "Surface quality: [{W%d{x%%]\n\r", surface_quality.getValue( ));
    ptc(ch, "Diving speed:    [{W%d{x sec]\n\r", diving_speed.getValue( ));
    ptc(ch, "Optimal_move:    [{W%s{x]\n\r", movetypes[optimal_move.getValue( )].name);
    ptc(ch, "Short desc:      [{W%s{x]\n\r", short_desc.getValue( ).c_str( ));
    ptc(ch, "Trap. Self:      [{W%s{x]\n\r", trap_on_s.getValue( ).c_str( ));
    ptc(ch, "Trap. Other:     [{W%s{x]\n\r", trap_on_o.getValue( ).c_str( ));
    ptc(ch, "Diving. Self:    [{W%s{x]\n\r", diving_on_s.getValue( ).c_str( ));
    ptc(ch, "Diving. Other:   [{W%s{x]\n\r", diving_on_o.getValue( ).c_str( ));
    ptc(ch, "Move. Self:      [{W%s{x]\n\r", move_on_s.getValue( ).c_str( ));
    ptc(ch, "Move. Other:     [{W%s{x]\n\r", move_on_o.getValue( ).c_str( ));

    return false;
}

TEDIT(target)
{
    Room *r;
    
    if(!*argument || !is_number(argument) || 
            !(r = get_room_index(atoi(argument)))) {
        stc("Usage: target <room_vnum>\n\r", ch);
        return false;
    }
    
    target = r->vnum;

    stc("Target room set.\n\r", ch);
    return true;
}

TEDIT(quality)
{
    int q;
    
    if(!*argument || !is_number(argument)) {
        stc("Usage: quality <percent>\n\r", ch);
        return false;
    }

    q = atoi(argument);
    
    if(q < 0 || q > 100) {
        stc("Quality percent must be in range 0..100\n\r", ch);
        return false;
    }

    surface_quality = q;

    stc("Surface quality set.\n\r", ch);
    return true;
}

TEDIT(speed)
{
    int s;
    
    if(!*argument || !is_number(argument)) {
        stc("Usage: speed <number>\n\r", ch);
        return false;
    }

    s = atoi(argument);

    diving_speed = s;

    stc("Diving speed set.\n\r", ch);
    return true;
}

TEDIT(optmove)
{
    int i;
    
    if(*argument)
        for(i=0;movetype_names[i] && *movetype_names[i];i++)
            if(is_name(argument, (char*)movetype_names[i])) {
                optimal_move = i;
                stc("Optimal move set.\n\r", ch);
                return true;
            }
    
    stc("Usage: optmove <movetypename>\n\rValid movetypenames:", ch);
    
    for(i=0;movetype_names[i] && *movetype_names[i];i++)
        ptc(ch, " %s", movetype_names[i]);
    
    stc("\n\r", ch);

    return false;
}
TEDIT(short)
{
    if(!*argument) {
        stc("Usage: short <desc>", ch);
        return false;
    }
    
    short_desc = argument;
    stc("Short description set.\n\r", ch);
    return true;
}
    
TEDIT(strap)
{
    if(*argument) {
        trap_on_s = argument;
    } else {
        if(!sedit(trap_on_s))
            return false;
    }
    
    stc("Trap on self message set.\n\r", ch);
    return true;
}

TEDIT(otrap)
{
    if(*argument) {
        trap_on_o = argument;
    } else {
        if(!sedit(trap_on_o))
            return false;
    }
    
    stc("Trap on other message set.\n\r", ch);
    return true;
}

TEDIT(sdiving)
{
    if(*argument) {
        diving_on_s = argument;
    } else {
        if(!sedit(diving_on_s))
            return false;
    }

    stc("Diving on self message set.\n\r", ch);
    return true;
}

TEDIT(odiving)
{
    if(*argument) {
        diving_on_o = argument;
    } else {
        if(!sedit(diving_on_o))
            return false;
    }
    
    stc("Diving on other message set.\n\r", ch);
    return true;
}

TEDIT(smove)
{
    if(*argument) {
        move_on_s = argument;
    } else {
        if(!sedit(move_on_s))
            return false;
    }
    
    stc("Move on self message set.\n\r", ch);
    return true;
}

TEDIT(omove)
{
    if(*argument) {
        move_on_o = argument;
    } else {
        if(!sedit(move_on_o))
            return false;
    }
    
    stc("Move on other message set.\n\r", ch);
    return true;
}

TEDIT(commands)
{
    do_commands(ch);
    return false;
}

TEDIT(done)
{
    commit();
    detach(ch);
    return true;
}

TEDIT(cancel)
{
    detach(ch);
    return false;
}

TEDIT(dump)
{
    ostringstream os;
    XMLStreamable<OLCState> xs( "OLCState" );
    
    xs.setPointer( this);
    xs.toStream(os);

    stc(os.str() + "\r\n", ch);
    return false;
}
#endif 
