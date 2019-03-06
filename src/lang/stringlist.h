#ifndef STRINGLIST_H
#define STRINGLIST_H

#include <vector>
#include "dlstring.h"

class StringList : public std::vector<DLString> {
public:
    StringList(const DLString &);
    bool superListOf(const StringList& smallerList) const;
};

#endif
