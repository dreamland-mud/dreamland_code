/* $Id$
 *
 * ruffina, 2004
 */
#include "occupations.h"

#include "npcharacter.h"
#include "room.h"
#include "object.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "act.h"
#include "interp.h"
#include "loadsave.h"
#include "merc.h"
#include "mercdb.h"
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
    0
};

static int occ_name2type( const char *name )
{
    for (int i = 0; occ_names[i]; i++)
        if (!str_cmp( name, occ_names[i] ))
            return i;

    return -1;
}


bool mob_has_occupation( NPCharacter *mob, const char *occName )
{
    int occType = occ_name2type( occName );

    if (occType <= 0)
        return false;

    return mob_has_occupation( mob, occType );
}

bool mob_has_occupation( NPCharacter *mob, int occType )
{
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

