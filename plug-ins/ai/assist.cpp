/* $Id$
 *
 * ruffina, 2004
 */
#include "basicmobilebehavior.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "skillreference.h"
#include "skill.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"
#include "race.h"

#include "dreamland.h"
#include "damage.h"
#include "fight.h"

#include "interp.h"
#include "loadsave.h"
#include "magic.h"
#include "act.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "aitrigger.h"
#include "merc.h"

#include "def.h"
#include "behavior.h"
#include "l10n.h"

BHV(cityguard);


/*
 * Auto-assist a fighting char against certain victim
 */
bool BasicMobileBehavior::assist( Character *fch, Character *victim )
{
    if (assistMaster( fch, victim ))
        return true;
    
    if (assistOffense( fch, victim ))
        return true;

    if (assistGroup( fch, victim ))
        return true;

    return false;
}


/* 
 * auto-assist by bit offenses 
 */
bool BasicMobileBehavior::assistOffense( Character *fch, Character *victim )
{
    if (number_bits( 1 ) == 0)
        return false;

    if (!canAssistOffense( fch, victim ))
        return false;

    return ai_trigger(true, ch, "onAssistAI", "CCCs", ch, fch, victim, "offense") > 0;
}

bool BasicMobileBehavior::canAssistOffense( Character *fch, Character *victim )
{
    NPCharacter *mob = fch->getNPC( );
    
    if (isAfterCharm( ))
        return false;

    if (!fch->is_npc( )) {
        if (IS_SET(ch->off_flags, ASSIST_PLAYERS)
            && ch->getModifyLevel( ) + 6 > victim->getModifyLevel( ))
            return true;
        else
            return false;
    }
    
    if (IS_SET(ch->off_flags, ASSIST_ALL))
        return true;
    
    if (IS_SET(ch->off_flags, ASSIST_RACE) && ch->getRace( ) == mob->getRace( ))
        return true;
    
    // Guards, patrolmen and guard assistants always assist each other
    if ( (ch->pIndexData->behaviors.isSet(bhv_cityguard) || IS_SET(ch->off_flags, ASSIST_GUARD)) && 
         (mob->pIndexData->behaviors.isSet(bhv_cityguard) || IS_SET(mob->off_flags, ASSIST_GUARD)) )
        return true;

    if (IS_SET(ch->off_flags, ASSIST_ALIGN) && ALIGNMENT(ch) == ALIGNMENT(mob))
        return true;

    // Every mob sides with its own kind (Kit 2026-10-15), assist_vnum or not.
    // Dummy-tier (10) onboarding mobs only with the flag: a new character must
    // be able to kill them one by one.
    if (ch->pIndexData == mob->pIndexData
        && (IS_SET(ch->off_flags, ASSIST_VNUM) || ch->getTier() != 10))
        return true;

    return false;
}

/*
 * charmices assist master, mounts assist riders 
 */
bool BasicMobileBehavior::assistMaster( Character *fch, Character *victim )
{
    if (IS_CHARMED(ch) && is_same_group( fch, ch ))
        return ai_trigger(true, ch, "onAssistAI", "CCCs", ch, fch, victim, "master") > 0;

    return false;
}

/* 
 * assist mobiles from the same pIndexData->group 
 */
static bool mprog_checkassist( NPCharacter *ch, NPCharacter *fch, Character *victim )
{
    FENIA_CALL( ch, "CheckAssist", "CC", fch, victim );
    FENIA_NDX_CALL( ch->getNPC( ), "CheckAssist", "CCC", ch, fch, victim );
    return false;
}

bool BasicMobileBehavior::canAssistGroup( Character *fch, Character *victim )
{
    if (ch->pIndexData->group == 0)
        return false;

    if (ch->wait > 0)
        return false;

    if (!IS_AWAKE( ch ))
        return false;

    if (ch->fighting)
        return false;

    if (!fch->is_npc( ))
        return false;

    if (fch->getNPC( )->behavior && fch->getNPC( )->behavior->hasDestiny( ))
        return false;
    
    if (!ch->can_see( fch ))
        return false;

    victim = getMaster( victim );

    if (victim->is_npc( ))
        return false;
     
    if (!mprog_checkassist( ch, fch->getNPC( ), victim ))
        return false;

    return true;
}

bool BasicMobileBehavior::assistGroup( Character *fch, Character *victim )
{
    if (!canAssistGroup( fch, victim ))
        return false;

    if (number_percent( ) < HEALTH(fch))
        return false;

    return ai_trigger(true, ch, "onAssistAI", "CCCs", ch, fch, victim, "group") > 0;
}

