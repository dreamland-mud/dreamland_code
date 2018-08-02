/* $Id: xmltableelement.h,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2005
 */
#ifndef __XMLTABLEELEMENT_H__
#define __XMLTABLEELEMENT_H__

#include "xmlpolymorphvariable.h"

class XMLTableElement : public virtual XMLPolymorphVariable {
public:
    typedef ::Pointer<XMLTableElement> Pointer;

    virtual void loaded( );
    virtual void unloaded( );
    
    virtual const DLString & getName( ) const = 0;
    virtual void setName( const DLString & ) = 0;
};

#endif
