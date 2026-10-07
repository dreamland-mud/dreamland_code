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
 * Returns AI_UNHANDLED when no handler is registered, so annulling the Fenia
 * handler brings the C++ fallback back. Otherwise 1 if the handler acted, 0 if
 * it decided to do nothing.
 *
 * fCombat: the caller sits inside a fight round that catches
 * VictimDeathException, so a kill made by the handler keeps travelling instead
 * of being filed as a script error. Any other script error is logged.
 */
int ai_trigger(bool fCombat, const char *trigName, const char *fmt, ...);

#endif
