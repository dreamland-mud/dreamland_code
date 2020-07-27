/* $Id$
 *
 * ruffina, 2004
 */
#include <math.h>

#include "skill.h"
#include "skillreference.h"
#include "logstream.h"
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

const int tier_step = 5;
const int tier_size = LEVEL_MORTAL / 5 + 1;
const int tier_count = 5;
typedef int damage_tier_t [tier_size];

damage_tier_t damroll_tiers [tier_count] = {
// tier 1
{
    1, 1, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 13, 15, 16, 18, 21, 23, 25, 27, 30,
},
// tier 2
{
    0, 1, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 14, 16, 18, 21, 23, 25, 27,
},
// tier 3
{
    0, 0, 0, 1, 1, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 13, 15, 16, 18, 21, 23,
},
// tier 4
{
    0, 0, 0, 0, 1, 1, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 13, 15, 16, 18, 21,
},
// tier 5
{
    0, 0, 0, 0, 0, 1, 1, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 13, 15, 16, 18,
}
};

damage_tier_t damage_tiers [tier_count] = {
// tier 1
{ 9,  15, 19, 22, 26,
 31, 35, 40, 45, 49,
 53, 57, 60, 63, 65,
 67, 68, 70, 71, 72,
 73 },

// tier 2
{ 8,  10, 15, 19, 22,
 26, 31, 35, 40, 45,
 49, 53, 57, 60, 63,
 65, 67, 68, 70, 71,
 72 },

// tier 3
{ 7,  8, 10, 12, 15,
 19, 22, 26, 31, 35,
 40, 45, 49, 53, 57,
 60, 63, 65, 67, 68,
 70 },

// tier 4
{ 6,  7,  8, 10, 12,
 15, 19, 22, 26, 31,
 35, 40, 45, 49, 53,
 57, 60, 63, 65, 67,
 68 },

// tier 5
{ 5,  6,  7,  8, 10,
 12, 15, 19, 22, 26,
 31, 35, 40, 45, 49,
 53, 57, 60, 63, 65,
 67 }
};

int weapon_value2_by_class[WEAPON_MAX] = {
   12, // exotic
    6, // sword
    3, // dagger
    8, // spear
    5, // mace
    6, // axe
    5, // flail
    8, // whip
   12, // polearm
    0, 0, 0, 0
};

float ave_bonus_by_class[WEAPON_MAX] = {
    1.2,
    1,
    1,
    1,
    1,
    1.2,
    0.8,
    0.8,
    1.2,
    0, 0, 0, 0
};

int weapon_ave(int level, int tier)
{
    if (tier <= 0 || tier > tier_size) {
        bug("weapon_ave: invalid tier %d for level %d", tier, level);
        return 0;
    }

    if (level <= 0 || level > MAX_LEVEL) {
        bug("weapon_ave: invalid level %d for tier %d", level, tier);
        return 0;
    }

    damage_tier_t &damages = damage_tiers[tier - 1];
    int index = min(level / 5, tier_size - 1);
    return damages[index];
}

int weapon_value2(bitnumber_t wclass)
{
    if (wclass < 0 || wclass >= WEAPON_MAX) {
        bug("weapon_value2: invalid weapon class %d", wclass);
        return 0;
    }

    return weapon_value2_by_class[wclass];
}

float weapon_ave_bonus(bitnumber_t wclass) 
{
    if (wclass < 0 || wclass >= WEAPON_MAX) {
        bug("weapon_ave_bonus: invalid weapon class %d", wclass);
        return 0;
    }

    return ave_bonus_by_class[wclass];
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

int weapon_damroll(int level, int tier)
{
    if (tier <= 0 || tier > tier_size) {
        bug("weapon_damroll: invalid tier %d for level %d", tier, level);
        return 0;
    }

    if (level <= 0 || level > MAX_LEVEL) {
        bug("weapon_damroll: invalid level %d for tier %d", level, tier);
        return 0;
    }

    damage_tier_t &damroll = damroll_tiers[tier - 1];
    int index = min(level / 5, tier_size - 1);
    return damroll[index];
}