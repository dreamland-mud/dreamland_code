#ifndef DAMAGELOG_H
#define DAMAGELOG_H

#include <list>
#include "dlstring.h"

class Character;

/*
 * Who took how many hit points off a tiered mob (champion, boss, legend).
 * Read by Fenia on the mob's death for raid loot shares and raid stats.
 * In memory only: a fight does not outlive a reboot or a fight_core reload.
 *
 * The same fight also keeps what the mob dealt to each player, before and
 * after armor. When a fight in which the mob touched a player ends (death,
 * extract, or the next fight replacing a stale one), both sides go to
 * var/log/mob_damage.jsonl, one line per fight, for tuning the tier numbers
 * against what players actually take. A fight still open at a reboot or a
 * fight_core reload is lost.
 */
struct DamageLogEntry {
    long long id;   // player ID; a charmed pet's damage counts for its master
    DLString name;
    int dam;
};

void damagelog_record( Character *ch, Character *victim, int dam, int hpBefore );
void damagelog_record_taken( Character *ch, Character *victim, int dam, int raw, int hpBefore );
std::list<DamageLogEntry> damagelog_get( Character *victim );
void damagelog_forget( Character *victim );

#endif
