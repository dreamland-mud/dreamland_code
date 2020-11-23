/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __WEARLOCATION_H__
#define __WEARLOCATION_H__

#include "oneallocate.h"
#include "globalregistryelement.h"
#include "globalregistry.h"
#include "globalreference.h"
#include "xmlglobalreference.h"

#define WEARLOC( name ) static WearlocationReference wear_##name( #name )

class Object;
class Character;
class Flags;

/*
 * Wearlocation
 */
class Wearlocation : public GlobalRegistryElement {
friend class WearlocationManager;
public:
    typedef ::Pointer<Wearlocation> Pointer;
    typedef list<pair<DLString, Object *> > DisplayList;
    
    Wearlocation( );
    Wearlocation( const DLString & );
    virtual ~Wearlocation( );

    virtual const DLString &getName( ) const;
    virtual bool isValid( ) const;
    virtual bool givesAffects() const;
    
    virtual const DLString &getRibName( ) const;
    virtual const DLString &getPurpose( ) const;
    virtual int getDestroyChance( ) const;
    virtual int getOrderWear( ) const;
    virtual int getOrderDisplay( ) const;
    virtual Object * find( Character *ch );
    virtual bool matches( Character *ch );
    virtual bool matches( Object *obj );
    
    virtual void reset( Object * );
    virtual bool equip( Object *obj );
    virtual void unequip( Object *obj );
    virtual bool remove( Object *, int flags );
    virtual bool remove( Character *ch, int flags );
    virtual  int wear( Object *obj, int flags );
    virtual bool wearAtomic( Character *ch, Object *obj, int flags );

    virtual void display( Character *, DisplayList & );
    virtual  int canWear( Character *ch, Object *obj, int flags );
    virtual bool canWear( Character *ch, int flags );
    virtual bool canRemove( Character *ch, Object *obj, int flags );
    virtual bool canRemove( Character *ch, int flags );

protected:
    DLString name;
};
    

/*
 * WearlocationManager
 */
class WearlocationManager : public GlobalRegistry<Wearlocation>, 
                            public OneAllocate
{
public:
    
    WearlocationManager( );
    virtual ~WearlocationManager( );
    
    int wear( Object *, int );
    void display( Character *, Wearlocation::DisplayList & );

    inline static WearlocationManager *getThis( );

private:
    virtual GlobalRegistryElement::Pointer getDumbElement( const DLString & ) const;
};

extern WearlocationManager * wearlocationManager;

inline WearlocationManager * WearlocationManager::getThis( )
{   
    return wearlocationManager;
}

GLOBALREF_DECL(Wearlocation)
XMLGLOBALREF_DECL(Wearlocation)

#endif
