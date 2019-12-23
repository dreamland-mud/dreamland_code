/* $Id: xmlstringlist.h,v 1.1.2.6 2011-04-17 19:37:54 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#ifndef XMLSTRINGLIST_H
#define XMLSTRINGLIST_H

#include "stringset.h"
#include "xmlstring.h"
#include "xmllist.h"

class XMLStringList : public XMLListBase<XMLString> {
public:

    XMLStringList();
    StringSet toSet() const;
};

class XMLStringSet : public StringSet {
public:
    bool toXML( XMLNode::Pointer& node ) const;
    void fromXML( const XMLNode::Pointer& node ) ;
};

class XMLNumberSet : public NumberSet {
public:
    bool toXML( XMLNode::Pointer& node ) const;
    void fromXML( const XMLNode::Pointer& node ) ;
};

#endif
