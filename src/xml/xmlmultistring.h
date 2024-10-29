#ifndef XMLMULTISTRING_H
#define XMLMULTISTRING_H

#include <map>
#include "xmlnode.h"
#include "xmlvariable.h"
#include "stringlist.h"
#include "lang.h"

/** XML representation of a string defined in several different languages. */
class XMLMultiString : public std::map<lang_t, DLString> {
public:
    XMLMultiString();

    bool toXML(XMLNode::Pointer& parent) const;
    void fromXML(const XMLNode::Pointer& parent);
    const DLString &get(lang_t lang) const;
    
    bool matchesStrict( const DLString &str ) const;
    bool matchesUnstrict( const DLString &str ) const;
    bool matchesSubstring( const DLString &str ) const;    

    void fromMixedString(const DLString &str);

    void clearValues();
    bool emptyValues() const;
};

#endif