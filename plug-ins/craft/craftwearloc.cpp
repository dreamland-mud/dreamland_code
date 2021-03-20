#include "craftwearloc.h"
#include "wearloc_utils.h"

#include "character.h"
#include "object.h"

#include "act.h"
#include "merc.h"
#include "def.h"

int CraftTattooWearloc::canWear( Character *ch, Object *obj, int flags )
{
    if (!ch->is_immortal( )) 
        return RC_WEAR_NOMATCH;
    else
        return DefaultWearlocation::canWear( ch, obj, flags );
}

bool CraftTattooWearloc::canRemove( Character *ch, Object *obj, int flags )
{
    if (IS_SET(flags, F_WEAR_VERBOSE))
        oldact("Только специальные средства могут избавить тебя от $o2.", ch, obj, 0, TO_CHAR);

    return false;
}

