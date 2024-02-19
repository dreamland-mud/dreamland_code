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
#include "handler.h"
#include "magic.h"
#include "act.h"
#include "act_move.h"
#include "merc.h"


#include "roomtraverse.h"

#include "def.h"

GSN(garble);


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
    Character *target;
    
    if (number_bits( 1 ) == 0)
        return false;

    if (!canAssistOffense( fch, victim ))
        return false;
    
    if (!( target = findAssistVictim( victim ) ))
        return false;

    oldact("$c1 вскрикивает и атакует!", ch, 0, 0, TO_ROOM);
    memoryAttacked.remember( target );
    attack( target );
    return true;
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
    if ( (ch->spec_fun.name == "spec_guard" || ch->spec_fun.name == "spec_patrolman" || IS_SET(ch->off_flags, ASSIST_GUARD)) && 
         (mob->spec_fun.name == "spec_guard" || mob->spec_fun.name == "spec_patrolman" || IS_SET(mob->off_flags, ASSIST_GUARD)) )
        return true;

    if (IS_SET(ch->off_flags, ASSIST_ALIGN) && ALIGNMENT(ch) == ALIGNMENT(mob))
        return true;

    if (IS_SET(ch->off_flags, ASSIST_VNUM) && ch->pIndexData == mob->pIndexData)
        return true;

    return false;
}

/*
 * charmices assist master, mounts assist riders 
 */
bool BasicMobileBehavior::assistMaster( Character *fch, Character *victim )
{
    if (IS_CHARMED(ch) && is_same_group( fch, ch )) {
        attack( victim );
        return true;
    }
/* 
    if (RIDDEN(ch) == fch) {
        attack( victim );
        return true;
    }
*/
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

    if (assistGroupHealing( fch )) 
        return true;
    
    if (!ch->can_see( victim ))
        return false;

    if (assistGroupDistance( fch, victim )) 
        return true;

    oldact("$c1 вступи$gло|л|ла в битву на стороне $C2.", ch, 0, fch, TO_NOTVICT);
    memoryAttacked.remember( victim );
    attack( victim );
    return true;
}

bool BasicMobileBehavior::assistGroupHealing( Character *fch )
{
    if (IS_SET(ch->act, ACT_CLERIC))
        if (healCleric( fch ))
            return true;

    if (IS_SET(ch->act, ACT_NECROMANCER|ACT_UNDEAD))
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
        oldact("$c1 пристально смотрит на $C4.", ch, 0, victim, TO_NOTVICT);
        oldact("$c1 пристально смотрит на тебя.", ch, 0, victim, TO_VICT);
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

Character * BasicMobileBehavior::findAssistVictim( Character *victim )
{
    Character *target = 0;
    int number = 0;

    for (Character *vch = victim->in_room->people; vch; vch = vch->next_in_room)
        if (ch->can_see( vch )
            && is_same_group( vch, victim )
            && !vch->is_npc( )
            && number_range( 0, number ) == 0)
        {
            target = vch;
            number++;
        }
    
    return target;
}

/*
 * cry out for help
 */
struct BasicMobileBehavior::FindAssistersComplete {
    typedef NodesEntry<RoomTraverseTraits> MyNodesEntry;
    FindAssistersComplete( int d, int g, vector<Character *> &f, NPCharacter *c ) 
            : depth( d ), group( g ), fighters( f ), cryer( c )
    { 
    }
    inline bool operator () ( const MyNodesEntry *const head, bool last ) 
    {
        for (Character *rch = head->node->people; rch; rch = rch->next_in_room) {
            BasicMobileBehavior::Pointer bhv;
            Character *victim;
            NPCharacter *ach;
            
            if (!( ach = rch->getNPC( ) ))
                continue;

            if (ach->getNPC( )->pIndexData->group == 0)
                continue;
        
            if (!ach->behavior || !( bhv = ach->behavior.getDynamicPointer<BasicMobileBehavior>( ) ))
                continue;
            
            if (!bhv->canTrack( ) || bhv->hasDestiny( ) || bhv->hasLastFought( ))
                continue;

            victim = fighters[number_range( 0, fighters.size( ) - 1 )];
            
            if (!mprog_checkassist( ach, cryer, victim ))
                continue;

            bhv->setLastFought( victim );
            bhv->memoryAttacked.remember( victim );
            
            for (const MyNodesEntry *i = head; i->prev; i = i->prev) 
                i->node->history.record( victim,
                                         dirs[i->hook.value.door].rev );
        }

        if (head->generation < depth && !last)
            return false;

        return true;
    }
    
    int depth, group;
    vector<Character *> &fighters;
    NPCharacter *cryer;
};

struct DoorFunc {
    DoorFunc( ) { 
    }
    bool operator () ( Room *const room, EXIT_DATA *exit ) const {
        EXIT_DATA *pExitRev;
        Room *toRoom;

        if (!( toRoom = exit->u1.to_room ))
            return false;

        if (!( pExitRev = toRoom->exit[dirs[exit->orig_door].rev] ))
            return false;

        if (pExitRev->u1.to_room != room)
            return false;

        if (IS_SET( pExitRev->exit_info, EX_LOCKED ))
            return false;
        
        return toRoom->isCommon( );
    }
};

struct ExtraExitFunc {
    ExtraExitFunc( ) { }
    bool operator () ( Room *const, EXTRA_EXIT_DATA * ) const { return false; }
};

struct PortalFunc {
    PortalFunc( ) { }
    bool operator () ( Room *const, Object * ) const { return false; }
};

typedef RoomRoadsIterator<DoorFunc, ExtraExitFunc, PortalFunc> MyHookIterator;

bool BasicMobileBehavior::doCallHelp( )
{
    vector<Character *> pcFighters;
    Character *fch;
    int helpers, fighters;

    if (ch->pIndexData->group == 0)
        return false;
    
    if (hasDestiny( ))
        return false;

    if (ch->hit > ch->max_hit / 2)
        return false;
    
    if (number_bits( 3 ))
        return false;
    
    for (helpers = 0, fighters = 0, fch = ch->in_room->people; fch; fch = fch->next_in_room) {
        if (fch == ch)
            continue;

        if (!fch->fighting)
            continue;
        
        if (fch->fighting == ch || is_same_group( ch->fighting, fch )) {
            if (!fch->is_npc( )) 
                pcFighters.push_back( fch );

            fighters++;
            continue;
        }
        
        if (is_same_group( fch->fighting, ch->fighting )) 
            helpers++;
    }
        
    if (pcFighters.size( ) < 2) 
        return false;

    if (fighters < helpers) 
        return false;
   
    yell_panic(ch->fighting, ch, "На помощь!", "На помощь!", "callHelp");
    
    if (ch->isAffected( gsn_garble ))
        return true;

    DoorFunc df; ExtraExitFunc eef; PortalFunc pf;
    MyHookIterator iter( df, eef, pf );
    
    FindAssistersComplete complete( pcFighters.size( ) - 1, 
                                      ch->pIndexData->group,
                                      pcFighters, 
                                      ch );

    room_traverse<MyHookIterator, FindAssistersComplete>( 
            ch->in_room, iter, complete, 10000 );
    
    return true;
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
