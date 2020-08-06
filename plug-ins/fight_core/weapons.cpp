/* $Id$
 *
 * ruffina, 2004
 */
#include <math.h>
#include "jsoncpp/json/json.h"
#include "skill.h"
#include "skillreference.h"
#include "logstream.h"
#include "object.h"
#include "character.h"
#include "configurable.h"

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
    switch (wield->value0())
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
        case(WEAPON_BOW):           return &*gsn_bow;
        case(WEAPON_ARROW):           return &*gsn_arrow;
        case(WEAPON_LANCE):           return &*gsn_lance;
        case(WEAPON_STONE):         return &*gsn_throw_stone;                                
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


Json::Value weapon_value2_by_class;
CONFIGURABLE_LOADED(fight, weapon_value2)
{
    weapon_value2_by_class = value;
}

Json::Value ave_bonus_by_class;
CONFIGURABLE_LOADED(fight, ave_bonus)
{
    ave_bonus_by_class = value;
}

Json::Value weapon_ave_tiers;
CONFIGURABLE_LOADED(fight, weapon_ave_tiers)
{
    weapon_ave_tiers = value;
}

int weapon_ave(int level, int tier)
{
    if (tier <= 0 || tier > (int)weapon_ave_tiers.size()) {
        bug("weapon_ave: invalid tier %d for level %d", tier, level);
        return 0;
    }

    if (level <= 0 || level > MAX_LEVEL) {
        bug("weapon_ave: invalid level %d for tier %d", level, tier);
        return 0;
    }

    Json::Value &one_tier = weapon_ave_tiers[tier-1];
    int index = level / 5;
    if (index >= (int)one_tier.size()) {
        bug("weapon_ave: tier %d of size %d doesn't have enough values for level %d", tier, one_tier.size(), level);
        return 0;
    }

    return one_tier[index].asInt();
}

int weapon_value2(bitnumber_t wclass)
{
    if (wclass < 0 || wclass >= (int)weapon_value2_by_class.size()) {
        bug("weapon_value2: invalid weapon class %d", wclass);
        return 0;
    }

    return weapon_value2_by_class[wclass].asInt();
}

float weapon_ave_bonus(bitnumber_t wclass) 
{
    if (wclass < 0 || wclass >= (int)ave_bonus_by_class.size()) {
        bug("weapon_ave_bonus: invalid weapon class %d", wclass);
        return 0;
    }

    return ave_bonus_by_class[wclass].asFloat();
}

int weapon_value1(int level, int tier, bitnumber_t wclass)
{
    int ave = weapon_ave(level, tier);
    if (ave <= 0)
        return 0;

    int value2 = weapon_value2(wclass);
    if (value2 <= 0)
        return 0;

    float value1 = 2 * ave / (value2 + 1);
    return ceil(value1);
}

