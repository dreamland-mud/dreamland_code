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

#include "vnum.h"
#include "def.h"

GSN(heal);
GSN(armor);
GSN(improved_invis);
GSN(invisibility);
GSN(shield);
GSN(stone_skin);
GSN(sanctuary);
GSN(protection_evil);
GSN(protection_good);
GSN(cure_blindness);
GSN(cure_poison);
GSN(cure_disease);
GSN(bow);
GSN(backstab);

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
    bool rc = false;

    if (!IS_AWAKE(ch))
        return false;

    if (!IS_AFFECTED(ch, AFF_SNEAK)) {
        if (IS_SET(ch->getNPC()->pIndexData->affected_by, AFF_SNEAK) && !MOUNTED(ch)) {
            ch->pecho("Ты пытаешься двигаться незаметно.");
            SET_BIT(ch->affected_by, AFF_SNEAK);
            rc = true;
        }
    }

    if (!IS_AFFECTED(ch, AFF_INVISIBLE)) {
        if (IS_SET(ch->getNPC()->pIndexData->affected_by, AFF_INVISIBLE) && !IS_AFFECTED(ch, AFF_FAERIE_FIRE)) {
            ::spell(gsn_invisibility, ch->getModifyLevel(), ch, ch, FSPELL_VERBOSE | FSPELL_BANE);
            rc = true;
        }
    }

    if (!IS_AFFECTED(ch, AFF_IMP_INVIS)) {
        if (IS_SET(ch->getNPC()->pIndexData->affected_by, AFF_IMP_INVIS) && !IS_AFFECTED(ch, AFF_FAERIE_FIRE)) {
            ::spell(gsn_improved_invis, ch->getModifyLevel(), ch, ch, FSPELL_VERBOSE | FSPELL_BANE);
            rc = true;
        }
    }

    return rc;
}

/*
 *  Potion using and stuff for intelligent mobs
 */
static int potion_cure_level( Object *potion )
{
    int cl;
    int i;
    cl = 0;
    for (i=1;i<5;i++)
    {
        if ( gsn_heal == potion->valueByIndex(i))
            cl += 2;
    }
    return(cl);
}
static int potion_arm_level( Object *potion )
{
    int al;
    int i;
    al = 0;
    for (i=1;i<5;i++)
    {
        if ( gsn_armor == potion->valueByIndex(i) )
            al += 1;
        if ( gsn_shield == potion->valueByIndex(i))
            al += 1;
        if ( gsn_stone_skin == potion->valueByIndex(i) )
            al += 2;
        if ( gsn_sanctuary == potion->valueByIndex(i))
            al += 4;
        if ( gsn_protection_evil == potion->valueByIndex(i) || gsn_protection_good == potion->valueByIndex(i))
            al += 3;
    }
    return(al);
}

static bool potion_cure_blind( Object *potion )
{
    int i;
    for (i=0;i<5;i++)
    {
        if (gsn_cure_blindness == potion->valueByIndex(i))
            return(true);
    }
    return(false);
}
static bool potion_cure_poison( Object *potion )
{
    int i;
    for (i=0;i<5;i++)
    {
        if (gsn_cure_poison == potion->valueByIndex(i) )
            return(true);
    }
    return(false);
}
static bool potion_cure_disease( Object *potion )
{
    int i;
    for (i=0;i<5;i++)
    {
        if (gsn_cure_disease == potion->valueByIndex(i) )
            return(true);
    }
    return(false);
}

