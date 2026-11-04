#include <algorithm>
#include <cstring>
#include <fstream>
#include <map>

#include "damagelog.h"
#include "fight_extract.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "json_utils.h"
#include "dlfile.h"
#include "logstream.h"
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

struct DamageTakenEntry {
    DLString name;
    int level = 0;
    long long dam = 0;   // after armor, clipped to the hit points the player had
    long long raw = 0;   // the same blows without armor
    int hits = 0;
};

/*
 * Players' blows on the mob (byId) keep their own clock, lastDealt: raid loot
 * shares are read from byId, so the mob's own blows never reset or extend them.
 * The mob's blows on players (taken) start a fight of their own only while
 * no player has hit the mob.
 */
struct DamageLog {
    time_t start = 0;
    time_t lastDealt = 0;
    time_t lastTaken = 0;
    std::map<long long, DamageLogEntry> byId;
    std::map<long long, DamageTakenEntry> taken;
};

static std::map<long long, DamageLog> logs;

static Character * damagelog_actor( Character *ch )
{
    if (ch->is_npc( ) && IS_CHARMED(ch))
        return ch->master;
    return ch;
}

static NPCharacter * damagelog_tiered( Character *ch )
{
    NPCharacter *npc = ch ? ch->getNPC( ) : 0;
    if (!npc)
        return 0;
    // 0 = no tier set (a normal mob).
    int tier = npc->getTier( );
    if (tier < 1 || tier > DAMAGELOG_MAX_TIER)
        return 0;
    return npc;
}

/* One line per fight in which the mob touched a player; fights where it
 * never did are covered by the raid stats already. A reset row is written
 * when the next fight starts, so it carries the old fight's last blow as
 * its time and no hit points: the mob has rested since. */
static void damagelog_flush( NPCharacter *mob, DamageLog &log, const char *end )
{
    if (log.taken.empty( ))
        return;

    try {
        bool reset = !strcmp( end, "reset" );
        Json::Value row;
        row["ts"] = Json::Int64( reset ? std::max( log.lastDealt, log.lastTaken )
                                       : dreamland->getCurrentTime( ) );
        row["end"] = end;
        row["vnum"] = mob->pIndexData ? mob->pIndexData->vnum : 0;
        row["tier"] = mob->getTier( );
        row["level"] = mob->getRealLevel( );
        row["max_hit"] = (int)mob->max_hit;
        if (!reset)
            row["hit"] = (int)mob->hit;
        row["secs"] = Json::Int64( std::max( log.lastDealt, log.lastTaken ) - log.start );

        Json::Value dealt( Json::arrayValue );
        for (auto &i: log.byId) {
            Json::Value e;
            e["name"] = i.second.name;
            e["dam"] = i.second.dam;
            dealt.append( e );
        }
        row["dealt"] = dealt;

        Json::Value taken( Json::arrayValue );
        for (auto &i: log.taken) {
            Json::Value e;
            e["name"] = i.second.name;
            e["level"] = i.second.level;
            e["dam"] = Json::Int64( i.second.dam );
            e["raw"] = Json::Int64( i.second.raw );
            e["hits"] = i.second.hits;
            taken.append( e );
        }
        row["taken"] = taken;

        DLString path = DLFile( dreamland->getBasePath( ), "var/log/mob_damage.jsonl" ).getPath( );
        std::ofstream out( path.c_str( ), std::ios::app );
        if (!out) {
            LogStream::sendError( ) << "damagelog: cannot open " << path << endl;
            return;
        }
        // FastWriter already ends the line.
        out << JsonUtils::toString( row );
    } catch (const std::exception &e) {
        LogStream::sendError( ) << "damagelog: " << e.what( ) << endl;
    }
}

static void damagelog_reset( NPCharacter *mob, DamageLog &log )
{
    damagelog_flush( mob, log, "reset" );
    log.byId.clear( );
    log.taken.clear( );
}

static void damagelog_start( DamageLog &log, time_t now )
{
    if (log.byId.empty( ) && log.taken.empty( ))
        log.start = now;
}

void damagelog_record( Character *ch, Character *victim, int dam, int hpBefore )
{
    if (!ch || !victim || dam <= 0 || hpBefore <= 0)
        return;

    NPCharacter *npc = damagelog_tiered( victim );
    if (!npc)
        return;

    Character *actor = damagelog_actor( ch );
    if (!actor || actor->is_npc( ))
        return;

    time_t now = dreamland->getCurrentTime( );
    DamageLog &log = logs[victim->getID( )];

    if (!log.byId.empty( )) {
        if (now - log.lastDealt > DAMAGELOG_STALE
            || (hpBefore >= victim->max_hit && now - log.lastDealt > DAMAGELOG_REST))
            damagelog_reset( npc, log );
    }
    else if (!log.taken.empty( )) {
        if (now - log.lastTaken > DAMAGELOG_STALE
            || (hpBefore >= victim->max_hit && now - log.lastTaken > DAMAGELOG_REST))
            damagelog_reset( npc, log );
    }

    damagelog_start( log, now );
    log.lastDealt = now;

    DamageLogEntry &e = log.byId[actor->getID( )];
    if (e.name.empty( )) {
        e.id = actor->getID( );
        e.name = actor->getNameC( );
        e.dam = 0;
    }
    // Overkill does not count: only the hit points the mob actually had.
    e.dam += std::min( dam, hpBefore );
}

void damagelog_record_taken( Character *ch, Character *victim, int dam, int raw, int hpBefore )
{
    if (!ch || !victim || dam < 0 || victim->is_npc( ))
        return;

    // The mob's own blows only: a charmed tiered mob fights for its master.
    if (IS_CHARMED(ch))
        return;

    NPCharacter *npc = damagelog_tiered( ch );
    if (!npc)
        return;

    // Immortal testing and the sandbox actor are not players taking damage.
    if (victim->is_immortal( ) || sandbox_actor( victim ))
        return;

    time_t now = dreamland->getCurrentTime( );
    DamageLog &log = logs[ch->getID( )];

    // Only a fight no player has hit the mob in yet ends on the mob's side.
    if (log.byId.empty( ) && !log.taken.empty( )) {
        if (now - log.lastTaken > DAMAGELOG_STALE
            || (ch->hit >= ch->max_hit && now - log.lastTaken > DAMAGELOG_REST))
            damagelog_reset( npc, log );
    }

    damagelog_start( log, now );
    log.lastTaken = now;

    DamageTakenEntry &e = log.taken[victim->getID( )];
    if (e.name.empty( ))
        e.name = victim->getNameC( );
    e.level = victim->getModifyLevel( );
    e.dam += hpBefore > 0 ? std::min( dam, hpBefore ) : dam;
    e.raw += std::max( raw, dam );
    e.hits++;
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
    NPCharacter *npc = victim->getNPC( );
    if (!npc)
        return;

    auto i = logs.find( victim->getID( ) );
    if (i == logs.end( ))
        return;

    // setDead covers instakills too (vorpal, slay, rawKill) where hp stays up.
    bool died = victim->isDead( ) || victim->hit <= 0;
    damagelog_flush( npc, i->second, died ? "death" : "extract" );
    logs.erase( i );
}
