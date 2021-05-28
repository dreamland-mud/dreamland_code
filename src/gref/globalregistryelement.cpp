/* $Id$
 *
 * ruffina, 2004
 */
#include "globalregistryelement.h"
#include "dl_strings.h"

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

    DLString str0 = str.colourStrip();
    if (is_name(str0.c_str(), getName().c_str()))
        return true;

    DLString rname = getRussianName().ruscase('1').colourStrip();
    if (is_name(str0.c_str(), rname.c_str()))
        return true;
    
    return false;
}

bool GlobalRegistryElement::matchesSubstring( const DLString &str ) const
{
    if (str.empty() || getName().empty())
        return false;

    if (getName().find(str) != DLString::npos)
        return true;

    if (getRussianName().find(str) != DLString::npos)
        return true;

    return false;
}