/* $Id$
 *
 * ruffina, 2018
 */
#include <jsoncpp/json/json.h>
#include <map>
#include <set>
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
#include "string_utils.h"
#include "player_menu.h"
#include "areaquestutils.h"
#include "xmlattributeareaquest.h"
#include "directions.h"
#include "loadsave.h"
#include "weather.h"
#include "stats_apply.h"
#include "dl_math.h"
#include "dl_ctype.h"
#include "player_utils.h"
#include "mudtags.h"
#include "merc.h"
#include "autoflags.h"
#include "profflags.h"
#include "configurable.h"
#include "xmlmultistring.h"
#include "so.h"
#include "def.h"

static const DLString EXITS = "exits";
static const DLString ROOM = "room";
static const DLString ZONE = "zone";
static const DLString SECTOR = "sect";
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
static const DLString AREAQUEST = "aq";

LIQ(none);


extern void help_save_ids();
static IconvMap koi2utf("koi8-u", "utf-8");

static string json_to_string( const Json::Value &value )
{
    Json::FastWriter writer;
    return writer.write( value );
}    

static void strip_colors_and_tags(const DLString &source, ostringstream &destBuf)
{
    mudtags_convert(source.c_str(), destBuf, 
        TAGS_CONVERT_VIS|TAGS_ENFORCE_RAW|
        TAGS_CONVERT_COLOR|TAGS_ENFORCE_NOCOLOR);
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
    json["sees"] = String::truncate(sees, 10);;
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
        Json::Value jsonSector( Descriptor *d, Character *ch );
        bool canSeeLocation( Character *ch );
        DLString sectorIcon(int sector);
};

void LocationWebPromptListener::run( Descriptor *d, Character *ch, Json::Value &json )
{
    WebPromptAttribute::Pointer attr = ch->getPC( )->getAttributes( ).getAttr<WebPromptAttribute>( "webprompt" );
    Json::Value &prompt = json["args"][0];

    attr->updateIfNew( ROOM, jsonRoom( d, ch ), prompt ); 
    attr->updateIfNew( ZONE, jsonZone( d, ch ), prompt ); 
    attr->updateIfNew( EXITS, jsonExits( d, ch ), prompt ); 
    attr->updateIfNew( SECTOR, jsonSector( d, ch ), prompt ); 
}

