/* $Id: xmltimestamp.h,v 1.1.2.2 2009/10/11 18:35:39 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#ifndef XMLTIMESTAMP_H
#define XMLTIMESTAMP_H

#include "xmllong.h"
#include "xmlvariable.h"

class XMLTimeStamp : public XMLLong, public virtual XMLVariable {
public:
    XMLTimeStamp( );

    virtual bool toXML( XMLNode::Pointer& ) const;
    virtual void fromXML( const XMLNode::Pointer& ) throw( ExceptionBadType );
};


#endif
