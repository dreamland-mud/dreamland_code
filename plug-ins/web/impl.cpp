/* $Id$
 *
 * ruffina, 2018
 */
#include "webprompt.h"
#include "webpromptattribute.h"
#include "logstream.h"
#include "schedulertaskroundplugin.h"
#include "dlscheduler.h"
#include "xmlattributeticker.h"
#include "xmlattributeplugin.h"
#include "json/json.h"
#include "iconvmap.h"
#include "descriptor.h"
#include "descriptorstatelistener.h"
#include "quest.h"
#include "commandmanager.h"

#include "grammar_entities_impl.h"
#include "dlfilestream.h"
#include "dldirectory.h"
#include "dreamland.h"
#include "bitstring.h"
#include "clanreference.h"
#include "affect.h"
#include "room.h"
#include "pcharacter.h"
#include "follow_utils.h"
#include "gsn_plugin.h"
#include "directions.h"
#include "handler.h"
#include "weather.h"
#include "stats_apply.h"
#include "dl_math.h"
#include "dl_ctype.h"
#include "mudtags.h"
#include "merc.h"
#include "autoflags.h"
#include "so.h"
#include "def.h"

static const DLString NONE = "none";
static const DLString EXITS = "exits";
static const DLString ROOM = "room";
static const DLString ZONE = "zone";
static const DLString GROUP = "group";
static const DLString TIME = "time";
static const DLString DATE = "date";
static const DLString WEATHER = "w";
static const DLString DETECT = "det";
static const DLString ENHANCE = "enh";
static const DLString PROTECT = "pro";
static const DLString TRAVEL = "trv";
static const DLString MALAD = "mal";
static const DLString CLAN = "cln";
static const DLString WHO = "who";
static const DLString PARAM1 = "p1";
static const DLString PARAM2 = "p2";
static const DLString PERM_STAT = "ps";
static const DLString CURR_STAT = "cs";
static const DLString QUESTOR = "q";

GSN(protection_negative);
GSN(stardust);
GSN(spell_resistance);
GSN(meld_into_stone);
GSN(rainbow_shield);
GSN(demonic_mantle);
GSN(magic_jar);
GSN(anathema);
GSN(stuck_arrow);
GSN(detect_trap);
GSN(evolve_lion);
GSN(prevent);
GSN(holy_armor);
GSN(shadow_shroud);
GSN(soul_lust);
GSN(randomizer);
GSN(ruler_aura);
CLAN(none);

static IconvMap koi2utf("koi8-r", "utf-8");

static string json_to_string( const Json::Value &value )
{
    Json::FastWriter writer;
    return writer.write( value );
}    

static void json_update_if_new( const DLString &field, const Json::Value &newValue, WebPromptAttribute::Pointer &attr, Json::Value &prompt )
{
    // First time a value disappears, we need to send "none" to front-end to hide corresponding row.
    // Subsequent calls can just omit the value from prompt, until it's back again.
    if (newValue.empty( )) {
        if (!attr->prompt.isAvailable( field ) || !attr->prompt[field].empty( )) {
            prompt[field] = NONE;
        }
        attr->prompt[field] = DLString::emptyString;
        return;
    }

    // Serialize new value to string and see if it differs from the stored value.
    DLString value = json_to_string( newValue );
    if (!attr->prompt.isAvailable( field ) || attr->prompt[field] != value) {
        // First occurence or changed value, update it in the attribute and send to front-end.
        attr->prompt[field] = value;
        prompt[field] = newValue;
        return;
    }

    // Nothing changed, send nothing in the prompt.
}

/*-------------------------------------------------------------------------
 * WhoWebPromptListener
 *------------------------------------------------------------------------*/
class WhoWebPromptListener : public WebPromptListener {
public:
        typedef ::Pointer<WhoWebPromptListener> Pointer;

        virtual void run( Descriptor *, Character *, Json::Value &json );

protected:
        static Json::Value jsonPlayer( Character *ch, PCharacter *wch );
        static Json::Value jsonWho( Character *ch );
};

void WhoWebPromptListener::run( Descriptor *d, Character *ch, Json::Value &json )
{
    WebPromptAttribute::Pointer attr = ch->getPC( )->getAttributes( ).getAttr<WebPromptAttribute>( "webprompt" );
    Json::Value &prompt = json["args"][0];

    json_update_if_new( WHO, jsonWho( ch ), attr, prompt ); 
}    

Json::Value WhoWebPromptListener::jsonPlayer( Character *ch, PCharacter *wch )
{
    Json::Value player;
    
    // Player name and sex.
    player["n"] = wch->toNoun(ch)->decline('1').cutSize(10);
    player["s"] = wch->getSex( ) == SEX_MALE ? "m" : "f";

    // First 2 letters of player race.
    player["r"] = wch->getRace( )->getName( ).substr(0, 2);

    // Clan name (first letter) and colour.
    player["cn"] = wch->getClan( )->getName( ).substr(0, 1);
    
    DLString clr = wch->getClan( )->getColor( );
    if (!clr.empty( )) {
        player["cc"] = DLString(dl_isupper(clr.at(0)) ? "b" : "d") + dl_tolower(clr.at(0));
    }

    return player;
}

