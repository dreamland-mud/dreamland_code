/* $Id: globalbitvector.cpp,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2006
 */
#include <sstream>
#include "globalbitvector.h"
#include "stringlist.h"

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

DLString GlobalBitvector::toString( char joiner ) const
{
    if (!registry)
        return DLString::emptyString;

    StringList result;

    for (unsigned int b = 0; b < bits.size( ); b++)
        if (bits[b]) {
            GlobalRegistryElement *e = registry->find(b);
            if (e)
                result.push_back(e->getName().quote());
        }

    return result.join(joiner);
}

DLString GlobalBitvector::toRussianString( char gcase, char joiner ) const
{
    if (!registry)
        return DLString::emptyString;

    StringList result;

    for (unsigned int b = 0; b < bits.size( ); b++)
        if (bits[b]) {
            GlobalRegistryElement *e = registry->find(b);
            if (e)
                result.push_back(e->getRussianName().ruscase(gcase).quote());
        }

    return result.join(joiner);
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

