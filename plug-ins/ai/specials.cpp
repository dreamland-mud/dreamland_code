/* $Id$
 *
 * ruffina, 2004
 */
#include "basicmobilebehavior.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "npcharacter.h"
#include "room.h"
#include "roomutils.h"
#include "core/object.h"
#include "skillreference.h"

#include "dreamland.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "fight.h"
#include "skill_utils.h"

#include "stats_apply.h"
#include "loadsave.h"
#include "act.h"
#include "interp.h"
#include "magic.h"
#include "merc.h"

#include "aitrigger.h"
#include "vnum.h"
#include "def.h"
#include "l10n.h"

/*
 * Specials. Called from mobile_update every 4 seconds.
 */
bool BasicMobileBehavior::spec( )
{
    if (ch->fighting) {
        if (specFight( ))
            return true;
        
        if (specFightCaster( ))
            return true;

        return false;
    }

    if (isAdrenalined( ))
        return specAdrenaline( );

    if (IS_AWAKE( ch ))
        return specIdle( );

    return false;
}

bool BasicMobileBehavior::specFight( )
{
    if (doWimpy( ))
        return true;

    if (isAfterCharm( ))
        return true;
    
    if (ch->wait > 0)
        return false;

    if (doCallHelp( ))
        return true;

    if (doPickWeapon( ))
        return true;

    if (doQuaff( ))
        return true;

    if (doScavenge( ))
        return true;

    return false;
}

bool BasicMobileBehavior::specAdrenaline( )
{
    if (!IS_AWAKE( ch ))
        return false;

    if (isAfterCharm( ))
        return false;
    
    if (ch->wait > 0)
        return false;
            
    if (doPickWeapon( ))
        return true;

    if (doQuaff( ))
        return true;
    
    if (doHeal( ))
        return true;

    if (doInvis())
        return true;

    return false;
}

bool BasicMobileBehavior::specIdle( )
{
    if (doInvis())
        return true;

    if (doWander( ))
        return true;

    if (doScavenge( ))
        return true;

    return false;
}

// Reset native invisibility state if not in battle.
bool BasicMobileBehavior::doInvis()
{
    if (!IS_AWAKE(ch))
        return false;

    bitstring_t innate = ch->getNPC()->pIndexData->affected_by & (AFF_SNEAK|AFF_INVISIBLE|AFF_IMP_INVIS);
    // Elite and stronger thieves ask for hide too, in a zone with players:
    // the class kit's stealth list in Fenia decides which of them keep it.
    int tier = ch->getNPC()->getTier();
    if (IS_SET(ch->act, ACT_THIEF) && tier >= 1 && tier <= 5
        && ch->in_room && !ch->in_room->area->empty)
        innate |= AFF_HIDE;
    if (!innate || (ch->affected_by & innate) == innate)
        return false;

    return ai_trigger(true, ch, "onInvisAI", "Ci", ch, (int)(innate & ~ch->affected_by)) > 0;
}

// Cheap gates before a Fenia call: is there anything to use at all.
static bool carries_type( Character *ch, int itemType )
{
    for (Object *obj = ch->carrying; obj; obj = obj->next_content)
        if (obj->item_type == itemType)
            return true;

    return false;
}

static bool carries_unworn( Character *ch )
{
    for (Object *obj = ch->carrying; obj; obj = obj->next_content)
        if (obj->wear_loc == wear_none)
            return true;

    return false;
}

bool BasicMobileBehavior::doQuaff( )
{
    if (ch->position != POS_STANDING
            && ch->position != POS_RESTING
            && ch->position != POS_FIGHTING)
        return false;

    if (ch->getCurrStat(STAT_INT ) <= 15)
        return false;

    if (ch->hit >= (ch->max_hit * 9) / 10
        && !IS_AFFECTED(ch, AFF_BLIND|AFF_POISON|AFF_PLAGUE)
        && !ch->fighting)
        return false;

    if (!carries_type( ch, ITEM_POTION ))
        return false;

    return ai_trigger(true, ch, "onQuaffAI", "C", ch) > 0;
}

/* 
 * Scavenge: a sentient picks up what it can wear (Fenia onScavengeAI)
 */
bool BasicMobileBehavior::doScavenge( )
{
    if (!ch->form.isSet( FORM_SENTIENT ))
        return false;
    
    if (!IS_AWAKE(ch))
        return false;
        
    if (ch->in_room->area->empty)
        return false;

    if (ch->in_room->contents == 0)
        return false;

    if (number_bits( 6 ))
        return false;

    return ai_trigger(true, ch, "onScavengeAI", "C", ch) > 0;
}


/*
 * Rearm and weapon pickup
 */
bool BasicMobileBehavior::doPickWeapon( )
{
    if (number_bits( 1 ))
        return false;

    if (!ch->in_room->contents && !carries_unworn( ch ))
        return false;

    return ai_trigger(true, ch, "onPickWeaponAI", "C", ch) > 0;
}


/* 
 * Wander 
 */
bool BasicMobileBehavior::doWander( )
{
    if (IS_SET(ch->act, ACT_SENTINEL))
        return false;
    
    if (ch->position != POS_STANDING)
        return false;
    
    if (ch->in_room->area->empty)
        return false;

    if (IS_SET(ch->in_room->room_flags, ROOM_MANSION)) {
        if (number_range( 0, 50 ))
            return false;
    } else {
        if (number_range( 0, 215 ))
            return false;
    }
        
    if (RIDDEN(ch))
        return false;

    return ai_trigger(true, ch, "onWanderAI", "C", ch) > 0;
}


/*
 * Heal oneself if damaged and adrenalined
 */
bool BasicMobileBehavior::doHeal( )
{
    if (ch->position != POS_STANDING)
        return false;

    if (number_bits( 4 ))
        return false;

    // Champions and above are asked too: Fenia lets them shake off debuffs.
    int tier = ch->getTier( );
    bool strong = tier > 0 && tier <= 3;
    if (!strong && !IS_SET(ch->act, ACT_RANGER|ACT_CLERIC|NPC_NECRO_ACTS))
        return false;

    return ai_trigger(true, ch, "onHealAI", "C", ch) > 0;
}

/*
 * Fenia mob AI: the class heal (ranger herbs, cleric and necromancer spells)
 */
bool BasicMobileBehavior::aiHeal( )
{
    if (!ch->in_room)
        return false;

    if (IS_SET(ch->act, ACT_RANGER))
        if (healRanger( ch ))
            return true;

    if (IS_SET(ch->act, ACT_CLERIC))
        if (healCleric( ch ))
            return true;

    if (IS_SET(ch->act, NPC_NECRO_ACTS))
        if (healNecro( ch ))
            return true;

    return false;
}