Json::Value WhoWebPromptListener::jsonWho( Character *ch )
{
    Json::Value who;
    list<PCharacter *> players;
    list<PCharacter *>::iterator p;

    // Find all visible players.
    for (Descriptor *d = descriptor_list; d; d = d->next) {
        PCharacter *victim;
        
        if (d->connected != CON_PLAYING || !d->character)
            continue;

        victim = d->character->getPC( );

        if (!can_see_god(ch, victim)) 
            continue;

        XMLAttributes *attrs = &victim->getAttributes( );
        if (attrs->isAvailable("nowho"))
            continue;
        
        players.push_back( victim );
    }

    // Populate player list.
    int pc = 0;
    for (p = players.begin( ); p != players.end( ); p++) {
        who["p"][pc++] = jsonPlayer( ch, *p );
    }

    // Total player count.
    who["t"] = DLString(players.size());
    return who;
}

/*-------------------------------------------------------------------------
 * GroupWebPromptListener
 *------------------------------------------------------------------------*/
class GroupWebPromptListener : public WebPromptListener {
public:
        typedef ::Pointer<GroupWebPromptListener> Pointer;

        virtual void run( Descriptor *, Character *, Json::Value &json );

protected:
        static Json::Value jsonGroupMember( Character *ch, Character *gch );
        static Json::Value jsonGroup( Character *ch );
};

void GroupWebPromptListener::run( Descriptor *d, Character *ch, Json::Value &json )
{
    WebPromptAttribute::Pointer attr = ch->getPC( )->getAttributes( ).getAttr<WebPromptAttribute>( "webprompt" );
    Json::Value &prompt = json["args"][0];

    json_update_if_new( GROUP, jsonGroup( ch ), attr, prompt ); 
}    

Json::Value GroupWebPromptListener::jsonGroupMember( Character *ch, Character *gch )
{
    Json::Value json;
    int hit = HEALTH(gch);
    DLString hit_clr;

    if (hit >= 100) hit_clr = "6";
    else if (hit >= 75) hit_clr = "4";
    else if (hit >= 50) hit_clr = "2";
    else if (hit >= 30) hit_clr = "3";
    else if (hit >= 15) hit_clr = "5";
    else  hit_clr = "1";

    DLString sees = ch->sees( gch, '1' ).colourStrip( );
    sees.cutSize( 10 );
    json["sees"] = sees;
    json["hit"] = gch->hit.getValue( );
    json["max_hit"] = gch->max_hit.getValue( );
    json["health"] = hit;
    json["hit_clr"] = hit_clr;
    json["level"] = gch->getRealLevel( );

    if (gch->is_npc( ))
        json["tnl"] = "";
    else
        json["tnl"] = gch->getPC( )->getExpToLevel( );

    return json;
}

Json::Value GroupWebPromptListener::jsonGroup( Character *ch )
{
    Json::Value group;
    Character *leader;
    list<Character *> players;
    list<Character *> mobs;
    list<Character *>::iterator p, m;

    leader = (ch->leader != 0) ? ch->leader : ch;

    for (Character *gch = char_list; gch != 0; gch = gch->next )
        if (is_same_group( gch, ch )) {
                if (gch->is_npc( ))
                    mobs.push_back( gch );
                else if (gch != leader)
                    players.push_back( gch );
        }

    int pc = 0;
    int npc = 0;
    for (p = players.begin( ); p != players.end( ); p++) {
        group["pc"][pc++] = jsonGroupMember( ch, *p );
    }
        
    for (m = mobs.begin( ); m != mobs.end( ); m++) {
        group["npc"][npc++] = jsonGroupMember( ch, *m );
    }

    group["leader"] = jsonGroupMember( ch, leader );
    group["ln"] = ch->sees( leader,'2' );
    return group;
}

/*-------------------------------------------------------------------------
 * LocationWebPromptListener
 *------------------------------------------------------------------------*/
class LocationWebPromptListener : public WebPromptListener {
public:
        typedef ::Pointer<LocationWebPromptListener> Pointer;

        virtual void run( Descriptor *, Character *, Json::Value &json );
protected:
        Json::Value jsonRoom( Descriptor *d, Character *ch );
        Json::Value jsonZone( Descriptor *d, Character *ch );
        Json::Value jsonExits( Descriptor *d, Character *ch );
        bool canSeeLocation( Character *ch );
};

void LocationWebPromptListener::run( Descriptor *d, Character *ch, Json::Value &json )
{
    WebPromptAttribute::Pointer attr = ch->getPC( )->getAttributes( ).getAttr<WebPromptAttribute>( "webprompt" );
    Json::Value &prompt = json["args"][0];

    json_update_if_new( ROOM, jsonRoom( d, ch ), attr, prompt ); 
    json_update_if_new( ZONE, jsonZone( d, ch ), attr, prompt ); 
    json_update_if_new( EXITS, jsonExits( d, ch ), attr, prompt ); 
}

bool LocationWebPromptListener::canSeeLocation( Character *ch )
{
    if (!ch->in_room 
            || IS_SET(ch->in_room->room_flags, ROOM_NOWHERE)
            || eyes_blinded( ch ) 
            || eyes_darkened( ch ))
        return false;

    return true;
}

Json::Value LocationWebPromptListener::jsonRoom( Descriptor *d, Character *ch )
{
    if (!canSeeLocation( ch ))
        return Json::Value( );

    return Json::Value( DLString( ch->in_room->name ).colourStrip( ) );
}

Json::Value LocationWebPromptListener::jsonZone( Descriptor *d, Character *ch )
{
    if (!canSeeLocation( ch ))
        return Json::Value( );

    return Json::Value( DLString( ch->in_room->area->name ).colourStrip( ) );
}

