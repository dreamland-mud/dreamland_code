/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DESIRE_H 
#define DESIRE_H 

#include "oneallocate.h"
#include "globalregistryelement.h"
#include "globalregistry.h"
#include "globalreference.h"
#include "xmlglobalreference.h"

#define DESIRE( name ) static DesireReference desire_##name( #name )

class PCharacter;
class Liquid;

/*
 * Desire
 */
class Desire : public GlobalRegistryElement {
public:
    typedef ::Pointer<Desire> Pointer;
    
    Desire( );
    Desire( const DLString & );
    virtual ~Desire( );

    virtual const DLString &getName( ) const;
    virtual bool isValid( ) const;

    virtual void reset( PCharacter * );
    virtual void update( PCharacter * );
    virtual void report( PCharacter *, ostringstream &buf );
    virtual void vomit( PCharacter * );
    virtual void eat( PCharacter *, int );
    virtual void drink( PCharacter *, int, Liquid * );
    virtual void gain( PCharacter *, int );
    
    virtual bool applicable( PCharacter * );
    virtual bool isActive( PCharacter * );

    virtual bool canEat( PCharacter * );
    virtual bool canDrink( PCharacter * );

protected:
    DLString name;
};
    

/*
 * DesireManager
 */
class DesireManager : public GlobalRegistry<Desire>, public OneAllocate {
public:
    DesireManager( );
    virtual ~DesireManager( );
    
    inline static DesireManager *getThis( );

private:
    virtual GlobalRegistryElement::Pointer getDumbElement( const DLString & ) const;
};

extern DesireManager * desireManager;

inline DesireManager * DesireManager::getThis( )
{   
    return desireManager;
}

GLOBALREF_DECL(Desire)
XMLGLOBALREF_DECL(Desire)

#endif
