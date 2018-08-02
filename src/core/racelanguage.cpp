/* $Id$
 *
 * ruffina, 2004
 */
#include "racelanguage.h"

/*-------------------------------------------------------------------
 * RaceLanguage
 *------------------------------------------------------------------*/
RaceLanguage::RaceLanguage( )
{
}

RaceLanguage::RaceLanguage( const DLString &n ) : name( n )
{
}

RaceLanguage::~RaceLanguage( )
{
}

const DLString &RaceLanguage::getName( ) const
{
    return name;
}

bool RaceLanguage::isValid( ) const
{
    return false;
}

const DLString &RaceLanguage::getShortDescr( ) const
{
    return DLString::emptyString;
}

bool RaceLanguage::available( Character * ) const
{
    return false;
}

DLString RaceLanguage::translate( const DLString &, Character *, Character * ) const
{
    return DLString::emptyString;
}

/*-------------------------------------------------------------------
 * RaceLanguageManager
 *------------------------------------------------------------------*/
RaceLanguageManager* raceLanguageManager = 0;

RaceLanguageManager::RaceLanguageManager( ) 
{
    checkDuplicate( raceLanguageManager );
    raceLanguageManager = this;
}

RaceLanguageManager::~RaceLanguageManager( )
{
    raceLanguageManager = 0;
}

GlobalRegistryElement::Pointer RaceLanguageManager::getDumbElement( const DLString &name ) const
{
    return RaceLanguage::Pointer( NEW, name );
}

GLOBALREF_IMPL(RaceLanguage, ' ')
XMLGLOBALREF_IMPL(RaceLanguage)
