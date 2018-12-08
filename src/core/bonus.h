/* $Id$
 *
 * ruffina, 2018
 */
#ifndef BONUS_H
#define BONUS_H

#include <sstream>
#include "oneallocate.h"
#include "globalregistryelement.h"
#include "globalregistry.h"
#include "globalreference.h"
#include "xmlglobalreference.h"
#include "bitstring.h"
#include "globalprofilearray.h"
#include "xmllong.h"

#define BONUS( name ) static BonusReference bonus_##name( #name )

class Flags;
class GlobalArray;
struct time_info_data;
class PCharacter;

/*
 * Bonus
 */
class Bonus : public GlobalRegistryElement {
public:
    typedef ::Pointer<Bonus> Pointer;
    
    Bonus( );
    Bonus( const DLString & );
    virtual ~Bonus( );

    virtual const DLString &getName( ) const;
    virtual const DLString &getRussianName( ) const;
    virtual const DLString &getShortDescr( ) const;
    virtual char getColor() const;
    virtual bool isValid( ) const;
    virtual bool isReligious() const;
    virtual bool isActive(PCharacter *, const struct time_info_data &) const;
    virtual void reportTime(PCharacter *, ostringstream &) const;
    virtual void reportAction(PCharacter *, ostringstream &) const;

protected:
    DLString name;
};
    

/*
 * BonusManager
 */
class BonusManager : public GlobalRegistry<Bonus>, public OneAllocate
{
public:
    
    BonusManager( );
    virtual ~BonusManager( );
    
    inline static BonusManager *getThis( );

private:
    virtual GlobalRegistryElement::Pointer getDumbElement( const DLString & ) const;
};

extern BonusManager * bonusManager;

inline BonusManager * BonusManager::getThis( )
{   
    return bonusManager;
}


GLOBALREF_DECL(Bonus)
XMLGLOBALREF_DECL(Bonus)

/*
 * Player profile entries
 */
class PCBonusData : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<PCBonusData> Pointer;
    
    PCBonusData( );

    bool isValid() const;
    static PCBonusData empty;

    XML_VARIABLE XMLLongNoEmpty start;
    XML_VARIABLE XMLLongNoEmpty end;
};

class PCBonuses : public GlobalProfileArray<PCBonusData> {
XML_OBJECT
public:
    typedef ::Pointer<PCBonuses> Pointer;
    typedef GlobalProfileArray<PCBonusData> Base;

    PCBonuses();
    
};


#endif
