/* $Id$
 *
 * ruffina, 2004
 */
#include "skill.h"
#include "skillreference.h"

#include "object.h"
#include "character.h"

#include "itemflags.h"
#include "merc.h"
#include "def.h"

WEARLOC(wield);
WEARLOC(second_wield);

GSN(none);  GSN(exotic);      GSN(sword);        GSN(dagger);
GSN(spear); GSN(mace);        GSN(axe);          GSN(flail);
GSN(whip);  GSN(polearm);     GSN(bow);          GSN(arrow);
GSN(lance); GSN(throw_stone); GSN(hand_to_hand);

Skill * get_weapon_skill( Object *wield )
{
    switch (wield->value[0])
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
        case(WEAPON_BOW):   	return &*gsn_bow;
        case(WEAPON_ARROW):   	return &*gsn_arrow;
        case(WEAPON_LANCE):   	return &*gsn_lance;
	case(WEAPON_STONE): 	return &*gsn_throw_stone;				
   }
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


