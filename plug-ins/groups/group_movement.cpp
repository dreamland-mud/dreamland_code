
/* $Id: group_movement.cpp,v 1.1.2.11 2009/03/16 20:24:06 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/

#include "spelltemplate.h"
#include "skillcommandtemplate.h"
#include "recallmovement.h"
#include "fleemovement.h"

#include "selfrate.h"

#include "so.h"

#include "pcharacter.h"
#include "mobilebehavior.h"
#include "room.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "playerattributes.h"
#include "object.h"
#include "affect.h"

#include "magic.h"
#include "fight.h"
#include "damage.h"
#include "act_move.h"
#include "interp.h"
#include "clanreference.h"
#include "gsn_plugin.h"

#include "stats_apply.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "effects.h"
#include "act.h"
#include "vnum.h"
#include "def.h"

CLAN(battlerager);




SPELL_DECL(Knock);
VOID_SPELL(Knock)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
	char arg[MAX_INPUT_LENGTH];
	int chance=0;
	int door;
        Room *room = ch->in_room;
	EXTRA_EXIT_DATA *peexit = 0;

	target_name = one_argument( target_name, arg );

	if (arg[0] == '\0')
	{
		ch->send_to("Постучать в какую дверь или в каком направлении.\n\r");
		return;
	}

	if (ch->fighting)
	{	
		ch->send_to("Подожди пока закончится сражение.\n\r");
		return;
	}
	
        if ( ((peexit = get_extra_exit( arg, room->extra_exit )) && ch->can_see(peexit)) ||
	     (door = find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY)) >= 0) 
	{
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev = 0;
                int exit_info;

                if ( peexit != 0 )
                {
                        door = DIR_SOMEWHERE;
                        exit_info = peexit->exit_info;
                }
                else
                {
                        pexit = room->exit[door];
                        exit_info = pexit->exit_info;
                }

		if ( !IS_SET(exit_info, EX_CLOSED) )
		{
			ch->send_to("Здесь уже открыто.\n\r");
			return;
		}
		if ( !IS_SET(exit_info, EX_LOCKED) )
		{
			ch->send_to("Попробуй просто открыть...\n\r");
			return;
		}
		if ( IS_SET(exit_info, EX_NOPASS) )
		{
			ch->send_to("Таинственная сила блокирует проход.\n\r");
			return;
		}
		chance = ch->getModifyLevel() / 5 + ch->getCurrStat(STAT_INT) + ch->getSkill( sn ) / 5;

                const char *doorname = peexit ? peexit->short_desc_from : direction_doorname(pexit);
		act("Ударом Магической Силы ты пытаешься открыть $N4!", ch, 0, doorname, TO_CHAR);
		act("Ударом Магической Силы $c1 пытается открыть $N4!", ch, 0, doorname,TO_ROOM);

		if (room->isDark())
			chance /= 2;

		// now the attack
		if (number_percent() < chance )
		{
                    if ( peexit != 0 )
                    {
                        REMOVE_BIT(peexit->exit_info, EX_LOCKED);
                        REMOVE_BIT(peexit->exit_info, EX_CLOSED);
			act( "$N1 с грохотом распахивается.", ch, 0, doorname, TO_ALL);

                    } else {
			REMOVE_BIT(pexit->exit_info, EX_LOCKED);
			REMOVE_BIT(pexit->exit_info, EX_CLOSED);
			act( "$N1 с грохотом распахивается.", ch, 0, doorname, TO_ALL);

			// open the other side
                        if ((pexit_rev = direction_reverse(room, door)))
			{
				REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
				REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
                                direction_target(room, door)->echo(POS_RESTING, "Внезапно с грохотом распахивается %N1.", doorname);
			}
                    }
		}
		else
		{
			act("Твой удар сотрясает все вокруг, но $N1 не поддается.", ch,0, doorname,TO_CHAR);
			act("Удар $c2 сотрясает все вокруг, но $N1 не поддается.", ch,0, doorname,TO_ROOM);
		}
		return;
	}

	ch->send_to("Тут нет такой двери.\n\r");
}


/*
 * 'sneak' skill command
 */

