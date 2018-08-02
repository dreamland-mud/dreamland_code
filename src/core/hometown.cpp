/* $Id$
 *
 * ruffina, 2004
 */
#include "class.h"
#include "hometown.h"
#include "pcharacter.h"

/*-------------------------------------------------------------------
 * Hometown 
 *------------------------------------------------------------------*/
Hometown::Hometown( )
{
}
Hometown::~Hometown( )
{
}
Hometown::Hometown( const DLString &n )
              : name( n )
{
}

const DLString & Hometown::getName( ) const
{
    return name;
}

bool Hometown::isValid( ) const
{
    return false;
}

int Hometown::getAltar( ) const
{
    return 0;
}

int Hometown::getRecall( ) const
{
    return 0;
}

int Hometown::getPit( ) const
{
    return 0;
}

int Hometown::getLanding( ) const
{
    return 0;
}
    
bool Hometown::isAllowed( PCharacter *pch ) const
{
    return false;
}

/*-------------------------------------------------------------------
 * HometownManager
 *------------------------------------------------------------------*/
HometownManager* hometownManager = 0;

HometownManager::HometownManager( ) 
{
    checkDuplicate( hometownManager );
    hometownManager = this;
}

HometownManager::~HometownManager( )
{
    hometownManager = 0;
}

GlobalRegistryElement::Pointer HometownManager::getDumbElement( const DLString &name ) const
{
    return Hometown::Pointer( NEW, name );
}

GLOBALREF_IMPL(Hometown, '_')
XMLGLOBALREF_IMPL(Hometown)
