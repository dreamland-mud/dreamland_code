/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __PROFESSION_H__
#define __PROFESSION_H__

#include "oneallocate.h"
#include "globalregistryelement.h"
#include "globalregistry.h"
#include "globalreference.h"
#include "globalbitvector.h"
#include "xmlglobalreference.h"
#include "bitstring.h"
#include "grammar_entities.h"

#define PROF( name ) static ProfessionReference prof_##name( #name )

class PCMemoryInterface;
class CharacterMemoryInterface;
class Room;
class Flags;
class EnumerationArray;
class Character;

/*
 * Profession
 */
class Profession : public GlobalRegistryElement {
public:
    typedef ::Pointer<Profession> Pointer;
    
    Profession( );
    Profession( const DLString & );
    virtual ~Profession( );

    virtual const DLString &getName( ) const;
    virtual bool isValid( ) const;

    virtual const DLString &getRusName( ) const;
    virtual const DLString &getMltName( ) const;
    virtual DLString getNameFor( Character *, const Grammar::Case & = Grammar::Case::NONE ) const;
    virtual DLString getWhoNameFor( Character * ) const;
    virtual int  getWeapon( ) const;
    virtual int  getSkillAdept( ) const;
    virtual int  getThac00( Character * = NULL ) const;
    virtual int  getThac32( Character * = NULL ) const;
    virtual int  getHpRate( ) const;
    virtual int  getManaRate( ) const;
    virtual int  getPoints( ) const;
    virtual int  getWearModifier( int ) const;
    virtual int getStat( bitnumber_t, Character * = NULL ) const;
    virtual const DLString & getTitle( const PCMemoryInterface * ) const;
    virtual GlobalBitvector toVector( CharacterMemoryInterface * = NULL ) const;
    virtual Flags getFlags( CharacterMemoryInterface * = NULL ) const;
    
    virtual bool isPlayed( ) const;
    virtual const Flags & getSex( ) const;
    virtual const Flags & getEthos( ) const;
    virtual const Flags & getAlign( ) const;
    virtual int getMinAlign( ) const;
    virtual int getMaxAlign( ) const;

protected:
    DLString name;
};


/*
 * ProfessionManager
 */
class ProfessionManager : public GlobalRegistry<Profession>, 
                          public OneAllocate
{
public:
    
    ProfessionManager( );
    virtual ~ProfessionManager( );
    
    inline static ProfessionManager *getThis( );

private:
    virtual GlobalRegistryElement::Pointer getDumbElement( const DLString & ) const;
};

extern ProfessionManager * professionManager;

inline ProfessionManager * ProfessionManager::getThis( )
{   
    return professionManager;
}

GLOBALREF_DECL(Profession)
XMLGLOBALREF_DECL(Profession)

#endif