SKILL_RUNP( sneak )
{
    Affect af;

    if (MOUNTED(ch))
    {
        ch->send_to("Ты не можешь двигаться бесшумно, когда ты в седле.\n\r");
        return;
    }

//    ch->send_to("Ты пытаешься двигаться более бесшумно.\n\r");
    affect_strip( ch, gsn_sneak );

    if( IS_AFFECTED(ch,AFF_SNEAK)) {
      ch->send_to("Ты и так двигаешься бесшумно.\n\r");
      return;
    }

    if ( number_percent( ) < gsn_sneak->getEffective( ch ))
    {
	gsn_sneak->improve( ch, true );
	af.where     = TO_AFFECTS;
	af.type      = gsn_sneak;
	af.level     = ch->getModifyLevel();
	af.duration  = ch->getModifyLevel();
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SNEAK;
	affect_to_char( ch, &af );
	ch->send_to("Ты начинаешь скрытно передвигаться.\n\r");
    } else {
      gsn_sneak->improve( ch, false );
      ch->send_to("Тебе не удается скрытно передвигаться.\n\r");
    }

    ch->setWait( gsn_sneak->getBeats( ) );
}

/*
 * 'hide' skill command
 */

SKILL_RUNP( hide )
{
	if ( MOUNTED(ch) )
	{
		ch->send_to("Ты не можешь скрыться, когда ты в седле.\n\r");
		return;
	}

	if ( RIDDEN(ch) )
	{
		ch->send_to("Ты не можешь скрыться, когда ты оседлан.\n\r");
		return;
	}

	if ( IS_AFFECTED( ch, AFF_FAERIE_FIRE ) )
	{
		ch->send_to("Ты не можешь скрыться, когда светишься.\n\r");
		return;
	}

	int forest = ch->in_room->sector_type == SECT_FOREST ? 60 : 0;
	forest += ch->in_room->sector_type == SECT_FIELD ? 60 : 0;

	ch->send_to("Ты пытаешься скрыться.\n\r");

	int k = ch->getLastFightDelay( );

	if ( k >= 0 && k < FIGHT_DELAY_TIME )
		k = k * 100 /	FIGHT_DELAY_TIME;
	else
		k = 100;
		
	if ( number_percent( ) < (gsn_hide->getEffective( ch ) - forest) * k / 100 )
	{
		SET_BIT(ch->affected_by, AFF_HIDE);
		gsn_hide->improve( ch, true );
	}
	else
	{
		if ( IS_AFFECTED(ch, AFF_HIDE) )
			REMOVE_BIT(ch->affected_by, AFF_HIDE);
		gsn_hide->improve( ch, false );
	}

	ch->setWait( gsn_hide->getBeats( ) );
}


/*
 * TempleRecallMovement 
 */
class TempleRecallMovement : public RecallMovement {
public:
    TempleRecallMovement( Character *ch )
                   : RecallMovement( ch )
    {
    }
    TempleRecallMovement( Character *ch, Character *actor, Room *to_room )
                   : RecallMovement( ch, actor, to_room )
    {
    }

protected:
    virtual void msgOnMove( Character *wch, bool fLeaving )
    {
	if (fLeaving)
	    msgRoomNoParty( wch, 
		            "%1$^C1 растворил%1$Gось|ся|ась в воздухе.",
		            "%1$^C1 и %2$C1 растворяются в воздухе." );
	else
	    msgRoomNoParty( wch, 
	                    "%1$^C1 появил%1$Gось|ся|ась в комнате.",
			    "%1$^C1 и %2$C1 появляются в комнате." );
    }
    virtual void msgOnStart( )
    {
	msgRoomNoParty( ch, 
	                "%1$^C1 просит о возвращении!",
			"%1$^C1 и %2$C1 просят о возвращении!" );
    }
    virtual void movePet( NPCharacter *pet )
    {
	TempleRecallMovement( pet, actor, to_room ).moveRecursive( );
    }
    virtual bool findTargetRoom( )
    {
	int point;
	
	if (to_room)
	    return true;

	if (!ch->getPC( )
	    && (!ch->leader || ch->leader->is_npc( ) || ch->leader->getPC( )->pet != ch))
	{
	    ch->pecho( "Тебе некуда возвращаться." );
	    return false;
	}

	if (ch->getPC( ))
	    point = ch->getPC( )->getHometown( )->getRecall( );
	else
	    point = ch->leader->getPC( )->getHometown( )->getRecall( );

	if (!( to_room = get_room_index( point ) )) {
	    ch->pecho( "Ты окончательно заблудил%1$Gось|ся|ась.", ch );
	    return false;
	}
	
	return true;			     
    }
    bool checkSelfrate( )
    {
	if (ch->is_npc( ))
	    return true;

	if (!ch->desc)
	    return true;

	if (rated_as_expert( ch->getPC( ) ))
	    return checkPumped( );
	
	if (rated_as_guru( ch->getPC( ) )) {
	    ch->pecho( "Ты ведь не ищешь легких путей, не так ли?" );
	    return false;
	}

	return true;
    }
    virtual bool canMove( Character *wch )
    {
	if (ch != actor)
	    return checkForsaken( wch );
	else
	    return checkMount( )
		   && checkShadow( )
		   && checkBloody( wch )
		   && checkSelfrate( )
		   && checkSameRoom( )
		   && checkForsaken( wch );
    }
    virtual bool tryMove( Character *wch )
    {
	if (ch != actor)
	    return applyInvis( wch );
	else
	    return applyInvis( wch )
		   && applyFightingSkill( wch, gsn_recall )
		   && applyMovepoints( );
    }
};

