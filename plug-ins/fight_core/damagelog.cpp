#include <algorithm>
#include <map>

#include "damagelog.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "dreamland.h"
#include "loadsave.h"
#include "merc.h"
#include "def.h"

// Tiers 1-3 (legend, boss, champion) carry a damage log: the ones with tier loot.
static const int DAMAGELOG_MAX_TIER = 3;
// A log untouched this long belongs to an old fight and starts over.
static const int DAMAGELOG_STALE = 30 * 60;
// A mob back at full health that nobody hit for this long starts a new fight.
static const int DAMAGELOG_REST = 60;

struct DamageLog {
    time_t last;
    std::map<long long, DamageLogEntry> byId;
};

static std::map<long long, DamageLog> logs;

static Character * damagelog_actor( Character *ch )
{
    if (ch->is_npc( ) && IS_CHARMED(ch))
        return ch->master;
    return ch;
}

void damagelog_record( Character *ch, Character *victim, int dam, int hpBefore )
{
    if (!ch || !victim || dam <= 0 || hpBefore <= 0)
        return;

    NPCharacter *npc = victim->getNPC( );
    if (!npc)
        return;
    // 0 = no tier set (a normal mob).
    int tier = npc->getTier( );
    if (tier < 1 || tier > DAMAGELOG_MAX_TIER)
        return;

    Character *actor = damagelog_actor( ch );
    if (!actor || actor->is_npc( ))
        return;

    time_t now = dreamland->getCurrentTime( );
    DamageLog &log = logs[victim->getID( )];

    if (!log.byId.empty( )) {
        if (now - log.last > DAMAGELOG_STALE
            || (hpBefore >= victim->max_hit && now - log.last > DAMAGELOG_REST))
            log.byId.clear( );
    }
    log.last = now;

    DamageLogEntry &e = log.byId[actor->getID( )];
    if (e.name.empty( )) {
        e.id = actor->getID( );
        e.name = actor->getNameC( );
        e.dam = 0;
    }
    // Overkill does not count: only the hit points the mob actually had.
    e.dam += std::min( dam, hpBefore );
}

std::list<DamageLogEntry> damagelog_get( Character *victim )
{
    std::list<DamageLogEntry> result;
    auto i = logs.find( victim->getID( ) );
    if (i == logs.end( ))
        return result;

    for (auto &e: i->second.byId)
        result.push_back( e.second );
    return result;
}

void damagelog_forget( Character *victim )
{
    if (victim->is_npc( ))
        logs.erase( victim->getID( ) );
}
