/* $Id: dlstring.cpp,v 1.2 2011/08/10 15:07:05 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/*
 * NoFate, 2001
 */
#include <sstream>
#include <list>

#include "logstream.h"
#include "dl_ctype.h"
#include "flexer.h"
#include "grammar_entities_impl.h"
#include "dlstring.h"

using std::basic_ostringstream;
using std::basic_istringstream;

const DLString DLString::emptyString;
const DLString DLString::SPACE = " ";

const locale DLString::LOCALE_RU = DLString::initLocale();

bool DLString::strPrefix( const DLString& str ) const
{
    if (length( ) > str.length( ))
        return false;

    for( size_type pos = 0; pos < length( ); pos++ )
    {
        if( dl_tolower( at( pos ) ) != dl_tolower( str.at( pos ) ) )
        {
                return false;
        }
    }
    return true;
}

bool DLString::strSuffix( const DLString& str ) const
{
    if (length( ) > str.length( ))
        return false;

    for (size_type p = length( ) - 1, sp = str.length( ) - 1;
         p >= 0 && sp >= 0;
         p--, sp--)
    {
        if (dl_tolower( at( p ) ) != dl_tolower( str.at( sp ) ) )
            return false;
        
        if (p == 0)
            return true;
    }

    return true;
}

static void colourstrip( const char *str, ostringstream &out )
{
    const char *point;

    for (point = str; *point; point++) 
        if (*point != '{') {
            out << *point;
        }
        else if (!*++point) {
            out << "{";
            break;
        }
        else if (*point == '{') {
            out << *point;
        }
        else if  (*point == '/') {
            out << endl;
        }
}

void DLString::colourstrip( )
{
    ostringstream out;
    
    ::colourstrip( c_str( ), out );
    assign( out.str( ) );
}

DLString DLString::colourStrip( ) const
{
    ostringstream out;
    
    ::colourstrip( c_str( ), out );
    return out.str( );
}

DLString& DLString::assign( short value )
{
        basic_ostringstream<char> buf;
        buf << value;
        return assign( buf.str( ) );
}

DLString& DLString::assign( unsigned int value )
{
        basic_ostringstream<char> buf;
        buf << value;
        return assign( buf.str( ) );
}

DLString& DLString::assign( long unsigned int value )
{
        basic_ostringstream<char> buf;
        buf << value;
        return assign( buf.str( ) );
}

DLString& DLString::assign( int value )
{
        basic_ostringstream<char> buf;
        buf << value;
        return assign( buf.str( ) );
}

DLString& DLString::assign( long value )
{
        basic_ostringstream<char> buf;
        buf << value;
        return assign( buf.str( ) );
}

DLString& DLString::assign( long long value )
{
        basic_ostringstream<char> buf;
        buf << value;
        return assign( buf.str( ) );
}

DLString& DLString::append( short value )
{
        basic_ostringstream<char> buf;
        buf << value;
        return append( buf.str( ) );
}

DLString& DLString::append( unsigned int value )
{
        basic_ostringstream<char> buf;
        buf << value;
        return append( buf.str( ) );
}

DLString& DLString::append( long unsigned int value )
{
        basic_ostringstream<char> buf;
        buf << value;
        return append( buf.str( ) );
}

DLString& DLString::append( int value )
{
        basic_ostringstream<char> buf;
        buf << value;
        return append( buf.str( ) );
}

DLString& DLString::append( long value )
{
        basic_ostringstream<char> buf;
        buf << value;
        return append( buf.str( ) );
}

DLString& DLString::append( long long value )
{
        basic_ostringstream<char> buf;
        buf << value;
        return append( buf.str( ) );
}

int DLString::toInt() const 
{
        basic_istringstream<char> s( this->c_str() );
        int i;
    
        s >> i;
    
        if( s )
        {
                return i;
        }
        else
        {
                throw ExceptionBadType( "DLString::toInteger", *this );
        }
}                                                                                                   

