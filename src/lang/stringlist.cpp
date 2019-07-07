#include "stringlist.h"
#include <sstream>

StringList::StringList()
{
}

StringList::StringList(const DLString &constStr)
{
    DLString arg;
    DLString str = constStr;
    
    while (!(arg = str.getOneArgument()).empty())
        push_back(arg);
}

bool StringList::superListOf(const StringList& smallerList) const
{
    if (smallerList.size() > this->size())
        return false;
    
    for (size_t i = 0; i < smallerList.size(); i++)
       if (!smallerList[i].strPrefix(at(i)))
            return false;

    return true;
}

DLString StringList::join(const DLString &delim) const
{
    ostringstream buf;

    for (const_iterator i = begin(); i != end(); i++) {
        if (i != begin())
            buf << delim;
        buf << *i;
    }

    return buf.str();
}

StringList StringList::wrap(const DLString &before, const DLString &after) const
{
    StringList result;

    for (const_iterator i = begin(); i != end(); i++) {
        ostringstream buf;
        buf << before << *i << after;
        result.push_back(buf.str());
    }

    return result;
}

