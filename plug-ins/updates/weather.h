/* $Id: weather.h,v 1.1.2.2 2008/02/23 13:41:51 rufina Exp $
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
#ifndef __WEATHER_H__
#define __WEATHER_H__

#include <sstream>
#include "lang.h"
using namespace std;

class DLString;

void sunlight_update( );
void weather_update( );
void weather_init( );
void make_date( ostringstream & );
DLString time_of_day( );
DLString time_of_day( lang_t lang );
int hour_of_day( );
DLString sunlight( );
DLString sunlight( lang_t lang );
DLString calendar_month( );
DLString calendar_month( lang_t lang );

enum{
    SEASON_WINTER = 0,
    SEASON_SPRING,
    SEASON_SUMMER,
    SEASON_AUTUMN
};

struct season_info {
    int number;
    const char *name;         // EN season key, e.g. "winter"
    const char *rname;         // RU nominative, e.g. "зима"
    const char *short_descr;   // RU Flexer pad for russian_case()
    const char *adjective;     // RU genitive adjective, e.g. "зимнего"
    const char *adjective_en;  // EN attributive, e.g. "winter"
    const char *adjective_ua;  // UA genitive adjective, e.g. "зимового"
    const char *genitive_en;   // EN genitive noun, e.g. "winter"
    const char *genitive_ua;   // UA genitive noun, e.g. "зими"
    char color;
};

extern const struct season_info season_table [];

/** Return English name of the current season. */
DLString season();

#endif
