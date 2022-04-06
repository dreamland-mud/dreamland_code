/* $Id$
 *
 * ruffina, 2004
 */
#include "flags.h"
#include "enumeration.h"
#include "globalarray.h"
#include "pcrace.h"

/*-------------------------------------------------------------------
 * PCRace
 *------------------------------------------------------------------*/
PCRace::PCRace( )
{
}

PCRace::PCRace( const DLString &n ) : Race( n )
{
}

PCRace::~PCRace( )
{
}

PCRace * PCRace::getPC( ) 
{
    return this;
}

bool PCRace::isPC( ) const
{
    return true;
}

const Flags & PCRace::getAlign( ) const
{
    return Flags::emptyFlags;
}
int PCRace::getMinAlign( ) const
{
    return 0;
}
int PCRace::getMaxAlign( ) const
{
    return 0;
}
GlobalArray & PCRace::getClasses( ) 
{
    static GlobalArray emptyArray;
    return emptyArray;
}
GlobalArray & PCRace::getEquipment( ) 
{
    static GlobalArray emptyArray;
    return emptyArray;
}
int PCRace::getPoints( ) const
{
    return 0;
}
int PCRace::getHpBonus( ) const
{
    return 0;
}
int PCRace::getManaBonus( ) const
{
    return 0;
}
int PCRace::getPracBonus( ) const
{
    return 0;
}

DLString PCRace::getWhoNameFor( Character *, Character * ) const
{
    return DLString::emptyString;
}

DLString PCRace::getScoreNameFor( Character *, Character * ) const
{
    return DLString::emptyString;
}
