/* $Id$
 *
 * ruffina, 2004
 */
#include "liquid.h"
#include "flags.h"

#include "mercdb.h"

LIQ(water);

Liquid * LiquidManager::random( bitstring_t flags )
{
    Liquid *result = &*liq_water;
    int count = 0;
    
    for (int i = 0; i < size( ); i++)
	if (find( i )->getFlags( ).isSet( flags ))
	    if (number_range( 0, count++ ) == 0) 
		result = find( i );

    return result;
}

