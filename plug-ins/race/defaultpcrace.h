/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DEFAULTPCRACE_H
#define DEFAULTPCRACE_H

#include "xmlvector.h"
#include "xmlmap.h"
#include "xmlenumeration.h"
#include "xmlglobalarray.h"

#include "pcrace.h"
#include "defaultrace.h"

class DefaultPCRace : public PCRace, public DefaultRace {
XML_OBJECT
public:        
    typedef ::Pointer<DefaultPCRace> Pointer;
    
    DefaultPCRace( );

    virtual const EnumerationArray & getStats( ) const;
    virtual GlobalArray & getClasses( );
    virtual const Flags & getAlign( ) const;
    virtual int getMinAlign( ) const;
    virtual int getMaxAlign( ) const;
    virtual int getPoints( ) const;
    virtual int getHpBonus( ) const;
    virtual int getManaBonus( ) const;
    virtual int getPracBonus( ) const;
    
    virtual DLString getWhoNameFor( Character *, Character * ) const;
    virtual DLString getScoreNameFor( Character *looker, Character *owner = NULL ) const;

protected:
    XML_VARIABLE XMLGlobalArray    classes;
    XML_VARIABLE XMLEnumerationArray stats; 
    XML_VARIABLE XMLIntegerNoEmpty points;
    XML_VARIABLE XMLIntegerNoEmpty hpBonus; 
    XML_VARIABLE XMLIntegerNoEmpty manaBonus;
    XML_VARIABLE XMLIntegerNoEmpty pracBonus;
    XML_VARIABLE XMLFlagsNoEmpty   align;
    XML_VARIABLE XMLInteger        minAlign, maxAlign;

    XML_VARIABLE XMLStringNoEmpty  nameWho; 
    XML_VARIABLE XMLStringNoEmpty  nameWhoRus; 
    XML_VARIABLE XMLStringNoEmpty  nameWhoFemale; 
    XML_VARIABLE XMLStringNoEmpty  nameScore; 
};

#endif
