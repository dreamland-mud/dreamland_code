/*
 * Mob reform: engine side of the body model and the tier curve.
 *
 * Holds the two world configs (fight/mob_forms.json, fight/mob_tiers.json,
 * loaded by the race plugin) and converts between the name-based resolver
 * (body.h) and the engine's bit tables. Both configs are optional: while one is
 * absent the engine keeps today's behaviour for whatever depends on it
 * (legacy race bodies, authored numbers).
 */
#ifndef MOBBODY_H
#define MOBBODY_H

#include "body.h"
#include "mobtiers.h"
#include "bitstring.h"

struct FlagTable;

/** True only while fight/mob_forms.json is loaded (see merc.h). */
extern bool mob_body_model_active;
class GlobalBitvector;

namespace MobBody {

/** fight/mob_forms.json. forms().loaded is false while the file is absent. */
Body::Config &forms();
/** fight/mob_tiers.json. tiers().loaded is false while the file is absent. */
MobTiers::Config &tiers();

/** Bumped on every (re)load of either config; resolved caches compare against it. */
unsigned long generation();
void touch();

/** Engine bit table of a resolver bit set (act_flags, off_flags, ...). */
const FlagTable *bitSetTable(int bitSet);

/** Bits -> resolver names. */
Body::NameSet names(const FlagTable *table, bitstring_t bits);
/** Resolver names -> bits. A name the table lacks is logged once and skipped. */
bitstring_t bits(const FlagTable *table, const Body::NameSet &names);
/** Resolver wearloc names -> wearloc bitvector (names are reserved in the registry like XML does). */
void wearlocs(const Body::NameSet &names, GlobalBitvector &out);

/** A resolved body in engine terms. */
struct Engine {
    bitstring_t form = 0, parts = 0;
    bitstring_t bits[Body::BS_MAX] = { 0, 0, 0, 0, 0, 0, 0 };
    Body::NameSet wearlocNames;
    std::string movetype, moveverb, movedanger;
    double acFactor = 3.0, formAc = 1.0;
    bool edible = true, bloodless = false, canHoldCards = false, rideable = false;

    void fromResult(const Body::Result &r);
};

}

#endif
