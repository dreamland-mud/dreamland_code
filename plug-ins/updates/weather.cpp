/* $Id: weather.cpp,v 1.1.2.8 2010-09-01 21:20:47 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/
#include "weather.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "date.h"
#include "commandtemplate.h"
#include "pcharacter.h"
#include "room.h"
#include "roomutils.h"
#include "dlscheduler.h"
#include "dreamland.h"
#include "descriptor.h"

#include "loadsave.h"
#include "act.h"
#include "messengers.h"
#include "screenreader.h"
#include "calendar_utils.h"
#include "bonus.h"
#include "merc.h"
#include "def.h"

#include "messengers.h"
#include "l10n.h"
#include "lang.h"

#include <fstream>
#include "xmldocument.h"
#include "xmlnode.h"
#include "xmlmultistring.h"
#include "dlfile.h"
#include "logstream.h"
#include "exception.h"
#include "plugin.h"
#include "plugininitializer.h"

PROF(vampire);
BONUS(experience);
BONUS(mana);
BONUS(learning);

const char * sunlight_ru [4] = {
    "темно",
    "светает",
    "светло",
    "сумерки"
};    
const char * sunlight_en [4] = {
    "dark",
    "sunrise",
    "light",
    "sunset"
};
const char * sunlight_ua [4] = {
    "темно",
    "світає",
    "світло",
    "сутінки"
};

const struct season_info season_table [4] = {
    { SEASON_WINTER, "winter", "зима",  "зим|а|ы|е|у|ой|е",  "зимнего",   'C' },
    { SEASON_SPRING, "spring", "весна", "весн|а|ы|е|у|ой|е", "весеннего", 'G' },
    { SEASON_SUMMER, "summer", "лето",  "лет|о|а|у|о|ом|е",  "летнего",   'R' },
    { SEASON_AUTUMN, "autumn", "осень", "осен|ь|и|и|ь|ью|и", "осеннего",  'Y' },
};

struct month_info {
    const char *name;
    int pressure;
    int sunrise;
    int sunset;
    int season;
    const char *name_en;
    const char *name_ua;
};

const struct month_info month_table [17] = {
  { "Зимы",                   -12,    7, 17, SEASON_WINTER, "Winter",        "Зими" },
  { "Зимнего Волка",          -14,    7, 18, SEASON_WINTER, "Winter Wolf",   "Зимового Вовка" },
  { "Холодного Гиганта",      -14,    7, 18, SEASON_WINTER, "Frost Giant",   "Морозного Велетня" },
  { "Древних Воинств",        -12,    7, 19, SEASON_WINTER, "Ancient Hosts", "Древніх Воїнств" },
  { "Великих Битв",            -8,    6, 19, SEASON_SPRING, "Great Battles", "Великих Битв" },
  { "Весны",                   -4,    5, 19, SEASON_SPRING, "Spring",        "Весни" },
  { "Природы",                  0,    5, 21, SEASON_SPRING, "Nature",        "Природи" },
  { "Тщетности",                4,    5, 22, SEASON_SPRING, "Futility",      "Марноти" },
  { "Дракона",                  8,    4, 22, SEASON_SUMMER, "Dragon",        "Дракона" },
  { "Солнца",                  12,    4, 22, SEASON_SUMMER, "Sun",           "Сонця" },
  { "Жары",                    16,    5, 21, SEASON_SUMMER, "Heat",          "Спеки" },
  { "Битвы",                   12,    5, 20, SEASON_SUMMER, "Battle",        "Битви" },
  { "Темноты",                  8,    6, 20, SEASON_AUTUMN, "Darkness",      "Темряви" },
  { "Тени",                     4,    6, 19, SEASON_AUTUMN, "Shadow",        "Тіні" },
  { "Длинных Теней",            0,    7, 18, SEASON_AUTUMN, "Long Shadows",  "Довгих Тіней" },
  { "Абсолютной Темноты",      -4,    8, 17, SEASON_AUTUMN, "Total Darkness","Цілковитої Темряви" },
  { "Великого Зла",            -8,    8, 17, SEASON_WINTER, "Great Evil",    "Великого Зла" },
};

const int month_table_size = sizeof(month_table) / sizeof(month_info);

const char *        const        day_name        [] =
{
    "Луны", "Быка", "Лжи", "Грома", "Свободы",
    "Великих Богов", "Солнца"
};

/*--------------------------------------------------------------------------
 * Ambient weather/time messages, loaded from config/weather.xml.
 *
 * One entry per transition event id (sunrise, precip_start, lightning_start,
 * ...); each holds season-tagged multilingual variants. At a transition the
 * engine picks ONE variant at random for the current season and renders it to
 * every viewer in that viewer's own language. This both adds variety and kills
 * the old RU-only leak into the EN/UA frame. Winter precipitation reads as snow
 * purely through season selection (matching the Fenia snowdrift layer in
 * global/update) -- no rain/snow branch needed here.
 *
 * Reloadable via `plug reload` (see WeatherMessagesLoader below): edit
 * config/weather.xml, reload, no server restart.
 *-------------------------------------------------------------------------*/
