#ifndef MAGICITEMGENERATOR_H
#define MAGICITEMGENERATOR_H

#include <vector>
#include <jsoncpp/json/json.h>
#include "dlstring.h"
#include "eventbus.h"
#include "plugin.h"
#include "oneallocate.h"

class Object;
class Character;
class ItemResetEvent;

/** The whole fight/magic_items.json (generator data, built by scripts/magic-items). */
const Json::Value & magic_items_config();

/** A spell the mastering staff never autocasts in melee ('blocked' + 'autocast_blocked'). */
bool magic_item_autocast_blocked(const DLString &spell);

/** One charge of a spell in M, from fight/spell_value.json; negative when unpriced. */
double magic_spell_charge(const DLString &spell);

/** A kind of fight/magic_items.json ("potion", "wand_off"...), or a family
 *  ("scroll", "wand", "staff") that rolls one of its kinds. */
bool magic_item_kind_exists(const DLString &kind);

/**
 * Random pills, potions, scrolls and charged trinkets (magic_items_revamp.md D1).
 * Spells come from the kind's pool, priced by their charge in spell_value.json,
 * and fill the tier's M window: consumables sum their spells, a trinket is one
 * spell times its charges. The item's level must be set, or is set by level().
 */
class MagicItemGenerator {
public:
    MagicItemGenerator(Object *obj, Character *ch, int tier, const DLString &kind);

    MagicItemGenerator & level(int level);
    MagicItemGenerator & alignment(int align);

    bool run();

    /** What the spells cost, in centi-M. */
    int total() const { return chosenTotal; }

private:
    struct Pick {
        DLString spell;
        int sn;
        double price;
    };

    bool roll();
    bool collect(std::vector<Pick> &cands) const;
    bool pickConsumable(const std::vector<Pick> &cands, std::vector<Pick> &picks) const;
    bool pickCharged(const std::vector<Pick> &cands, Pick &pick, int &charges) const;
    void assignNames(const std::vector<Pick> &picks);
    void assignFlags();

    Object *obj;
    Character *ch;
    int tier;
    DLString kind;
    int align;
    bool hasAlign;
    int chosenTotal;
    const Json::Value *kindConfig;
    const Json::Value *tierConfig;
};

/**
 * Rolls a reset stub that carries props "random magic" (the kind) when the
 * area resets it: reset bestTier, level from the carrier or the area window.
 */
class MagicItemRandomizer : public Plugin, public EventHandler, public OneAllocate {
public:
    typedef ::Pointer<MagicItemRandomizer> Pointer;

    MagicItemRandomizer();
    virtual ~MagicItemRandomizer();
    virtual void initialization();
    virtual void destruction();
    virtual void handleEvent(const type_index &eventType, const Event &event) const;

protected:
    static MagicItemRandomizer *thisClass;

    void eventItemReset(const ItemResetEvent &event) const;
};

#endif
