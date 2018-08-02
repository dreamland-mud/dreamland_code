/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __HOMETOWN_H__
#define __HOMETOWN_H__

#include "oneallocate.h"
#include "globalregistryelement.h"
#include "globalregistry.h"
#include "globalreference.h"
#include "xmlglobalreference.h"

class PCharacter;

#define HOMETOWN( name ) static HometownReference home_##name( #name )

/*
 * Hometown
 */
class Hometown : public GlobalRegistryElement {
public:
    typedef ::Pointer<Hometown> Pointer;
    
    Hometown( );
    Hometown( const DLString & );
    virtual ~Hometown( );
    
    virtual const DLString &getName( ) const;
    virtual bool isValid( ) const;
    
    virtual int getLanding( ) const;
    virtual int getPit( ) const;
    virtual int getRecall( ) const;
    virtual int getAltar( ) const;

    virtual bool isAllowed( PCharacter * ) const;

protected:
    DLString name;
};


/*
 * HometownManager
 */
class HometownManager : public GlobalRegistry<Hometown>, public OneAllocate
{
public:
    
    HometownManager( );
    virtual ~HometownManager( );
    
    inline static HometownManager *getThis( );
    
private:
    virtual GlobalRegistryElement::Pointer getDumbElement( const DLString & ) const;
};

extern HometownManager * hometownManager;

inline HometownManager * HometownManager::getThis( )
{
    return hometownManager;
}

GLOBALREF_DECL(Hometown)
XMLGLOBALREF_DECL(Hometown)

#endif
