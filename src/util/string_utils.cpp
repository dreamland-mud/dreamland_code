#include "string_utils.h"
#include "dl_ctype.h"

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



