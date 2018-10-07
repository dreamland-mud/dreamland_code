/* $Id: social.cpp,v 1.1.2.2.6.11 2009/11/04 03:24:33 rufina Exp $
 * 
 * ruffina, 2004
 */
/* 
 *
 * sturm, 2003
 */

#include "social.h"
#include "socialmanager.h"

#include "logstream.h"
#include "npcharacter.h"
#include "object.h"
#include "behavior_utils.h"
#include "room.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "dreamland.h"
#include "merc.h"
#include "act.h"
#include "interp.h"
#include "mercdb.h"
#include "def.h"

Social::Social( ) : position( POS_RESTING, &position_table )
{
}

Social::~Social( )
{
}

bool Social::matches( const DLString& argument ) const
{
    if (argument.empty( )) 
	return false;
    
    if (SocialBase::matches( argument ))
	return true;
    
    for (XMLStringList::const_iterator a = aliases.begin( ); a != aliases.end( ); a++)
	if (argument.strPrefix( *a ))
	    return true;
    
    return false;
}

static bool mprog_social( Character *ch, Character *actor, Character *victim, const char *social )
{
    FENIA_CALL( ch, "Social", "CCs", actor, victim, social );
    FENIA_NDX_CALL( ch->getNPC( ), "Social", "CCCs", ch, actor, victim, social );
    BEHAVIOR_CALL( ch->getNPC( ), social, actor, victim, social );
    return false;
}

static bool oprog_social( Object *obj, Character *actor, Character *victim, const char *social )
{
    FENIA_CALL( obj, "Social", "CCs", actor, victim, social );
    FENIA_NDX_CALL( obj, "Social", "OCCs", obj, actor, victim, social );
    return false;
}

bool Social::mprog( Character *ch, Character *victim )
{
    bool rc = false;

    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room) {
	if (mprog_social( rch, ch, victim, getName( ).c_str( ) ))
	    rc = true;

	for (Object *obj = rch->carrying; obj; obj = obj->next_content)
	    if (oprog_social( obj, ch, victim, getName( ).c_str( ) ))
		rc = true;
    }

    return rc;
}

static bool rprog_social( Room *room, Character *actor, Character *victim, const char *social, const char *arg)
{
    FENIA_CALL( room, "Social", "CCss", actor, victim, social, arg );
    return false;
}

void Social::reaction( Character *ch, Character *victim, const DLString &arg )
{
    if (rprog_social( ch->in_room, ch, victim, getName( ).c_str( ), arg.c_str( ) ))
	return;

    if (mprog( ch, victim ))
	return;
    
    if (!victim && !arg.empty( )) {
	if (!getErrorMsg( ).empty( ))
	    act_p( getErrorMsg( ).c_str( ), ch, 0, 0, TO_CHAR, getPosition( ) );
	else
	    ch->println("Нет этого здесь.");
    }

    if (!victim || victim == ch)
	return;

    if (ch->is_npc( ) || !victim->is_npc( ) || victim->desc)
	return;
	
    if (IS_AFFECTED(victim, AFF_CHARM) || !IS_AWAKE(victim))
	return;
    
    switch (number_bits( 4 )) {
    case 0:
    case 1: case 2: case 3: case 4:
    case 5: case 6: case 7: case 8:
	act( getArgOther( ).c_str( ), victim, 0, ch, TO_NOTVICT );
	act_p( getArgMe( ).c_str( ), victim, 0, ch, TO_CHAR, getPosition( ) );
	act( getArgVictim( ).c_str( ), victim, 0, ch, TO_VICT );
	break;

    case 9: case 10: case 11: case 12:
	act( "$c1 шлепает $C4.",  victim, 0, ch, TO_NOTVICT );
	act_p( "Ты шлепаешь $C4.",  victim, 0, ch, TO_CHAR, getPosition( ) );
	act( "$c1 шлепает тебя.", victim, 0, ch, TO_VICT );
	break;
    case 13: 
	interpret_fmt( victim, "sigh %s", ch->getNameP( ) );
	break;
    case 14:
	interpret_fmt( victim, "shrug %s", ch->getNameP( ) );
	break;
    case 15: 
	interpret_fmt( victim, "eyebrow %s", ch->getNameP( ) );
	break;
    }
}

