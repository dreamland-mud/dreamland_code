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
    return !str.empty( ) && str == getName( );
}

bool GlobalRegistryElement::matchesUnstrict( const DLString &str ) const 
{
    if (str.empty( ) || getName( ).empty( ))
        return false;

    if (!str.strPrefix( getName( ) ))
        return false;
// TODO unstrict match by russian name, all cases
// Use getRussianName method in child classes instead of custom getRusName
    return true;
}