bool BasicMobileBehavior::doQuaff( )
{
    Object *obj;
    
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

    for ( obj=ch->carrying;obj!=0;obj=obj->next_content ) {
        if (obj->item_type != ITEM_POTION)
            continue;
        if (!ch->can_see( obj ))
            continue;
        if (ch->getRealLevel( ) < obj->level)
            continue;

        if ( ch->hit < ch->max_hit*0.9 ) 
        {
            int cl;
            cl = potion_cure_level( obj );
            if ( cl > 0 )
            {
                if ( ch->hit<ch->max_hit*0.5 && cl > 3 )
                    break;
                if ( ch->hit<ch->max_hit*0.7 )
                    break;
            }
        }

        if ( IS_AFFECTED(ch,AFF_POISON) && potion_cure_poison(obj) )
            break;
        if ( IS_AFFECTED(ch,AFF_PLAGUE) && potion_cure_disease(obj) )
            break;
        if ( IS_AFFECTED(ch,AFF_BLIND) && potion_cure_blind(obj) )
            break;

        if ( ch->fighting != 0 )
        {
            int al;
            al = potion_arm_level( obj );
            
            if ( ch->getModifyLevel() - ch->fighting->getModifyLevel() < 7 && al>3)
                break;
            if ( ch->getModifyLevel() - ch->fighting->getModifyLevel() < 8 && al>2 )
                break;
            if ( ch->getModifyLevel() - ch->fighting->getModifyLevel() < 9 && al>1 )
                break;
            if ( ch->getModifyLevel() - ch->fighting->getModifyLevel() < 10 && al>0 )
                break;
        }
    }

    if (obj) {
        oldact("$c1 осушает $o4.", ch, obj, 0, TO_ROOM );
        oldact("Ты осушаешь $o4.", ch, obj, 0 ,TO_CHAR );
        
        spell_by_item( ch, obj );
        obj_to_char( create_object(get_obj_index(OBJ_VNUM_POTION_VIAL),0),ch);
        extract_obj( obj );
        return true;
    }

    return false;
}

/*
 * utils for taking and wearing objects
 */
static bool can_take_obj( Character *ch, Object *obj )
{
    if (!obj->can_wear( ITEM_TAKE )) 
        return false;
        
    // replacing NOSAC with NOPURGE restriction here
    if (IS_OBJ_STAT(obj,ITEM_NOPURGE))
        return false;

    if (!obj->getOwner().empty())
        return false;
    if (obj->behavior && obj->behavior->getType() != "BasicObjectBehavior")
        return false;
    if (!ch->can_see( obj ))
        return false;
    if (obj->isAntiAligned( ch ))
        return false;
    if (obj->pIndexData->limit > 0)
        return false;
    // mobs shouldn't pick up ANY corpses
    if (obj->item_type == ITEM_CORPSE_NPC)
        return false;
    return true;
}

static bool can_wield_obj( Character *ch, Object *obj )
{
    if (!can_take_obj( ch, obj ))
        return false;
    if (obj->item_type != ITEM_WEAPON)
        return false;
    if (!obj->can_wear( ITEM_WIELD ))
        return false;
    if (obj->getWeight( ) > get_str_app(ch).wield * 10)
        return false;
    if (!get_weapon_skill( obj )->usable( ch ))
        return false;
    if (get_wear_level( ch, obj ) > ch->getRealLevel( ))
        return false;
    return true;
}

static bool can_shield_obj( Character *ch, Object *obj )
{
    if (!can_take_obj( ch, obj ))
        return false;
    if (!obj->can_wear( ITEM_WEAR_SHIELD ))
        return false;
    if (get_wear_level( ch, obj ) > ch->getRealLevel( ))
        return false;
    return true;
}

/* 
 * Scavenge 
 */
bool BasicMobileBehavior::doScavenge( )
{
    Object *obj, *target = 0;
    int count = 0;
    
    if (!IS_SET(ch->act, ACT_SCAVENGER))
        return false;
    
    if (!IS_AWAKE(ch))
        return false;
        
    if (ch->in_room->area->empty)
        return false;

    if (ch->in_room->contents == 0)
        return false;

    if (number_bits( 6 ))
        return false;

    for (obj = ch->in_room->contents; obj; obj = obj->next_content) {
    int v = obj->pIndexData->vnum;
    // Mobs shouldn't pick up guts, useless gore    
        if (   v == OBJ_VNUM_GUTS       || v == OBJ_VNUM_SEVERED_HEAD
            || v == OBJ_VNUM_TORN_HEART || v == OBJ_VNUM_SLICED_ARM
            || v == OBJ_VNUM_SLICED_LEG || v == OBJ_VNUM_BRAINS )
            continue;
        if (!can_take_obj( ch, obj ))
            continue;
        if (count_obj_list( obj->pIndexData, ch->carrying ) >= 3)
            continue;
        
        if (number_range( 0, count++ ) == 0) 
            target = obj;
    }
    
    if (!target)
        return false;

    interpret_raw(ch, "get", "%lld", target->getID());

    if (target->carried_by != ch)
        return false;
        
    if ((can_wield_obj( ch, target ) && !get_eq_char( ch, wear_wield ))
        || (can_shield_obj( ch, target ) && !get_eq_char( ch, wear_shield )))
    {
        oldact("$c1 оценивающе рассматривает $o4.", ch, target, 0, TO_ROOM);
        wear_obj( ch, target, F_WEAR_VERBOSE );
    }

    return true;
}


