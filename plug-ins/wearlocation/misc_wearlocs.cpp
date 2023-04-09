/* $Id$
 *
 * ruffina, 2004
 */
#include "misc_wearlocs.h"
#include "wearloc_utils.h"

#include "skillreference.h"
#include "room.h"
#include "affect.h"
#include "character.h"
#include "object.h"

#include "stats_apply.h"
#include "weapons.h"
#include "save.h"
#include "loadsave.h"
#include "act.h"
#include "merc.h"
#include "def.h"

GSN(second_weapon);
GSN(hand_to_hand);
GSN(exotic);

WEARLOC(tail);
WEARLOC(hair);


/*
 * stuck-in
 */
bool StuckInWearloc::equip( Character *ch, Object *obj )
{
    obj->wear_loc.assign( this );
    saveDrops( ch );
    return true;
}

void StuckInWearloc::unequip( Character *ch, Object *obj )
{
    obj->wear_loc.assign( wear_none );
    saveDrops( ch );
}

/*
 * horse part of centaurs
 */

int HorseWearloc::canWear( Character *ch, Object *obj, int flags )
{
    int rc = DefaultWearlocation::canWear( ch, obj, flags );
    
    if (rc != RC_WEAR_OK)
        return rc;

    if (RIDDEN(ch)) {
        if (IS_SET(flags, F_WEAR_VERBOSE)) {
            echo_master(ch, "Ты не цирков%1$Gой|ой|ая %1$Gконь|конь|лошадь! Попроси всадника спешиться.", ch);
        }
        return RC_WEAR_CONFLICT;
    }

    return RC_WEAR_OK;
}

bool HorseWearloc::canRemove( Character *ch, Object *obj, int flags )
{
    int rc = DefaultWearlocation::canRemove( ch, obj, flags );
    if (!rc)
        return rc;

    if (RIDDEN(ch)) {
        if (IS_SET(flags, F_WEAR_VERBOSE)) {
            echo_master(ch, "Ты не цирков%1$Gой|ой|ая %1$Gконь|конь|лошадь! Попроси всадника спешиться.", ch);
        }
        return false;
    }

    return true;
}

/*
 * hair 
 */
bool HairWearloc::matches( Character *ch )
{
    return true;
}

bool HairWearloc::equip( Character *ch, Object *obj )
{
    obj->wear_loc.assign( this );
    saveDrops( ch );
    return true;
}

void HairWearloc::unequip( Character *ch, Object *obj )
{
    obj->wear_loc.assign( wear_none );
    saveDrops( ch );
}
void HairWearloc::affectsOnEquip( Character *ch, Object *obj ) { 
}

void HairWearloc::affectsOnUnequip( Character *ch, Object *obj ) { }
int HairWearloc::canWear( Character *ch, Object *obj, int flags ) {
    if (find( ch ) != NULL) {
        if (IS_SET(flags, F_WEAR_VERBOSE))
            echo_master(ch, "В твоих волосах уже запуталось что-то другое.");
        return RC_WEAR_CONFLICT;
    }
    return DefaultWearlocation::canWear( ch, obj, flags );
}

/*
 * shield 
 */
int ShieldWearloc::canWear( Character *ch, Object *obj, int flags )
{
    Object *weapon;
    int rc;
    
    if (( rc = DefaultWearlocation::canWear( ch, obj, flags ) ) != RC_WEAR_OK)
        return rc;

    weapon = wear_wield->find( ch );
    
    if (weapon 
        && ch->size < SIZE_LARGE 
        && !ch->is_npc()
        && IS_WEAPON_STAT(weapon, WEAPON_TWO_HANDS))
    {
        if (IS_SET(flags, F_WEAR_VERBOSE)) {
            echo_master(ch, "Твои руки заняты оружием!");
            ch->recho("%^C1 крутит в руках %O4, но одновременно с оружием это надеть никак не выходит.", ch, obj);
        }
        return RC_WEAR_CONFLICT;
    }

    return RC_WEAR_OK;
}

/*
 * sheath (random weapons wearloc)
 */

bool SheathWearloc::matches( Character *ch )
{
    return true;
}

bool SheathWearloc::displayFlags(Character *ch, Object *obj)
{
    return false;
}

/** 
 * Sheath wearloc shows shortened item name for auto-generated weapons, stored during
 * generation as eqName property. Prefixes and suffixes are hidden.
 */
DLString SheathWearloc::displayName(Character *ch, Object *obj)
{
    DLString eqName = obj->getProperty("eqName");

    if (!eqName.empty())
        return eqName.ruscase('1');

    return DefaultWearlocation::displayName(ch, obj);
}

/**
 * Sheath wearloc looks different depending on item type or item proto properties.
 */
DLString SheathWearloc::displayLocation(Character *ch, Object *obj)
{
    return getConfig(obj).msgDisplay;
}

