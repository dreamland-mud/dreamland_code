/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __REMORTBONUS_H__
#define __REMORTBONUS_H__

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmlinteger.h"
#include "xmlenumeration.h"
#include "xmlpointer.h"
#include "xmllist.h"

#include "service.h"
#include "price.h"

class PCharacter;

class RemortBonus : public Service, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<RemortBonus> Pointer;
    
    RemortBonus( );

    virtual bool matches( const DLString & ) const;
    virtual bool purchase( Character *, NPCharacter *, const DLString &, int = 1 );
    virtual bool sell( Character *, NPCharacter * );
    virtual bool sellable( Character * );
    virtual bool available( Character *, NPCharacter * ) const;
    virtual bool visible( Character * ) const;
    virtual void toStream( Character *, ostringstream & ) const;

protected:
    virtual  int bonusMaximum( PCharacter * ) const = 0;
    virtual void bonusBuy( PCharacter * ) const = 0;
    virtual void bonusSell( PCharacter * ) const = 0; 
    virtual bool bonusBought( PCharacter * ) const = 0; 

    virtual bool matchesAlias( const DLString & ) const;
    virtual DLString getShortDescr( ) const;
    const DLString & getGender( ) const;
    
    XML_VARIABLE XMLEnumeration gender;
    XML_VARIABLE XMLPointer<Price> price;
    XML_VARIABLE XMLStringNoEmpty shortDescr;
    XML_VARIABLE XMLListBase<XMLString> aliases;
};

class IntegerRemortBonus : public virtual RemortBonus {
XML_OBJECT
public:
    virtual int getQuantity( ) const;
protected:
    virtual int & bonusField( PCharacter * ) const = 0;
    virtual void bonusBuy( PCharacter * ) const;
    virtual void bonusSell( PCharacter * ) const;
    virtual bool bonusBought( PCharacter * ) const; 
    
    XML_VARIABLE XMLInteger amount;
};

class BooleanRemortBonus : public virtual RemortBonus {
public:
    virtual int getQuantity( ) const;
protected:
    virtual bool & bonusField( PCharacter * ) const = 0;
    virtual void bonusBuy( PCharacter * ) const;
    virtual void bonusSell( PCharacter * ) const;
    virtual bool bonusBought( PCharacter * ) const; 
    virtual int bonusMaximum( PCharacter * ) const;
};

class AppliedRemortBonus : public IntegerRemortBonus {
protected:
    virtual void bonusApply( PCharacter * ) const = 0;
    virtual void bonusRemove( PCharacter * ) const = 0;
    virtual void bonusBuy( PCharacter * ) const;
    virtual void bonusSell( PCharacter * ) const;
};

#endif
