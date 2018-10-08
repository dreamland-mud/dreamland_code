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
#include "dreamland.h"
#include "descriptor.h"
#include "mercdb.h"
#include "handler.h"
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

struct month_info {
    const char *name;
    int pressure;
    int sunrise;
    int sunset;
};

const struct month_info month_table [17] = {
  { "Зимы",                   -12,    7, 17 },
  { "Зимнего Волка",           -14,    7, 18 },
  { "Холодного Гиганта",   -14,    7, 18 },
  { "Древних Воинств",           -12,    7, 19 },   /* весна */
  { "Великих Битв",           -8,     6, 19 },
  { "Весны",                   -4,     5, 19 }, 
  { "Природы",                   0,      5, 21 },
  { "Тщетности",           4,      5, 22 },   /* лето */
  { "Дракона",                   8,      4, 22 },
  { "Солнца",                   12,     4, 22 },
  { "Жары",                   16,     5, 21 },    
  { "Битвы",                   12,     5, 20 },   /* осень */
  { "Темноты",                   8,      6, 20 },
  { "Тени",                   4,      6, 19 },
  { "Длинных Теней",           0,      7, 18 },         
  { "Абсолютной Темноты",  -4,     8, 17 },   /* зима */
  { "Великого Зла",           -8,     8, 17 },
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
    ostringstream buf;

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
                && !IS_SET(ch->in_room->room_flags, ROOM_NO_TIME))
            {
                ch->send_to( buf );
            }
        }
    }

    DLString newTime;
    if (!buf.str( ).empty( )) {
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


const char *        const        day_name        [] =
{
    "Луны", "Быка", "Лжи", "Грома", "Свободы",
    "Великих Богов", "Солнца"
};

static const char * adjective_ext(int d) 
{
    static const char * EXT_ONE = "ый";
    static const char * EXT_TWO = "ой";
    static const char * EXT_THREE = "ий";
    
    switch (d % 10) {
    case 0: 
        return d == 0 ? EXT_TWO : EXT_ONE;
    case 1: case 4: case 5: case 9:
        return EXT_ONE;
    case 3:
        return EXT_THREE;
    default:
        return EXT_TWO;
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
    const char * suf = adjective_ext( day );
    
    buf << hour_of_day( ) << " " << time_of_day( ) << ", " 
        << "День: " << day_name[day % 7] << ", " << day << "-" << suf << "  "
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

    buf << "Сейчас: ";
    make_date( buf );

    if (ch->getProfession( ) == prof_vampire && weather_info.sunlight == SUN_DARK)
    {
        buf <<  "Время {rубивать{x, {Dсоздание ночи{x!" << endl;
    }

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

    ch->printf( "Небо %s и %s.\n\r",
        sky_look[weather_info.sky],
        weather_info.change >= 0
        ? "дует теплый южный ветер"
        : "дует холодный северный ветер"
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