long DLString::toLong() const 
{
        basic_istringstream<char> s( this->c_str() );
        long i;
    
        s >> i;
    
        if( s )
        {
                return i;
        }
        else
        {
                throw ExceptionBadType( "DLString::toLong", *this );
        }

}

long long DLString::toLongLong() const 
{
        basic_istringstream<char> s( this->c_str( ) );
        long long i;

        s >> i;

        if( s ) {
                return i;
        }
        else
        {
                throw ExceptionBadType( "DLString::toLongLong", *this );
        }

}

long int DLString::toLongInt() const 
{
        basic_istringstream<char> s( this->c_str( ) );
        long int i;

        s >> i;

        if( s ) {
                return i;
        }
        else
        {
                throw ExceptionBadType( "DLString::toLongInt", *this );
        }

}

bool DLString::toBoolean() const 
{
        if( *this == "true" )
        {
                return true;
        }
        else if( *this == "false" )
        {
                return false;
        }
        else
        {
                throw ExceptionBadType( "DLString::toBoolean", *this );
        }

}

DLString DLString::getOneArgument( )
{
    char cEnd;
    DLString ret = "";
    iterator i = begin( );
    
    while (i != end( ) && dl_isspace( *i ))
        i++;

    if(i == end( ))
        return "";

    cEnd = ' ';

    if (dl_is_arg_separator(*i))
        cEnd = *i++;
    
    while (i != end( )) {
        if (*i == cEnd) {
            i++;
            break;
        }

        ret += *i;
        i++;
    }

    while (i != end( ) && dl_isspace( *i ))
        i++;

    erase( begin( ), i );
    return ret;
}

bool DLString::isNumber( ) const
{
    char ch;
    
    if( empty( ) ) 
        return false;
        
    ch = at( 0 );
    if (ch != '-' && (ch < '0' || ch > '9'))
        return false;
        
    for(size_type i = 1; i < length( ); i++ ) {
        ch = at( i );
        if( ch < '0' || ch > '9' ) 
            return false;
    }

    return true;
}

bool DLString::isCyrillic( ) const
{
    if (empty( ))
        return false;

    for (size_type i = 0; i < length( ); i++) 
        if (!dl_is_cyrillic(at(i)) && !dl_isspace(at(i)))
            return false;

    return true;
}

DLString & DLString::stripLeftWhiteSpace( )
{
        erase( 0, find_first_not_of( ' ' ) );
        return *this;
}

DLString & DLString::stripRightWhiteSpace( )
{
        erase( find_last_not_of( ' ' ) + 1 );
        return *this;
}

DLString& DLString::toLower( )
{
        for( size_type pos = 0; pos < length( ); pos++ )
        {
                char& ch = at( pos );
                ch = dl_tolower( ch );
        }
        return *this;
}

DLString DLString::toLower( ) const
{
    DLString rc;

    for( size_type pos = 0; pos < length( ); pos++ ) {
        rc << dl_tolower( at( pos ) );
    }

    return rc;
}

DLString& DLString::toUpper( )
{
        for( size_type pos = 0; pos < length( ); pos++ )
        {
                char& ch = at( pos );
                ch = dl_toupper( ch );
        }
        return *this;
}

DLString DLString::toUpper( ) const
{
    DLString rc;

    for( size_type pos = 0; pos < length( ); pos++ ) {
        rc << dl_toupper( at( pos ) );
    }

    return rc;
}

bool DLString::isName( const DLString &msg ) const {
    size_type pos; 
    DLString m( msg ), s( *this );
    
    m.toLower( );
    s.toLower( );
    
    if (s == m)
        return true;
    
    pos = s.find( m );
    
    if (pos < 0 || pos >= s.length( ))
        return false;
    
    if (pos == 0 || dl_isdelim( s.at(pos - 1) )) {
        if (pos + m.length( ) == s.length( ))
            return true;
        
        if (pos + m.length( ) < s.length( ) && dl_isdelim( s.at(pos + m.length( )) ))
            return true;

        return false;
    }

    return false;
}

