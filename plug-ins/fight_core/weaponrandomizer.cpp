#include "weaponrandomizer.h"
#include "logstream.h"
#include "npcharacter.h"
#include "core/object.h"
#include "room.h"
#include "loadsave.h"
#include "itemevents.h"
#include "occupations.h"
#include "material.h"
#include "damageflags.h"
#include "weapongenerator.h"
#include "armorgenerator.h"
#include "affectflags.h"
#include "weapontier.h"
#include "merc.h"
#include "json_utils_ext.h"

#include "vnum.h"
#include "def.h"
#include "l10n.h"

WeaponRandomizer * WeaponRandomizer::thisClass = 0;

WeaponRandomizer::WeaponRandomizer() 
{
    checkDuplicate(thisClass);
    thisClass = this;
}

WeaponRandomizer::~WeaponRandomizer() 
{
    thisClass = 0;
}

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

    if (IS_SET(obj->pIndexData->area->area_flag, AREA_SYSTEM))
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
    RESET_DATA *pReset = event.pReset;

    if (obj->item_type == ITEM_ARMOR && item_is_random_armor(obj->pIndexData)) {
        if (obj->getProperty("tier").empty())
            randomizeArmorStats(obj, pReset->bestTier);
        return;
    }

    if (obj->item_type != ITEM_WEAPON)
        return;

    // Refreshing existing item.
    if (!obj->getProperty("tier").empty()) {
        if (!item_is_sold(obj))        
            return;

        if (chance(90))
            return;

        obj->getCarrier()->recho(_("%^C1 проводит инвентаризацию."), obj->getCarrier());
        clearWeapon(obj);
    }

    if (pReset->rand == RAND_ALL) {
        randomizeWeapon(obj, pReset->bestTier);
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

    // Before the early returns below: an already-generated weapon carries 'tier'
    // but not 'random', so a repair placed after them would never see one.
    weapon_repair_names(obj);

    if (obj->getProperty("random").empty())
        return;

    // Converted hand-made items (Kit 2026-10-08): copies players already own keep
    // the stats they had; only fresh resets roll.
    if (obj->getProperty("keepOld") == "true")
        return;

    if (!obj->getProperty("tier").empty())
        return;

    randomizeWeaponStats(obj);
}

// Remove 'randomness' from a weapon.
void WeaponRandomizer::clearWeapon(Object *obj) const
{
    obj->props.clear();
    obj->affected.deallocate();
    obj->timer = 0;
    obj->level = obj->pIndexData->level;
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

    gen.randomizeStats();

    Character *carrier = obj->getCarrier();
    if (carrier && !carrier->is_npc())
        carrier->pecho(POS_DEAD, _("%^O1 меняет свои характеристики на случайные."), obj);
}

// Hand-made armor with the 'random armor' behavior: own names, rolled stats.
// No killer on a reset: the profile leans caster when the prototype's own stats do.
void WeaponRandomizer::randomizeArmorStats(Object *obj, int bestTierOverride) const
{
    DLString slot = armorSlot(obj);
    if (slot.empty())
        return;

    ArmorGenerator(obj, 0, random_weapon_tier(getTier(obj, bestTierOverride)), slot)
        .keepIdentity(true)
        .caster(armorLeansCaster(obj))
        .alignment(getAlign(obj))
        .run();
}

// The measure slot of a hand-made armor piece, from its wear flags.
DLString WeaponRandomizer::armorSlot(Object *obj) const
{
    static const struct { int bit; const char *slot; } slots[] = {
        { ITEM_WEAR_SHIELD, "shield" }, { ITEM_WEAR_HEAD, "head" }, { ITEM_WEAR_BODY, "body" },
        { ITEM_WEAR_ARMS, "arms" }, { ITEM_WEAR_HANDS, "hands" }, { ITEM_WEAR_LEGS, "legs" },
        { ITEM_WEAR_FEET, "feet" }, { ITEM_WEAR_WAIST, "waist" }, { ITEM_WEAR_NECK, "neck" },
        { ITEM_WEAR_FINGER, "finger" }, { ITEM_WEAR_WRIST, "wrist" }, { ITEM_WEAR_ABOUT, "about" },
        { ITEM_WEAR_FLOAT, "float" }, { ITEM_WEAR_FACE, "face" }, { ITEM_WEAR_EARS, "ears" },
        { ITEM_HOLD, "hold" },
    };

    for (auto &s: slots)
        if (IS_SET(obj->wear_flags, s.bit))
            return s.slot;

    return DLString::emptyString;
}

bool WeaponRandomizer::armorLeansCaster(Object *obj) const
{
    int caster = 0, melee = 0;
    for (auto &paf: obj->pIndexData->affected) {
        switch (paf->location.getValue()) {
        case APPLY_MANA: case APPLY_MANA_GAIN: case APPLY_INT: case APPLY_WIS:
            if (paf->modifier > 0) caster++;
            break;
        case APPLY_HITROLL: case APPLY_DAMROLL: case APPLY_STR: case APPLY_DEX:
            if (paf->modifier > 0) melee++;
            break;
        }
    }

    if (caster != melee)
        return caster > melee;

    return number_range(1, 3) == 1;
}

// Full randomize of a weapon. Most often the weapon will be a fixed "stub" item w/o any properties.
void WeaponRandomizer::randomizeWeapon(Object *obj, int bestTier) const
{
    obj->level = getLevel(obj);

    WeaponGenerator()
        .item(obj)
        .randomTier(getTier(obj, bestTier))
        .alignment(getAlign(obj))
        .randomizeAll();
}

// NPC carrying this item can influence align restrictions, unless it's just a shop.
int WeaponRandomizer::getAlign(Object *obj) const
{
    Character *carrier = obj->getCarrier();
    if (carrier && carrier->is_npc() && !mob_has_occupation(carrier->getNPC(), OCC_SHOPPER))
        return carrier->alignment;

    return ALIGN_NONE;
}

// Ensure rand_all weapons always have a level assigned: random one for shops or chests,
// mob's level for equip/inv resets.
int WeaponRandomizer::getLevel(Object *obj) const
{
    if (obj->level > 0)
        return obj->level;

    if (item_is_sold(obj))
        return number_range(1, LEVEL_MORTAL);
    
    Character *carrier = obj->getCarrier();
    if (carrier)
        return carrier->getRealLevel();

    return number_range(1, LEVEL_MORTAL);    
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
