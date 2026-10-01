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

namespace MobBody {

/** fight/mob_forms.json. forms().loaded is false while the file is absent. */
Body::Config &forms();
/** fight/mob_tiers.json. tiers().loaded is false while the file is absent. */
MobTiers::Config &tiers();

/** Bumped on every (re)load of either config; resolved caches compare against it. */
unsigned long generation();
void touch();

}

#endif
