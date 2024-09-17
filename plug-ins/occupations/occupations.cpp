/* $Id$
 *
 * ruffina, 2004
 */
#include "occupations.h"

#include "npcharacter.h"
#include "room.h"
#include "object.h"
#include "behavior.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "act.h"
#include "interp.h"
#include "loadsave.h"
#include "merc.h"

#include "def.h"

static const char * occ_names [] = 
{
    "none",
    "shopper",
    "practicer",
    "repairman",
    "quest_trader",
    "quest_master",
    "healer",
    "smithman",
    "trainer",
    "clanguard",
    "adept",
    "battlehorse",
    0
};

static int occ_name2type( const char *name )
{
    for (int i = 0; occ_names[i]; i++)
        if (!str_cmp( name, occ_names[i] ))
            return i;

    return -1;
}

static const char * occ_type2name(int occType)
{
    if (occType < 0 || occType >= OCC_MAX)
        return 0;

    return occ_names[occType];
}

/**
  This method checks if mob behavior class supports specified occupation.
  Alternatively, it checks for a corresponding property on mob's prototype to exist.
  This alternative check would allow to eventually move from C++ behaviors to Fenia-defined triggers.
*/ 
bool mob_has_occupation( NPCharacter *mob, const char *occName )
{
    int occType = occ_name2type( occName );

    if (occType <= 0)
        return false;

    return mob_has_occupation( mob, occType );
}

bool mob_has_occupation( NPCharacter *mob, int occType )
{
    const char *occName = occ_type2name(occType);

    if (!occName)
        return false;

    // Old-style 'healer' property, to be removed
    if (mob->pIndexData->properties.count(occName) > 0)
        return true;

    // Mob behaviors: try to find existing behavior called occName and see if it's assigned
    Behavior *bhv = behaviorManager->findExisting(occName);
    if (bhv && mob->pIndexData->behaviors.isSet(bhv))
        return true;

    // Old-style behavior
    return mob->behavior 
           && IS_SET(mob->behavior->getOccupation( ), (1 << occType) );
}

bool obj_has_trigger( Object *obj, const DLString& trigger )
{
    return obj->behavior && obj->behavior->hasTrigger( trigger );
}

bool obj_is_special(Object *obj)
{
    FENIA_NDX_HAS_TRIGGER(obj, "Use");
    FENIA_NDX_HAS_TRIGGER(obj, "Equip");
    FENIA_NDX_HAS_TRIGGER(obj, "Fight");
    FENIA_NDX_HAS_TRIGGER(obj, "Spec");
    
    return obj_has_trigger(obj, "use")
        || obj_has_trigger(obj, "fight")
        || obj_has_trigger(obj, "equip")
        || obj_has_trigger(obj, "spec");
}