void SheathWearloc::onFight(Character *ch, Object *obj)
{
    // Unequip odd weapons first.
    Object *wield = wear_wield->find(ch);
    if (wield && wield->reset_mob != ch->getID()) 
        wear_wield->unequip(wield);

    if (wear_wield->find(ch))
        return;
        
    // Change location from sheath to wield; no need to remove and reapply the affects.
    ch->pecho(getMsgSelfRemove(obj).c_str(), ch, obj);
    ch->recho(getMsgRoomRemove(obj).c_str(), ch, obj);

    obj->wear_loc = wear_wield;
}

const DLString &SheathWearloc::getMsgSelfRemove(Object *obj) const
{
    return getConfig(obj).msgSelfRemove;
}

const DLString &SheathWearloc::getMsgRoomRemove(Object *obj) const
{
    return getConfig(obj).msgRoomRemove;
}

const DLString &SheathWearloc::getMsgSelfWear(Object *obj) const
{
    return getConfig(obj).msgSelfWear;
}

const DLString &SheathWearloc::getMsgRoomWear(Object *obj) const
{
    return getConfig(obj).msgRoomWear;
}

/** Decide which group of messages to show for the item. */
const SheathConfig & SheathWearloc::getConfig(Object *obj) const
{
    DLString key;

    if (!obj)
        key = "back";
    else if (!obj->getProperty("eqKey").empty())
        key = obj->getProperty("eqKey");
    else if (IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS))
        key = "back";
    else {
        switch (obj->value0()) {
            case WEAPON_SWORD:
            case WEAPON_DAGGER:
                key = "scabbard";
                break;
            case WEAPON_SPEAR:
            case WEAPON_POLEARM:
            case WEAPON_LANCE:
            case WEAPON_BOW:
                key = "back";
                break;
            default:
                key = "belt";
                break;
        }
    }

    return config.find(key)->second;
}

/*
 * wield 
 */
bool WieldWearloc::remove( Object *obj, int flags )
{
    Object *dual;

    if (!DefaultWearlocation::remove( obj, flags ))
        return false;

    if (IS_SET(flags, F_WEAR_REPLACE))
        return true;

    if (( dual = wear_second_wield->find( obj->carried_by ) )) 
        dual->wear_loc.assign( this );
    
    return true;
}

int WieldWearloc::canWear( Character *ch, Object *obj, int flags )
{
    int rc;
    
    if (( rc = DefaultWearlocation::canWear( ch, obj, flags ) ) != RC_WEAR_OK)
        return rc;
        
    if (!ch->is_npc( ) && obj->getWeight( ) > (get_str_app(ch).wield * 10)) {
        if (IS_SET(flags, F_WEAR_VERBOSE)) {
            ch->pecho("Ты не можешь этим вооружиться. Оно слишком тяжело для тебя.");
            ch->recho("%^C1, кряхтя, пытается надеть %O4, но сил к сожалению не хватает.", ch, obj);
        }
        return RC_WEAR_HEAVY;
    }

    if (IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS)
        && !ch->is_npc( )
        && ((ch->size < SIZE_LARGE && wear_shield->find( ch ))
            || (ch->getRace( )->getSize( ) < SIZE_HUGE && wear_second_wield->find( ch ))))
    {
        if (IS_SET(flags, F_WEAR_VERBOSE)) {
            ch->pecho("Чтобы вооружиться этим, у тебя должны быть свободны обе руки.");
            ch->recho("%1$^C1 пытается вооружиться %2$O4, но %1$P3 не хватает обеих свободных рук.", ch, obj);
        }
        return RC_WEAR_LARGE;
    }

    return RC_WEAR_OK;
}

int WieldWearloc::wear(  Object *obj, int flags )
{
    int rc;
    
    if (( rc = DefaultWearlocation::wear( obj, flags ) ) != RC_WEAR_OK)
        return rc;

    if (obj->wear_loc != this)
        return rc;
    
    if (IS_SET(flags, F_WEAR_VERBOSE))
        reportWeaponSkill( obj->carried_by, obj );

    return RC_WEAR_OK;
}

void WieldWearloc::reportWeaponSkill( Character *ch, Object *obj )
{
    int skill, sn;
    const char *msg;
    
    sn = get_weapon_sn( obj );

    if (sn == gsn_hand_to_hand)
        return;

    skill = ch->getSkill( sn );

    if (skill >= 100)
        msg = "$o1 становится частью тебя!";
    else if (skill > 85)
        msg = "Ты чувствуешь себя с $o5 абсолютно уверенно.";
    else if (skill > 70)
        msg = "Ты хорошо владеешь $o5.";
    else if (skill > 50)
        msg = "Ты довольно посредственно владеешь $o5.";
    else if (skill > 25) {
        if (sn != gsn_exotic)
            msg = "Ты чувствуешь себя неуклюже, вооружившись $o5.{/Попрактикуйся еще в умении владеть этим типом оружия.";
        else
            msg = "Ты все еще не готов{Sfа{Sx как следует владеть $o5.{/Владение экзотическим оружием зависит от твоего опыта и интеллекта.";    
    } else {
        if (sn != gsn_exotic)
            msg = "Ты понятия не имеешь, как орудовать $o5.{/Возможно, стоит попрактиковаться в умении владеть этим типом оружия.";
        else
            msg = "Ты пока не готов{Sfа{Sx как следует владеть $o5.{/Владение экзотическим оружием зависит от твоего опыта и интеллекта.";
    }
    oldact( msg, ch, obj, 0, TO_CHAR );
}

