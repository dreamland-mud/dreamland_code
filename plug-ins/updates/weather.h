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
using namespace std;

class DLString;

void sunlight_update( );
void weather_update( );
void weather_init( );
void make_date( ostringstream & );
DLString time_of_day( );
int hour_of_day( );
DLString sunlight( );
DLString calendar_month( );

#endif
