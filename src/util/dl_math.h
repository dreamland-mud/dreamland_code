/* $Id: dl_math.h,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#ifndef DL_MATH_H
#define DL_MATH_H

int        number_fuzzy        ( int number );
int        number_range        ( int from, int to );
bool    chance( int );
int        number_disperse        ( int value, int disperse );
int        number_percent        ( );
int        number_door        ( );
int        number_bits        ( int width );
long    number_mm       ( );
int        dice                ( int number, int size );
int        interpolate        ( short level, int value_00, int value_32 );
void    init_mm( );

#endif

