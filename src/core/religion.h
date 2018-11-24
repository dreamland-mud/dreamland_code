/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __RELIGION_H__
#define __RELIGION_H__

#include "oneallocate.h"
#include "globalregistryelement.h"
#include "globalregistry.h"
#include "globalreference.h"
#include "bitstring.h"
#include "xmlglobalreference.h"

#define RELIG( name ) static ReligionReference god_##name( #name )

class Character;
class PCharacter;
class Object;
class Flags;
struct time_info_data;

/*
 * Religion
 */
class Religion : public GlobalRegistryElement {
public:
    typedef ::Pointer<Religion> Pointer;
    
    Religion( );
    Religion( const DLString & );
    virtual ~Religion( );

    virtual const DLString &getName( ) const;
    virtual const DLString &getRussianName( ) const;
    virtual bool isValid( ) const;
    virtual int getSex() const;

    virtual const DLString & getShortDescr( ) const;
    virtual const DLString & getDescription( ) const;
    virtual bool isAllowed( Character * ) const;
    virtual const DLString& getNameFor( Character * ) const;
    virtual void tattooFight( Object *, Character * ) const;
    virtual bool hasBonus(Character *, const bitstring_t &, const struct time_info_data &) const;

protected:
    DLString name;
};


/*
 * ReligionManager
 */
class ReligionManager : public GlobalRegistry<Religion>, 
                          public OneAllocate
{
public:
    
    ReligionManager( );
    virtual ~ReligionManager( );
    
    inline static ReligionManager *getThis( );

private:
    virtual GlobalRegistryElement::Pointer getDumbElement( const DLString & ) const;
};

extern ReligionManager * religionManager;

inline ReligionManager * ReligionManager::getThis( )
{   
    return religionManager;
}


GLOBALREF_DECL(Religion)
XMLGLOBALREF_DECL(Religion)

#endif
