/*
 * MSP ambience: looping background sound for every player with sound on.
 *
 * Once a second each playing connection gets its set of layers (weather,
 * city by day or night, birds or crickets, desert, air, cave, dungeon,
 * stream, sea, underwater, campfire) and its fight music recomputed, and only
 * a change goes out. Music is music/boss against a tier 1-3 NPC and
 * music/fight in any other fight; boss wins when both apply. The web client
 * mixes every layer and takes them as one 'ambience' frame; a telnet MSP
 * client has a single music channel, so it gets one !!MUSIC track picked by
 * priority, music first.
 */
#include <map>
#include <string.h>
#include <jsoncpp/json/json.h>

#include "update.h"
#include "weather.h"
#include "msp.h"
#include "descriptor.h"
#include "character.h"
#include "npcharacter.h"
#include "room.h"
#include "roomutils.h"
#include "move_utils.h"
#include "dreamland.h"
#include "merc.h"
#include "def.h"

namespace {

enum {
    LAYER_AIR         = 1 << 0,
    LAYER_BIRDS       = 1 << 1,
    LAYER_CAVE        = 1 << 2,
    LAYER_CITY        = 1 << 3,
    LAYER_CITY_NIGHT  = 1 << 4,
    LAYER_CRICKETS    = 1 << 5,
    LAYER_DESERT      = 1 << 6,
    LAYER_DUNGEON     = 1 << 7,
    LAYER_FIRE        = 1 << 8,
    LAYER_RAIN        = 1 << 9,
    LAYER_SEA         = 1 << 10,
    LAYER_STREAM      = 1 << 11,
    LAYER_THUNDER     = 1 << 12,
    LAYER_UNDERWATER  = 1 << 13,
    LAYER_WIND        = 1 << 14,
    LAYER_WIND_FOREST = 1 << 15,
};

struct LayerName {
    int bit;
    const char *name;
};

/* Alphabetical, so the web frame's list comes out sorted. */
const LayerName layerNames[] = {
    { LAYER_AIR,         "ambient/air" },
    { LAYER_BIRDS,       "ambient/birds" },
    { LAYER_CAVE,        "ambient/cave" },
    { LAYER_CITY,        "ambient/city" },
    { LAYER_CITY_NIGHT,  "ambient/city_night" },
    { LAYER_CRICKETS,    "ambient/crickets" },
    { LAYER_DESERT,      "ambient/desert" },
    { LAYER_DUNGEON,     "ambient/dungeon" },
    { LAYER_FIRE,        "ambient/fire" },
    { LAYER_RAIN,        "ambient/rain" },
    { LAYER_SEA,         "ambient/sea" },
    { LAYER_STREAM,      "ambient/stream" },
    { LAYER_THUNDER,     "ambient/thunder" },
    { LAYER_UNDERWATER,  "ambient/underwater" },
    { LAYER_WIND,        "ambient/wind" },
    { LAYER_WIND_FOREST, "ambient/wind_forest" },
};

/* Telnet's single channel plays the layer that says the most about where the
 * player is. Thunder is a crack over the rain loop, meaningless alone. */
const LayerName telnetOrder[] = {
    { LAYER_RAIN,        "ambient/rain" },
    { LAYER_UNDERWATER,  "ambient/underwater" },
    { LAYER_CITY,        "ambient/city" },
    { LAYER_CITY_NIGHT,  "ambient/city_night" },
    { LAYER_CAVE,        "ambient/cave" },
    { LAYER_DUNGEON,     "ambient/dungeon" },
    { LAYER_SEA,         "ambient/sea" },
    { LAYER_STREAM,      "ambient/stream" },
    { LAYER_AIR,         "ambient/air" },
    { LAYER_DESERT,      "ambient/desert" },
    { LAYER_WIND_FOREST, "ambient/wind_forest" },
    { LAYER_WIND,        "ambient/wind" },
    { LAYER_BIRDS,       "ambient/birds" },
    { LAYER_CRICKETS,    "ambient/crickets" },
    { LAYER_FIRE,        "ambient/fire" },
};

const char *BOSS_MUSIC = "music/boss";
const char *FIGHT_MUSIC = "music/fight";
const char *NO_MUSIC = "";

/* Seconds a track outlives its fight: a boss that stops for a round
 * (a flee, a re-target) should not cut the track off and restart it. */
const time_t FIGHT_HOLD = 6;

/* Nothing sent yet: differs from every real layer mask, so the first pass
 * after a connect, a handshake or a plugin reload always sends. */
const int MASK_UNKNOWN = -1;

struct AmbienceState {
    int mask;           // layers last sent
    const char *music;  // track last sent, one of the *_MUSIC constants
    time_t bossUntil;   // boss music holds until then
    time_t fightUntil;  // fight music holds until then
    unsigned long pass; // last pass that saw this connection eligible
};

std::map<Descriptor *, AmbienceState> states;
unsigned long passCounter = 0;

/* Legend, boss and champion: the fights that earn their own music. */
bool is_music_tier( Character *c )
{
    NPCharacter *npc = c->getNPC( );
    if (!npc)
        return false;

    int tier = npc->getTier( );
    return tier >= 1 && tier <= 3;
}

bool in_boss_fight( Character *ch )
{
    if (ch->fighting && is_music_tier( ch->fighting ))
        return true;

    // A boss beating on someone who hasn't swung back yet.
    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room)
        if (rch->fighting == ch && is_music_tier( rch ))
            return true;

