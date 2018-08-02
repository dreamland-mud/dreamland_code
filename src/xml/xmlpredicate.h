/* $Id: xmlpredicate.h,v 1.1.2.3.28.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#ifndef XMLPREDICATE_H
#define XMLPREDICATE_H

#include "xmlpolymorphvariable.h"

class XMLPredicate : public virtual XMLPolymorphVariable {
public:

    virtual bool eval( DLObject * ) const = 0;  
    virtual void fromXML( const XMLNode::Pointer& ) throw (ExceptionBadType);
    virtual bool toXML( XMLNode::Pointer& ) const;

    static const DLString ATTRIBUTE_INVERT;

protected:
    bool invert;
};

#endif
