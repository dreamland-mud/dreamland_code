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
#include "xmlmultistring.h"
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
    
    virtual const DLString &getShortDescr( lang_t lang = LANG_DEFAULT ) const;
    virtual const DLString &getColor( lang_t lang = LANG_DEFAULT ) const;
    virtual int getSipSize( ) const;
    virtual GlobalArray & getDesires( );
    virtual const Flags & getFlags( ) const;

protected:
    XML_VARIABLE XMLMultiString      shortDescr;
    XML_VARIABLE XMLMultiString      color;
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
