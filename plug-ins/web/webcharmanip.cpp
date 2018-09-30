/* $Id$
 *
 * ruffina, 2018
 */
#include "webmanipcommandtemplate.h"
#include "maniplist.h"

#include <map>
#include <list>
#include <sstream>

#include "logstream.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "command.h"
#include "commandmanager.h"
#include "mobilebehavior.h"
#include "behavior_utils.h"
#include "object.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "descriptor.h"
#include "comm.h"
#include "gsn_plugin.h"
#include "attract.h"
#include "occupations.h"
#include "shoptrader.h"
#include "lover.h"
#include "move_utils.h"
#include "act_lock.h"
#include "handler.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

/*
 * Hold the list of all commands and their arguments that are applicable
 * to the player in given context.
 */
struct PlayerManipList : public ManipList {
    PCharacter *victim;

    PlayerManipList( PCharacter *victim, const DLString &descr ) {
        this->victim = victim;
        this->descr = descr;
    }

    // Add commands to the list that is going to be shown
    // below the divider and passed in the 'l' attribute.
    void addLocal( const DLString &cmdName ) {
        locals.push_back( Manip( cmdName, THIS ) );
    }

    // Add commands to the main list, with various arguments.
    void add( const DLString &cmdName ) {
        manips.push_back( Manip( cmdName, THIS ) );
    }

    virtual DLString getID( ) const {
        return victim->getName( );
    }
};

static bool has_trigger_group( Character *ch, Character *victim )
{
    if (ch->master != 0 || (ch->leader != 0 && ch->leader != ch))
        return false;

    if (victim->master != ch)
        return false;

    return true;
}

static bool has_trigger_nuke( Character *ch, Character *victim )
{
    return ch != victim && is_same_group(victim, ch);
}

static bool has_trigger_mount( Character *victim )
{
    if (!victim->is_npc())
	return IS_SET(victim->form, FORM_CENTAUR);
    else
	return IS_SET(victim->act, ACT_RIDEABLE);
}

static bool has_trigger_murder(Character *ch, Character *victim)
{
    return !ch->is_npc() 
		&& !victim->is_npc() 
		&& ch != victim
		&& !is_safe_nomessage(ch, victim);
}

/*
 * Decorate a PC character with drop-down menu of applicable commands.
 */
WEBMANIP_RUN(decoratePlayer)
{
    const PlayerManipArgs &myArgs = static_cast<const PlayerManipArgs &>( args );
    Character *ch = myArgs.target;
    PCharacter *victim = myArgs.victim;

    PlayerManipList manips( victim, myArgs.descr );

    if (ch->in_room == victim->in_room) {
        manips.add( "look" );
        manips.add( "follow" );

	if (has_trigger_group( ch, victim ))
	    manips.add( "group" );

	if (has_trigger_nuke( ch, victim ))
	    manips.add( "nuke" );

	if (has_trigger_mount( victim ))
	    manips.add( "mount" );

        manips.add( "smell" );

        if (mlove_accepts(ch, victim))
	    manips.add("mlove");
         
        if (has_trigger_murder(ch, victim))
	    manips.add("murder");
    }

    manips.add( "grats" );
    manips.add( "whois" );

    buf << manips.toString( );
    return true;
}



    
