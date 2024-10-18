/* $Id: xmlinflectedstring.h,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#ifndef XMLRUSSIANSTRING_H
#define XMLRUSSIANSTRING_H

#include "xmlnode.h"
#include "inflectedstring.h"

class XMLInflectedString : public InflectedString {
public:
    static const DLString ATTRIBUTE_GRAMMAR;

    bool toXML( XMLNode::Pointer& node ) const;
    void fromXML( const XMLNode::Pointer& node ) ;
};

#endif
