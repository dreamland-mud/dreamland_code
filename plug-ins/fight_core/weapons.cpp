/* $Id$
 *
 * ruffina, 2004
 */

#include "skill.h"
#include "skillreference.h"
#include "core/object.h"
#include "character.h"

#include "weapons.h"
#include "math_utils.h"
#include "itemflags.h"
#include "affectflags.h"
#include "attacks.h"
#include "def.h"

WEARLOC(wield);
WEARLOC(second_wield);

GSN(none);  GSN(exotic);      GSN(sword);        GSN(dagger);
GSN(spear); GSN(mace);        GSN(axe);          GSN(flail);
GSN(whip);  GSN(polearm);     GSN(bow);          GSN(arrow);
GSN(lance); GSN(throwing_weapon); GSN(hand_to_hand);

/*
 * Both overrides come from an affect, so the value is whatever a builder or a
 * script put in af.modifier -- unlike value0/value3, which OLC validates. Every
 * caller of these two indexes a fixed table with the answer, so an override
 * outside the table is refused here and the weapon keeps the value it was built
 * with. Refusing beats clamping: clamping would silently turn a typo into some
 * other real weapon.
 */
static int attack_table_size()
{
    static int size = -1;

    if (size < 0)
        for (size = 0; attack_table[size].name; size++)
            ;

    return size;
}

int get_weapon_class(Object *wield)
{
    if (wield->item_type != ITEM_WEAPON)
        return wield->value0();

    int wclass = wield->affectedValue(APPLY_WEAPON_CLASS, wield->value0(), true);

    if (wclass < 0 || wclass >= WEAPON_MAX)
        return wield->value0();

    return wclass;
}

int get_weapon_attack(Object *wield)
{
    if (wield->item_type != ITEM_WEAPON)
        return wield->value3();

    int attack = wield->affectedValue(APPLY_WEAPON_ATTACK, wield->value3(), true);

    if (attack < 0 || attack >= attack_table_size())
        return wield->value3();

    return attack;
}

int get_weapon_dice_number(Object *wield)
{
    if (wield->item_type != ITEM_WEAPON)
        return wield->value1();

    // A debuff may take the dice down to nothing, but never past it: dice()
    // walks the count, and dice_ave multiplies by it.
    int number = wield->affectedValue(APPLY_DICE_NUMBER, wield->value1(), false);
    return number < 0 ? 0 : number;
}

int get_weapon_dice_size(Object *wield)
{
    if (wield->item_type != ITEM_WEAPON)
        return wield->value2();

    int size = wield->affectedValue(APPLY_DICE_SIZE, wield->value2(), false);
    return size < 0 ? 0 : size;
}

Skill * get_weapon_skill( Object *wield )
{
    switch (get_weapon_class(wield))
    {
        default :               return &*gsn_none;
        case(WEAPON_EXOTIC):    return &*gsn_exotic;
        case(WEAPON_SWORD):     return &*gsn_sword;
        case(WEAPON_DAGGER):    return &*gsn_dagger;
        case(WEAPON_SPEAR):     return &*gsn_spear;
        case(WEAPON_MACE):      return &*gsn_mace;
        case(WEAPON_AXE):       return &*gsn_axe;
        case(WEAPON_FLAIL):     return &*gsn_flail;
        case(WEAPON_WHIP):      return &*gsn_whip;
        case(WEAPON_POLEARM):   return &*gsn_polearm;
        case(WEAPON_BOW):       return &*gsn_bow;
        case(WEAPON_ARROW):     return &*gsn_arrow;
        case(WEAPON_LANCE):     return &*gsn_lance;
        case(WEAPON_STONE):     return &*gsn_throwing_weapon;                                
   }
}

bitnumber_t get_weapon_for_skill(Skill *skill)
{
    int sn = skill->getIndex();
    
    if (sn == gsn_sword)
        return WEAPON_SWORD;
    else if (sn == gsn_dagger)
        return WEAPON_DAGGER; 
    else if (sn == gsn_spear)
        return WEAPON_SPEAR; 
    else if (sn == gsn_mace)
        return WEAPON_MACE; 
    else if (sn == gsn_axe)
        return WEAPON_AXE; 
    else if (sn == gsn_flail)
        return WEAPON_FLAIL; 
    else if (sn == gsn_polearm)
        return WEAPON_POLEARM; 
    else if (sn == gsn_bow)
        return WEAPON_BOW; 
    else
        return -1;
}
    
Object * get_wield( Character *ch, bool secondary )
{
    return secondary ? wear_second_wield->find( ch ) : wear_wield->find( ch );
}


int get_weapon_sn( Object *wield )
{
    int sn;

    if (wield == 0 || wield->item_type != ITEM_WEAPON)
        sn = gsn_hand_to_hand;
    else
        sn = get_weapon_skill( wield )->getIndex( );

   return sn;
}

    
int get_weapon_sn( Character *ch, bool secondary )
{
    return get_weapon_sn( get_wield( ch, secondary ) );
}

int weapon_ave(Object *wield)
{
    if (wield->item_type == ITEM_WEAPON)
        return dice_ave(get_weapon_dice_number(wield), get_weapon_dice_size(wield));
    else
        return 0;
}

int weapon_ave(struct obj_index_data *pWield)
{
    if (pWield->item_type == ITEM_WEAPON)
        return dice_ave(pWield->value[1], pWield->value[2]);
    else
        return 0;
}

// Prototype form of get_weapon_sn: a prototype has no APPLY_WEAPON_CLASS affect
// to resolve, so its base class in value0 maps straight to the wielding skill.
// Mirrors the get_weapon_skill switch but off the prototype, the way weapon_ave
// above has a prototype twin. Kept separate so the combat hot path stays put.
int get_weapon_sn( struct obj_index_data *pWield )
{
    if (pWield == 0 || pWield->item_type != ITEM_WEAPON)
        return gsn_hand_to_hand;

    switch (pWield->value[0])
    {
        case WEAPON_EXOTIC:   return gsn_exotic;
        case WEAPON_SWORD:    return gsn_sword;
        case WEAPON_DAGGER:   return gsn_dagger;
        case WEAPON_SPEAR:    return gsn_spear;
        case WEAPON_MACE:     return gsn_mace;
        case WEAPON_AXE:      return gsn_axe;
        case WEAPON_FLAIL:    return gsn_flail;
        case WEAPON_WHIP:     return gsn_whip;
        case WEAPON_POLEARM:  return gsn_polearm;
        case WEAPON_BOW:      return gsn_bow;
        case WEAPON_ARROW:    return gsn_arrow;
        case WEAPON_LANCE:    return gsn_lance;
        case WEAPON_STONE:    return gsn_throwing_weapon;
        default:              return gsn_none;
    }
}