/*
 * Rearm and weapon pickup
 */
static bool mprog_cant_scavenge( Character *ch, Object *obj )
{
    FENIA_CALL( ch, "CantScavenge", "O", obj );
    FENIA_NDX_CALL( ch->getNPC(), "CantScavenge", "CO", ch, obj);
    return false;
}
bool BasicMobileBehavior::doPickWeapon( )
{
    Object *obj, *wield, *shield, *choice;
    int count;
    
    if (number_bits( 1 ))
        return false;

    wield = get_eq_char( ch, wear_wield );
    shield = get_eq_char( ch, wear_shield );
    choice = 0;
    count = 0;
   
    for (obj = ch->carrying; obj; obj = obj->next_content) {
        bool isGood = false;

        if (obj->wear_loc != wear_none)
            continue;
        
        if (!can_wield_obj( ch, obj ))
            continue;
        
        if (ch->fighting) { // wield some weapon for combat
            if (!wield) 
                isGood = true;
        }
        else { // while tracking, wield dagger to stab or bow to shoot
            if (gsn_bow->usable( ch )
                && (!wield || wield->value0() != WEAPON_BOW)
                && obj->value0() == WEAPON_BOW)
            {
                isGood = true;
            }

            if (gsn_backstab->usable( ch )
                && (!wield || attack_table[wield->value3()].damage != DAM_PIERCE) 
                && attack_table[obj->value3()].damage == DAM_PIERCE)
            {
                isGood = true;
            }
        }

        if (isGood)
            if (number_range( 0, count++ ) == 0) 
                choice = obj;
    }

    if (choice) {
        if (wear_obj( ch, choice, F_WEAR_VERBOSE | F_WEAR_REPLACE ) == RC_WEAR_OK)
            return true;
    }
    
    if (!shield && !IS_SET( ch->act, ACT_RANGER )) { // wear shield for non-archers
        choice = 0;
        count = 0;

        for (obj = ch->carrying; obj; obj = obj->next_content) 
            if (can_shield_obj( ch, obj )) 
                if (number_range( 0, count++ ) == 0) 
                    choice = obj;
    
        if (choice) {
            if (wear_obj( ch, choice, F_WEAR_VERBOSE ) == RC_WEAR_OK)
                return true;
        }
    }
    
    // pickup insteresting things from the ground
    choice = 0;
    count = 0;
    for (obj = ch->in_room->contents; obj; obj = obj->next_content)
        if (can_wield_obj( ch, obj ) || can_shield_obj( ch, obj ))
            if (number_range( 0, count++ ) == 0) 
                choice = obj;
    
    if (choice && !mprog_cant_scavenge(ch, choice)) {
        interpret_raw(ch, "get", "%lld", choice->getID());
        return true;
    }
    
    return false;
}    


/* 
 * Wander 
 */
bool BasicMobileBehavior::doWander( )
{
    int door;
    Room *room;
    EXIT_DATA *pexit;
    
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
    
    door = number_door( );
    pexit = ch->in_room->exit[door];

    if (!pexit || !ch->can_see( pexit ))
        return false;

    room = pexit->u1.to_room;

    if (IS_SET(pexit->exit_info, EX_CLOSED) && !IS_AFFECTED( ch, AFF_PASS_DOOR ))
        return false;
        
    if (IS_SET( room->room_flags, ROOM_NO_MOB ))
        return false;
        
    if (IS_SET(ch->act, ACT_STAY_AREA) && room->area != ch->in_room->area)
        return false;

    if (IS_SET(ch->act, ACT_OUTDOORS) && IS_SET( room->room_flags,ROOM_INDOORS|ROOM_MANSION))
        return false;

    if (IS_SET(ch->act, ACT_INDOORS) && !IS_SET( room->room_flags,ROOM_INDOORS))
        return false;

    if (IS_SET(ch->form, FORM_FISH) && !RoomUtils::isWater(room))
        return false;

    // Avoid entering solitary and private rooms and getting stuck there.
    if (room->getCapacity() != -1)
        return false;

    return move_char( ch, door );
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

    if (IS_SET(ch->act, ACT_RANGER))
        if (healRanger( ch ))
            return true;

    if (IS_SET(ch->act, ACT_CLERIC))
        if (healCleric( ch ))
            return true;

    if (IS_SET(ch->act, ACT_NECROMANCER|ACT_UNDEAD))
        if (healNecro( ch ))
            return true;

    return false;
}
