/* $Id$
 *
 * ruffina, 2018
 */
#include "flags.h"
#include "globalarray.h"
#include "bonus.h"
/*-------------------------------------------------------------------
 * Bonus
 *------------------------------------------------------------------*/
Bonus::Bonus( )
{
}

Bonus::Bonus( const DLString &n ) : name( n )
{
}

Bonus::~Bonus( )
{
}

const DLString &Bonus::getName( ) const
{
    return name;
}

const DLString &Bonus::getRussianName( ) const
{
    return name;
}

bool Bonus::isValid( ) const
{
    return false;
}

bool Bonus::isReligious( ) const
{
    return false;
}

char Bonus::getColor() const
{
    return 'x';
}

const DLString &Bonus::getShortDescr( ) const
{
    return DLString::emptyString;
}

bool Bonus::isActive(PCharacter *, const struct time_info_data &) const
{
    return false;
}

void Bonus::reportTime(PCharacter *, ostringstream &) const
{
}

void Bonus::reportAction(PCharacter *, ostringstream &) const
{
}

/*-------------------------------------------------------------------
 * BonusManager
 *------------------------------------------------------------------*/
BonusManager* bonusManager = 0;

BonusManager::BonusManager( ) 
{
    checkDuplicate( bonusManager );
    bonusManager = this;
}

BonusManager::~BonusManager( )
{
    bonusManager = 0;
}

GlobalRegistryElement::Pointer BonusManager::getDumbElement( const DLString &name ) const
{
    return Bonus::Pointer( NEW, name );
}

GLOBALREF_IMPL(Bonus, ' ')
XMLGLOBALREF_IMPL(Bonus)


/*
 * Player profile entries.
 */
PCBonusData PCBonusData::empty;

PCBonusData::PCBonusData( )
{
}


bool PCBonusData::isValid() const
{
    return (start.getValue() != 0) || (end.getValue() != 0);
}

PCBonuses::PCBonuses()
                : Base(bonusManager)
{
}

