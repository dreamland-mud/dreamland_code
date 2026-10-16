#ifndef DAMAGELOG_H
#define DAMAGELOG_H

#include <list>
#include "dlstring.h"

class Character;

/*
 * Who took how many hit points off a tiered mob (champion, boss, legend).
 * Read by Fenia on the mob's death for raid loot shares and raid stats.
 * In memory only: a fight does not outlive a reboot or a fight_core reload.
 */
struct DamageLogEntry {
    long long id;   // player ID; a charmed pet's damage counts for its master
    DLString name;
    int dam;
};

void damagelog_record( Character *ch, Character *victim, int dam, int hpBefore );
std::list<DamageLogEntry> damagelog_get( Character *victim );
void damagelog_forget( Character *victim );

#endif
