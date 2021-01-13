#include "weaponrandomizer.h"
#include "logstream.h"
#include "npcharacter.h"
#include "core/object.h"
#include "loadsave.h"
#include "itemevents.h"
#include "occupations.h"
#include "material.h"
#include "damageflags.h"
#include "weapongenerator.h"
#include "weapontier.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

void WeaponRandomizer::initialization() 
{
    eventBus->subscribe(typeid(ItemResetEvent), Pointer(this));
    eventBus->subscribe(typeid(ItemReadEvent), Pointer(this));
    eventBus->subscribe(typeid(ItemEditedEvent), Pointer(this));
}

void WeaponRandomizer::destruction() 
{
    eventBus->unsubscribe(typeid(ItemResetEvent), Pointer(this));
    eventBus->unsubscribe(typeid(ItemReadEvent), Pointer(this));
    eventBus->unsubscribe(typeid(ItemEditedEvent), Pointer(this));
}

void WeaponRandomizer::handleEvent(const type_index &eventType, const Event &event) const
{
    if (eventType == typeid(ItemReadEvent))
        eventItemRead(static_cast<const ItemReadEvent &>(event));
    else if (eventType == typeid(ItemResetEvent))
        eventItemReset(static_cast<const ItemResetEvent &>(event));
    else if (eventType == typeid(ItemEditedEvent))
        eventItemEdited(static_cast<const ItemEditedEvent &>(event));
}

// Called from 'oedit commit'. Need to re-roll items that just became 'rand_stat'
// and viceversa.
void WeaponRandomizer::eventItemEdited(const ItemEditedEvent &event) const
{    
    Object *obj = event.obj;

    if (obj->item_type != ITEM_WEAPON)
        return;

    if (obj->getProperty("random").empty() && !obj->getProperty("tier").empty()) {
        clearWeapon(obj);
        return;
    }

    if (!obj->getProperty("random").empty() && obj->getProperty("tier").empty()) {
        randomizeWeaponStats(obj);
        return;
    }
}

static bool item_is_sold(Object *obj)
{
    Character *carrier = obj->getCarrier();
    return carrier 
            && carrier->is_npc() 
            && mob_has_occupation(carrier->getNPC(), OCC_SHOPPER)
            && IS_OBJ_STAT(obj, ITEM_INVENTORY);
}

// Called when an item is created or refreshed during area update:
// randomize rand_stat and rand_all weapons. For shops, do a periodic refresh.
void WeaponRandomizer::eventItemReset(const ItemResetEvent &event) const
{
    Object *obj = event.obj;
    int level = event.level;
    RESET_DATA *pReset = event.pReset;

    if (obj->item_type != ITEM_WEAPON)
        return;

    // Refreshing existing item.
    if (!obj->getProperty("tier").empty()) {
        if (!item_is_sold(obj))        
            return;

        if (chance(90))
            return;

        obj->getCarrier()->recho("%^C1 проводит инвентаризацию.", obj->getCarrier());
    }

    if (pReset->rand == RAND_ALL) {
        randomizeWeapon(obj, level, pReset->bestTier);
        return;
    }

    if (!obj->getProperty("random").empty() || pReset->rand == RAND_STAT) {
        randomizeWeaponStats(obj, pReset->bestTier);
        return;
    }
}

// Called when an item is read from player profile or drops:
// randomize rand_stat weapons unless done prior.
void WeaponRandomizer::eventItemRead(const ItemReadEvent &event) const
{
    Object *obj = event.obj;

    if (obj->item_type != ITEM_WEAPON)
        return;

    adjustTimer(obj);

    if (obj->getProperty("random").empty())
        return;

    if (!obj->getProperty("tier").empty())
        return;

    randomizeWeaponStats(obj);
}

// Remove 'randomness' from a weapon.
void WeaponRandomizer::clearWeapon(Object *obj) const
{
    obj->properties.clear();
    obj->enchanted = false;
    obj->affected.deallocate();
    obj->timer = 0;
    obj->setShortDescr(&str_empty[0]);
}