/*
 * second wield
 */
bool SecondWieldWearloc::remove( Object *obj, int flags )
{
    return DefaultWearlocation::remove( obj, flags );
}

bool SecondWieldWearloc::matches( Character *ch )
{
    if (!DefaultWearlocation::matches( ch ))
        return false;

    if (!gsn_second_weapon->usable( ch )) 
        return false;

    return true;
}

int SecondWieldWearloc::canWear( Character *ch, Object *obj, int flags )
{
    int rc;
    Object *wield;
    
    if (( rc = DefaultWearlocation::canWear( ch, obj, flags ) ) != RC_WEAR_OK)
        return rc;
    
    if (wear_shield->find( ch )) {
        if (IS_SET(flags, F_WEAR_VERBOSE)) {
            ch->pecho("Твои руки уже заняты щитом.");
            ch->recho("%^C1 пытается ухватить %O4, но одновременно со щитом это надеть не получается.", ch, obj);
        }
        return RC_WEAR_CONFLICT;
    }

    if (wear_hold->find( ch )) {
        if (IS_SET(flags, F_WEAR_VERBOSE)) {
            ch->pecho("В твоей левой руке уже что-то зажато.");
            ch->recho("%1$^C1 пытается ухватить %2$O4, но в %1$P2 левой руке уже что-то зажато.", ch, obj);
        }
        return RC_WEAR_CONFLICT;
    }

    if (IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS) && ch->getRace( )->getSize( ) < SIZE_HUGE) {
        if (IS_SET(flags, F_WEAR_VERBOSE)) {
            ch->pecho("Ты не можешь взять двуручное оружие в левую руку!");
            ch->recho("%^C1 пытается взять %O4 в левую руку, но такое под силу только гигантам.", ch, obj);
        }
        return RC_WEAR_LARGE;
    }

    if (!( wield = wear_wield->find( ch ) )) {
        if (IS_SET(flags, F_WEAR_VERBOSE)) {
            ch->pecho("У тебя нет даже первичного оружия!");
            ch->recho("%^C1 пытается взять %O4 в левую руку, но понимает, что сначала стоит взять что-то в правую.", ch, obj);
        }
        return RC_WEAR_CONFLICT;
    }

    if (IS_WEAPON_STAT(wield, WEAPON_TWO_HANDS) && ch->getRace( )->getSize( ) < SIZE_HUGE) {
        if (IS_SET(flags, F_WEAR_VERBOSE)) {
            ch->pecho("Обе руки заняты двуручным оружием!");
            ch->recho("%1$^C1 пытается ухватить %2$O4, но %1$P2 руки уже заняты двуручным оружием.", ch, obj);
        }
        return RC_WEAR_LARGE;
    }

    if (obj->getWeight( ) > (get_str_app(ch).wield * 5)) {
        if (IS_SET(flags, F_WEAR_VERBOSE)) {
            ch->pecho("Это оружие слишком тяжело для тебя, чтобы использовать его как вторичное.");
            ch->recho("%^C1 пытается взять %O4 в левую руку, но сил явно не хватает.", ch, obj);
        }
        return RC_WEAR_HEAVY;
    }

    return RC_WEAR_OK;
}

/*
 * tattoo
 */
int TattooWearloc::canWear( Character *ch, Object *obj, int flags )
{
    if (!ch->is_immortal( )) 
        return RC_WEAR_NOMATCH;
    else
        return DefaultWearlocation::canWear( ch, obj, flags );
}

bool TattooWearloc::canRemove( Character *ch, Object *obj, int flags )
{
    if (IS_SET(flags, F_WEAR_VERBOSE))
        echo_master(ch, "Лишь Божественные Силы могут избавить тебя от %O2.", obj);

    return false;
}

/*
 * tail 
 */
bool TailWearloc::equip( Character *ch, Object *obj )
{
    obj->wear_loc.assign( this );
    saveDrops( ch );
    return true;
}

void TailWearloc::unequip( Character *ch, Object *obj )
{
    obj->wear_loc.assign( wear_none );
    saveDrops( ch );
}

void TailWearloc::affectsOnEquip( Character *ch, Object *obj ) 
{ 
}

void TailWearloc::affectsOnUnequip( Character *ch, Object *obj ) 
{ 
}

int TailWearloc::canWear( Character *ch, Object *obj, int flags ) {
    if (find( ch ) != NULL) {
        if (IS_SET(flags, F_WEAR_VERBOSE))
            echo_master(ch, "На твой хвост уже что-то надето.");
        return RC_WEAR_CONFLICT;
    }
    return DefaultWearlocation::canWear( ch, obj, flags );
}

/*
 * utils
 */

bool obj_is_worn(Object *obj)
{
    return obj->wear_loc->givesAffects();
}
