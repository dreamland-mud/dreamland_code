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
    DLString toString( ) const;
    StringSet & fromString( const DLString & );
};

class NumberSet : public std::set<int> {
public:
    void fromStringSet( const StringSet & );
    StringSet toStringSet( ) const;
};

#endif

