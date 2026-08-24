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

const DLString &Liquid::getRussianName( ) const
{
    return getShortDescr();
}

// Liquids keep their per-language names in shortDescr (see the <shortDescr
// l="ua"> forms). getUkrainianName is what GlobalBitvector::toString queries for
// a UA viewer -- without this it returned empty and the 'poured liquid' affect
// smell leaked the Russian liquid name ("запах воды" instead of "запах води").
const DLString &Liquid::getUkrainianName( ) const
{
    return getShortDescr( LANG_UA );
}

bool Liquid::isValid( ) const
{
    return false;
}

const DLString &Liquid::getShortDescr( lang_t ) const
{
    return DLString::emptyString;
}

const DLString &Liquid::getColor( lang_t ) const
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
    setRegistryName("liquid");    
    saveRegistryName();
}

LiquidManager::~LiquidManager( )
{
    eraseRegistryName();
    liquidManager = 0;
}

GlobalRegistryElement::Pointer LiquidManager::getDumbElement( const DLString &name ) const
{
    return Liquid::Pointer( NEW, name );
}

GLOBALREF_IMPL(Liquid, ' ')
XMLGLOBALREF_IMPL(Liquid)