void WeaponRandomizer::adjustTimer(Object *obj) const
{
    if (obj->timer > 0)
        return;

    if (obj->getProperty("tier").empty())
        return;

    WeaponGenerator gen;
    gen.item(obj)
        .tier(obj->getProperty("tier").toInt())
        .assignTimers();

    if (obj->timer > 0)
        notice("rand: timer for obj %d %ld set to %d.", obj->pIndexData->vnum, obj->getID(), obj->timer);
}

void WeaponRandomizer::randomizeWeaponStats(Object *obj, int bestTierOverride) const
{
    WeaponGenerator gen;
    gen.item(obj)
        .randomTier(getTier(obj, bestTierOverride))
        .alignment(getAlign(obj));

    // Restrict or enforce certain affixes based on existing properties of this weapon.
    // TODO should iterate through weapon flags and extra flags.
    if (IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS))
        gen.addRequirement("two_hands");
    else
        gen.addForbidden("two_hands");

    if (material_is_flagged(obj, MAT_INDESTR))
        gen.addRequirement("platinum");
    else if (material_is_flagged(obj, MAT_TOUGH))
        gen.addRequirement("titanium");
    else if (material_is_typed(obj, MAT_WOOD))
        gen.addRequirement("wood");
    else if (material_is_flagged(obj, MAT_MELTING))
        gen.addRequirement("ice");
    else {
        gen.addForbidden("platinum");
        gen.addForbidden("titanium");
        gen.addForbidden("wood");
        gen.addForbidden("ice");
    }
    
    if (IS_SET(obj->extra_flags, ITEM_ANTI_EVIL))
        gen.addRequirement("anti_evil");
    
    if (IS_SET(obj->extra_flags, ITEM_ANTI_GOOD))
        gen.addRequirement("anti_good");
    
    if (IS_SET(obj->extra_flags, ITEM_EVIL))
        gen.addRequirement("evil");
    
    if (IS_SET(obj->extra_flags, ITEM_NOREMOVE))
        gen.addRequirement("noremove");

    gen.randomAffixes()
        .assignHitroll()
        .assignDamroll()
        .assignValues()
        .assignFlags()
        .assignAffects()
        .assignTimers()
        .assignColours();

    notice("rand_stat: created item [%d] [%lld] tier %s",
            obj->pIndexData->vnum, obj->getID(), 
            obj->getProperty("tier").c_str());

    Character *carrier = obj->getCarrier();
    if (carrier && !carrier->is_npc())
        carrier->pecho(POS_DEAD, "%^O1 меняет свои характеристики на случайные.", obj);
}

// Full randomize of a weapon. Most often the weapon will be a fixed "stub" item w/o any properties.
// TODO: not sure yet who makes decision about the item level.
void WeaponRandomizer::randomizeWeapon(Object *obj, int level, int bestTier) const
{
    WeaponGenerator gen;
    gen.item(obj)
        .randomTier(getTier(obj, bestTier))
        .alignment(getAlign(obj))
        .randomWeaponClass()
        .randomNames()
        .randomAffixes()
        .assignHitroll()
        .assignDamroll()
        .assignValues()
        .assignFlags()
        .assignAffects()
        .assignTimers()
        .assignNames()
        .assignDamageType()
        .assignColours();

    notice("rand_all: created item [%d] [%lld] tier %s level %d",
            obj->pIndexData->vnum, obj->getID(), 
            obj->getProperty("tier").c_str(), obj->level);
}

// NPC carrying this item can influence align restrictions, unless it's just a shop.
int WeaponRandomizer::getAlign(Object *obj) const
{
    Character *carrier = obj->getCarrier();
    if (carrier && carrier->is_npc() && !mob_has_occupation(carrier->getNPC(), OCC_SHOPPER))
        return carrier->alignment;

    return ALIGN_NONE;
}

// Calculate best random tier: 3 by default, can be overridden explicitly e.g. from
// reset tier, otherwise taken from the prototype.
int WeaponRandomizer::getTier(Object *obj, int bestTierOverride) const
{
    int tier = DEFAULT_TIER;

    if (bestTierOverride > 0)
        tier = bestTierOverride;
    else if (obj->getProperty("bestTier").isNumber())
        tier = obj->getProperty("bestTier").toInt();

    return URANGE(BEST_TIER, tier, WORST_TIER);    
}
