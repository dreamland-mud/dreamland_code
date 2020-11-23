/* $Id$
 *
 * ruffina, 2004
 */
#include "flags.h"
#include "wearlocation.h"

/*-------------------------------------------------------------------
 * Wearlocation
 *------------------------------------------------------------------*/
Wearlocation::Wearlocation( )
{
}

Wearlocation::Wearlocation( const DLString &n ) : name( n )
{
}

Wearlocation::~Wearlocation( )
{
}

const DLString &Wearlocation::getName( ) const
{
    return name;
}

bool Wearlocation::isValid( ) const
{
    return false;
}

bool Wearlocation::givesAffects() const
{
    return false;
}

const DLString &Wearlocation::getRibName( ) const
{
    return DLString::emptyString;
}

const DLString &Wearlocation::getPurpose( ) const
{
    return DLString::emptyString;
}

int Wearlocation::getOrderWear( ) const
{
    return 0;
}
int Wearlocation::getOrderDisplay( ) const
{
    return 0;
}
int Wearlocation::getDestroyChance( ) const
{
    return 0;
}
Object * Wearlocation::find( Character *ch )
{
    return NULL;
}
bool Wearlocation::matches( Character *ch )
{
    return false;
}
bool Wearlocation::matches( Object *obj )
{
    return false;
}
void Wearlocation::reset( Object * )
{
}
bool Wearlocation::equip( Object *obj )
{
    return false;
}
void Wearlocation::unequip( Object *obj )
{
}
bool Wearlocation::remove( Object *, int flags )
{
    return false;
}
bool Wearlocation::remove( Character *ch, int flags )
{
    return false;
}
int Wearlocation::wear( Object *obj, int flags )
{
    return -1;
}
void Wearlocation::display( Character *, Wearlocation::DisplayList & )
{
}
bool Wearlocation::wearAtomic( Character *ch, Object *obj, int flags )
{
    return false;
}
int Wearlocation::canWear( Character *ch, Object *obj, int flags )
{
    return -1;
}
bool Wearlocation::canRemove( Character *ch, Object *obj, int flags )
{
    return false;
}
bool Wearlocation::canRemove( Character *ch, int flags )
{
    return false;
}
bool Wearlocation::canWear( Character *ch, int flags )
{
    return false;
}

/*-------------------------------------------------------------------
 * WearlocationManager
 *------------------------------------------------------------------*/
WearlocationManager* wearlocationManager = 0;

WearlocationManager::WearlocationManager( ) 
{
    checkDuplicate( wearlocationManager );
    wearlocationManager = this;
    setRegistryName("wearlocation");
    saveRegistryName();
}

WearlocationManager::~WearlocationManager( )
{
    eraseRegistryName();
    wearlocationManager = 0;
}

GlobalRegistryElement::Pointer WearlocationManager::getDumbElement( const DLString &name ) const
{
    return Wearlocation::Pointer( NEW, name );
}

GLOBALREF_IMPL(Wearlocation, '_')
XMLGLOBALREF_IMPL(Wearlocation)