class WeatherMessages {
public:
    static WeatherMessages & getThis( )
    {
        static WeatherMessages instance;
        return instance;
    }

    void clear( )
    {
        events.clear( );
    }

    void load( )
    {
        clear( );

        DLFile file( dreamland->getTableDir( ), "config/weather", ".xml" );
        ifstream istr( file.getCPath( ) );
        if (!istr) {
            LogStream::sendError( ) << "weather.xml: cannot open " << file.getPath( ) << endl;
            return;
        }

        try {
            XMLDocument::Pointer doc( NEW );
            doc->load( istr );

            // Find the <weather> root, skipping any leading comment/PI nodes.
            XMLNode::Pointer root;
            for (auto &n: doc->getNodeList( ))
                if (n->getName( ) == "weather") { root = n; break; }
            if (root.isEmpty( ))
                return;

            int count = 0;
            for (auto &ev: root->getNodeList( )) {
                if (ev->getName( ) != "event")
                    continue;
                DLString id = ev->getAttribute( "id" );
                if (id.empty( ))
                    continue;

                vector<Variant> &vars = events[id];
                for (auto &v: ev->getNodeList( )) {
                    if (v->getName( ) != "variant")
                        continue;
                    Variant variant;
                    variant.season = v->getAttribute( "season" );
                    if (variant.season.empty( ))
                        variant.season = "any";
                    for (auto &ln: v->getNodeList( )) {
                        if (ln->getName( ) != "line")
                            continue;
                        lang_t lang = attr2lang( ln->getAttribute( "l" ) );
                        XMLNode::Pointer txt = ln->getFirstNode( );
                        variant.msg[lang] = txt.isEmpty( ) ? DLString::emptyString : txt->getCData( );
                    }
                    vars.push_back( variant );
                    count++;
                }
            }
            LogStream::sendNotice( ) << "weather.xml: " << count << " variants across "
                                     << events.size( ) << " events." << endl;
        }
        catch (const Exception &ex) {
            LogStream::sendError( ) << "weather.xml parse error: " << ex << endl;
        }
    }

    /** One random variant for 'eventId' suited to 'seasonName' (variants tagged
     *  with that season plus season="any"), or 0 when the event has no usable
     *  variants -- the caller then stays silent. */
    const XMLMultiString * pick( const DLString &eventId, const DLString &seasonName ) const
    {
        map<DLString, vector<Variant> >::const_iterator it = events.find( eventId );
        if (it == events.end( ))
            return 0;

        vector<const XMLMultiString *> pool;
        for (auto &v: it->second)
            if (v.season == seasonName || v.season == "any")
                pool.push_back( &v.msg );

        if (pool.empty( ))
            return 0;
        return pool[number_range( 0, (int)pool.size( ) - 1 )];
    }

private:
    struct Variant {
        DLString season;
        XMLMultiString msg;
    };
    map<DLString, vector<Variant> > events;
};

/** Loads config/weather.xml on boot and on every `plug reload`. */
class WeatherMessagesLoader : public Plugin {
public:
    typedef ::Pointer<WeatherMessagesLoader> Pointer;

    virtual void initialization( ) { WeatherMessages::getThis( ).load( ); }
    virtual void destruction( )    { WeatherMessages::getThis( ).clear( ); }
};

PluginInitializer<WeatherMessagesLoader> initWeatherMessagesLoader;

