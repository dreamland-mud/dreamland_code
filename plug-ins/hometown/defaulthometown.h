/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DEFAULTHOMETOWN_H
#define DEFAULTHOMETOWN_H

#include "xmllimits.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlvariablecontainer.h"
#include "xmltableelement.h"

#include "hometown.h"

class DefaultHometown : public Hometown, public XMLTableElement, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<DefaultHometown> Pointer;

    inline virtual const DLString & getName( ) const;
    inline virtual void setName( const DLString & );
    inline virtual bool isValid( ) const;
    virtual void loaded( );
    virtual void unloaded( );
    
    virtual int getLanding( ) const;
    virtual int getPit( ) const;
    virtual int getRecall( ) const;
    virtual int getAltar( ) const;

    virtual bool isAllowed( PCharacter * ) const;

protected:
    XML_VARIABLE XMLInteger altar;
    XML_VARIABLE XMLInteger recall;
    XML_VARIABLE XMLInteger pit;
    XML_VARIABLE XMLInteger map;
    XML_VARIABLE XMLInteger landing;

    XML_VARIABLE XMLLimits limits;
};

inline const DLString & DefaultHometown::getName( ) const
{
    return Hometown::getName( );
}

inline void DefaultHometown::setName( const DLString &name ) 
{
    this->name = name;
}

inline bool DefaultHometown::isValid( ) const
{
    return true;
}

#endif
