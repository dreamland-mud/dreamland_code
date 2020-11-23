/* $Id$
 *
 * ruffina, 2004
 */
#ifndef LIQUID_H
#define LIQUID_H

#include "oneallocate.h"
#include "globalregistryelement.h"
#include "globalregistry.h"
#include "globalreference.h"
#include "xmlglobalreference.h"
#include "bitstring.h"

#define LIQ( name ) static LiquidReference liq_##name( #name )

class Flags;
class GlobalArray;

/*
 * Liquid
 */
class Liquid : public GlobalRegistryElement {
public:
    typedef ::Pointer<Liquid> Pointer;
    
    Liquid( );
    Liquid( const DLString & );
    virtual ~Liquid( );

    virtual const DLString &getName( ) const;
    virtual const DLString &getRussianName( ) const;
    virtual bool isValid( ) const;
    
    virtual const DLString &getShortDescr( ) const;
    virtual const DLString &getColor( ) const;
    virtual int getSipSize( ) const;
    virtual GlobalArray & getDesires( );
    virtual const Flags & getFlags( ) const;

protected:
    DLString name;
};
    

/*
 * LiquidManager
 */
class LiquidManager : public GlobalRegistry<Liquid>, public OneAllocate
{
public:
    
    LiquidManager( );
    virtual ~LiquidManager( );
    
    Liquid * random( bitstring_t );
    
    inline static LiquidManager *getThis( );

private:
    virtual GlobalRegistryElement::Pointer getDumbElement( const DLString & ) const;
};

extern LiquidManager * liquidManager;

inline LiquidManager * LiquidManager::getThis( )
{   
    return liquidManager;
}


GLOBALREF_DECL(Liquid)
XMLGLOBALREF_DECL(Liquid)

#endif
