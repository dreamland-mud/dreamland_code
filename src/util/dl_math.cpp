/* $Id: dl_math.cpp,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#include <algorithm>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dl_math.h"

#ifndef __MINGW32__

#else
#define OLD_RAND
#endif

using std::max;

/**
 * Stick a little fuzz on a number: return number, or number -1, or number + 1
 */
int number_fuzzy( int number )
{
    switch ( number_bits( 2 ) )
    {
    case 0:  number -= 1; break;
    case 3:  number += 1; break;
    }

    return max( 1, number );
}



/**
 * Generate a random number in the [from..to] interval.
 */
int number_range( int from, int to )
{
    if ( from >= to )
        return from;

    return from + number_mm( ) % (to - from + 1);
}

/**
 * A 'num out of 100' chance of success.
 */
bool chance(int num)
{
    if (number_range(1,100) <= num) return true;
    else return false;
}

/**
 *@param value - Value to disperse
 *@param disperse - Despersion in percents
 */
int number_disperse( int value, int disperse )
{
    if ( disperse == 0 ) return value;
    if ( disperse < 0 ) return value;
    
    int diffraction = value * disperse / 100;
    int result = value - ( number_range( 0, diffraction * 2 ) - diffraction );
    return result > 0 ? result : 1;
}


/**
 * Generate a percentile roll: a number from 1 to 100.
 */
int number_percent( void )
{
    int percent;

    while ( (percent = number_mm() & (128-1) ) > 99 )
        ;

    return 1 + percent;
}



/**
 * Generate a random door.
 */
int number_door( void )
{
    int door;

    while ( ( door = number_mm() & (8-1) ) >= 6)
        ;

    return door;
}

/**
 * Generate a random number between 0 and 2^width - 1.
 */
int number_bits( int width )
{
    return number_mm( ) & ( ( 1 << width ) - 1 );
}




/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */

/* I noticed streaking with this random number generator, so I switched
   back to the system srandom call.  If this doesn't work for you,
   define OLD_RAND to use the old system -- Alander */

#if defined (OLD_RAND)
static  int     rgiState[2+55];
#endif

/*
 * Init random number generator.
 */
void init_mm( )
{
#if defined (OLD_RAND)
    int *piState;
    int iState;

    piState     = &rgiState[2];

    piState[-2] = 55 - 55;
    piState[-1] = 55 - 24;

    piState[0]  = ((int) clock( )) & ((1 << 30) - 1);
    piState[1]  = 1;
    for ( iState = 2; iState < 55; iState++ )
    {
        piState[iState] = (piState[iState-1] + piState[iState-2])
                        & ((1 << 30) - 1);
    }
#else
    srandom(time(0)^getpid());
#endif
    return;
}



long number_mm( void )
{
#if defined (OLD_RAND)
    int *piState;
    int iState1;
    int iState2;
    int iRand;

    piState             = &rgiState[2];
    iState1             = piState[-2];
    iState2             = piState[-1];
    iRand               = (piState[iState1] + piState[iState2])
                        & ((1 << 30) - 1);
    piState[iState1]    = iRand;
    if ( ++iState1 == 55 )
        iState1 = 0;
    if ( ++iState2 == 55 )
        iState2 = 0;
    piState[-2]         = iState1;
    piState[-1]         = iState2;
    return iRand >> 6;
#else
    return random() >> 6;
#endif
}


/*
 * Roll some dice.
 */
int dice( int number, int size )
{
    int idice;
    int sum;

    switch ( size )
    {
    case 0: return 0;
    case 1: return number;
    }

    for ( idice = 0, sum = 0; idice < number; idice++ )
        sum += number_range( 1, size );

    return sum;
}



/*
 * Simple linear interpolation.
 */
int interpolate( short level, int value_00, int value_32 )
{
    return value_00 + level * (value_32 - value_00) / 32;
}

float linear_interpolation(float x, float x1, float x2, float y1, float y2 )
{
    return y1 + ((x - x1) / (x2 - x1)) * (y2 - y1);
}