Json::Value LocationWebPromptListener::jsonExits( Descriptor *d, Character *ch )
{    
    if (!canSeeLocation( ch ))
        return Json::Value( );

    ostringstream hidden, visible;

    for (int door = 0; door <= 5; door++)
    {
        EXIT_DATA *pexit;
        Room *room;

        if (!( pexit = ch->in_room->exit[door] ))
            continue;
        if (!( room = pexit->u1.to_room ))
            continue;
        if (!ch->can_see( room ))
            continue;

        if (!IS_SET(pexit->exit_info, EX_CLOSED)) {
            visible << dirs[door].name[0];
        } else if (number_percent() < gsn_perception->getEffective( ch )) {
            hidden << dirs[door].name[0];
        }
    }

    Json::Value exits;
    exits["h"] = hidden.str( );
    exits["e"] = visible.str( );
    exits["l"] = ch->getConfig( )->ruexits ? "r" : "e";
    return exits;
}    

/*-------------------------------------------------------------------------
 * CalendarWebPromptListener
 *------------------------------------------------------------------------*/
class CalendarWebPromptListener : public WebPromptListener {
public:
        typedef ::Pointer<CalendarWebPromptListener> Pointer;

        virtual void run( Descriptor *, Character *, Json::Value &json );

protected:        
        Json::Value jsonTime( Descriptor *d, Character *ch );
        Json::Value jsonDate( Descriptor *d, Character *ch );
        Json::Value jsonWeather( Descriptor *d, Character *ch );
        bool canSeeSunlight( Character *ch );
        bool canSeeTime( Character *ch );
        bool canSeeWeather( Character *ch );
};


bool CalendarWebPromptListener::canSeeTime( Character *ch )
{
    return ch->in_room && !IS_SET(ch->in_room->room_flags, ROOM_NO_TIME);
}

bool CalendarWebPromptListener::canSeeSunlight( Character *ch )
{
    return ch->in_room && IS_OUTSIDE(ch);
}

bool CalendarWebPromptListener::canSeeWeather( Character *ch )
{
    return ch->in_room 
        && IS_OUTSIDE(ch)
        && !IS_SET(ch->in_room->room_flags, ROOM_NO_WEATHER);
}

Json::Value CalendarWebPromptListener::jsonWeather( Descriptor *d, Character *ch )
{
    DLString msg, icon, icon_day, icon_night;
    if (!canSeeWeather( ch ))
        return Json::Value( );

    // Небо (ясное|облачное|дождливое|во вспышках молний)
    // ->     ясно   облачно  дождь    гроза
    // и дует (теплый южный | холодный северный) ветер
    // ->      теплый        холодный          ветер
    switch (weather_info.sky) {
        case SKY_CLOUDY:    
            msg = "облачно";
            icon_day = "day-cloudy";
            icon_night = "night-alt-cloudy";
            break;
        case SKY_CLOUDLESS:
            msg = "ясно";
            icon_day = "day-sunny";
            icon_night = "night-clear";
            break;
        case SKY_RAINING:
            msg = "дождь";
            icon_day = "day-showers";
            icon_night = "night-alt-showers";
            break;
        case SKY_LIGHTNING:
            msg = "гроза";
            icon_day = "day-lightning";
            icon_night = "night-alt-lightning";
            break;
    }
    
    if (weather_info.change >= 0)
        msg += ", теплый ветер";
    else
        msg += ", холодный ветер";

    DLString sl = sunlight( );
    if (sl == "светло" || sl == "светает") 
        icon = icon_day;
    else 
        icon = icon_night;

    Json::Value weather;
    weather["i"] = icon;
    weather["m"] = msg;
    return weather;

}

Json::Value CalendarWebPromptListener::jsonDate( Descriptor *d, Character *ch )
{
    if (!canSeeTime( ch ))
        return Json::Value( );

    Json::Value date;
    date["d"] = time_info.day + 1;
    date["m"] = calendar_month( );
    date["y"] = time_info.year;
    return date;
}

Json::Value CalendarWebPromptListener::jsonTime( Descriptor *d, Character *ch )
{
    if (!canSeeTime( ch ))
        return Json::Value( );

    Json::Value time;
    time["h"] = hour_of_day( );
    time["tod"] = time_of_day( );
    if (canSeeSunlight( ch ))
        time["l"] = sunlight( );
    return time;
}    

void CalendarWebPromptListener::run( Descriptor *d, Character *ch, Json::Value &json )
{
    WebPromptAttribute::Pointer attr = ch->getPC( )->getAttributes( ).getAttr<WebPromptAttribute>( "webprompt" );
    Json::Value &prompt = json["args"][0];

    json_update_if_new( TIME, jsonTime( d, ch ), attr, prompt ); 
    json_update_if_new( DATE, jsonDate( d, ch ), attr, prompt ); 
    json_update_if_new( WEATHER, jsonWeather( d, ch ), attr, prompt ); 
}    

/*-------------------------------------------------------------------------
 * AffectsWebPromptListener
 *------------------------------------------------------------------------*/
static Bitstring zero_affect_bitstring( int where, Affect *list, const Bitstring &bits )
{
    Bitstring zero;

    for (Affect *paf = list; paf; paf = paf->next) {
        if (paf->duration != 0)
            continue;
        if (paf->where != where)
            continue;

        if (bits.isSet( paf->bitvector ))
            zero.setBit( paf->bitvector & bits.getValue( ) ); 
    }

    return zero;
}

