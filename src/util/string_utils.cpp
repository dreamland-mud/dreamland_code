#include <sstream>
#include "string_utils.h"
#include "dl_ctype.h"
#include "xmlmultistring.h"

using namespace std;

bool String::equalLess(const DLString &a, const DLString &b)
{
    if (a.length( ) != b.length( ))
        return false;

    for (DLString::size_type pos = 0; pos < b.length(); pos++) 
        if( dl_tolower(a.at( pos ) ) != dl_tolower( b.at( pos ) ) )
                return false;

    return true;
}

DLString &String::truncate(DLString &str, size_t size)
{
    if (str.length( ) > size)
        str.erase( size );

    return str;
}

bool String::hasUaSymbol(const DLString &str)
{
    for (DLString::size_type pos = 0; pos < str.length(); pos++) 
        if (dl_is_ukr_specific(str.at(pos)))
            return true;

    return false;
}

bool String::hasRuSymbol(const DLString &str)
{
    for (DLString::size_type pos = 0; pos < str.length(); pos++) 
        if (dl_is_rus_specific(str.at(pos)))
            return true;

    return false;
}

bool String::hasCyrillic(const DLString &str)
{
    if (str.empty( ))
        return false;

    for (DLString::size_type i = 0; i < str.length( ); i++) 
        if (dl_is_cyrillic(str.at(i)))
            return true;

    return false;
}

bool String::lessCase( const DLString &a, const DLString& b )
{
        DLString::size_type len = a.length( ) < b.length( ) ? a.length( ) : b.length( );
        for( DLString::size_type i = 0; i < len; i++ )
        {
                char ch1 = dl_toupper( a.at( i ) );
                char ch2 = dl_toupper( b.at(i) );
                if( ch1 < ch2 )
                {
                        return true;
                }
                else if( ch1 > ch2 )
                {
                        return false;
                }
        }
        if( a.length( ) < b.length( ) )
        {
                return true;
        }
        return false;
}

bool String::isEmpty( const char *arg )
{
    DLString descr;

    if (arg == 0 || arg[0] == 0)
        return true;

    descr = arg;
    descr.colourstrip( );
    
    if (descr.empty( ))
        return true;

    return false;
}

const DLString& String::firstNonEmpty(const XMLMultiString& a, const XMLMultiString& b, lang_t lang)
{
    const DLString &a_value = a.get(lang);
    if (!a_value.empty())
        return a_value;

    const DLString &b_value = b.get(lang);
    if (!b_value.empty())
        return b_value;

    return DLString::emptyString;
}

DLString String::addLine(const DLString& text, const DLString& line)
{
    ostringstream buf;

    buf << text << line << endl;

    return buf.str();
}

DLString String::delLine(const DLString& text)
{
    DLString buf = text;
    DLString::size_type i1, i2;

    if (buf.empty())
        return buf;
    
    i1 = buf.find_last_of( "\n" );

    if (i1 == DLString::npos || i1 == 0) {
        buf.erase( );
    }
    else {
        i2 = buf.find_last_of( "\n", i1 - 1 );
        
        if (i2 == DLString::npos) 
            buf.erase( );
        else
            buf.erase( i2 + 1 );
    }

    return buf;
}

bool String::contains(const DLString& bigString, const DLString& smallString)
{
    return bigString.find(smallString) != DLString::npos;
}

std::list<DLString> String::toLines(const DLString& text)
{
    char buf[1024];
    list<DLString> result;

    if (text.empty())
        return result;

    istringstream is(text);
    while (is.getline(buf, sizeof(buf)))
        result.push_back(buf);

    return result;
}

DLString String::fromLines(const std::list<DLString>& lines)
{
    ostringstream buf;

    for (auto &l: lines)
        buf << l << endl;

    return buf.str();
}
