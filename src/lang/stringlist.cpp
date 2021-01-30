#include "stringlist.h"
#include <sstream>
#include <algorithm>

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
    
    int matchesFound = 0;

    // Make sure that all words in the smaller list have a match in the bigger list (this).
    // Order needs to be preserved,  i.e. 'a b' doesn't match 'bbb aaa', but 'a b' matches 'aaa xxx bbb'.
    for (size_t i = 0, j = 0; i < this->size() && j < smallerList.size(); i++) {
        if (smallerList[j].strPrefix(at(i))) {
            matchesFound++;
            j++;
        }

        // First words in both lists need to match.
        if (i == 0 && j == 0)
            return false;
    }

    return (int)smallerList.size() == matchesFound;
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

DLString StringList::toString() const
{
    DLString result;

    for (const_iterator s = begin( ); s != end( ); s++) 
        result << s->quote( ) << " ";
        
    result.stripWhiteSpace( );
    return result;
}

void StringList::addUnique(const DLString &elem)
{
    if (find(begin(), end(), elem) == end())
        push_back(elem);
}

void StringList::split(const DLString &string, const DLString &delim)
{
    size_type pos1, pos2;
       
    // Keep track of beginning and end of the token. 
    pos1 = 0;
    pos2 = 0;    

    // Parse all tokens, allowing for delimiters to repeat themselves
    // several times, e.g. "x   y  z".
    do {
        // Remember where a token starts.
        pos1 = string.find_first_not_of(delim, pos2);
        if (pos1 == DLString::npos)
            break;

        // Remember where a token ends.
        pos2 = string.find(delim, pos1);

        // Cut off a substring with this token.
        this->push_back(string.substr(pos1, pos2 - pos1));
    }
    while (pos1 < string.size() && pos2 < string.size());
}
