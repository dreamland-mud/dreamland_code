/* $Id$
 *
 * ruffina, 2018
 */
#include <jsoncpp/json/json.h>
#include "webprompt.h"
#include "logstream.h"
#include "schedulertaskroundplugin.h"
#include "dlscheduler.h"
#include "xmlattributeticker.h"
#include "xmlattributeplugin.h"
#include "commandflags.h"
#include "iconvmap.h"
#include "descriptor.h"
#include "descriptorstatelistener.h"
#include "quest.h"
#include "genericskill.h"
#include "pcharactermanager.h"
#include "commandmanager.h"
#include "commonattributes.h"
#include "grammar_entities_impl.h"
#include "damageflags.h"
#include "skillgroup.h"
#include "dlfilestream.h"
#include "dldirectory.h"
#include "dreamland.h"
#include "bitstring.h"
#include "affecthandler.h"
#include "skillcommand.h"
#include "spell.h"
#include "clanreference.h"
#include "affect.h"
#include "room.h"
#include "roomutils.h"
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
#include "profflags.h"
#include "so.h"
#include "def.h"

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
GSN(bark_skin);
CLAN(none);

extern void help_save_ids();
static IconvMap koi2utf("koi8-r", "utf-8");

static string json_to_string( const Json::Value &value )
{
    Json::FastWriter writer;
    return writer.write( value );
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
    
    attr->updateIfNew( GROUP, jsonGroup( ch ), prompt ); 
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
        if (is_same_group( gch, ch ) && gch != leader) {
                if (gch->is_npc( ))
                    mobs.push_back( gch );
                else
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
    group["ln"] = ch->sees( leader,'2' ).colourStrip();
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

    attr->updateIfNew( ROOM, jsonRoom( d, ch ), prompt ); 
    attr->updateIfNew( ZONE, jsonZone( d, ch ), prompt ); 
    attr->updateIfNew( EXITS, jsonExits( d, ch ), prompt ); 
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

    return Json::Value( DLString( ch->in_room->getName() ).colourStrip( ) );
}

Json::Value LocationWebPromptListener::jsonZone( Descriptor *d, Character *ch )
{
    if (!canSeeLocation( ch ))
        return Json::Value( );

    return Json::Value( DLString( ch->in_room->areaName() ).colourStrip( ) );
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
        } else {
            hidden << dirs[door].name[0];
        }
    }

    Json::Value exits;
    exits["h"] = hidden.str( );
    exits["e"] = visible.str( );
    exits["l"] = ch->getConfig( ).ruexits ? "r" : "e";
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
    return ch->in_room && RoomUtils::isOutside(ch);
}

bool CalendarWebPromptListener::canSeeWeather( Character *ch )
{
    return ch->in_room 
        && RoomUtils::isOutside(ch)
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

    attr->updateIfNew( TIME, jsonTime( d, ch ), prompt ); 
    attr->updateIfNew( DATE, jsonDate( d, ch ), prompt ); 
    attr->updateIfNew( WEATHER, jsonWeather( d, ch ), prompt ); 
}    

/*-------------------------------------------------------------------------
 * AffectsWebPromptListener
 *------------------------------------------------------------------------*/
