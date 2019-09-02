/* $Id$
 *
 * ruffina, 2004
 */
#include "desire.h"

/*-------------------------------------------------------------------
 * Desire
 *------------------------------------------------------------------*/
Desire::Desire( )
{
}

Desire::Desire( const DLString &n ) : name( n )
{
}

Desire::~Desire( )
{
}

const DLString &Desire::getName( ) const
{
    return name;
}

bool Desire::isValid( ) const
{
    return false;
}

void Desire::reset( PCharacter * )
{
}

void Desire::update( PCharacter * )
{
}

void Desire::report( PCharacter *, ostringstream &buf )
{
}

void Desire::vomit( PCharacter * )
{
}
void Desire::eat( PCharacter *, int )
{
}
void Desire::drink( PCharacter *, int, Liquid * )
{
}
void Desire::gain( PCharacter *, int )
{
}
bool Desire::applicable( PCharacter * )
{
    return false;
}
bool Desire::isActive( PCharacter * )
{
    return false;
}
bool Desire::canEat( PCharacter * )
{
    return true;
}
bool Desire::canDrink( PCharacter * )
{
    return true;
}

/*-------------------------------------------------------------------
 * DesireManager
 *------------------------------------------------------------------*/
DesireManager* desireManager = 0;

DesireManager::DesireManager( ) 
{
    checkDuplicate( desireManager );
    desireManager = this;
    setRegistryName("desire");
    saveRegistryName();
}

DesireManager::~DesireManager( )
{
    eraseRegistryName();
    desireManager = 0;
}

GlobalRegistryElement::Pointer DesireManager::getDumbElement( const DLString &name ) const
{
    return Desire::Pointer( NEW, name );
}

GLOBALREF_IMPL(Desire, ' ')
XMLGLOBALREF_IMPL(Desire)

