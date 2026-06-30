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

    /** Display-time getter: value in 'lang', falling back to RU then EN when empty.
     *  Use for player-facing output so a missing translation never shows as a blank.
     *  Keep 'get' for storage/matching where the exact per-language value is needed. */
    const DLString &getForLang(lang_t lang) const;

    bool matchesStrict( const DLString &str ) const;
    bool matchesUnstrict( const DLString &str ) const;
    bool matchesSubstring( const DLString &str ) const;    

    void fromMixedString(const DLString &str);

    void clearValues();
    bool emptyValues() const;
};

#endif