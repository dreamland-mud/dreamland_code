/* $Id$
 *
 * ruffina, 2004
 */
#ifndef OCCUPATIONS_H
#define OCCUPATIONS_H

#include "npcharacter.h"
#include "pcharacter.h"

class Object;
class BehaviorReference;

enum {
    OCC_NONE = 0,
    OCC_SHOPPER,
    OCC_PRACTICER,
    OCC_REPAIRMAN,
    OCC_QUEST_TRADER,
    OCC_QUEST_MASTER,
    OCC_HEALER,
    OCC_SMITHMAN,
    OCC_TRAINER,
    OCC_CLANGUARD,
    OCC_ADEPT,
    OCC_BATTLEHORSE,
    OCC_MAX,
};

bool mob_has_occupation( NPCharacter *, const DLString & );
bool mob_has_occupation( NPCharacter *, int );
bool mob_has_behavior(NPCharacter *, BehaviorReference &bhv);

// Jewelers (props "jeweler") repair jewelry and treasure: Fenia .tmp.mob.canRepair.
bool mob_is_jeweler( NPCharacter * );
NPCharacter * find_jeweler_mob( Room * );

// Fenia shops: a keeper is a mob proto with the bedit 'shopper' behavior, its
// stock and buys live in props["shopper"] (utils/shop).
bool mob_index_is_shopper( MOB_INDEX_DATA * );
list<int> mob_index_shop_stock( MOB_INDEX_DATA * );
bool mob_index_shop_buys( MOB_INDEX_DATA *, int itemType );

// Fenia shop pets: bedit 'pet' (props["pet"]["adaptive"]), 'rat', and the
// Rat God's 'rat god'. They keep the destiny of the C++ classes they replace:
// no summon, charm, teleport or quest target.
bool mob_index_has_pet_destiny( MOB_INDEX_DATA * );
// Does a pet of this prototype take its buyer's level? 'rat', or 'pet' with
// props pet.adaptive set (bool, non-zero number or the string "true" / "1").
bool mob_index_pet_adaptive( MOB_INDEX_DATA * );

NPCharacter * find_attracted_mob( Character *, int );

template <typename Bhv>
::Pointer<Bhv> find_attracted_mob_behavior( Character *ch, int occType )
{
    NPCharacter *amob;
    ::Pointer<Bhv> behavior;
    
   amob = find_attracted_mob( ch, occType );

   if (amob && amob->behavior)
       behavior = amob->behavior.getDynamicPointer<Bhv>( );

   return behavior;
}

#endif
