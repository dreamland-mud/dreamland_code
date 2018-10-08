/* $Id$
 *
 * ruffina, 2004
 */
#include "wearlocation.h"
#include "wearloc_codes.h"

static bool compareForWear( int a, int b )
{
    return wearlocationManager->find( a )->getOrderWear( ) 
           <= wearlocationManager->find( b )->getOrderWear( );
}

int WearlocationManager::wear( Object *obj, int flags )
{
    SortedIndexes order;
    SortedIndexes::iterator o;
    
    sortIndexes( order, compareForWear );

    for (o = order.begin( ); o != order.end( ); o++)
        if (find( *o )->matches( obj ))
            return find( *o )->wear( obj, flags );

    return RC_WEAR_NOMATCH;
}

static bool compareForDisplay( int a, int b)
{
    return wearlocationManager->find( a )->getOrderDisplay( ) 
           <= wearlocationManager->find( b )->getOrderDisplay( );
}

void WearlocationManager::display( Character *ch, Wearlocation::DisplayList &eq )
{
    SortedIndexes order;
    SortedIndexes::iterator o;
    
    sortIndexes( order, compareForDisplay );
    
    for (o = order.begin( ); o != order.end( ); o++)
        find( *o )->display( ch, eq );
}