static DLString aff_to_string( const Bitstring &b )
{
    DLString s;

    if (b.isSet( AFF_INFRARED ))
        s += "r";

    return s;
}

static DLString det_to_string( const Bitstring &b )
{
    DLString s;

    if (b.isSet( DETECT_HIDDEN ))
        s += "h";
    if (b.isSet( DETECT_INVIS ))
        s += "i";
    if (b.isSet( DETECT_IMP_INVIS ))
        s += "w";
    if (b.isSet( DETECT_FADE ))
        s += "f";
    if (b.isSet( ACUTE_VISION ))
        s += "a";
    if (b.isSet( DETECT_EVIL ))
        s += "e";
    if (b.isSet( DETECT_GOOD ))
        s += "g";
    if (b.isSet( DETECT_UNDEAD ))
        s += "u";
    if (b.isSet( DETECT_MAGIC ))
        s += "m";
    if (b.isSet( DETECT_OBSERVATION ))
        s += "o";
    if (b.isSet( DETECT_LIFE ))
        s += "l";

    return s;
}

class AffectsWebPromptListener : public WebPromptListener {
public:
        typedef ::Pointer<AffectsWebPromptListener> Pointer;

        virtual void run( Descriptor *, Character *, Json::Value &json );
protected:
        Json::Value jsonDetect( Descriptor *d, Character *ch );
        Json::Value jsonEnhance( Descriptor *d, Character *ch );
        Json::Value jsonProtect( Descriptor *d, Character *ch );
        Json::Value jsonTravel( Descriptor *d, Character *ch );
        Json::Value jsonMalad( Descriptor *d, Character *ch );
        Json::Value jsonClan( Descriptor *d, Character *ch );
};

void AffectsWebPromptListener::run( Descriptor *d, Character *ch, Json::Value &json )
{
    WebPromptAttribute::Pointer attr = ch->getPC( )->getAttributes( ).getAttr<WebPromptAttribute>( "webprompt" );
    Json::Value &prompt = json["args"][0];

    json_update_if_new( DETECT, jsonDetect( d, ch ), attr, prompt ); 
    json_update_if_new( ENHANCE, jsonEnhance( d, ch ), attr, prompt ); 
    json_update_if_new( PROTECT, jsonProtect( d, ch ), attr, prompt ); 
    json_update_if_new( TRAVEL, jsonTravel( d, ch ), attr, prompt ); 
    json_update_if_new( MALAD, jsonMalad( d, ch ), attr, prompt ); 
    json_update_if_new( CLAN, jsonClan( d, ch ), attr, prompt ); 
}

static void mark_affect( Character *ch, DLString &output, bitstring_t bit, char marker )
{    
    if (IS_AFFECTED(ch, bit) && output.find( marker ) == DLString::npos)
        output += marker;
}

Json::Value AffectsWebPromptListener::jsonMalad( Descriptor *d, Character *ch )
{
    DLString active, zero;
    
    for (Affect *paf = ch->affected; paf; paf = paf->next) {
        char m;
        
        if (paf->type == gsn_blindness) 
           m = 'b';
        else if (paf->type == gsn_poison)
           m = 'p';
        else if (paf->type == gsn_plague)
           m = 'P';
        else if (paf->type == gsn_corruption)
           m = 'C';
        else if (paf->type == gsn_faerie_fire)
           m = 'f';
        else if (paf->type == gsn_charm_person)
           m = 'W';
        else if (paf->type == gsn_curse)
           m = 'c';
        else if (paf->type == gsn_weaken)
           m = 'w';
        else if (paf->type == gsn_slow)
           m = 's';
        else if (paf->type == gsn_scream)
           m = 'S';
        else if (paf->type == gsn_bloodthirst)
           m = 'B';
        else if (paf->type == gsn_slice)
           m = 'i';
        else if (paf->type == gsn_stuck_arrow)
           m = 'I';
        else if (paf->type == gsn_magic_jar)
           m = 'j';
        else if (paf->type == gsn_anathema)
           m = 'a';
        else
            continue;

        if (active.find( m ) != DLString::npos)
            continue;

        active += m;

        if (paf->duration == 0)
            zero += m;
    }

    mark_affect( ch, active, AFF_BLIND, 'b' );
    mark_affect( ch, active, AFF_POISON, 'p' );
    mark_affect( ch, active, AFF_PLAGUE, 'P' );
    mark_affect( ch, active, AFF_CORRUPTION, 'C' );
    mark_affect( ch, active, AFF_FAERIE_FIRE, 'f' );
    mark_affect( ch, active, AFF_CHARM, 'W' );
    mark_affect( ch, active, AFF_CURSE, 'c' );
    mark_affect( ch, active, AFF_WEAKEN, 'w' );
    mark_affect( ch, active, AFF_SLOW, 's' );
    mark_affect( ch, active, AFF_SCREAM, 'S' );
    if (!ch->isAffected(gsn_bloodthirst))
        mark_affect( ch, active, AFF_BLOODTHIRST, 'B' );
    mark_affect( ch, active, AFF_STUN|AFF_WEAK_STUN, 'T' );

    if (active.empty( ))
        return Json::Value( );

    Json::Value mal;
    mal["a"] = active;
    mal["z"] = zero;
    return mal;
}

