/* $Id$
 *
 * ruffina, 2004
 */
#include "flags.h"
#include "enumeration.h"
#include "profession.h"

/*-------------------------------------------------------------------
 * Profession
 *------------------------------------------------------------------*/
Profession::Profession( )
{
}

Profession::Profession( const DLString &n ) : name( n )
{
}

Profession::~Profession( )
{
}

const DLString &Profession::getName( ) const
{
    return name;
}

bool Profession::isValid( ) const
{
    return false;
}

DLString Profession::getNameFor( Character *, const Grammar::Case & ) const
{
    return getName( );
}

DLString Profession::getWhoNameFor( Character * ) const
{
    return DLString::emptyString;
}

const Flags & Profession::getAlign( ) const
{
    return Flags::emptyFlags;
}
int Profession::getMinAlign( ) const
{
    return 0;
}
int Profession::getMaxAlign( ) const
{
    return 0;
}
const Flags & Profession::getEthos( ) const
{
    return Flags::emptyFlags;
}
const Flags & Profession::getSex( ) const
{
    return Flags::emptyFlags;
}
const DLString & Profession::getRusName( ) const
{
    return DLString::emptyString;
}
const DLString & Profession::getMltName( ) const
{
    return DLString::emptyString;
}
int Profession::getWeapon( ) const
{
    return 0;
}
int Profession::getSkillAdept( ) const
{
    return 0;
}
int Profession::getParentAdept( ) const
{
    return 0;
}
int Profession::getThac00( Character * ) const
{
    return 0;
}
int Profession::getThac32( Character * ) const
{
    return 0;
}
int Profession::getHpRate( ) const
{
    return 70;
}
int Profession::getManaRate( ) const
{
    return 70;
}
Flags Profession::getFlags( Character * ) const
{
    return Flags::emptyFlags;
}
int Profession::getPoints( ) const
{
    return 0;
}
int Profession::getWearModifier( int ) const
{
    return 0;
}
int Profession::getStat( bitnumber_t, Character * ) const
{
    return 0;
}
const DLString & Profession::getTitle( const PCMemoryInterface * ) const
{
    return DLString::emptyString;
}

bool Profession::isPlayed( ) const
{
    return false;
}

GlobalBitvector Profession::toVector( Character * ) const
{
    return GlobalBitvector( professionManager );
}

/*-------------------------------------------------------------------
 * ProfessionManager
 *------------------------------------------------------------------*/
ProfessionManager* professionManager = 0;

ProfessionManager::ProfessionManager( ) 
{
    checkDuplicate( professionManager );
    professionManager = this;
    setRegistryName("profession");    
    saveRegistryName();
}

ProfessionManager::~ProfessionManager( )
{
    eraseRegistryName();
    professionManager = 0;
}

GlobalRegistryElement::Pointer ProfessionManager::getDumbElement( const DLString &name ) const
{
    return Profession::Pointer( NEW, name );
}

GLOBALREF_IMPL(Profession, '-')
XMLGLOBALREF_IMPL(Profession)
