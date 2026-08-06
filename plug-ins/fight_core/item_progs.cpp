#include "item_progs.h"
#include "character.h"
#include "npcharacter.h"
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
#include "l10n.h"

/* RT part of the corpse looting code */
static bool oprog_get_money( Character *ch, Object *obj )
{
    ch->silver += obj->value0();
    ch->gold += obj->value1();

    if (obj->pIndexData->vnum > 5 && (obj->value0() > 0 || obj->value1() > 0)) {
        DLString moneyArg = Money::describe(obj->value1( ), obj->value0( ), 4);
        ch->pecho(_("Твой кошелек пополнился на %s."), moneyArg.c_str());
    }

    // 'autosplit' config option retired -- coins are always split with the party.
    if (!ch->is_npc( ))
        if (obj->value0() > 1 || obj->value1())
            if (party_members_room( ch ).size( ) > 1)
                interpret_raw( ch, "split", "%d %d", obj->value0(), obj->value1() );
    
    extract_obj( obj );
    return true;
}

bool oprog_get( Object *obj, Character *ch )
{
    // Named items go back on the floor no matter who tried to take them.
    // oprog_get itself is called last in the give chain, so a quest handler
    // that consumes what it was given has already had its say by now.
    if (!obj_owner_enforce( obj, ch ))
        return true;

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

bool oprog_give_blocked( Object *obj, Character *ch, Character *victim )
{
    for (auto &paf: obj->affected.findAllWithHandler())
        if (paf->type->getAffect() && paf->type->getAffect()->onGive(SpellTarget::Pointer(NEW, obj), paf, ch, victim))
            return true;

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

/*
 * A whole hand-over: area quests, behaviors and both sides' Fenia triggers each
 * get a say, and any of them may take the item back, which is what every
 * `obj->carried_by != victim` bail-out is checking. Lives here rather than in
 * the 'give' command because 'request' and the Fenia obj.trigger("Give") wrapper
 * need the same chain.
 */
bool omprog_give( Object *obj, Character *ch, Character *victim )
{
    if (aquest_trigger(obj, ch, "Give", "OCC", obj, ch, victim))
        return true;
    if (obj->carried_by != victim)
        return true;

    if (aquest_trigger(victim, ch, "Give", "CCO", victim, ch, obj))
        return true;
    if (obj->carried_by != victim)
        return true;

    if (behavior_trigger(victim, "Give", "CCO", victim, ch, obj))
        return true;        
    if (obj->carried_by != victim)
        return true;

    FENIA_CALL( obj, "Give", "CC", ch, victim )
    if (obj->carried_by != victim)
        return true;

    FENIA_NDX_CALL( obj, "Give", "OCC", obj, ch, victim )
    if (obj->carried_by != victim)
        return true;

    BEHAVIOR_VOID_CALL( obj, give, ch, victim )
    if (obj->carried_by != victim)
        return true;
    
    FENIA_CALL( victim, "Give", "CO", ch, obj );
    if (obj->carried_by != victim)
        return true;

    FENIA_NDX_CALL( victim->getNPC( ), "Give", "CCO", victim, ch, obj );
    if (obj->carried_by != victim)
        return true;

    BEHAVIOR_VOID_CALL( victim->getNPC( ), give, ch, obj );        
    if (obj->carried_by != victim)
        return true;
        
    return oprog_get( obj, victim );
}
