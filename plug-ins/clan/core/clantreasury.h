#ifndef CLANTREASURY_H
#define CLANTREASURY_H

#include "dlstring.h"

class Clan;
class PCMemoryInterface;

/** Treasury gold cap of a reformed clan. */
int clan_gold_cap( const Clan &clan );

/**
 * Change the treasury by these deltas, all or nothing, and save. Refuses a negative
 * balance and an overflow; in a reformed clan also gold above the cap and any silver
 * deposit. Returns a refusal reason, empty on success.
 */
DLString clan_bank_add( Clan &clan, int gold, int silver, int qp );

/** Rank tables of a reformed clan (rank 0-8), with defaults when the clan XML has none. */
int clan_rank_cap( const Clan &clan, int rank );
int clan_rank_level_bonus( const Clan &clan, int rank );
int clan_rank_power( const Clan &clan, int rank );

/** Has the clan bought this catalog item. */
bool clan_owns( Clan &clan, const DLString &id );

/**
 * Buy a catalog item with treasury qp: must exist, not be owned, have its prerequisite
 * owned and be affordable. Records buyer and price, saves. Returns a refusal reason.
 */
DLString clan_purchase( Clan &clan, const DLString &id, const DLString &buyer );

/**
 * Donate qp from the player to their reformed clan. All of it lands in the treasury;
 * it also pays toward donation ranks 1-4 (1k/2k/4k/8k per step, progress carries over).
 * Saves both. Returns a refusal reason, empty on success.
 */
DLString clan_donate( PCMemoryInterface *pcm, int qp );

#endif
