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
#include "bonusflags.h"
#include "dreamland.h"
#include "descriptor.h"
#include "mercdb.h"
#include "today.h"
#include "handler.h"
#include "act.h"
#include "merc.h"
#include "def.h"

PROF(vampire);

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

enum{
    SEASON_WINTER = 0,
    SEASON_SPRING,
    SEASON_SUMMER,
    SEASON_AUTUMN
};
struct season_info {
    int number;
    const char *name;
    const char *short_descr;
    const char *adjective;
    char color;
};

const struct season_info season_table [4] = {
    { SEASON_WINTER, "зима",  "зим|а|ы|е|у|ой|е",  "зимнего",   'C' },
    { SEASON_SPRING, "весна", "весн|а|ы|е|у|ой|е", "весеннего", 'g' },
    { SEASON_SUMMER, "лето",  "лет|о|а|у|о|ом|е",  "летнего",   'Y' },
    { SEASON_AUTUMN, "осень", "осен|ь|и|и|ь|ью|и", "осеннего",  'y' },
};

struct month_info {
    const char *name;
    int pressure;
    int sunrise;
    int sunset;
    int season;
};

const struct month_info month_table [17] = {
  { "Зимы",                   -12,    7, 17, SEASON_WINTER },
  { "Зимнего Волка",          -14,    7, 18, SEASON_WINTER },
  { "Холодного Гиганта",      -14,    7, 18, SEASON_WINTER },
  { "Древних Воинств",        -12,    7, 19, SEASON_WINTER },   
  { "Великих Битв",            -8,    6, 19, SEASON_SPRING },
  { "Весны",                   -4,    5, 19, SEASON_SPRING }, 
  { "Природы",                  0,    5, 21, SEASON_SPRING },
  { "Тщетности",                4,    5, 22, SEASON_SPRING },  
  { "Дракона",                  8,    4, 22, SEASON_SUMMER },
  { "Солнца",                  12,    4, 22, SEASON_SUMMER },
  { "Жары",                    16,    5, 21, SEASON_SUMMER },    
  { "Битвы",                   12,    5, 20, SEASON_SUMMER },   
  { "Темноты",                  8,    6, 20, SEASON_AUTUMN },
  { "Тени",                     4,    6, 19, SEASON_AUTUMN },
  { "Длинных Теней",            0,    7, 18, SEASON_AUTUMN },         
  { "Абсолютной Темноты",      -4,    8, 17, SEASON_AUTUMN },   
  { "Великого Зла",            -8,    8, 17, SEASON_WINTER },
};

const int month_table_size = sizeof(month_table) / sizeof(month_info);

const char *        const        day_name        [] =
{
    "Луны", "Быка", "Лжи", "Грома", "Свободы",
    "Великих Богов", "Солнца"
};

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

/*
 * Daytime change
 */
