/* $Id$
 *
 * ruffina, 2004
 */
#include "globalregistryelement.h"

GlobalRegistryElement::GlobalRegistryElement( )
         : index( -1 )
{
}

GlobalRegistryElement::~GlobalRegistryElement( )
{
}

const DLString &GlobalRegistryElement::getRussianName( ) const
{
    return getName( );
}

bool GlobalRegistryElement::matchesStrict( const DLString &str ) const 
{
    if (str.empty())
        return false;
    
    DLString lstr = str.toLower();
    if (lstr == getName().toLower())
        return true;

    if (lstr == getRussianName().ruscase('1').toLower())
        return true;

    return false;
}

bool GlobalRegistryElement::matchesUnstrict( const DLString &str ) const 
{
    if (str.empty() || getName().empty())
        return false;
    
    if (str.strPrefix(getName()))
        return true;

    if (str.strPrefix(getRussianName().ruscase('1')))
        return true;
    
    return false;
}
