/* $Id: race.h,v 1.1.2.9.6.9 2010/01/01 15:48:16 rufina Exp $
 *
 * ruffina, 2007
 */
#ifndef RACE_H
#define RACE_H

#include "oneallocate.h"
#include "bitstring.h"
#include "globalregistryelement.h"
#include "globalreference.h"
#include "xmlglobalreference.h"
#include "globalregistry.h"
#include "grammar_entities.h"

#define RACE(var) static RaceReference race_##var( #var )

class PCRace;
class RaceManager;
class Enumeration;
class GlobalBitvector;
class Flags;
class Character;

/*
 * Race
 */
class Race : public GlobalRegistryElement {
public:	
    typedef ::Pointer<Race> Pointer;
    
    Race( );
    Race( const DLString & );
    virtual ~Race( );

    virtual const DLString &getName( ) const;
    virtual bool isValid( ) const;

    virtual PCRace * getPC( );
    virtual bool  isPC( ) const;
    virtual const Flags & getDet( ) const;
    virtual const Flags & getAct( ) const;
    virtual const Flags & getAff( ) const;
    virtual const Flags & getOff( ) const;
    virtual const Flags & getImm( ) const;
    virtual const Flags & getRes( ) const;
    virtual const Flags & getVuln( ) const;
    virtual const Flags & getForm( ) const;
    virtual const Flags & getParts( ) const;
    virtual const GlobalBitvector & getWearloc( ) const;
    virtual const Enumeration & getSize( ) const;
    virtual Flags getAttitude( const Race & ) const;

    virtual const DLString & getMaleName( ) const;
    virtual const DLString & getFemaleName( ) const;
    virtual const DLString & getNeuterName( ) const;
    virtual const DLString & getMltName( ) const;
    virtual DLString getNameFor( Character *, Character *, const Grammar::Case & = Grammar::Case::NONE ) const;

protected:
    DLString name;
};

/*
 * RaceManager
 */
extern RaceManager *raceManager;

class RaceManager : public GlobalRegistry<Race>, public OneAllocate {
public:
    RaceManager( );
    virtual ~RaceManager( );
    
    inline static RaceManager * getThis( );
    const PCRace * findUnstrictPC( const DLString &name );

private:
    virtual GlobalRegistryElement::Pointer getDumbElement( const DLString & ) const;
};

inline RaceManager * RaceManager::getThis( )
{
    return raceManager;
}


GLOBALREF_DECL(Race)
XMLGLOBALREF_DECL(Race)

#endif