void mmhg_update()
{
    int diff, d;
    
    if (number_range( 0, 10 * 24 ) == 0) {
        weather_info.avg_mmhg = 1000 + month_table[time_info.month].pressure;
        weather_info.avg_mmhg += dice(1, 5) - dice(1, 5);
    }
    
    diff = weather_info.mmhg > weather_info.avg_mmhg ? -2 : 2;

    weather_info.change += diff * dice(1, 4);
    weather_info.change = URANGE( -12, weather_info.change, 12 );

    if(weather_info.change > 0) {
        if(weather_info.change >= number_range(0, 12*2))
            weather_info.mmhg++;
    } else {
        if(weather_info.change <= -number_range(0, 12*2))
            weather_info.mmhg--;
    }

    d = dice(10, 5) - dice(10, 5);
    
    weather_info.change_ += (d - weather_info.change_) / 2;
    weather_info.change_ = URANGE( -20, weather_info.change_, 20 );
    
    weather_info.change_ = weather_info.change * 8/10;

    weather_info.mmhg += weather_info.change_ * 4/3;
    weather_info.mmhg = URANGE( 960, weather_info.mmhg, 1040 );
}

DLString season()
{
    int this_season = month_table[time_info.month].season;
    return season_table[this_season].name;
}

/*
 * Broadcast one ambient weather/time transition to every eligible outdoor
 * player, each in their own language, using a random variant from
 * config/weather.xml. 'noFlag' is the room flag that suppresses this class of
 * message (ROOM_NO_WEATHER for sky, ROOM_NO_TIME for daylight). Silent if the
 * event has no variants configured.
 */
static void weather_broadcast( const DLString &eventId, int noFlag )
{
    const XMLMultiString *msg = WeatherMessages::getThis( ).pick( eventId, season( ) );
    if (msg == 0)
        return;

    for (Descriptor *d = descriptor_list; d != 0; d = d->next) {
        if (d->connected != CON_PLAYING)
            continue;

        Character *ch = d->character;
        if (ch
            && RoomUtils::isOutside( ch )
            && IS_AWAKE( ch )
            && !IS_SET( ch->in_room->room_flags, noFlag ))
        {
            ch->send_to( msg->getForLang( viewerLang( ch ) ) + "\r\n" );
        }
    }
}

/*
 * Daytime change
 */
void sunlight_update( )
{
    ostringstream tbuf;
    DLString lightEvent;
    int this_season = month_table[time_info.month].season;
    int this_day = time_info.day;
    int this_month = time_info.month;
    int this_year = time_info.year;

    dreamland->setWorldTime( dreamland->getWorldTime( ) + 1 );
    dreamland->save(false); // Save configuration XML to disk.
    ++time_info.hour;

    if (time_info.hour == month_table[time_info.month].sunrise) {
        weather_info.sunlight = SUN_RISE;
        lightEvent = "sunrise";
    }
    else if (time_info.hour == month_table[time_info.month].sunrise + 1) {
        weather_info.sunlight = SUN_LIGHT;
        lightEvent = "day_begin";
    }
    else if (time_info.hour == month_table[time_info.month].sunset) {
        weather_info.sunlight = SUN_SET;
        lightEvent = "sunset";
    }
    else if (time_info.hour == month_table[time_info.month].sunset + 1) {
        weather_info.sunlight = SUN_DARK;
        lightEvent = "night_begin";
    }

    if (time_info.hour == 24) {
        time_info.hour = 0;
        time_info.day++;
    }
    
    if (time_info.day >= 35) {
        time_info.day = 0;
        time_info.month++;
    } 

    if (time_info.month >= 17) {
        time_info.month = 0;
        time_info.year++;
    }
    
    if (this_day != time_info.day)
        tbuf << "Полночь. Начинается день " << day_name[time_info.day % 7] << "." << endl;
    
    if (this_month != time_info.month) {
        const struct month_info &month = month_table[time_info.month];
        const struct season_info &season = season_table[month.season];
        tbuf << "Сегодня первый день месяца " << month.name;
        if (this_season != month.season) 
            tbuf << " и первый день {" << season.color << russian_case(season.short_descr, '2') << "{x!" << endl;
        else if (this_year != time_info.year)
            tbuf << " и {Cновый год{x!" << endl;
        else
            tbuf << "." << endl;
    }

    // Output new day's bonus.
    if (this_day != time_info.day) {
        ostringstream bonus_buf;
        for (int bn = 0; bn < bonusManager->size(); bn++) {
            Bonus *bonus = bonusManager->find(bn);
            if (bonus->isActive(0, time_info))
                bonus->reportTime(0, bonus_buf);
        }

        string bonus_str = bonus_buf.str();
        if (!bonus_str.empty()) {
            tbuf << bonus_str;
            send_discord_bonus(bonus_str);
        }
    }

    // Daylight transition -> random multilingual variant from weather.xml,
    // rendered to each outdoor viewer in their own language.
    if (!lightEvent.empty())
        weather_broadcast(lightEvent, ROOM_NO_TIME);

    // Calendar / bonus announcements (still RU-only -- Phase 2). Sent to every
    // awake player in a non-timeless room, indoors included, as before.
    string tbuf_str = tbuf.str();
    if (!tbuf_str.empty()) {
        for (Descriptor *d = descriptor_list; d != 0; d = d->next) {
            if (d->connected != CON_PLAYING)
                continue;

            Character *ch = d->character;
            if (ch
                && IS_AWAKE(ch)
                && !IS_SET(ch->in_room->room_flags, ROOM_NO_TIME))
            {
                ch->send_to(tbuf_str);
            }
        }
    }

    DLString newTime;
    if (!lightEvent.empty()) {
        if (weather_info.sunlight >= SUN_DARK && weather_info.sunlight <= SUN_SET)
            newTime = sunlight_en[weather_info.sunlight];
    }

    for (auto &r: roomInstances) {
        FENIA_VOID_CALL(r, "Time", "s", newTime.c_str( ));
    }
}