static Bitstring zero_affect_bitstring( const FlagTable *table, AffectList &list, const Bitstring &bits )
{
    Bitstring zero;

    for (auto &paf: list) {
        if (paf->duration != 0)
            continue;
        if (paf->bitvector.getTable() != table)
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

    attr->updateIfNew( DETECT, jsonDetect( d, ch ), prompt ); 
    attr->updateIfNew( ENHANCE, jsonEnhance( d, ch ), prompt ); 
    attr->updateIfNew( PROTECT, jsonProtect( d, ch ), prompt ); 
    attr->updateIfNew( TRAVEL, jsonTravel( d, ch ), prompt ); 
    attr->updateIfNew( MALAD, jsonMalad( d, ch ), prompt ); 
    attr->updateIfNew( CLAN, jsonClan( d, ch ), prompt ); 
}

static void mark_affect( Character *ch, DLString &output, bitstring_t bit, char marker )
{    
    if (IS_AFFECTED(ch, bit) && output.find( marker ) == DLString::npos)
        output += marker;
}

Json::Value AffectsWebPromptListener::jsonMalad( Descriptor *d, Character *ch )
{
    DLString active, zero;
    
    for (auto &paf: ch->affected) {
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
        else if (paf->type == gsn_nerve)
           m = 'n';
        else if (paf->type == gsn_web)
           m = 'e';
        else if (paf->type == gsn_bonedagger)
           m = 'r';
        else if (paf->type == gsn_entangle)
           m = 'E';
        else if (paf->type == gsn_vampiric_bite)
           m = 'y';
        else if (paf->type == gsn_caltraps)
           m = 'A';
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
    mark_affect( ch, active, AFF_SLEEP, 'l' );
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
    
    for (auto &paf: ch->affected) {
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
    
    for (auto &paf: ch->affected) {
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
    mark_affect( ch, active, AFF_CAMOUFLAGE, 'c' );

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
    
    for (auto &paf: ch->affected) {
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
        else if (paf->type == gsn_liturgy)
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
        else if (paf->type == gsn_bark_skin)
           m = 'B';
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
    
    for (auto &paf: ch->affected) {
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
        else if (paf->type == gsn_tiger_power)
           m = 't';
        else if (paf->type == gsn_vampire)
           m = 'v';
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
    Bitstring zeroDetects = zero_affect_bitstring( &detect_flags, ch->affected, reported_det );
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

    attr->updateIfNew( PARAM1, jsonParam1( d, ch ), prompt ); 
    attr->updateIfNew( PARAM2, jsonParam2( d, ch ), prompt ); 
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
    // Position and flying status.
    p2["pos"] = position_table.name(ch->position);
    p2["posf"] = position_flags.names(ch->posFlags);
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

    attr->updateIfNew( QUESTOR, jsonQuestor( d, ch ), prompt ); 
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

    virtual void run()
    {
        if (dreamland->hasOption(DL_BUILDPLOT))
            return;

        dumpSkills();
        dumpCommands();
        dumpClassSkills();
    }

    void dumpSkills() 
    {
        ostringstream buf, cmdbuf;

        buf << "name,rname,group,what,cmd,rcmd,position,target" << endl;
        cmdbuf << "skill,name,rname,position,order,extra,hint" << endl;

        for (int sn = 0; sn < skillManager->size(); sn++) {
            Skill *s = skillManager->find(sn);
            Command *command = 0;            
            SpellPointer spell;

            if (s->getCommand()) {
                command = s->getCommand().getDynamicPointer<Command>();
                if (command && command->getExtra().isSet(CMD_NO_INTERPRET))
                    command = 0;
            }

            if (s->getSpell() && s->getSpell()->isCasted())
                spell = s->getSpell();

            buf << s->getName() << "," << s->getRussianName() << ","
                << s->getGroups().toString() << ",";

            DLString what = "none";
            if (spell)
                what = "spell";
            else if (command)
                what = "command";
            else if (s->getAffect())
                what = "affect";
            buf << what << ",";

            DLString cmd, rcmd;
            int position = NO_FLAG;
            int target = NO_FLAG;
            if (command) {
                cmd = command->getName();
                rcmd = command->getRussianName();
                position = command->getPosition();
            } else if (spell) {
                position = spell->getPosition();
                target = spell->getTarget();
            }

            buf << cmd << "," << rcmd << "," 
                << position_table.name(position) << ","
                << target_table.names(target) << endl;                       

            if (command) {
                cmdbuf 
                    << s->getName() << ","
                    << command->getName() << ","
                    << command->getRussianName() << ","
                    << command->getPosition().name() << ","
                    << command->getOrder().names() << ","
                    << command->getExtra().names() << ","
                    << command->getHint() 
                    << endl;
            }
        }

        try {
            DLFileStream( "/tmp/skills.csv" ).fromString( 
                koi2utf(buf.str())
            );
            DLFileStream( "/tmp/skill-commands.csv" ).fromString( 
                koi2utf(cmdbuf.str())
            );
        } catch (const ExceptionDBIO &ex) {
            LogStream::sendError() << ex.what() << endl;
        }
    }

    void dumpCommands()
    {
        ostringstream buf;

        buf << "name,rname,position,order,extra,hint" << endl;

        // Use this guy to choose only commands visible to mortals.
        PCharacter dummy;
        dummy.setLevel(1);

        // Output commands sorted according to their priorities in English.
        auto &commands = commandManager->getCommands().getCommands();
        for (auto &cmd: commands) {
            if (cmd->available(&dummy)) {
                buf << cmd->getName() << ","
                    << cmd->getRussianName() << ","
                    << cmd->getPosition().name() << ","
                    << cmd->getOrder().names() << ","
                    << cmd->getExtra().names() << ","
                    << cmd->getHint() 
                    << endl;
            }
        }

        try {
            DLFileStream( "/tmp/commands.csv" ).fromString( 
                koi2utf(buf.str())
            );
        } catch (const ExceptionDBIO &ex) {
            LogStream::sendError() << ex.what() << endl;
        }
    }

    /** Create a CSV file with all class skills and their levels. */
    void dumpClassSkills()
    {
        ostringstream buf;
        StringList profs;
        
        // Header: name,warrior,cleric,witch,...
        for (int i = 0; i < professionManager->size( ); i++) {
            Profession *prof = professionManager->find( i );
            if (prof->isValid() 
                && prof->isPlayed() 
                && !prof->getFlags().isSet(PROF_NEWLOCK))
            {
                profs.push_back(prof->getName());
            }
        }
        buf << "name,group," << profs.join(",") << endl;

        // Body: armor,,1,1,...
        for (int sn = 0; sn < skillManager->size(); sn++) {
            Skill *s = skillManager->find(sn);
            GenericSkill *skill = dynamic_cast<GenericSkill *>(s);

            if (!skill || skill->getClasses().empty())
                continue;

            ostringstream levels;
            bool found = false;

            for (auto &profName : profs) {
                Profession *prof = professionManager->find( profName );
                const auto &c = skill->getClasses().find(prof->getName());
                if (c != skill->getClasses().end()) {
                    levels << "," << c->second.getLevel();
                    found = true;
                }
                else
                    levels << ",";
            }

            if (found)
                buf << skill->getName() << "," << skill->getGroups().toString() << levels.str() << endl;
        }

        try {
            DLFileStream( "/tmp/class_skills.csv" ).fromString( buf.str( ) );
        } catch (const ExceptionDBIO &ex) {
            LogStream::sendError() << ex.what() << endl;
        }
    }

};    

/**
 * Help dumper task: periodically save help JSON to disk.
 */
class HelpDumpPlugin : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<HelpDumpPlugin> Pointer;

    virtual int getPriority( ) const
    {
        return SCDP_ROUND + 30;
    }

    virtual void after( )
    {
        DLScheduler::getThis( )->putTaskInSecond( 1 * Date::SECOND_IN_HOUR, Pointer( this ) );
    }

    /**
     * Output help categories and list of unique links to disk, all formated for
     * a 1st level player with Russian language settings.
     */
    virtual void run( )
    {
        if (dreamland->hasOption(DL_BUILDPLOT))
            return;

        help_save_ids();

        PCharacter dummy;

        dummy.setName("Kadm");
        dummy.setRussianName("Кадм||а|у|а|ом|е");
        dummy.setLevel(1);
        dummy.setSex(SEX_MALE);
        dummy.config.setBit(CONFIG_RUSKILLS);
        dummy.config.setBit(CONFIG_RUCOMMANDS);
        dummy.config.setBit(CONFIG_RUOTHER);
        SET_BIT(dummy.act, PLR_COLOR);

        Json::Value helps;
        HelpArticles::const_iterator a;

        for (a = helpManager->getArticles( ).begin( ); a != helpManager->getArticles( ).end( ); a++) {
            if ((*a)->visible(&dummy) && (*a)->getID() > 0) {
                Json::Value h;

                h["id"] = (*a)->getID();
	    	    
                if (!(*a)->aka.empty())
                    h["kw"] = (*a)->getAllKeywordsString() + " " + (*a)->aka.toString();
                else
                    h["kw"] = (*a)->getAllKeywordsString();

                for (auto &k: (*a)->getAllKeywords())
                    h["kwList"].append(k);
                
                for (auto &k: (*a)->aka)
                    h["kwList"].append(k);

                for (auto &l: (*a)->labels.all) {
                    h["labels"].append(l);
                    h["titles"][l] = (*a)->getTitle(l).colourStrip();
                }

                ostringstream textStream;
                DLString text = (*a)->getText(&dummy);
                mudtags_convert(text.c_str(), textStream, TAGS_CONVERT_VIS|TAGS_CONVERT_COLOR|TAGS_ENFORCE_WEB, &dummy);
                h["text"] = textStream.str();

                helps.append(h);
            }
        }

        LogStream::sendNotice() << "Dumping all helps to disk." << endl;
        DLFileStream("/tmp", "helps", ".json").fromString(
            koi2utf(
                json_to_string(helps))
        );
    }
};    

extern "C"
{
    SO::PluginList initialize_web( )
    {
        SO::PluginList ppl;
        Plugin::registerPlugin<HelpDumpPlugin>( ppl );
        Plugin::registerPlugin<CommandDumpPlugin>( ppl );
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

