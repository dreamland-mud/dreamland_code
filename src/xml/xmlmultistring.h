#ifndef XMLMULTISTRING_H
#define XMLMULTISTRING_H

#include <map>
#include "dlstring.h"
#include "xmlnode.h"
#include "xmlvariable.h"

// quick fix
typedef enum {
    LANG_MIN = 0,
    LANG_EN = 0,
    LANG_RU = 1,
    LANG_UA = 2,
    EN = LANG_EN,
    RU = LANG_RU,
    UA = LANG_UA,
    LANG_MAX = 3,
    LANG_DEFAULT = RU
} lang_t;

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
};

#endif