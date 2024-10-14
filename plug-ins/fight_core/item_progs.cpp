#include "item_progs.h"
#include "character.h"
#include "core/object.h"
#include "affect.h"
#include "affecthandler.h"
#include "spelltarget.h"
#include "wrapperbase.h"
#include "wrappertarget.h"
#include "core/behavior/behavior_utils.h"
#include "../loadsave/behavior_utils.h"
#include "loadsave.h"
#include "interp.h"
#include "areaquestutils.h"
#include "follow_utils.h"
#include "merc.h"
#include "def.h"

/* RT part of the corpse looting code */
static bool oprog_get_money( Character *ch, Object *obj )
{
    ch->silver += obj->value0();
    ch->gold += obj->value1();

    if (obj->pIndexData->vnum > 5 && (obj->value0() > 0 || obj->value1() > 0)) {
        DLString moneyArg = Money::describe(obj->value1( ), obj->value0( ), 4);
        ch->pecho("Твой кошелек пополнился на %s.", moneyArg.c_str());
    }

    if (IS_SET(ch->act,PLR_AUTOSPLIT))
        if (obj->value0() > 1 || obj->value1())
            if (party_members_room( ch ).size( ) > 1)
                interpret_raw( ch, "split", "%d %d", obj->value0(), obj->value1() );
    
    extract_obj( obj );
    return true;
}

bool oprog_get( Object *obj, Character *ch )
{
    aquest_trigger(obj, ch, "Get", "OC", obj, ch);
    FENIA_CALL( obj, "Get", "C", ch );
    FENIA_NDX_CALL( obj, "Get", "OC", obj, ch );
    BEHAVIOR_VOID_CALL( obj, get, ch );

    for (auto &paf: obj->affected.findAllWithHandler())
        if (paf->type->getAffect() && paf->type->getAffect()->onGet(SpellTarget::Pointer(NEW, obj), paf, ch))
            return true;

    if (obj->extracted)
        return true;

    switch (obj->item_type) {
    case ITEM_MONEY:
        return oprog_get_money( ch, obj );
    }

    return false;
}

bool oprog_drop( Object *obj, Character *ch )
{
    if (behavior_trigger(obj, "Drop", "OC", obj, ch))
        return true;

    FENIA_CALL( obj, "Drop", "C", ch )
    FENIA_NDX_CALL( obj, "Drop", "OC", obj, ch )
    BEHAVIOR_CALL( obj, drop, ch )

    return false;
}