/*
 * 'recall' skill command
 */

SKILL_RUNP( recall )
{
    TempleRecallMovement( ch ).move( );
}

/*
 * EscapeMovement
 */
class EscapeMovement : public FleeMovement {
public:
    EscapeMovement( Character *ch, const char *arg )
               : FleeMovement( ch )
    {
	this->arg = arg;
    }

protected:
    virtual bool findTargetRoom( )
    {
	peexit = get_extra_exit( arg, from_room->extra_exit );
	door = find_exit( ch, arg, FEX_NO_EMPTY|FEX_NO_INVIS );

	if ((!peexit || !ch->can_see( peexit )) && door < 0) {
	    ch->pecho( "И куда это мы намылились?" );
	    return false;
	}

	if (peexit) {
	    door = DIR_SOMEWHERE;
	    exit_info = peexit->exit_info;
	    to_room = peexit->u1.to_room;
	}
	else {
	    pexit = from_room->exit[door];
	    exit_info = pexit->exit_info;
	    to_room = pexit->u1.to_room;
	}

	return true;
    }
    virtual bool canMove( Character *wch )
    {
	if (!checkMovepoints( wch ))
	    return false;

	if (!canFlee( wch )) {
	    ch->pecho( "Что-то не дает тебе сбежать в этом направлении." );
	    return false;
	}
	else
	    return true;
    }
    virtual bool tryMove( Character *wch )
    {
	if (!FleeMovement::tryMove( wch ))
	    return false;

	if (wch != ch)
	    return true;
	
	return applySkill( gsn_escape );
    }
    virtual int getMoveCost( Character *wch )
    {
	return 1;
    }
    virtual bool checkCyclicRooms( Character *wch ) 
    {
	if (from_room == to_room) {
	    ch->pecho( "Ты не можешь сбежать туда, попробуй другое место." );
	    return false;
	}

	return true;
    }
    virtual bool checkPositionHorse( )
    {
	ch->pecho( "Сначала слезь, а потом уже убегай." );
	return false;
    }
    virtual bool checkPositionRider( )
    {
	ch->pecho( "На тебе сверху кто-то сидит и не дает сбежать." );
	return false;
    }
    virtual bool checkPositionWalkman( )
    {
	if (ch->fighting == 0) {
	    if (ch->position == POS_FIGHTING)
		ch->position = POS_STANDING;

	    ch->pecho( "Ты сейчас ни с кем не дерешься." );
	    return false;
	}

	return true;
    }

    const char *arg;
};

/*
 * 'escape' skill command
 */

SKILL_RUNP( escape )
{
    char arg[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );

    if (!gsn_escape->usable( ch )) {
	ch->println( "Попробуй flee. Может, это тебя спасет?" );
	return;
    }

    if (arg[0] == '\0') {
	ch->println( "Укажи направление." );
	return;
    }

    EscapeMovement( ch, arg ).move( );
}

