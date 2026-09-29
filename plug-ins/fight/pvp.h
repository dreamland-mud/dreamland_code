#ifndef PVP_H
#define PVP_H

#include <time.h>

class PCharacter;
class PCMemoryInterface;

/**
 * Count a player kill toward the killer's 'pvp' attribute (clan entry rules).
 * The caller excludes pet kills. Immortals, the same account, a linkdead side,
 * the same current address or any shared address in the lasthost history don't
 * count, and the same victim counts once per day.
 */
bool pvp_count_kill(PCharacter *killer, PCharacter *victim);

/**
 * Replay a past kill from the Fenia PK log: immortal, account and lasthost
 * history filters only. The log can't tell pet kills or linkdead victims apart,
 * so those are accepted here on purpose. Replay in time order. Doesn't save.
 */
bool pvp_seed_kill(PCMemoryInterface *killer, PCMemoryInterface *victim, time_t when);

#endif