void sunlight_update( )
{
    ostringstream buf, tbuf;
    int this_season = month_table[time_info.month].season;
    int this_day = time_info.day;
    int this_month = time_info.month;
    int this_year = time_info.year;

    dreamland->setWorldTime( dreamland->getWorldTime( ) + 1 );
    ++time_info.hour;

    if (time_info.hour == month_table[time_info.month].sunrise) {
        weather_info.sunlight = SUN_RISE;
        buf << "Первые лучи солнца пробиваются с востока." << endl;
    }
    else if (time_info.hour == month_table[time_info.month].sunrise + 1) {
        weather_info.sunlight = SUN_LIGHT;
        buf << "Начался новый день." << endl;
    }
    else if (time_info.hour == month_table[time_info.month].sunset) {
        weather_info.sunlight = SUN_SET;
        buf << "Солнце медленно прячется за горизонтом." << endl;
    }
    else if (time_info.hour == month_table[time_info.month].sunset + 1) {
        weather_info.sunlight = SUN_DARK;
        buf << "Начинается ночь." << endl;
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
    
    string buf_str = buf.str();
    string tbuf_str = tbuf.str();

    if (!buf_str.empty() || !tbuf_str.empty()) {
        for (Descriptor *d = descriptor_list; d != 0; d = d->next) {
            if (d->connected != CON_PLAYING)
                continue;

            Character *ch = d->character;

            if (ch
                && IS_AWAKE(ch) 
                && !IS_SET(ch->in_room->room_flags, ROOM_NO_TIME))
            {
                if (IS_OUTSIDE(ch) && !buf_str.empty())
                    ch->send_to(buf_str);
                if (!tbuf_str.empty())
                    ch->send_to(tbuf_str);
            }
        }
    }

    DLString newTime;
    if (!buf_str.empty()) {
        if (weather_info.sunlight >= SUN_DARK && weather_info.sunlight <= SUN_SET)
            newTime = sunlight_en[weather_info.sunlight];
    }

    for(int i=0;i<MAX_KEY_HASH;i++)
        for(Room *r = room_index_hash[i]; r; r = r->next) {
            FENIA_VOID_CALL(r, "Time", "s", newTime.c_str( ));
        }
}

DLString sunlight( )
{
    if (weather_info.sunlight >= SUN_DARK && weather_info.sunlight <= SUN_SET)
        return sunlight_ru[weather_info.sunlight];

    return DLString::emptyString;
}

/*
 * Weather change.
 */
void weather_update( )
{
    ostringstream buf;

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
            buf << "Небо затягивается тучами." << endl;
            weather_info.sky = SKY_CLOUDY;
        }
        break;

    case SKY_CLOUDY:
        if ( weather_info.mmhg <  970
                || ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
        {
            buf << "Начинается дождь." << endl;
            weather_info.sky = SKY_RAINING;
        }

        if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
        {
            buf << "Тучи рассеиваются." << endl;
            weather_info.sky = SKY_CLOUDLESS;
        }
        break;

    case SKY_RAINING:
        if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
        {
            buf << "Молнии сверкают на небе." << endl;
            weather_info.sky = SKY_LIGHTNING;
        }

        if ( weather_info.mmhg > 1030
                || ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
        {
            buf << "Дождь прекращается." << endl;
            weather_info.sky = SKY_CLOUDY;
        }
        break;

    case SKY_LIGHTNING:
        if ( weather_info.mmhg > 1010
                || ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
        {
            buf << "Молний больше нет." << endl;
            weather_info.sky = SKY_RAINING;
            break;
        }
        break;
    }

    if (!buf.str( ).empty( )) {
        Descriptor *d;
        Character *ch;
        
        for (d = descriptor_list; d != 0; d = d->next) {
            if (d->connected != CON_PLAYING)
                continue;

            ch = d->character;

            if (ch
                && IS_OUTSIDE(ch) 
                && IS_AWAKE(ch) 
                && !IS_SET(ch->in_room->room_flags, ROOM_NO_WEATHER))
            {
                ch->send_to( buf );
            }
        }
    }
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

int hour_of_day( )
{
    return (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12;
}

DLString calendar_month( )
{
    return month_table[time_info.month].name;
}

void make_date( ostringstream &buf )
{
    int day = time_info.day + 1;
    
    buf << hour_of_day( ) << " " << time_of_day( ) << ", " 
        << "День: " << day_name[day % 7] << ", " << day << "й, "
        << "Месяц " << calendar_month( ) << ", "
        << "Год " << time_info.year << "." << endl;
}

CMDRUN( time )
{
    ostringstream buf;
    
    if (IS_SET(ch->in_room->room_flags, ROOM_NO_TIME)) {
        ch->println( "Похоже, в этом месте время остановило свой ход." );
        return;
    }

    // Output time of day and sunlight.
    buf << "Сейчас " << hour_of_day() << " " << time_of_day();
    if (IS_OUTSIDE(ch))
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

    if (ch->getProfession( ) == prof_vampire && weather_info.sunlight == SUN_DARK)
        buf <<  "Время {rубивать{x, {Dсоздание ночи{x!" << endl;
    

    if (ch->getReligion()->hasBonus(ch, RB_MANA, time_info))
        buf << fmt(0, "Сегодня благодаря %N1 заклинания и молитвы отнимают меньше маны.",
                      ch->getReligion()->getRussianName().c_str()) << endl;
    else if (today_mana_bonus(ch, time_info))
        buf << "В этот день заклинания и молитвы отнимают меньше маны." << endl;

    if (ch->getReligion()->hasBonus(ch, RB_LEARN, time_info))
        buf << fmt(0, "Сегодня %N1 помогает тебе быстрее учиться.",
                      ch->getReligion()->getRussianName().c_str()) << endl;
    else if (today_learn_bonus(ch, time_info))
        buf << "В этот день умения учатся быстрее чем обычно." << endl;

    if (ch->getReligion()->hasBonus(ch, RB_KILLEXP, time_info))
        buf << fmt(0, "Сегодня %N1 дарит тебе больше опыта за убийства.",
                      ch->getReligion()->getRussianName().c_str()) << endl;
    else if (today_kill_bonus(ch, time_info))
        buf << "В этот день можно получить больше опыта за убийства." << endl;     

    ch->send_to(buf);

    if (ch->is_immortal( )) 
        ch->printf( "Dream Land загружен %s\r\nСистемное время: %s\r\n",
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
        ch->println( "Похоже, в этом месте погода всегда одинаковая." );
        return;
    }
    
    if ( !IS_OUTSIDE(ch) )
    {
        ch->send_to( "Ты не можешь видеть погоду в помещении.\n\r");
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
            wind = "прохладный северный";
    }

    ch->printf( "Небо %s и дует %s ветер.\n\r",
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
}

/*--------------------------------------------------------------------------
 * Calendar and bonuses.
 *-------------------------------------------------------------------------*/
struct Calendar {
    Calendar(Character *ch) {
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

protected:
    Character *ch;
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
        if (today_kill_bonus(ch, ti)) 
            return 'c';
        if (today_mana_bonus(ch, ti))
            return 'b';
        if (today_learn_bonus(ch, ti))
            return 'G';
        return 'x';
    }

    void draw_day(int day, int month)
    {
        char color = day_color(day, month);
        if (color != 'x')
            buf << "{" << color;

        buf << dlprintf("%2d", day+1);

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
        buf << " {" << season_table[month.season].color << dlprintf("%-18s", month.name);

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
    ostringstream buf;
    
    Calendar calendar(ch); 

    calendar.draw(buf);
    buf << "{WЛегенда{x: {RX{x - сегодняшний день, {cX{x - больше опыта за убийства, {bX{x - меньше расход маны," << endl
        << "         {GX{x - быстрее учатся умения" << endl;
    ch->send_to(buf);
}