DLString sunlight( )
{
    if (weather_info.sunlight >= SUN_DARK && weather_info.sunlight <= SUN_SET)
        return sunlight_ru[weather_info.sunlight];

    return DLString::emptyString;
}

DLString sunlight( lang_t lang )
{
    if (weather_info.sunlight >= SUN_DARK && weather_info.sunlight <= SUN_SET) {
        switch (lang) {
            case LANG_EN: return sunlight_en[weather_info.sunlight];
            case LANG_UA: return sunlight_ua[weather_info.sunlight];
            default:      return sunlight_ru[weather_info.sunlight];
        }
    }

    return DLString::emptyString;
}

/*
 * Weather change.
 */
void weather_update( )
{
    DLString skyEvent;

    mmhg_update( );

    switch ( weather_info.sky )
    {
    default:
        bug( "Weather_update: bad sky %d.", weather_info.sky );
        weather_info.sky = SKY_CLOUDLESS;
        break;

    case SKY_CLOUDLESS:
        if ( weather_info.mmhg <  990
                || ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
        {
            skyEvent = "clouds_gather";
            weather_info.sky = SKY_CLOUDY;
        }
        break;

    case SKY_CLOUDY:
        if ( weather_info.mmhg <  970
                || ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
        {
            skyEvent = "precip_start";
            weather_info.sky = SKY_RAINING;
        }

        if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
        {
            skyEvent = "clouds_clear";
            weather_info.sky = SKY_CLOUDLESS;
        }
        break;

    case SKY_RAINING:
        if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
        {
            skyEvent = "lightning_start";
            weather_info.sky = SKY_LIGHTNING;
        }

        if ( weather_info.mmhg > 1030
                || ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
        {
            skyEvent = "precip_stop";
            weather_info.sky = SKY_CLOUDY;
        }
        break;

    case SKY_LIGHTNING:
        if ( weather_info.mmhg > 1010
                || ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
        {
            skyEvent = "storm_ease";
            weather_info.sky = SKY_RAINING;
            break;
        }
        break;
    }

    // Sky transition -> random multilingual variant from weather.xml, rendered
    // to each outdoor viewer in their own language. Winter precipitation reads
    // as snow via the winter variants of precip_start / precip_stop.
    if (!skyEvent.empty())
        weather_broadcast( skyEvent, ROOM_NO_WEATHER );
}


DLString time_of_day( )
{
    if ((time_info.hour > 16) && (time_info.hour < 24))
        return "вечера";
    if (time_info.hour < 4)
        return "ночи";
    if ((time_info.hour > 3) && (time_info.hour < 12))
        return "утра";
    if ((time_info.hour > 11) && (time_info.hour < 17))
        return "дня";
    return DLString::emptyString;
}

DLString time_of_day( lang_t lang )
{
    // RU/UA use a genitive daypart ("5 утра"); EN pairs the 12h clock with am/pm ("5 am").
    bool morning = (time_info.hour > 3)  && (time_info.hour < 12); // утра
    bool day     = (time_info.hour > 11) && (time_info.hour < 17); // дня
    bool evening = (time_info.hour > 16) && (time_info.hour < 24); // вечера
    bool night   = (time_info.hour < 4);                           // ночи

    if (lang == LANG_EN) {
        if (morning || night) return "am";
        if (day || evening)   return "pm";
        return DLString::emptyString;
    }

    if (lang == LANG_UA) {
        if (evening) return "вечора";
        if (night)   return "ночі";
        if (morning) return "ранку";
        if (day)     return "дня";
        return DLString::emptyString;
    }

    return time_of_day();
}

int hour_of_day( )
{
    return (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12;
}

DLString calendar_month( )
{
    return month_table[time_info.month].name;
}

DLString calendar_month( lang_t lang )
{
    const month_info &month = month_table[time_info.month];
    switch (lang) {
        case LANG_EN: return month.name_en;
        case LANG_UA: return month.name_ua;
        default:      return month.name;
    }
}

void make_date( ostringstream &buf )
{
    int day = time_info.day + 1;
    
    buf << hour_of_day( ) << " " << time_of_day( ) << ", " 
        << "День: " << day_name[day % 7] << ", " << day << "й, "
        << "Месяц " << calendar_month( ) << ", "
        << "Год " << time_info.year << ".";
}

CMDRUN( time )
{
    ostringstream buf;
    
    if (IS_SET(ch->in_room->room_flags, ROOM_NO_TIME)) {
        ch->pecho( _("Похоже, в этом месте время остановило свой ход.") );
        return;
    }

    // Output time of day and sunlight.
    buf << "Сейчас " << hour_of_day() << " " << time_of_day();
    if (RoomUtils::isOutside(ch))
        buf << ", " << sunlight();
    buf << ". ";

    // Output day of the week, day, season, month, year.
    int day = time_info.day + 1;
    const month_info &month = month_table[time_info.month];
    const season_info &season = season_table[month.season];

    buf << "Сегодня день " << day_name[day % 7] << ", "
        << day << "й день "
        << season.adjective << " месяца " << month.name << ", "
        << "года " << time_info.year << "." << endl;

    if (ch->is_npc())
        return;

    PCharacter *pch = ch->getPC();

    if (ch->getProfession( ) == prof_vampire && weather_info.sunlight == SUN_DARK)
        buf <<  "Время {rубивать{x, {Dсоздание ночи{x!" << endl;
  
    for (int bn = 0; bn < bonusManager->size(); bn++) {
        Bonus *bonus = bonusManager->find(bn);
        if (bonus->isActive(pch, time_info))
            bonus->reportTime(pch, buf);
    }
    ch->send_to(buf);

    if (ch->is_immortal( )) 
        ch->pecho( _("Мир Мечты загружен %s\r\nСистемное время: %s"),
                    Date::getTimeAsString( dreamland->getBootTime( ) ).c_str( ),
                    Date::getTimeAsString( dreamland->getCurrentTime( ) ).c_str( ) );
}

CMDRUN( weather )
{
    static const char * const sky_look[4] =
    {
        "ясное",
        "облачное",
        "дождливое",
        "во вспышках молний"
    };
    
    if (IS_SET(ch->in_room->room_flags, ROOM_NO_WEATHER)) {
        ch->pecho( _("Похоже, в этом месте погода всегда одинаковая.") );
        return;
    }
    
    if ( !RoomUtils::isOutside(ch) )
    {
        ch->pecho(_("В помещении погоду не видно -- попробуй выйти наружу!"));
        return;
    }
    
    const char *wind;
    int season = month_table[time_info.month].season;
    if (weather_info.change >= 0) {
        if (season == SEASON_SUMMER)
            wind = "теплый летний";
        else if (season == SEASON_SPRING)
            wind = "теплый весенний";
        else
            wind = "теплый южный";
    }
    else {
        if (season == SEASON_WINTER)
            wind = "холодный зимний";
        else if (season == SEASON_AUTUMN)
            wind = "пронизывающий осенний";
        else if (season == SEASON_SPRING)
            wind = "прохладный весенний";
        else
            wind = "резкий северный";
    }

    ch->pecho( _("Небо %s и дует %s ветер."),
        sky_look[weather_info.sky],
        wind
    );
}


/*
 * Set time and weather on world startup
 */
void weather_init( )
{
    long lhour, lday, lmonth;

    lhour                = dreamland->getWorldTime( );

    if ( lhour == 0 )
    {
            lhour = ( dreamland->getCurrentTime( ) - 650336715)
                    / (PULSE_TICK / dreamland->getPulsePerSecond( ));

            dreamland->setWorldTime( lhour );
            dreamland->save(false);
    }

    time_info.hour        = lhour  % 24;

    lday                = lhour  / 24;
    time_info.day        = lday   % 35;

    lmonth                = lday   / 35;
    time_info.month        = lmonth % 17;

    time_info.year        = lmonth / 17;

    if (time_info.hour < month_table[time_info.month].sunrise) 
        weather_info.sunlight = SUN_DARK;
    else if (time_info.hour < month_table[time_info.month].sunrise + 1) 
        weather_info.sunlight = SUN_RISE;
    else if (time_info.hour < month_table[time_info.month].sunset) 
        weather_info.sunlight = SUN_LIGHT;
    else if (time_info.hour < month_table[time_info.month].sunset + 1) 
        weather_info.sunlight = SUN_SET;
    else
        weather_info.sunlight = SUN_DARK;

    weather_info.change          = 0;
    weather_info.change_  = 0;
    weather_info.mmhg          = 960;
    weather_info.avg_mmhg = 1000;

    if ( time_info.month >= 7 && time_info.month <=12 )
        weather_info.mmhg += number_range( 1, 50 );
    else
        weather_info.mmhg += number_range( 1, 80 );

         if ( weather_info.mmhg <=  980 ) weather_info.sky = SKY_LIGHTNING;
    else if ( weather_info.mmhg <= 1000 ) weather_info.sky = SKY_RAINING;
    else if ( weather_info.mmhg <= 1020 ) weather_info.sky = SKY_CLOUDY;
    else                                  weather_info.sky = SKY_CLOUDLESS;
	
    if(DLScheduler::getThis()->getCurrentTick( ) == 0) {
	// Since this is happening on the world startup, let's notify users
	// the world is up!
	DLString msg;
    	msg = "Мир Мечты перезапустился и готов к игре, уииииии!";
    	send_to_discord_stream(":green_circle: " + msg);
    	send_telegram(msg);	
    }
}

/*--------------------------------------------------------------------------
 * Calendar and bonuses.
 *-------------------------------------------------------------------------*/
struct Calendar {
    Calendar(PCharacter *ch) {
        this->ch = ch;
    }

    void draw(ostringstream &out) {
        buf.str().clear();

        for (int season = 0; season < 4; season++) {
            draw_divider();

            for (int month = season*4; month < season*4 + 4; month++) 
                draw_month_name(month);
            
            for (int week = 0; week < 5; week++) {
                for (int month = 0; month < 4; month++) 
                    draw_week(week, season*4 + month);
                
                buf << endl;
            }
        }

        draw_divider();
        draw_month_name(month_table_size-1);
        for (int week = 0; week < 5; week++) 
            draw_week(week, month_table_size-1);
        
        buf << "{D--------------------+{x" << endl;
        out << buf.str();
    }

    // Plain-text calendar for screenreader users. Lists today's date, the
    // bonuses currently in effect (with days remaining for time-bound ones
    // like altar bonuses), and the next 35 days of upcoming bonuses.
    void draw_screenreader(ostringstream &out)
    {
        long today = day_of_epoch(time_info);

        out << fmt(0, _("Сегодня -- %d число месяца %s, год %d."),
                   time_info.day + 1,
                   month_table[time_info.month].name,
                   time_info.year)
            << endl << endl;

        // Active bonuses today.
        ostringstream active;
        for (int bn = 0; bn < bonusManager->size(); bn++) {
            Bonus *bonus = bonusManager->find(bn);
            if (!bonus->isActive(ch, time_info))
                continue;

            active << "  -- " << bonus->getRussianName();

            PCBonusData &data = ch->getBonuses().get(bn);
            if (data.start <= today && today <= data.end) {
                long days_left = data.end - today;
                int end_day   = data.end % 35;
                int end_month = (data.end / 35) % 17;
                if (days_left == 0) {
                    active << " (до конца дня)";
                } else {
                    active << fmt(0, _(": осталось %1$d дн%1$Iень|я|ей (до %2$d числа месяца %3$s)"),
                                  (int)days_left, end_day + 1, month_table[end_month].name);
                }
            } else {
                active << " (общий, действует сегодня для всех)";
            }
            active << "." << endl;
        }

        if (active.tellp() > 0)
            out << "Активные бонусы:" << endl << active.str() << endl;
        else
            out << "Сегодня нет активных бонусов." << endl << endl;

        // Upcoming bonuses in the next 35 days.
        ostringstream upcoming;
        for (int bn = 0; bn < bonusManager->size(); bn++) {
            Bonus *bonus = bonusManager->find(bn);
            if (bonus->isActive(ch, time_info))
                continue;

            for (int days_ahead = 1; days_ahead <= 35; days_ahead++) {
                long fut = today + days_ahead;
                struct time_info_data future_ti;
                future_ti.day   = fut % 35;
                future_ti.month = (fut / 35) % 17;
                future_ti.year  = fut / (35 * 17);
                future_ti.hour  = time_info.hour;

                if (!bonus->isActive(ch, future_ti))
                    continue;

                upcoming << "  -- " << bonus->getRussianName()
                         << fmt(0, _(": через %1$d дн%1$Iень|я|ей (%2$d числа месяца %3$s)"),
                                days_ahead,
                                future_ti.day + 1,
                                month_table[future_ti.month].name)
                         << "." << endl;
                break;
            }
        }

        if (upcoming.tellp() > 0)
            out << "Ближайшие бонусы:" << endl << upcoming.str();
    }

protected:
    PCharacter *ch;
    ostringstream buf;

    void draw_divider()
    {
        buf << "{D--------------------+--------------------+--------------------+--------------------" << endl;
    }

    char day_color(int day, int month)
    {
        struct time_info_data ti;
        ti.day = day;
        ti.month = month;
        ti.year = time_info.year;

        if (day == time_info.day && month == time_info.month)
            return 'R';
        
        for (int bn = 0; bn < bonusManager->size(); bn++) {
            Bonus *bonus = bonusManager->find(bn);
            if (bonus->isActive(ch, ti))
                return bonus->getColor();
        }
        return 'x';
    }

    void draw_day(int day, int month)
    {
        char color = day_color(day, month);
        if (color != 'x')
            buf << "{" << color;

        buf << fmt(0, "%2d", day+1);

        if (color != 'x')
            buf << "{x";

        if (day%7 != 6)
            buf << " ";
    }

    void draw_week(int week, int month)
    {
        for (int day = 0; day < 7; day++) 
            draw_day((week*7+day), month);

        if ((month+1)%4 != 0)
            buf << "{D|{x";

        if (month == month_table_size - 1)
            buf << endl;
    }

    void draw_month_name(int m)
    {
        const month_info &month = month_table[m];
        buf << " {" << season_table[month.season].color << fmt(0, "%-18s", month.name);

        if ((m+1)%4 != 0)
            buf << "{D|{x";
        else
            buf << "{x" << endl;

        if (m == month_table_size - 1)
            buf << endl;
    }
};

CMDRUN( calendar )
{
    if (ch->is_npc())
        return;

    PCharacter *pch = ch->getPC();
    ostringstream buf;
    Calendar calendar(pch);

    if (uses_screenreader(pch)) {
        calendar.draw_screenreader(buf);
    } else {
        calendar.draw(buf);
        buf << "{WУсловные обозначения цветом{x: " << endl
            << "         {RX{x: сегодня, {cX{x: больше опыта за убийства, {bX{x: меньше расход маны," << endl
            << "         {GX{x: быстрее прокачка умений, {DX{x: низкие цены, {MX{x: удачнее воровские навыки," << endl
            << "         {gX{x: существа быстрее улучшают параметры" << endl;
    }
    ch->send_to(buf);
}

