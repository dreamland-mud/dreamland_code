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

    // Add commands to the main list, with various arguments.
    void add( const DLString &cmdName ) {
        manips.push_back( Manip( cmdName, THIS ) );
    }

    virtual DLString getID( ) const {
        return victim->getName( );
    }
};

struct MobileManipList : public ManipList {
    NPCharacter *victim;

    MobileManipList( NPCharacter *victim, const DLString &descr ) {
        this->victim = victim;
        this->descr = descr;
    }

    // Add commands to the list that is going to be shown
    // below the divider and passed in the 'l' attribute.
    void addLocal( const DLString &cmdName, const DLString &args ) {
        locals.push_back( Manip( cmdName, args ) );
    }

    // Add commands to the main list, with various arguments.
    void add( const DLString &cmdName ) {
        manips.push_back( Manip( cmdName, THIS ) );
    }

    virtual DLString getID( ) const {
        return DLString( victim->getID( ) );
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

static bool has_trigger_kill(Character *ch, Character *victim)
{
    return victim->master != ch
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

/*
 * Decorate a NPC character with drop-down menu of applicable commands.
 */
WEBMANIP_RUN(decorateMobile)
{
    const MobManipArgs &myArgs = static_cast<const MobManipArgs &>( args );
    Character *ch = myArgs.target;
    NPCharacter *victim = myArgs.victim;

    MobileManipList manips( victim, myArgs.descr );

    if (ch->in_room == victim->in_room) {
        manips.add( "look" );

	if (mob_has_occupation(victim, OCC_TRAINER) && ch->getPC()) {
	    for (int i = 0; i < stat_table.size; i++)
		if (ch->perm_stat[stat_table.fields[i].value] < ch->getPC()->getMaxTrain( stat_table.fields[i].value))
		    manips.addLocal("train", russian_case(stat_table.fields[i].message, '4'));

	    if (ch->perm_stat[STAT_CON] < ch->getPC()->getMaxTrain( STAT_CON ))
		manips.addLocal("train", "сложение кп");

	    manips.addLocal("gain", "продать");
	    manips.addLocal("gain", "купить");
        }

	if (mob_has_occupation(victim, OCC_PRACTICER)) 
	    manips.addLocal("practice", "здесь");

	if (mob_has_occupation(victim, OCC_HEALER)) 
	    manips.addLocal("heal", "");

	if (mob_has_occupation(victim, OCC_SMITHMAN)) 
	    manips.addLocal("smith", "");

	if (mob_has_occupation(victim, OCC_SHOPPER)) 
	    manips.addLocal("list", "");

	if (mob_has_occupation(victim, OCC_QUEST_MASTER)) {
	    manips.addLocal("quest", "попросить");
	    manips.addLocal("quest", "сдать");
	    manips.addLocal("quest", "отменить");
        }

	if (mob_has_occupation(victim, OCC_QUEST_TRADER)) {
	    manips.addLocal("quest", "список");
        }

	if (has_trigger_mount( victim ))
	    manips.add( "mount" );

        manips.add( "consider" );
        if (has_trigger_kill(ch, victim))
	    manips.add("kill");

        manips.add( "smell" );
        manips.add( "follow" );
	if (has_trigger_nuke( ch, victim ))
	    manips.add( "nuke" );
    }

    buf << manips.toString( );
    return true;
}