Json::Value AffectsWebPromptListener::jsonClan( Descriptor *d, Character *ch )
{
    DLString active, zero;
    
    for (Affect *paf = ch->affected; paf; paf = paf->next) {
        char m;
        
        if (paf->type == gsn_resistance) 
           m = 'r';
        else if (paf->type == gsn_spellbane)
           m = 's';
        else if (paf->type == gsn_bloodthirst)
           m = 'B';
        else if (paf->type == gsn_bandage)
           m = 'b';
        else if (paf->type == gsn_trophy)
           m = 't';
        else if (paf->type == gsn_truesight)
           m = 'T';
        else if (paf->type == gsn_detect_trap)
           m = 'd';
        else if (paf->type == gsn_evolve_lion)
           m = 'e';
        else if (paf->type == gsn_prevent)
           m = 'p';
        else if (paf->type == gsn_transform)
           m = 'f';
        else if (paf->type == gsn_golden_aura)
           m = 'g';
        else if (paf->type == gsn_holy_armor)
           m = 'h';
        else if (paf->type == gsn_shadow_shroud || paf->type == gsn_soul_lust)
           m = 'S';
        else if (paf->type == gsn_doppelganger)
           m = 'D';
        else if (paf->type == gsn_mirror)
           m = 'm';
        else if (paf->type == gsn_randomizer)
           m = 'R';
        else if (paf->type == gsn_disgrace)
           m = 'i';
        else if (paf->type == gsn_garble)
           m = 'G';
        else if (paf->type == gsn_confuse)
           m = 'c';
        else if (paf->type == gsn_manacles)
           m = 'M';
        else if (paf->type == gsn_suspect)
           m = 'u';
        else if (paf->type == gsn_jail)
           m = 'j';
        else if (paf->type == gsn_dismiss)
           m = 'J';
        else if (paf->type == gsn_ruler_aura)
           m = 'A';
        else
            continue;

        if (active.find( m ) != DLString::npos)
            continue;

        active += m;

        if (paf->duration == 0)
            zero += m;
    }

    if (active.empty( ))
        return Json::Value( );

    Json::Value cln;
    cln["a"] = active;
    cln["z"] = zero;
    return cln;
}

Json::Value AffectsWebPromptListener::jsonTravel( Descriptor *d, Character *ch )
{
    DLString active, zero;
    
    for (Affect *paf = ch->affected; paf; paf = paf->next) {
        char m;
        
        if (paf->type == gsn_invisibility) 
           m = 'i';
        else if (paf->type == gsn_improved_invis)
           m = 'I';
        else if (paf->type == gsn_hide)
           m = 'h';
        else if (paf->type == gsn_sneak)
           m = 's';
        else if (paf->type == gsn_fly)
           m = 'f';
        else if (paf->type == gsn_pass_door)
           m = 'p';
        else if (paf->type == gsn_mental_block)
           m = 'm';
        else
            continue;

        if (active.find( m ) != DLString::npos)
            continue;

        active += m;

        if (paf->duration == 0)
            zero += m;
    }

    mark_affect( ch, active, AFF_INVISIBLE, 'i' );
    mark_affect( ch, active, AFF_IMP_INVIS, 'I' );
    mark_affect( ch, active, AFF_HIDE, 'h' );
    mark_affect( ch, active, AFF_SNEAK, 's' );
    mark_affect( ch, active, AFF_FLYING, 'f' );
    mark_affect( ch, active, AFF_PASS_DOOR, 'p' );
    mark_affect( ch, active, AFF_FADE, 'F' );

    if (active.empty( ))
        return Json::Value( );

    Json::Value trv;
    trv["a"] = active;
    trv["z"] = zero;
    return trv;
}

Json::Value AffectsWebPromptListener::jsonProtect( Descriptor *d, Character *ch )
{
    DLString active, zero;
    
    for (Affect *paf = ch->affected; paf; paf = paf->next) {
        char m;

        if (paf->type == gsn_stardust) 
           m = 'z';
        else if (paf->type == gsn_sanctuary)
           m = 's';
        else if (paf->type == gsn_dark_shroud)
           m = 'd';
        else if (paf->type == gsn_protective_shield)
           m = 'p';
        else if (paf->type == gsn_protection_evil)
           m = 'e';
        else if (paf->type == gsn_protection_good)
           m = 'g';
        else if (paf->type == gsn_spell_resistance)
           m = 'm';
        else if (paf->type == gsn_prayer)
           m = 'P';
        else if (paf->type == gsn_protection_negative)
           m = 'n';
        else if (paf->type == gsn_armor)
           m = 'a';
        else if (paf->type == gsn_enhanced_armor)
           m = 'A';
        else if (paf->type == gsn_shield)
           m = 'S';
        else if (paf->type == gsn_dragon_skin)
           m = 'D';
        else if (paf->type == gsn_stone_skin)
           m = 'k';
        else if (paf->type == gsn_meld_into_stone)
           m = 'r';
        else if (paf->type == gsn_protection_cold)
           m = 'c';
        else if (paf->type == gsn_protection_heat)
           m = 'h';
        else if (paf->type == gsn_bat_swarm)
           m = 'b';
        else if (paf->type == gsn_rainbow_shield)
           m = 'R';
        else if (paf->type == gsn_demonic_mantle)
           m = 'M';
        else if (paf->type == gsn_black_feeble)
           m = 'F';
        else if (paf->type == gsn_endure)
           m = 'E';
        else if (paf->type == gsn_sebat)
           m = 'Z';
        else
          continue;
        
        if (active.find( m ) != DLString::npos)
            continue;

        active += m;

        if (paf->duration == 0)
            zero += m;
    }

    mark_affect( ch, active, AFF_PROTECT_EVIL, 'e' );
    mark_affect( ch, active, AFF_PROTECT_GOOD, 'g' );

    if (active.empty( ))
        return Json::Value( );

    Json::Value pro;
    pro["a"] = active;
    pro["z"] = zero;
    return pro;
}

