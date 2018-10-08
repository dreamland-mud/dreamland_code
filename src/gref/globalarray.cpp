/* $Id: globalarray.cpp,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2006
 */
#include "globalarray.h"

static int zeroValue;

const GlobalArray GlobalArray::emptyArray;

GlobalArray::GlobalArray( )
                         : registry( NULL )
{
}

GlobalArray::GlobalArray( GlobalRegistryBase *reg )
                         : registry( reg )
{
    clear( );
}

GlobalArray::~GlobalArray( )
{
}

void GlobalArray::clear( )
{
    vector<int>::clear( );

    if (registry)
        resize( registry->size( ) );
}

int & GlobalArray::operator [] (size_type ndx)
{
    if (!registry)
        return zeroValue;
    if (!registry->goodIndex( ndx ))
        return zeroValue;
    if (ndx >= size( ))
        resize( ndx + 1 );
    
    return vector<int>::operator [](ndx);
}

const int & GlobalArray::operator [] (size_type ndx) const
{
    return vector<int>::operator [](ndx);
}

