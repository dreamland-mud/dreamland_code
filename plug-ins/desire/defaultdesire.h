/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DEFAULTDESIRE_H
#define DEFAULTDESIRE_H

#include "xmlvector.h"
#include "xmlinteger.h"
#include "xmlboolean.h"
#include "xmlstring.h"
#include "xmlflags.h"
#include "xmlenumeration.h"
#include "xmltableelement.h"

#include "desire.h"

class DefaultDesire : public Desire, public XMLTableElement, public XMLVariableContainer 
{
XML_OBJECT
public:
    typedef ::Pointer<DefaultDesire> Pointer;
    
    DefaultDesire( );
    virtual ~DefaultDesire( );
    
    inline virtual const DLString & getName( ) const;
    inline virtual void setName( const DLString & );
    inline virtual bool isValid( ) const;
    virtual void loaded( );
    virtual void unloaded( );

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
    virtual void damage( PCharacter * );
    virtual  int getUpdateAmount( PCharacter * );
    virtual bool isOverflow( PCharacter * );

    static bool isVampire( PCharacter * );

    XML_VARIABLE XMLInteger drinkCoef;     
    XML_VARIABLE XMLInteger vomitAmount;     
    XML_VARIABLE XMLInteger resetAmount;     
    XML_VARIABLE XMLInteger damageLimit;     
    XML_VARIABLE XMLInteger activeLimit;     
    XML_VARIABLE XMLInteger minValue, maxValue;     

    XML_VARIABLE XMLString  msgStop, msgStart;
    XML_VARIABLE XMLString  msgActive;
    XML_VARIABLE XMLString  msgDamageSelf, msgDamageRoom;
    XML_VARIABLE XMLString  msgReport;
};

inline const DLString & DefaultDesire::getName( ) const
{
    return Desire::getName( );
}

inline void DefaultDesire::setName( const DLString &name ) 
{
    this->name = name;
}

inline bool DefaultDesire::isValid( ) const
{
    return true;
}

#endif
