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
