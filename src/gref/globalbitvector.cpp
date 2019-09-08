/* $Id: globalbitvector.cpp,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2006
 */
#include <sstream>
#include "globalbitvector.h"

const GlobalBitvector GlobalBitvector::emptyBitvector;

GlobalBitvector::GlobalBitvector( )
                         : registry( NULL )
{
}

GlobalBitvector::GlobalBitvector( GlobalRegistryBase *reg )
                         : registry( reg )
{
    bits.resize( reg->size( ), false );
}

GlobalBitvector::~GlobalBitvector( )
{
}

void GlobalBitvector::fromString( const DLString &source )
{
    DLString arguments = source, arg;
    
    clear( );

    if (!registry)
        return;

    while (!arguments.empty( )) {
        arg = arguments.getOneArgument( );
        set( registry->lookup( arg ) );
    }
}

DLString GlobalBitvector::toString( ) const
{
    ostringstream result; 

    if (!registry)
        return DLString::emptyString;

    for (unsigned int b = 0; b < bits.size( ); b++)
        if (bits[b]) {
            GlobalRegistryElement *e = registry->find(b);
            if (e)
                result << e->getName().quote() << " ";
        }

    DLString r = result.str( );
    r.stripRightWhiteSpace();
    return r;
}

DLString GlobalBitvector::toRussianString( ) const
{
    ostringstream result; 

    if (!registry)
        return DLString::emptyString;

    for (unsigned int b = 0; b < bits.size( ); b++)
        if (bits[b]) {
            GlobalRegistryElement *e = registry->find(b);
            if (e)
                result << e->getRussianName().quote() << " ";
        }

    DLString r = result.str( );
    r.stripRightWhiteSpace();
    return r;
}

vector<int> GlobalBitvector::toArray( ) const
{
    vector<int> array;

    if (registry)
        for (unsigned int b = 0; b < bits.size( ); b++) 
            if (bits[b])
                array.push_back( b );

    return array;
}

