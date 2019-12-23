/* $Id: xmlclause.h,v 1.1.2.3.28.3 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#ifndef XMLCLAUSE_H
#define XMLCLAUSE_H

#include "xmllist.h"
#include "xmlpredicate.h"

typedef XMLListBase<XMLPointer<XMLPredicate> > XMLPredicatesList;

class XMLClause : public XMLPredicatesList
{
public:
    XMLClause( );

    bool match( DLObject * ) const;

    virtual void fromXML( const XMLNode::Pointer& ) ;
    virtual bool toXML( XMLNode::Pointer& ) const;

    static const DLString TYPE;
    static const DLString ATTRIBUTE_ALLOW;

    bool allow;
};

#endif
