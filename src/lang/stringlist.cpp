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

// Check that 'b c' or 'a b c d' or 'a d' matches 'aa bb cc dd'.
// Order is preserved: 'c b' or 'dd cc bb aa' would not match.
bool StringList::superListOf(const StringList& smallList) const
{
    const StringList &bigList = *this;

    // Quick sanity check.
    if (smallList.size() > bigList.size())
        return false;
    
    size_t smallIndex;
    int lastBigIndex = -1;

    for (smallIndex = 0; smallIndex < smallList.size(); smallIndex++) {
        // Look for the current small list entry inside big list,
        // starting from the last matched position (to preserve overall order).
        bool matchFound = false;

        // If we reached the end of the big list but there are still entries
        // left in the smaller one - it's a fail.
        if (lastBigIndex >= (int)bigList.size())
            return false;

        for (size_t i = lastBigIndex + 1; i < bigList.size(); i++) {
            if (smallList[smallIndex].strPrefix(bigList.at(i))) {
                // A match!
                matchFound = true;
                lastBigIndex = i;
                break;
            }
        }

        // Every entry in the small list must have a match in the bigger list,
        // otherwise it's a fail.
        if (!matchFound)
            return false;
    }

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
