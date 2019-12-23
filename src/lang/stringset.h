/* $Id: stringset.h,v 1.1.2.3 2011-04-17 19:37:54 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#ifndef STRINGSET_H
#define STRINGSET_H

#include <set>
#include "dlstring.h"

class StringSet : public std::set<DLString> {
public:
    /** 
     * Return a space-separated string of entries,
     * long entries surrounded by single quotes.
     */
    DLString toString( ) const;

    /** Split provided string into words and append to the current set. */
    StringSet & fromString( const DLString &str );
};

class NumberSet : public std::set<int> {
public:
    void fromStringSet( const StringSet & );
    StringSet toStringSet( ) const;
};

struct StringStorage {
    void addTransient(const DLString &entry);
    void addPersistent(const DLString &entry);
    void refresh();

    StringSet transient;
    StringSet persistent;
    StringSet all;
    DLString allString;
};
#endif