Json::Value AffectsWebPromptListener::jsonEnhance( Descriptor *d, Character *ch )
{
    DLString active, zero;
    
    for (Affect *paf = ch->affected; paf; paf = paf->next) {
        char m;

        if (paf->type == gsn_haste) 
           m = 'h';
        else if (paf->type == gsn_giant_strength)
           m = 'g';
        else if (paf->type == gsn_learning)
           m = 'l';
        else if (paf->type == gsn_magic_concentrate)
           m = 'm';
        else if (paf->type == gsn_bless) 
           m = 'b';
        else if (paf->type == gsn_frenzy)
           m = 'f';
        else if (paf->type == gsn_benediction)
           m = 'B';
        else if (paf->type == gsn_inspire)
           m = 'i';
        else if (paf->type == gsn_calm)
           m = 'c';
        else if (paf->type == gsn_concentrate)
           m = 'C';
        else if (paf->type == gsn_berserk)
           m = 'z';
        else if (paf->type == gsn_warcry)
           m = 'w';
        else if (paf->type == gsn_forest_fighting)
           m = 'F';
        else
          continue;
        
        if (active.find( m ) != DLString::npos)
            continue;

        active += m;

        if (paf->duration == 0)
            zero += m;
    }

    mark_affect( ch, active, AFF_REGENERATION, 'r' );
    mark_affect( ch, active, AFF_HASTE,        'h' );

    if (active.empty( ))
        return Json::Value( );

    Json::Value enh;
    enh["a"] = active;
    enh["z"] = zero;
    return enh;
}

Json::Value AffectsWebPromptListener::jsonDetect( Descriptor *d, Character *ch )
{
    static bitstring_t reported_affdet = AFF_INFRARED;
    static bitstring_t reported_det = 
            DETECT_HIDDEN|DETECT_INVIS|DETECT_IMP_INVIS|DETECT_FADE|ACUTE_VISION|DETECT_EVIL|DETECT_GOOD|DETECT_UNDEAD|DETECT_MAGIC|DETECT_OBSERVATION|DETECT_LIFE;

    DLString active = aff_to_string( Bitstring( ch->affected_by & reported_affdet ) );
    active += det_to_string( Bitstring( ch->detection ) ); 
    
    if (active.empty( ))
        return Json::Value( );

    Json::Value det;
    det["a"] = active;
    Bitstring zeroDetects = zero_affect_bitstring( TO_DETECTS, ch->affected, reported_det );
    det["z"] = det_to_string( zeroDetects );
    return det;
}

/*-------------------------------------------------------------------------
 * ParamsWebPromptListener
 *------------------------------------------------------------------------*/
class ParamsWebPromptListener : public WebPromptListener {
public:
        typedef ::Pointer<ParamsWebPromptListener> Pointer;

        virtual void run( Descriptor *, Character *, Json::Value &json );
protected:
        Json::Value jsonParam1( Descriptor *d, Character *ch );
        Json::Value jsonParam2( Descriptor *d, Character *ch );
};

void ParamsWebPromptListener::run( Descriptor *d, Character *ch, Json::Value &json )
{
    WebPromptAttribute::Pointer attr = ch->getPC( )->getAttributes( ).getAttr<WebPromptAttribute>( "webprompt" );
    Json::Value &prompt = json["args"][0];

    json_update_if_new( PARAM1, jsonParam1( d, ch ), attr, prompt ); 
    json_update_if_new( PARAM2, jsonParam2( d, ch ), attr, prompt ); 
}

Json::Value ParamsWebPromptListener::jsonParam1( Descriptor *d, Character *ch )
{
    Json::Value p1;

    // Return permanent and current player stats: str, int, wis, dex, con, cha.
    for (int i = 0; i < stat_table.size; i++) {
        p1[PERM_STAT][i] = DLString(ch->perm_stat[i]);
        p1[CURR_STAT][i] = DLString(ch->getCurrStat(i));
    }

    return p1;
}

Json::Value ParamsWebPromptListener::jsonParam2( Descriptor *d, Character *ch )
{
    Json::Value p2;

    // Hitroll, damroll.
    p2["h"] = DLString(ch->hitroll);
    p2["d"] = DLString(ch->damroll);
    // Armor class (pierce only) and saves.
    p2["a"] = DLString(GET_AC(ch,AC_PIERCE));
    p2["s"] = DLString(ch->saving_throw);
    return p2;
}

/*-------------------------------------------------------------------------
 * QuestorWebPromptListener
 *------------------------------------------------------------------------*/
class QuestorWebPromptListener : public WebPromptListener {
public:
        typedef ::Pointer<QuestorWebPromptListener> Pointer;

        virtual void run( Descriptor *, Character *, Json::Value &json );
protected:
        Json::Value jsonQuestor( Descriptor *d, Character *ch );
};