DLString& DLString::substitute( char a, char b )
{
    for (size_type i = 0; i < length( ); i++)
        if (at( i ) == a)
            (*this)[i] = b;

    return *this;
}

DLString DLString::substitute( char a, const char *b ) const
{
    DLString rc;
    
    for (const_iterator i = begin( ); i != end( ); i++)
        if (*i == a)
            rc += b;
        else
            rc += *i;

    return rc;
}

DLString & DLString::replaces( const DLString &a, const DLString &b )
{
    DLString rc;
    const_iterator i, j, old_i;
    
    for (i = begin( ); i != end( ); ) {
        old_i = i;
        j = a.begin( );
        
        while (*i == *j && j != a.end( ) && i != end( ))  {
            i++;
            j++;
        }

        if (j == a.end( ))
            rc += b;
        else {
            i = old_i;
            rc += *i;
            i++;
        }
    }
    
    this->assign( rc );
    return *this;
}

inline void skipcolor( const char *&a )
{
    while (*a == '{')
        if (*++a == '{' || !*a)
            break;
        else
            a++;
}

bool DLString::operator ^ ( const DLString &that ) const
{
    const char *a = this->c_str( );
    const char *b = that.c_str( );

    while (*a) {
        skipcolor(a);
        skipcolor(b);

        if (dl_tolower( *a++ ) != dl_tolower( *b++ ))
            return false;
    }
 
    return !(*b);
}

DLString & DLString::capitalize( )
{
    for( size_type pos = 0; pos < length( ); pos++ )
    {
        char& ch = at( pos );
        ch = dl_tolower( ch );
    }

    upperFirstCharacter( );
    return *this;
}

DLString DLString::ruscase( char gram_case ) const
{
    if (gram_case == '7')
        return *this;
    return Flexer::flex( *this, Grammar::Case( gram_case ) + 1 );
}

DLString DLString::quote( ) const
{
    DLString q;

    if (empty( ))
        return *this;

    if (find( ' ' ) == npos)
        return *this;
    
    if (at( 0 ) == '\'')
        return *this;

    q << "\'" << *this << "\'";
    return q;
}

DLString & DLString::upperFirstCharacter( )
{
    if (!empty( ))
        at( 0 ) = dl_toupper( at( 0 ) );

    return *this;
}

DLString DLString::upperFirstCharacter( ) const
{
    DLString rc;

    if (empty())
        return rc;
    
    rc << dl_toupper(at(0));
    for(size_type pos = 1; pos < length(); pos++) 
        rc << at(pos);

    return rc;
}

std::list<DLString> DLString::split(const DLString &delim) const
{    
    std::list<DLString> tokens;
    size_type pos1, pos2;
       
    // Keep track of beginning and end of the token. 
    pos1 = 0;
    pos2 = 0;    

    // Parse all tokens, allowing for delimiters to repeat themselves
    // several times, e.g. "x   y  z".
    do {
        // Remember where a token starts.
        pos1 = find_first_not_of(delim, pos2);
        if (pos1 == DLString::npos)
            break;

        // Remember where a token ends.
        pos2 = find(delim, pos1);

        // Cut off a substring with this token.
        tokens.push_back(substr(pos1, pos2 - pos1));
    }
    while (pos1 < size() && pos2 < size());

    return tokens;
}

int DLString::compareRussian(const DLString &other) const
{
    const collate<char> &col = use_facet<collate<char>>(LOCALE_RU);

    const char *pb1 = data();
    const char *pb2 = other.data();

    return col.compare(pb1, pb1 + size(),
                        pb2, pb2 + other.size());
}

locale DLString::initLocale()
{
    try {
        return locale("uk_UA.koi8u");
    } catch (const exception &ex) {
        return locale();
    }
}
