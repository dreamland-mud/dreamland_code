/* $Id$
 *
 * ruffina, 2004
 */
#include "flags.h"
#include "globalarray.h"
#include "liquid.h"

/*-------------------------------------------------------------------
 * Liquid
 *------------------------------------------------------------------*/
Liquid::Liquid( )
{
}

Liquid::Liquid( const DLString &n ) : name( n )
{
}

Liquid::~Liquid( )
{
}

const DLString &Liquid::getName( ) const
{
    return name;
}

bool Liquid::isValid( ) const
{
    return false;
}

const DLString &Liquid::getShortDescr( ) const
{
    return DLString::emptyString;
}

const DLString &Liquid::getColor( ) const
{
    return DLString::emptyString;
}

int Liquid::getSipSize( ) const
{
    return 0;
}

GlobalArray & Liquid::getDesires( ) 
{
    static GlobalArray emptyArray;
    return emptyArray;
}

const Flags & Liquid::getFlags( ) const
{
    return Flags::emptyFlags;
}


/*-------------------------------------------------------------------
 * LiquidManager
 *------------------------------------------------------------------*/
LiquidManager* liquidManager = 0;

LiquidManager::LiquidManager( ) 
{
    checkDuplicate( liquidManager );
    liquidManager = this;
}

LiquidManager::~LiquidManager( )
{
    liquidManager = 0;
}

GlobalRegistryElement::Pointer LiquidManager::getDumbElement( const DLString &name ) const
{
    return Liquid::Pointer( NEW, name );
}

GLOBALREF_IMPL(Liquid, ' ')
XMLGLOBALREF_IMPL(Liquid)