DLString LocationWebPromptListener::sectorIcon(int sector)
{
    switch (sector) {
        case SECT_INSIDE:
            return "f6d9"; // dungeon
        case SECT_CITY:
            return "f015"; // house
        case SECT_FIELD:
            return "f18c"; // pagelines
        case SECT_FOREST:
            return "f1bb"; // tree
        case SECT_HILLS:
            return "e52d"; // mound
        case SECT_MOUNTAIN:
            return "e52f"; // mountain-sun
        case SECT_WATER_SWIM:
        case SECT_WATER_NOSWIM:
            return "f773"; // water
        case SECT_AIR:
            return "f72e"; // wind
        case SECT_DESERT:
            return "f76f"; // tornado
        case SECT_UNDERWATER:
            return "f578"; // fish
        default:
            return "3f"; // question
    }
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

Json::Value LocationWebPromptListener::jsonSector( Descriptor *d, Character *ch )
{
    if (!canSeeLocation( ch ))
        return Json::Value( );

    Json::Value sect;
    int sector = ch->in_room->getSectorType();
    sect["i"] = sectorIcon(sector);

    ostringstream buf;
    LiquidReference &liq = ch->in_room->getLiquid();
    bool indoors = IS_SET(ch->in_room->room_flags, ROOM_INDOORS);

    if (indoors)
        buf << "(";

    buf <<  sector_table.message(sector, '1', Player::displayLang(ch));
                    
    if (liq_none != liq)
        buf << ", " << liq->getShortDescr(Player::lang(ch)).ruscase('1').colourStrip();

    if (indoors)
        buf << ")";

    sect["n"] = buf.str();
    return sect;
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
    exits["l"] = Player::displayLang( ch ) != LANG_EN ? "r" : "e";
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

    lang_t lang = Player::displayLang( ch );

    // Небо (ясное|облачное|дождливое|во вспышках молний)
    // ->     ясно   облачно  дождь    гроза
    // и дует (теплый южный | холодный северный) ветер
    // ->      теплый        холодный          ветер
    switch (weather_info.sky) {
        case SKY_CLOUDY:
            msg = lmsg(lang, "cloudy", "облачно", "хмарно");
            icon_day = "day-cloudy";
            icon_night = "night-alt-cloudy";
            break;
        case SKY_CLOUDLESS:
            msg = lmsg(lang, "clear", "ясно", "ясно");
            icon_day = "day-sunny";
            icon_night = "night-clear";
            break;
        case SKY_RAINING:
            msg = lmsg(lang, "rain", "дождь", "дощ");
            icon_day = "day-showers";
            icon_night = "night-alt-showers";
            break;
        case SKY_LIGHTNING:
            msg = lmsg(lang, "storm", "гроза", "гроза");
            icon_day = "day-lightning";
            icon_night = "night-alt-lightning";
            break;
    }

    if (weather_info.change >= 0)
        msg += lmsg(lang, ", warm wind", ", теплый ветер", ", теплий вітер");
    else
        msg += lmsg(lang, ", cold wind", ", холодный ветер", ", холодний вітер");

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
    date["m"] = calendar_month( Player::displayLang( ch ) );
    date["y"] = time_info.year;
    return date;
}

Json::Value CalendarWebPromptListener::jsonTime( Descriptor *d, Character *ch )
{
    if (!canSeeTime( ch ))
        return Json::Value( );

    Json::Value time;
    lang_t lang = Player::displayLang( ch );
    time["h"] = hour_of_day( );
    time["tod"] = time_of_day( lang );
    if (canSeeSunlight( ch ))
        time["l"] = sunlight( lang );
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
 *
 * Fully data-driven affect panel. Each active affect goes into one of six columns
 * (pro/det/trv/enh/mal/cln) and renders with a short, viewer-language label. Column
 * and label come from the skill profile fields <webColumn>/<webLabel> (dreamland_world
 * skills), or from config/affectpanel.json for raw engine bits (detects, item/racial
 * affects with no spell). An affect whose skill sets no <webColumn> still shows --
 * auto-classified offensive->mal, else ->enh -- so new affects are never silently
 * missing; run scripts/lint-affect-labels.py to find those still on the auto-fallback.
 *------------------------------------------------------------------------*/

// config/affectpanel.json -> column -> raw-bit key -> per-language label.
static std::map<DLString, std::map<DLString, XMLMultiString> > affectPanelStore;

CONFIGURABLE_LOADED(config, affectpanel)
{
    static const struct { const char *key; lang_t lang; } LANGS[] = {
        { "en", LANG_EN }, { "ru", LANG_RU }, { "ua", LANG_UA },
    };

    affectPanelStore.clear( );

    for (const auto &col: value.getMemberNames( )) {
        const Json::Value &keys = value[col];
        if (!keys.isObject( ))
            continue;

        for (const auto &key: keys.getMemberNames( )) {
            const Json::Value &langs = keys[key];
            if (!langs.isObject( ))
                continue;

            XMLMultiString ms;
            for (const auto &le: LANGS) {
                const Json::Value &s = langs[le.key];
                if (s.isString( ))
                    ms[le.lang] = s.asString( );
            }
            affectPanelStore[col][key] = ms;
        }
    }

    LogStream::sendNotice( ) << "affectpanel: loaded raw-bit labels for "
                             << affectPanelStore.size( ) << " columns." << endl;
}

// True if some active spell/skill affect owns this bit -- then the curated skill loop
// renders it, and the raw columns below must stay silent to avoid a duplicate. Raw
// columns therefore surface only bits granted without a spell (equipment, race).
static bool bitOwnedBySpell( Character *ch, const FlagTable *table, bitstring_t bit )
{
    for (auto &paf: ch->affected) {
        if (paf->bitvector.getTable( ) != table)
            continue;
        if (paf->bitvector.isSet( bit ))
            return true;
    }
    return false;
}

static DLString affectPanelLabel( const DLString &col, const char *key, lang_t lang )
{
    std::map<DLString, std::map<DLString, XMLMultiString> >::const_iterator c = affectPanelStore.find( col );
    if (c == affectPanelStore.end( ))
        return DLString::emptyString;

    std::map<DLString, XMLMultiString>::const_iterator k = c->second.find( key );
    if (k == c->second.end( ))
        return DLString::emptyString;

    return k->second.getForLang( lang );
}

// Auto-fallback panel label when a skill has no curated <webLabel>: the first word of
// the skill's own (already trilingual) name, capped to fit the widget.
static DLString webAbbrev( const DLString &name )
{
    DLString word = name;
    string::size_type sp = word.find( ' ' );
    if (sp != string::npos)
        word = word.substr( 0, sp ).c_str( );
    if (word.size( ) > 10)
        word = word.substr( 0, 10 ).c_str( );
    return word;
}

// Accumulates affects into columns, de-duplicating by (column, label) so a skill
// affect and its raw-bit fallback never render twice.
struct AffectPanelColumns {
    Json::Value cols;
    std::map<DLString, std::set<DLString> > seen;

    // duration: remaining ticks; -1 = permanent. The client colors by it
    // (permanent = cyan, a couple ticks = yellow, 1 tick = red, else column base).
    void add( const DLString &col, const DLString &label, int duration )
    {
        if (col.empty( ) || col == "none" || label.empty( ))
            return;
        if (!seen[col].insert( label ).second)
            return;

        Json::Value o;
        o["n"] = label.c_str( );
        o["d"] = duration;
        cols[col].append( o );
    }
};

// Detect column: purely flag-based (ch->detection + AFF_INFRARED), labels from the store.
static void addDetectColumn( AffectPanelColumns &pc, Character *ch, lang_t lang )
{
    static const struct { bitstring_t bit; const char *key; } DETS[] = {
        { DETECT_HIDDEN,      "hidden" },
        { DETECT_INVIS,       "invis" },
        { DETECT_IMP_INVIS,   "imp_invis" },
        { DETECT_FADE,        "fade" },
        { ACUTE_VISION,       "acute" },
        { DETECT_EVIL,        "evil" },
        { DETECT_GOOD,        "good" },
        { DETECT_UNDEAD,      "undead" },
        { DETECT_MAGIC,       "magic" },
        { DETECT_OBSERVATION, "observation" },
        { DETECT_LIFE,        "life" },
    };

    if (IS_AFFECTED( ch, AFF_INFRARED ) && !bitOwnedBySpell( ch, &affect_flags, AFF_INFRARED ))
        pc.add( DETECT, affectPanelLabel( DETECT, "infrared", lang ), -1 );

    Bitstring detection( ch->detection );

    // Raw detect bits reaching here are not owned by any spell paf -> equipment/racial,
    // i.e. permanent for as long as they apply. Duration -1 renders them cyan.
    for (const auto &e: DETS) {
        if (!detection.isSet( e.bit ))
            continue;
        if (bitOwnedBySpell( ch, &detect_flags, e.bit ))
            continue;
        pc.add( DETECT, affectPanelLabel( DETECT, e.key, lang ), -1 );
    }
}

// Raw affect-flag fallbacks: effects set on the character without an owning spell
// (from equipment or race). Labels come from config/affectpanel.json.
static void addAffFallbacks( AffectPanelColumns &pc, Character *ch, lang_t lang )
{
    static const struct { const char *col; bitstring_t bit; const char *key; } AFFS[] = {
        { "trv", AFF_INVISIBLE,    "invisible" },
        { "trv", AFF_IMP_INVIS,    "imp_invis" },
        { "trv", AFF_HIDE,         "hide" },
        { "trv", AFF_SNEAK,        "sneak" },
        { "trv", AFF_FLYING,       "flying" },
        { "trv", AFF_PASS_DOOR,    "pass_door" },
        { "trv", AFF_FADE,         "fade" },
        { "trv", AFF_CAMOUFLAGE,   "camouflage" },
        { "pro", AFF_SANCTUARY,    "sanctuary" },
        { "pro", AFF_PROTECT_EVIL, "protect_evil" },
        { "pro", AFF_PROTECT_GOOD, "protect_good" },
        { "enh", AFF_REGENERATION, "regeneration" },
        { "enh", AFF_HASTE,        "haste" },
        { "mal", AFF_BLIND,        "blind" },
        { "mal", AFF_POISON,       "poison" },
        { "mal", AFF_PLAGUE,       "plague" },
        { "mal", AFF_CORRUPTION,   "corruption" },
        { "mal", AFF_FAERIE_FIRE,  "faerie_fire" },
        { "mal", AFF_CHARM,        "charm" },
        { "mal", AFF_CURSE,        "curse" },
        { "mal", AFF_WEAKEN,       "weaken" },
        { "mal", AFF_SLOW,         "slow" },
        { "mal", AFF_SCREAM,       "scream" },
        { "mal", AFF_SLEEP,        "sleep" },
        { "mal", AFF_BLOODTHIRST,  "bloodthirst" },
    };

    // Not owned by any spell paf -> equipment/racial -> permanent (cyan).
    for (const auto &e: AFFS) {
        if (IS_AFFECTED( ch, e.bit ) && !bitOwnedBySpell( ch, &affect_flags, e.bit ))
            pc.add( e.col, affectPanelLabel( e.col, e.key, lang ), -1 );
    }

    if ((IS_AFFECTED( ch, AFF_STUN ) && !bitOwnedBySpell( ch, &affect_flags, AFF_STUN ))
        || (IS_AFFECTED( ch, AFF_WEAK_STUN ) && !bitOwnedBySpell( ch, &affect_flags, AFF_WEAK_STUN )))
        pc.add( MALAD, affectPanelLabel( MALAD, "stun", lang ), -1 );
}

class AffectsWebPromptListener : public WebPromptListener {
public:
        typedef ::Pointer<AffectsWebPromptListener> Pointer;

        virtual void run( Descriptor *, Character *, Json::Value &json );
};

void AffectsWebPromptListener::run( Descriptor *d, Character *ch, Json::Value &json )
{
    WebPromptAttribute::Pointer attr = ch->getPC( )->getAttributes( ).getAttr<WebPromptAttribute>( "webprompt" );
    Json::Value &prompt = json["args"][0];
    lang_t lang = Player::displayLang( ch );

    AffectPanelColumns pc;

    // 1) Skill-applied affects: column + label from the skill profile, else auto-derived.
    for (auto &paf: ch->affected) {
        Skill *skill = paf->type.getElement( );
        if (skill == 0)
            continue;

        DLString col = skill->getWebColumn( );
        SpellPointer spell = skill->getSpell( );

        if (col.empty( )) {
            // Not curated: show only real castable spells (skip internal/tracking affects);
            // auto-classify offensive -> maladictions, everything else -> the enhance bucket.
            if (!spell || !spell->isCasted( ))
                continue;
            col = (spell->getSpellType( ) == SPELL_OFFENSIVE) ? MALAD : ENHANCE;
        }

        DLString label = skill->getWebLabel( lang );
        if (label.empty( ))
            label = webAbbrev( skill->getNameFor( ch ) );

        pc.add( col, label, paf->duration );
    }

    // 2) Detect column and 3) raw affect-flag fallbacks (no owning spell).
    addDetectColumn( pc, ch, lang );
    addAffFallbacks( pc, ch, lang );

    attr->updateIfNew( DETECT,  pc.cols[DETECT],  prompt );
    attr->updateIfNew( ENHANCE, pc.cols[ENHANCE], prompt );
    attr->updateIfNew( PROTECT, pc.cols[PROTECT], prompt );
    attr->updateIfNew( TRAVEL,  pc.cols[TRAVEL],  prompt );
    attr->updateIfNew( MALAD,   pc.cols[MALAD],   prompt );
    attr->updateIfNew( CLAN,    pc.cols[CLAN],    prompt );
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
 * AreaQuestWebPromptListener
 *------------------------------------------------------------------------*/
class AreaQuestWebPromptListener : public WebPromptListener {
public:
        typedef ::Pointer<AreaQuestWebPromptListener> Pointer;

        virtual void run( Descriptor *, Character *, Json::Value &json );
protected:
        Json::Value jsonAreaQuest( Descriptor *d, Character *ch );
};

void AreaQuestWebPromptListener::run( Descriptor *d, Character *ch, Json::Value &json )
{
    WebPromptAttribute::Pointer attr = ch->getPC( )->getAttributes( ).getAttr<WebPromptAttribute>( "webprompt" );
    Json::Value &prompt = json["args"][0];

    attr->updateIfNew( AREAQUEST, jsonAreaQuest( d, ch ), prompt ); 
}

Json::Value AreaQuestWebPromptListener::jsonAreaQuest( Descriptor *d, Character *ch )
{
    Json::Value aq;
    PCharacter *pch = ch->getPC( );
    if (!pch)
        return aq;

    // Show nothing if there's no active area quest.
    DLString latestQuestId = aquest_find_latest(pch);
    if (latestQuestId.empty())
        return aq;

    AreaQuest *quest = get_area_quest(latestQuestId);
    // Some odd broken quest in the player attrs.
    if (!quest)
        return aq;

    AreaQuestData &qdata = aquest_data(pch, latestQuestId);
    int step = qdata.step;
    if (step >= (int)quest->steps.size())
        return aq;

    lang_t lang = Player::displayLang( ch );

    ostringstream buf;

    // Add quest title to info for non-newbie zones; different windowlet title for onboarding
    if (!quest->flags.isSet(AQUEST_ONBOARDING)) {
        buf << "\"";
        strip_colors_and_tags(quest->title.getForLang(lang), buf);
        buf << "\"" << ": ";

        // getForLang alone leaks the raw Flexer pad ("Шабаш||а|у||ом|е"); decline
        // the area name to the prepositional case so it reads "Задание в Шабаше".
        aq["t"] = lmsg(lang, "Quest in ", "Задание в ", "Завдання в ")
                  + quest->pAreaIndex->getName(lang, '6').colourStrip();
    } else {
        aq["t"] = lmsg(lang, "Training", "Обучение", "Навчання");
    }

    strip_colors_and_tags(quest->steps[step]->info.getForLang(lang), buf);

    // TODO show full menu in the windowlet (needs newline and {hc handling)
    if (Player::menuAvailable(pch)) {
        buf << endl << lmsg(lang, "Choose an option from the list.",
                                  "Выбери вариант ответа из списка.",
                                  "Обери варіант відповіді зі списку.");
    }

    aq["i"] = buf.str();

    return aq;
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
        dummy.getAttributes( ).getAttr<XMLStringAttribute>( "lang" )->setValue( "ru" );
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
                }

                h["toc"] = (*a)->getTitle("toc").colourStrip();       
                h["title"] = (*a)->getTitle("title").colourStrip();           

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
        Plugin::registerPlugin<GroupWebPromptListener>( ppl );
        Plugin::registerPlugin<CalendarWebPromptListener>( ppl );
        Plugin::registerPlugin<LocationWebPromptListener>( ppl );
        Plugin::registerPlugin<AffectsWebPromptListener>( ppl );
        Plugin::registerPlugin<ParamsWebPromptListener>( ppl );
        Plugin::registerPlugin<QuestorWebPromptListener>( ppl );
        Plugin::registerPlugin<AreaQuestWebPromptListener>( ppl );
        Plugin::registerPlugin<WebPromptDescriptorStateListener>( ppl );
        Plugin::registerPlugin<XMLAttributeRegistrator<WebPromptAttribute> >( ppl );
        
        return ppl;
    }
}

