/* Hand-over of mob AI decisions from the C++ tick to the Fenia mob AI.
 *
 * dreamland-mud, 2026
 */
#ifndef AITRIGGER_H
#define AITRIGGER_H

/** No Fenia handler is registered: the C++ code below the call decides. */
#define AI_UNHANDLED (-1)

/**
 * Ask the Fenia mob AI to make one decision through the global trigger trigName.
 *
 * C++ keeps the cheap gates that say WHEN a mob should think (dice rolls, flags,
 * hp thresholds); the handler decides WHAT to do and runs every state check.
 * Returns AI_UNHANDLED when no handler is registered in .tmp.gtrig[trigName]
 * (an .Array(), see global/gtrig), so annulling the Fenia handler brings the C++
 * fallback back. Otherwise 1 if the handler acted, 0 if it decided to do
 * nothing. A handler that returns nothing counts as 0 and still owns the
 * decision, and so does one that throws: no C++ fallback runs on top of what it
 * may already have done.
 *
 * mob is the deciding mob. If the handler extracted it (it lost its room or its
 * ID changed), the result is 1 whatever the handler returned, so the C++ caller
 * stops instead of reading its room. The caller must return at once then: the
 * behavior has lost ch and may already be freed.
 * After a 0 the same handler may be asked again in the same pulse with another
 * kind (assist master -> offense, aggress lastFought -> memorized -> normal):
 * handlers check mob.fighting and every other state themselves. A try/catch in
 * a handler swallows VictimDeathException like in any Fenia trigger, so a kill
 * inside it no longer stops the fight round. track_update does not skip mobs
 * without a room, so onTrackAI must check mob.in_room.
 *
 * fCombat: the caller sits inside a fight round that catches
 * VictimDeathException, so a kill made by the handler keeps travelling instead
 * of being filed as a script error. Any other script error is logged.
 */
class Character;

int ai_trigger(bool fCombat, Character *mob, const char *trigName, const char *fmt, ...);

#endif
