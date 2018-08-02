/* $Id$
 *
 * ruffina, 2004
 */
#ifndef RACELANGUAGE_H 
#define RACELANGUAGE_H 

#include "oneallocate.h"
#include "globalregistryelement.h"
#include "globalregistry.h"
#include "globalreference.h"
#include "xmlglobalreference.h"
#include "bitstring.h"

#define LANG( name ) static RaceLanguageReference lang_##name( #name )

class Character;

/*
 * RaceLanguage
 */
class RaceLanguage : public GlobalRegistryElement {
public:
    typedef ::Pointer<RaceLanguage> Pointer;
    
    RaceLanguage( );
    RaceLanguage( const DLString & );
    virtual ~RaceLanguage( );

    virtual const DLString &getName( ) const;
    virtual bool isValid( ) const;
    virtual const DLString &getShortDescr( ) const;
    virtual bool available( Character * ) const;
    virtual DLString translate( const DLString &, Character *, Character * ) const;

protected:
    DLString name;
};
    

/*
 * RaceLanguageManager
 */
class RaceLanguageManager : public GlobalRegistry<RaceLanguage>, public OneAllocate {
public:
    RaceLanguageManager( );
    virtual ~RaceLanguageManager( );
    
    inline static RaceLanguageManager *getThis( );

private:
    virtual GlobalRegistryElement::Pointer getDumbElement( const DLString & ) const;
};

extern RaceLanguageManager * raceLanguageManager;

inline RaceLanguageManager * RaceLanguageManager::getThis( )
{   
    return raceLanguageManager;
}


GLOBALREF_DECL(RaceLanguage)
XMLGLOBALREF_DECL(RaceLanguage)

#endif
