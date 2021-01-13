/* $Id$
 *
 * ruffina, 2004
 */
#include "basicmobilebehavior.h"
#include "profflags.h"

#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"

#include "dreamland.h"
#include "loadsave.h"
#include "fight.h"
#include "gsn_plugin.h"
#include "interp.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

PROF(vampire);

/*
 * Aggression. Called every pulse from aggr_update.
 */
bool BasicMobileBehavior::aggress( )
{
    if (!canAggress( ))
        return false;
    
    if (IS_SET( ch->act, ACT_VAMPIRE ))
        return aggressVampire( );

    if (aggressLastFought( ))
        return true;

    if (aggressMemorized( ))
        return true;
    
    if (aggressRanged( ))
        return true;

    return aggressNormal( );
}

bool BasicMobileBehavior::canAggress( )
{
    if (!IS_AWAKE( ch ))
        return false;
        
    if (ch->fighting != 0)
        return false;
        
    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE | ROOM_NO_DAMAGE))
        return false;
        
    if (RIDDEN(ch))
        return false;

    if (IS_CHARMED(ch))
        return false;

    if (IS_AFFECTED(ch, AFF_CALM))
        return false;

    if (ch->is_mirror( ))
        return false;

    return true;
}

bool BasicMobileBehavior::canAggress( Character *victim )
{
    if (victim->is_npc( ))
        return false;

    if (dreamland->hasOption( DL_BUILDPLOT ))
        return false;

    if (victim->is_immortal() && !victim->getPC()->getAttributes().isAvailable("ai_aggress"))
        return false;

    if (is_safe_nomessage( ch, victim ))
        return false;
        
    if (!ch->can_see( victim ))
        return false;

    if (IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(victim))
        return false;

    return true;
}

/*
 * normal aggression, from ACT_AGGRESSIVE bit
 */
bool BasicMobileBehavior::canAggressNormal( Character *victim )
{
    int recall;
    AreaIndexData *area;
    
    if (!canAggress( victim ))
        return false;

    if (ch->getModifyLevel() < victim->getModifyLevel() - 5)
        return false;

    if (victim->getProfession( ) == prof_vampire) /* do not attack vampires */
        return false;

    if (IS_GOOD(ch) && IS_GOOD(victim)) /* good vs good :( */
        return false;
    
    /* do not attack co-citizens */
    area = ch->getNPC()->pIndexData->area;
    recall = victim->getPC( )->getHometown( )->getRecall( );

    if (area->min_vnum <= recall && recall <= area->max_vnum)
        return false;

    return true;
}

bool BasicMobileBehavior::aggressNormal( )
{
    Character *rch;
    Character *victim = NULL;
    int count = 0;
    
    if (!IS_SET( ch->act, ACT_AGGRESSIVE ))
        return false;

    if (hasLastFought( ) && homeVnum != 0)
        return false;

    for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
        if (!canAggressNormal( rch ))
            continue;
            
        if (number_range( 0, count++ ) == 0) 
            victim = rch;
    }

    if (victim) {
        attack( victim );
        return true;
    }

    return false;
}

/*
 * Aggress lastFought remembered victim
 */
bool BasicMobileBehavior::canAggressLastFought( Character *victim )
{
    if (!canAggress( victim ))
        return false;
        
    if (IS_AFFECTED(ch, AFF_SCREAM)) 
        return false;

    return true;
}

bool BasicMobileBehavior::aggressLastFought( )
{
    Character *victim = getLastFoughtRoom( );

    if (!victim)
        return false;

    if (chance( 50 ))
        return false;
    
    if (!canAggressLastFought( victim ))
        return false;

    interpret_raw( ch, "yell", 
                   fmt( ch, "%^C1! Теперь ты%s умрешь!", victim, 
                            (chance(50) ? " может быть" : "" ) ).c_str( ) );
    
    attack( victim );
    return true;
}

/*
 * Aggress random memorized victim in same room
 */
bool BasicMobileBehavior::aggressMemorized( )
{
    Character *rch;
    Character *victim = NULL;
    int count = 0;
    
    if (memoryFought.empty( ))
        return false;

    if (number_range( 1, 6 * dreamland->getPulsePerSecond( ) ) != 1)
        return false;

    for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
        if (!memoryFought.memorized( rch ))
            continue;
    
        if (!canAggress( rch ))
            continue;

        if (number_range( 0, count++ ) == 0) 
            victim = rch;
    }

    if (victim) {
        interpret_raw( ch, "yell", "Вот мы и встретились! %s!",
                           victim->getNameP( '1' ).c_str( ) );
        
        attack( victim );
        return true;
    }

    return false;
}        

/*
 * Aggress a memorized victim visible by scan
 */
bool BasicMobileBehavior::aggressRanged( )
{
    if (memoryFought.empty( ))
        return false;
        
    if (ch->wait > 0) 
        return false;

    if (isAfterCharm( ))
        return false;

    if (IS_SET( ch->act, ACT_RANGER ))
        if (aggressRanger( ))
            return true;

    if (ch->getProfession( )->getFlags( ch ).isSet(PROF_CASTER))
        if (aggressCaster( ))
            return true;
    
    return false;
}

Character * BasicMobileBehavior::findRangeVictim( int maxRange, int &victDoor, int &victRange )
{
    Character *rch;
    Character *victim = 0;

    for (int door = 0; door < DIR_SOMEWHERE; door++) {
        Room *room = ch->in_room;
        EXIT_DATA *pExit;

        for (int r = 0; r < maxRange; r++) {
            if (!( pExit = room->exit[door] ))
                break;
            
            if (!ch->can_see( pExit ))
                break;

            if (IS_SET(pExit->exit_info, EX_CLOSED))
                break;
            
            room = pExit->u1.to_room;
            for (rch = room->people; rch; rch = rch->next_in_room) 
                if (memoryFought.memorized( rch ) && canAggress( rch )) 
                    if (!victim || victim->hit > rch->hit) {
                        victim = rch;
                        victDoor = door;
                        victRange = r + 1;
                    }
        }
    }
    
    if (victim) 
        for (rch = victim->in_room->people; rch; rch = rch->next_in_room) 
            if (rch->is_mirror( ) && rch->doppel == victim)
                if (chance( 10 ))
                    return rch;

    return victim;
}