void QuestorWebPromptListener::run( Descriptor *d, Character *ch, Json::Value &json )
{
    WebPromptAttribute::Pointer attr = ch->getPC( )->getAttributes( ).getAttr<WebPromptAttribute>( "webprompt" );
    Json::Value &prompt = json["args"][0];

    json_update_if_new( QUESTOR, jsonQuestor( d, ch ), attr, prompt ); 
}

Json::Value QuestorWebPromptListener::jsonQuestor( Descriptor *d, Character *ch )
{
    Json::Value q;
    PCharacter *pch = ch->getPC( );
    if (!pch)
        return Json::Value( );

    XMLAttributeTimer::Pointer questData = pch->getAttributes( ).findAttr<XMLAttributeTimer>( "questdata" );
    if (!questData)
        return Json::Value( );

    // Show nothing if no active quest running.
    Quest::Pointer quest = pch->getAttributes( ).findAttr<Quest>( "quest" );
    if (!quest)
        return Json::Value( );

    // Questor quest time.
    q["t"] = questData->getTime( );

    // Current quest description.
    ostringstream buf;
    quest->shortInfo( buf, pch );
    q["i"] = DLString(buf.str( )).colourStrip( );

    return q;
}


/*-------------------------------------------------------------------------
 * WebPromptDescriptorStateListener
 *------------------------------------------------------------------------*/
class WebPromptDescriptorStateListener: public DescriptorStateListener {
public:
        typedef ::Pointer<WebPromptDescriptorStateListener> Pointer;

        virtual void run( int, int, Descriptor * );
};

void WebPromptDescriptorStateListener::run( int oldState, int newState, Descriptor *d )
{
    if (newState != CON_PLAYING)
        return;

    if (!d->character || !d->character->getPC( ))
        return;

    WebPromptAttribute::Pointer attr = d->character->getPC( )->getAttributes( ).findAttr<WebPromptAttribute>( "webprompt" );
    if (attr)
            attr->clear( );
}

/*-------------------------------------------------------------------------
 * initialize_web
 *------------------------------------------------------------------------*/
class CommandDumpPlugin : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<CommandDumpPlugin> Pointer;

    virtual int getPriority( ) const
    {
        return SCDP_BOOT + 25;
    }
    virtual void run( )
    {
        Json::Value json;
        list<Command::Pointer>::const_iterator c;

        // Use this guy to choose only commands visible to mortals.
        PCharacter dummy;
        dummy.setLevel(1);

        // Output English commands sorted according to their priorities.
        // TODO: aliases
        list<Command::Pointer> commands = commandManager->getCommands().getCommands();
        for (c = commands.begin(); c != commands.end(); c++) {
            if ((*c)->visible(&dummy)) {
                Json::Value cmd;
                cmd["name"] = (*c)->getName();
                cmd["lvl"] = (*c)->getLevel();
                json.append(cmd);
            }
        }

        // Append Russian commands sorted according to their priorities.
        commands = commandManager->getCommands().getCommandsRU();
        for (c = commands.begin(); c != commands.end(); c++) {
            if ((*c)->visible(&dummy)) {
                Json::Value cmd;
                cmd["name"] = (*c)->getRussianName();
                cmd["lvl"] = (*c)->getLevel();
                json.append(cmd);
            }
        }

        DLFileStream(dreamland->getMiscDir(), "commands", ".json").fromString(
            json_to_string(json)
        );
    }
};    

/**
 * Help dumper task: save help HTML to disk each time this plugin is loaded.
 * It's prioritized to run after all area initialization has completed.
 */
class HelpDumpPlugin : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<HelpDumpPlugin> Pointer;

    virtual int getPriority( ) const
    {
        return SCDP_BOOT + 25;
    }

    /**
     * Output help categories and list of unique links to disk, all formated for
     * a 1st level player with Russian language settings.
     */
    virtual void run( )
    {
        saveUniqueRefs();
#if 0        
        PCharacter dummy;

        dummy.setName("Kadm");
        dummy.setRussianName("Кадм||а|у|а|ом|е");
        dummy.setLevel(1);
        dummy.config.setBit(CONFIG_RUSKILLS);
        dummy.config.setBit(CONFIG_RUCOMMANDS);
        dummy.config.setBit(CONFIG_RUOTHER);
        SET_BIT(dummy.act, PLR_COLOR);

        saveAllLinks(&dummy);
        saveHelpCategory("race", "Расы", &dummy);
        saveHelpCategory("class", "Классы", &dummy);
        saveHelpCategory("religion", "Религии", &dummy);
        saveHelpCategory("clan", "Кланы", &dummy);
        saveHelpCategory("skill", "Умения", &dummy);
        saveHelpCategory("spell", "Заклинания", &dummy);
        saveHelpCategory("area", "Зоны", &dummy);
        saveHelpCategory("social", "Социалы", &dummy);
        saveHelpCategory("cmd", "Все команды", &dummy);
        saveHelpCategory("quest", "Квесты", &dummy);
        saveHelpCategory("char", "Персонаж", &dummy);
        saveHelpCategory("info", "Информация", &dummy);
        saveHelpCategory("shop", "Торговля и услуги", &dummy);
        saveHelpCategory("learn", "Обучение", &dummy);
        saveHelpCategory("item", "Предметы", &dummy);
        saveHelpCategory("move", "Перемещение", &dummy);
        saveHelpCategory("fight", "Битвы", &dummy);
        saveHelpCategory("magic", "Магия", &dummy);
        saveHelpCategory("note", "Переписка", &dummy);
        saveHelpCategory("comm", "Общение", &dummy);
        saveHelpCategory("genericskill", "Профессиональные навыки", &dummy);
        saveHelpCategory("raceaptitude", "Расовые навыки", &dummy);
        saveHelpCategory("clanskill", "Клановые навыки", &dummy);
        saveHelpCategory("craftskill", "Крафт", &dummy);
        saveHelpCategory("craft", "Крафт", &dummy);
        saveHelpCategory("cardskill", "Навыки картежника", &dummy);
        saveHelpCategory("language", "Языки", &dummy);
#endif        
    }


