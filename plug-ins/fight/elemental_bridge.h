/* Hand-over from C++ combat code to the Fenia elemental engine.
 *
 * This replaces effects.cpp, which held seven ROM-era functions (acid, cold,
 * fire, poison, shock, sand, scream) that rolled a flat chance to extract_obj()
 * per carried item per proc, blind to material, hardness, weather and whether
 * the item was already red-hot. All seven now live in Fenia as
 * .tmp.mob.effectX and .tmp.room.effectX; nothing decides here any more.
 *
 * dreamland-mud, 2026
 */
#ifndef ELEMENTAL_BRIDGE_H
#define ELEMENTAL_BRIDGE_H

class Character;
class Room;

/** Element names understood by both entry points: "fire", "cold", "shock",
 *  "acid", "poison", "sand", "scream". The character entry also takes
 *  "poisoncloud" and "sandcloud" -- pick those when the victim BREATHES the
 *  effect rather than being splashed by it, because a cloud is scattered by
 *  weather and venom in a wound is not.
 *
 *  power is the 1..10 scale from Projects/Dreamland/ELEMENTAL_PHYSICS.md.
 *  The legacy calls passed (level, dam), (level/2, dam/4) and (level/4, dam/8)
 *  for their full, halved and glancing tiers; those map to 10, 6 and 4.
 *
 *  Neither entry applies damage: every caller does its own damage_nocatch.
 */

/** Contact: reaches the victim and what the victim is carrying. */
void elemental_effect(const char *element, Character *source, Character *victim, int power);

/** Area: reaches the floor items and the room itself, and can set it alight.
 *  Never use this for a weapon or a touch -- that is what the contact entry is
 *  for, and the split is deliberate.
 */
void elemental_effect_room(const char *element, Character *source, Room *room, int power);

#endif
