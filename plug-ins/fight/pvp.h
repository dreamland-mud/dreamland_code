#ifndef PVP_H
#define PVP_H

#include <time.h>

class PCharacter;
class PCMemoryInterface;

/**
 * Count a player kill toward the killer's 'pvp' attribute (clan entry rules).
 * The caller excludes pet kills. Immortals, the same account, a shared address
 * and a linkdead side don't count, and the same victim counts once per day.
 */
bool pvp_count_kill(PCharacter *killer, PCharacter *victim);

/**
 * Replay a past kill from the Fenia PK log. Same filters, except that a shared
 * address is judged by both players' lasthost history. Doesn't save.
 */
bool pvp_seed_kill(PCMemoryInterface *killer, PCMemoryInterface *victim, time_t when);

#endif