/*
 * Help fch from the same group against victim: heal it, step back to shoot
 * or cast from afar, or join the fight. Called by the Fenia mob AI (ai_assistGroup).
 */
bool BasicMobileBehavior::aiAssistGroup( Character *fch, Character *victim )
{
    if (!ch->in_room || fch->in_room != ch->in_room || victim->in_room != ch->in_room)
        return false;

    if (assistGroupHealing( fch )) 
        return true;
    
    if (!ch->can_see( victim ) || is_safe_nomessage( ch, victim ))
        return false;

    if (assistGroupDistance( fch, victim )) 
        return true;

    oldact(_("$c1 вступи$gло|л|ла в битву на стороне $C2."), ch, 0, fch, TO_NOTVICT);
    memoryAttacked.remember( victim );
    attack( victim );
    return true;
}

bool BasicMobileBehavior::assistGroupHealing( Character *fch )
{
    if (IS_SET(ch->act, ACT_CLERIC))
        if (healCleric( fch ))
            return true;

    if (IS_SET(ch->act, NPC_NECRO_ACTS))
        if (healNecro( fch ))
            return true;

    if (IS_SET(ch->act, ACT_RANGER))
        if (healRanger( fch ))
            return true;

    return false;
}

bool BasicMobileBehavior::assistGroupDistance( Character *fch, Character *victim )
{
    int door;

    if (!canAggressDistanceRanger( ) && !canAggressDistanceCaster( victim )) 
        return false;

    if (( door = findRetreatDoor( ) ) < 0) 
        return false;

    if (!memoryFought.memorized( victim )) {
        oldact(_("$c1 пристально смотрит на $C4."), ch, 0, victim, TO_NOTVICT);
        oldact(_("$c1 пристально смотрит на тебя."), ch, 0, victim, TO_VICT);
        memoryFought.remember( victim );
    }
    
    memoryAttacked.remember( victim );

    if (IS_SET(ch->in_room->exit[door]->exit_info, EX_CLOSED)) 
        open_door_extra( ch, door, ch->in_room->exit[door] );

    return move_char( ch, door );
}

int BasicMobileBehavior::findRetreatDoor( )
{
    EXIT_DATA *pExit, *pExitRev;
    Room *room;
    int count = 0, door = -1;

    for (int d = 0; d < DIR_SOMEWHERE; d++) {
        pExit = ch->in_room->exit[d];
        
        if (!pExit || !ch->can_see( pExit ))
            continue;

        if (IS_SET(pExit->exit_info, EX_LOCKED))
            continue;
        
        room = pExit->u1.to_room;
        if (!( pExitRev = room->exit[dirs[d].rev] ))
            continue;

        if (pExitRev->u1.to_room != ch->in_room)
            continue;
        
        if (number_range( 0, count++ ) == 0) 
            door = d;
    }

    return door;
}

/*
 * cry out for help
 */
bool BasicMobileBehavior::doCallHelp( )
{
    if (ch->pIndexData->group == 0)
        return false;
    
    if (hasDestiny( ))
        return false;

    if (ch->hit > ch->max_hit / 2)
        return false;
    
    if (number_bits( 3 ))
        return false;

    return ai_trigger(true, ch, "onCallHelpAI", "C", ch) > 0;
}

/*
 * cast a spell on char, or use magic items to do so
 */
bool BasicMobileBehavior::assistSpell( NPCharacter *whoNeeds, SkillReference &sn, Character *whosHunted )
{
    if (whoNeeds->isAffected( sn )) 
        return true;

    if (chance( 90 ) && sn->usable( ch )) {
        ::spell( sn, ch->getModifyLevel( ), ch, whoNeeds, FSPELL_VERBOSE );
        return true;
    }
    
    if (chance( 90 ) && useItemWithSpell( sn, whoNeeds ))
        return true;

    if (whoNeeds != ch)
        return false;

    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room) {
        BasicMobileBehavior::Pointer assister;

        if (rch != ch && rch != whoNeeds
            && rch->is_npc( ) && rch->getNPC( )->behavior
            && (assister = rch->getNPC( )->behavior.getDynamicPointer<BasicMobileBehavior>( ) ))
        {
            if (assister->canAssistGroup( whoNeeds, whosHunted )) 
                if (assister->assistSpell( whoNeeds, sn, whosHunted )) 
                    return true;
        }
    }

    return false;
}
