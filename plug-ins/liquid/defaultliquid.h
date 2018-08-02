/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DEFAULTLIQUID_H
#define DEFAULTLIQUID_H

#include "xmlvector.h"
#include "xmlinteger.h"
#include "xmlboolean.h"
#include "xmlstring.h"
#include "xmlflags.h"
#include "xmlglobalarray.h"
#include "xmltableelement.h"

#include "liquid.h"

class DefaultLiquid : public Liquid, public XMLTableElement, public XMLVariableContainer 
{
XML_OBJECT
public:
    typedef ::Pointer<DefaultLiquid> Pointer;
    
    DefaultLiquid( );
    
    inline virtual const DLString & getName( ) const;
    inline virtual void setName( const DLString & );
    inline virtual bool isValid( ) const;
    virtual void loaded( );
    virtual void unloaded( );
    
    virtual const DLString &getShortDescr( ) const;
    virtual const DLString &getColor( ) const;
    virtual int getSipSize( ) const;
    virtual GlobalArray & getDesires( );
    virtual const Flags & getFlags( ) const;

protected:
    XML_VARIABLE XMLStringNoEmpty    shortDescr;
    XML_VARIABLE XMLStringNoEmpty    color;
    XML_VARIABLE XMLIntegerNoEmpty   sipSize;
    XML_VARIABLE XMLGlobalArray      desires;
    XML_VARIABLE XMLFlagsNoEmpty     flags;
};

inline const DLString & DefaultLiquid::getName( ) const
{
    return Liquid::getName( );
}

inline void DefaultLiquid::setName( const DLString &name ) 
{
    this->name = name;
}

inline bool DefaultLiquid::isValid( ) const
{
    return true;
}

#endif
