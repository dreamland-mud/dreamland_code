/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DEFAULTRACELANGUAGE_H
#define DEFAULTRACELANGUAGE_H

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmlboolean.h"
#include "xmlglobalbitvector.h"
#include "xmltableelement.h"

#include "racelanguage.h"

class DefaultRaceLanguage : public RaceLanguage, public XMLTableElement, public XMLVariableContainer 
{
XML_OBJECT
public:
    typedef ::Pointer<DefaultRaceLanguage> Pointer;
    
    DefaultRaceLanguage( );
    
    inline virtual const DLString & getName( ) const;
    inline virtual void setName( const DLString & );
    inline virtual bool isValid( ) const;
    virtual void loaded( );
    virtual void unloaded( );
    
    virtual const DLString &getShortDescr( ) const;
    virtual bool available( Character * ) const;
    virtual DLString translate( const DLString &, Character *, Character * ) const;

protected:
    XML_VARIABLE XMLStringNoEmpty    shortDescr;
    XML_VARIABLE XMLGlobalBitvector  races;
    XML_VARIABLE XMLBooleanNoFalse   common;
};

inline const DLString & DefaultRaceLanguage::getName( ) const
{
    return RaceLanguage::getName( );
}

inline void DefaultRaceLanguage::setName( const DLString &name ) 
{
    this->name = name;
}

inline bool DefaultRaceLanguage::isValid( ) const
{
    return true;
}

#endif