protected:
    /**
     * Save a JSON file with all keywords and unique ID, to be used inside hedit.
     */
    void saveUniqueRefs() {
        Json::Value typeahead;
        HelpArticles::const_iterator a;

        for (a = helpManager->getArticles( ).begin( ); a != helpManager->getArticles( ).end( ); a++) {
            Json::Value b;
            b["kw"] = koi2utf((*a)->getAllKeywordsString());
            b["id"] = DLString((*a)->getID());
            typeahead.append(b);
        }

        DLFileStream("/tmp", "hedit", ".json").fromString(
            json_to_string(typeahead)
        );
    }
#if 0
    /**
     * Save a JSON file that contains mapping of each help keyword to its unique ID
     * (and list of labels). This file is used to generate unique HTML links to the articles.
     */
    void saveAllLinks(PCharacter *dummy) {
        Json::Value helps, typeahead;
        HelpArticles::const_iterator a;

        for (a = helpManager->getArticles( ).begin( ); a != helpManager->getArticles( ).end( ); a++) {
            if ((*a)->visible(dummy) && (*a)->getLabels().size() > 0) {
                // Collect data for generating hyper links between the articles.
                for (StringSet::const_iterator k = (*a)->getKeywords().begin(); k != (*a)->getKeywords().end(); k++) {
                    DLString key = k->toLower();
                    helps[key]["id"] = (*a)->getID();
                    helps[key]["labels"].clear();
                    for (StringSet::const_iterator label = (*a)->getLabels().begin(); label != (*a)->getLabels().end(); label++) {
                        helps[key]["labels"].append(*label);
                    }
                }

                // Collect data for autodropdown JS suggestions.
                Json::Value b;
                b["n"] = (*a)->getKeyword();
                const DLString &label = *((*a)->getLabels().begin());
                b["l"] = label + ".html#h" + DLString((*a)->getID());
                typeahead.append(b);
            }
        }

        DLFileStream(dreamland->getMiscDir(), "allhelp", ".json").fromString(
            json_to_string(helps)
        );

        DLFileStream(dreamland->getMiscDir(), "typeahead-help", ".json").fromString(
            json_to_string(typeahead)
        );
    }

    /**
     * Collect all articles with a given label and save to disk. 
     * First output a list of topics under help_menu div, followed by all articles,
     * each wrapped in a help_content div.
     */
    void saveHelpCategory(const DLString &label, const DLString &title, PCharacter *dummy) {
        DLDirectory dir(dreamland->getMiscDir(), "help");
        HelpArticles helps;
        HelpArticles::const_iterator a;
        ostringstream buf;

        for (a = helpManager->getArticles( ).begin( ); a != helpManager->getArticles( ).end( ); a++) {
            if ((*a)->visible(dummy) && (*a)->getLabels().count(label) > 0)
                helps.push_back(*a);
        }

        buf << "<div class=\"help_menu\"><h2>" << title << "</h2>" << endl
            << "<ul>" << endl;
        
        for (a = helps.begin(); a != helps.end(); a++) {
            buf << "<li><a href=\"#h" << (*a)->getID() << "\">" << (*a)->getTitle(label).toUpper() << "</a></li>" << endl; 
        }
        buf << "</ul></div>" << endl 
             << "<div class=\"help_content\">";

        for (a = helps.begin(); a != helps.end(); a++) {
            ostringstream textStream;
            DLString text = (*a)->getText(dummy);
            mudtags_convert_web(text.c_str(), textStream, dummy);

            buf << "<div class=\"help_panel\">"
                << "<a class=\"help_title\" name=\"h" << (*a)->getID() << "\"></a><span class=\"help_title\">"
                << (*a)->getKeyword().toUpper()
                << "</span><div class=\"help_article\">" 
                << textStream.str()
                << "</div></div>" << endl;
        }

        buf << "</div>" << endl;

        DLFileStream(dir, label, ".html").fromString(buf.str());
    }
#endif
};    

extern "C"
{
    SO::PluginList initialize_web( )
    {
        SO::PluginList ppl;
        Plugin::registerPlugin<HelpDumpPlugin>( ppl );
        Plugin::registerPlugin<CommandDumpPlugin>( ppl );
        Plugin::registerPlugin<WhoWebPromptListener>( ppl );
        Plugin::registerPlugin<GroupWebPromptListener>( ppl );
        Plugin::registerPlugin<CalendarWebPromptListener>( ppl );
        Plugin::registerPlugin<LocationWebPromptListener>( ppl );
        Plugin::registerPlugin<AffectsWebPromptListener>( ppl );
        Plugin::registerPlugin<ParamsWebPromptListener>( ppl );
        Plugin::registerPlugin<QuestorWebPromptListener>( ppl );
        Plugin::registerPlugin<WebPromptDescriptorStateListener>( ppl );
        Plugin::registerPlugin<XMLAttributeRegistrator<WebPromptAttribute> >( ppl );
        
        return ppl;
    }
}

