/* $Id$
 *
 * ruffina, 2004
 */
#include "flags.h"
#include "enumeration.h"
#include "globalbitvector.h"
#include "race.h"
#include "pcrace.h"

/*-------------------------------------------------------------------
 * Race
 *------------------------------------------------------------------*/
Race::Race( )
{
}

Race::Race( const DLString &n ) : name( n )
{
}

Race::~Race( )
{
}

const DLString& Race::getName( ) const
{
    return name;
}

bool Race::isValid( ) const
{
    return false;
}

PCRace emptyPCRace( "none" );

PCRace * Race::getPC( ) 
{
    return &emptyPCRace;
}

bool Race::isPC( ) const
{
    return false;
}

const Flags & Race::getDet( ) const
{
    return Flags::emptyFlags;
}
const Flags & Race::getAct( ) const
{
    return Flags::emptyFlags;
}
const Flags & Race::getAff( ) const
{
    return Flags::emptyFlags;
}
const Flags & Race::getOff( ) const
{
    return Flags::emptyFlags;
}
const Flags & Race::getImm( ) const
{
    return Flags::emptyFlags;
}
const Flags & Race::getRes( ) const
{
    return Flags::emptyFlags;
}
const Flags & Race::getVuln( ) const
{
    return Flags::emptyFlags;
}
const Flags & Race::getForm( ) const
{
    return Flags::emptyFlags;
}
const Flags & Race::getParts( ) const
{
    return Flags::emptyFlags;
}
const GlobalBitvector & Race::getWearloc( ) const
{
    return GlobalBitvector::emptyBitvector;
}
const Enumeration & Race::getSize( ) const
{
    return Enumeration::emptyEnumeration;
}
Flags Race::getAttitude( const Race & ) const
{
    return Flags::emptyFlags;
}

const DLString & Race::getMaleName( ) const
{
    return DLString::emptyString;
}
const DLString & Race::getNeuterName( ) const
{
    return getMaleName( );
}
const DLString & Race::getFemaleName( ) const
{
    return DLString::emptyString;
}
const DLString & Race::getMltName( ) const
{
    return DLString::emptyString;
}

DLString Race::getNameFor( Character *, Character *, const Grammar::Case & ) const
{
    return getName( );
}

/*-------------------------------------------------------------------
 * RaceManager
 *------------------------------------------------------------------*/
RaceManager* raceManager = 0;

RaceManager::RaceManager( ) 
{
    checkDuplicate( raceManager );
    raceManager = this;
}

RaceManager::~RaceManager( )
{
    raceManager = 0;
}

GlobalRegistryElement::Pointer RaceManager::getDumbElement( const DLString &name ) const
{
    return Race::Pointer( NEW, name );
}

const PCRace * RaceManager::findUnstrictPC( const DLString &name )
{
    Race *race;
    
    if (name.empty( ))
	return NULL;
	
    if (( race = findExisting( name ) )
	&& race->isValid( ) 
	&& race->isPC( ))
    {
	return race->getPC( );
    }
    
    for (int i = 0; i < size( ); i++) {
	race = find( i );
	
	if (race->isValid( ) 
	    && race->isPC( )
	    && name.strPrefix( race->getName( ) ))
	{
	    return race->getPC( );
	}
    }

    return NULL;
}

GLOBALREF_IMPL(Race, ' ')
XMLGLOBALREF_IMPL(Race)
