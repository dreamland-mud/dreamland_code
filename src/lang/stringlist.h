#ifndef STRINGLIST_H
#define STRINGLIST_H

#include <vector>
#include "dlstring.h"

class StringList : public std::vector<DLString> {
public:
    StringList();
    StringList(const DLString &);

    /**
     * Checks if smaller list elements are prefixes for some elements of this list.
     */
    bool superListOf(const StringList& smallerList) const;

    /**
     * Concatenates all elements of the list using delimiter.
     */
    DLString join(const DLString &delim) const;

    /**
     * Returns new list, containing all elements 'wrapped' on both sides
     * with provided strings.
     */
    StringList wrap(const DLString &before, const DLString &after) const;
};

#endif