    return false;
}

bool in_any_fight( Character *ch )
{
    if (ch->fighting)
        return true;

    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room)
        if (rch->fighting == ch)
            return true;

    return false;
}

int compute_layers( Character *ch, bool winter )
{
    Room *room = ch->in_room;
    int sector = room->getSectorType( );
    bool outdoors = RoomUtils::isOutside( ch ) && !IS_SET( room->room_flags, ROOM_NO_WEATHER );
    int wind = sector == SECT_FOREST ? LAYER_WIND_FOREST : LAYER_WIND;
    bool day = weather_info.sunlight == SUN_LIGHT || weather_info.sunlight == SUN_RISE;
    int mask = 0;

    if (outdoors) {
        int sky = weather_info.sky;
        bool dry = sky == SKY_CLOUDLESS || sky == SKY_CLOUDY;
        bool wild = sector == SECT_FIELD || sector == SECT_FOREST || sector == SECT_HILLS;

        // Winter precipitation is snow, which howls rather than patters.
        if (sky == SKY_RAINING || sky == SKY_LIGHTNING)
            mask |= winter ? wind : LAYER_RAIN;
        if (sky == SKY_LIGHTNING)
            mask |= LAYER_THUNDER;
        if (sky == SKY_CLOUDY)
            mask |= wind;
        if (sector == SECT_CITY)
            mask |= day ? LAYER_CITY : LAYER_CITY_NIGHT;
        if (wild && dry && !winter)
            mask |= day ? LAYER_BIRDS : LAYER_CRICKETS;
        if (sector == SECT_DESERT)
            mask |= LAYER_DESERT;
    }
    else if (sector == SECT_MOUNTAIN) {
        mask |= LAYER_CAVE;
    }
    else if (IS_SET( room->room_flags, ROOM_DARK ) && sector != SECT_CITY) {
        mask |= LAYER_DUNGEON;
    }

    if (sector == SECT_AIR)
        mask |= LAYER_AIR;
    if (sector == SECT_UNDERWATER)
        mask |= LAYER_UNDERWATER;
    if (sector == SECT_WATER_SWIM && !is_flying( ch ))
        mask |= LAYER_STREAM;
    if (sector == SECT_WATER_NOSWIM && !is_flying( ch ))
        mask |= LAYER_SEA;

    // A campfire for a willing rest; magic sleep is not a cosy evening.
    if ((ch->position == POS_RESTING || ch->position == POS_SLEEPING) && !IS_AFFECTED( ch, AFF_SLEEP ))
        mask |= LAYER_FIRE;

    return mask;
}

const char * telnet_track( int mask, const char *music )
{
    if (*music)
        return music;

    for (const LayerName &l: telnetOrder)
        if (mask & l.bit)
            return l.name;

    return "";
}

void send_web( Descriptor *d, int mask, const char *music )
{
    Json::Value frame;
    Json::Value list( Json::arrayValue );

    for (const LayerName &l: layerNames)
        if (mask & l.bit)
            list.append( l.name );

    frame["command"] = "ambience";
    frame["args"][0] = list;
    frame["args"][1] = music;
    d->writeWSCommand( frame );
}

void send_telnet( Descriptor *d, const AmbienceState &old, int mask, const char *music )
{
    const char *track = telnet_track( mask, music );

    // A change among the layers telnet cannot play is no change for it.
    if (old.mask != MASK_UNKNOWN && strcmp( track, telnet_track( old.mask, old.music ) ) == 0)
        return;

    msp_music( d, track );
}

}

void ambience_update( )
{
    bool winter = season_is_winter( );
    time_t now = dreamland->getCurrentTime( );

    passCounter++;

    for (Descriptor *d = descriptor_list; d != 0; d = d->next) {
        if (d->connected != CON_PLAYING || !msp_enabled( d ))
            continue;

        Character *ch = d->character;
        if (!ch || !ch->in_room)
            continue;

        if (msp_ambience_take_reset( d ))
            states.erase( d );

        std::map<Descriptor *, AmbienceState>::iterator it = states.find( d );
        if (it == states.end( )) {
            AmbienceState fresh = { MASK_UNKNOWN, NO_MUSIC, 0, 0, 0 };
            it = states.insert( std::make_pair( d, fresh ) ).first;
        }

        AmbienceState &st = it->second;
        st.pass = passCounter;

        if (in_any_fight( ch ))
            st.fightUntil = now + FIGHT_HOLD;
        if (in_boss_fight( ch ))
            st.bossUntil = now + FIGHT_HOLD;

        int mask = compute_layers( ch, winter );
        const char *music = NO_MUSIC;
        if (now < st.bossUntil)
            music = BOSS_MUSIC;
        else if (now < st.fightUntil)
            music = FIGHT_MUSIC;

        // Music always holds one of the constants above, so pointers compare.
        if (mask == st.mask && music == st.music)
            continue;

        if (d->websock.state == WS_ESTABLISHED)
            send_web( d, mask, music );
        else
            send_telnet( d, st, mask, music );

        st.mask = mask;
        st.music = music;
    }

    // Closed connections, linkless characters and clients that turned MSP
    // off: forget them, so a return starts from "nothing sent".
    for (std::map<Descriptor *, AmbienceState>::iterator it = states.begin( ); it != states.end( ); )
        if (it->second.pass != passCounter)
            it = states.erase( it );
        else
            ++it;
}
