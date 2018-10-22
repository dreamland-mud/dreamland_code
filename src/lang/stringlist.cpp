#include "stringlist.h"

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
    
    for (int i = 0; i < smallerList.size(); i++)
       if (!smallerList[i].strPrefix(at(i)))
            return false;

    return true;
}

