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
    StringSet();
    StringSet(const DLString &);

    /** 
     * Return a space-separated string of entries,
     * long entries surrounded by single quotes.
     */
    DLString toString( ) const;

    /** Split provided string into words and append to the current set. */
    StringSet & fromString( const DLString &str );

    /** Check if this set contains any element of the other set. */
    bool containsAny(const StringSet &other) const;

    /** Concatenates all elements of the list using delimiter. */
    DLString join(const DLString &delim) const;
};

class NumberSet : public std::set<int> {
public:
    NumberSet();
    NumberSet(const DLString &);
    void fromStringSet( const StringSet & );
    StringSet toStringSet( ) const;
    int randomNumber() const;
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

