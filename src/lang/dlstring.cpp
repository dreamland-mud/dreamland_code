/* $Id: dlstring.cpp,v 1.2 2011/08/10 15:07:05 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/*
 * NoFate, 2001
 */
#include <sstream>

#include "logstream.h"
#include "dl_ctype.h"
#include "flexer.h"
#include "grammar_entities_impl.h"
#include "dlstring.h"
#include "char.h"

using std::basic_ostringstream;
using std::basic_istringstream;

const DLString DLString::emptyString;

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

int DLString::toInt() const throw( ExceptionBadType )
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

long DLString::toLong() const throw( ExceptionBadType )
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

long long DLString::toLongLong() const throw( ExceptionBadType )
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

long int DLString::toLongInt() const throw( ExceptionBadType )
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

bool DLString::toBoolean() const throw( ExceptionBadType )
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

char DLString::toChar() const throw( ExceptionBadType )
{
	if( ( this->length( ) == 0 ) || ( this->length( ) > 1 ) )
	{
		throw ExceptionBadType( "DLString::toChar", *this );
	}
	else
	{
		return this->c_str()[0];
	}

}

unsigned char DLString::toByte() const throw( ExceptionBadType )
{
	basic_istringstream<char> s( this->c_str( ) );
	unsigned char i;
    
	s >> i;
    
	if( s )
	{
		return i;
	}
	else
	{
		throw ExceptionBadType( "DLString::toByte", *this );
	}

}


DLString::size_type DLString::colorLength( ) const
{
	size_type len_colour = 0;
	size_type len_s = 0;
	size_type pos = find( '{' );
	while( pos != npos )
	{
		if( pos + 1 < length( ) )
		{
			if( at( pos + 1 ) == '{' )
			{
				len_s++;
			}
			else
			{
				len_colour++;
			}
			pos++;
		}
		if( pos == length( ) )
		{
			break;
		}
		pos = find( '{', pos + 1 );
	}
	
	return length( ) - len_colour * 2 - len_s;
}
#if 0
DLString DLString::getOneArgument( )
{
	char terminate_char = ' ';

	if ( empty( ) )
	{
		DLString ret = "";
		return ret;
	}

	size_type no_space = find_first_not_of( ' ', 0 );

	if ( at(no_space) == '\'' || at(no_space) == '"' )
	{
		terminate_char = at(no_space);
		no_space++;
	}

	size_type pos = find( terminate_char, no_space );

	if( pos != npos )
	{
		DLString ret = substr( no_space, pos - no_space );

		if ( terminate_char != ' ' )
			pos++;

		size_type pos2 = find_first_not_of( ' ', pos );

		if( pos2 != npos )
			pos = pos2;
		else
			pos++;
		
		erase( 0, pos );

		return ret;
	}
	else
	{
		DLString ret = substr( no_space ); 

		erase( );

		return ret;
	}
}
#endif

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

    if (*i == '\'' || *i == '"')
	cEnd = *i++;
    
    while (i != end( )) {
	if (*i == cEnd) {
	    i++;
	    break;
	}

//	ret += dl_tolower( *i );
	ret += *i;
	i++;
    }

    while (i != end( ) && dl_isspace( *i ))
	i++;

    erase( begin( ), i );
    return ret;
}

bool DLString::lessCase( const DLString& str ) const
{
	size_type len = length( ) < str.length( ) ? length( ) : str.length( );
	for( size_type i = 0; i < len; i++ )
	{
		char ch1 = dl_toupper( at( i ) );
		char ch2 = dl_toupper( str[i] );
		if( ch1 < ch2 )
		{
			return true;
		}
		else if( ch1 > ch2 )
		{
			return false;
		}
	}
	if( length( ) < str.length( ) )
	{
		return true;
	}
	return false;
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

bool DLString::isRussian( ) const
{
    if (empty( ))
	return false;

    for (size_type i = 0; i < length( ); i++) 
	if (!dl_isrusalpha(at(i)) && !dl_isspace(at(i)))
	    return false;

    return true;
}

void DLString::stripLeftWhiteSpace( )
{
	erase( 0, find_first_not_of( ' ' ) );
}

void DLString::stripRightWhiteSpace( )
{
	erase( find_last_not_of( ' ' ) + 1 );
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

bool DLString::isName( const DLString &msg ) {
    size_type pos; 
    DLString m( msg ), s( *this );
    
    m.toLower( );
    s.toLower( );
    
    if (s == m)
	return true;
    
    pos = s.find( m );
    
    if (pos < 0 || pos >= s.length( ))
	return false;
    
    if (pos == 0 || Char( s.at(pos - 1) ).isDelimiter( )) {
	if (pos + m.length( ) == s.length( ))
	    return true;
	
	if (pos + m.length( ) < s.length( ) && Char( s.at(pos + m.length( )) ).isDelimiter( ))
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

int DLString::splitFirstNumber( char separator )
{
    int number = 0;
    iterator i = begin( );

    if (find( separator ) == npos)
	return 1;

    while (i != end( ) && dl_isspace( *i ))
	i++;

    if (i == end( ) || !isdigit( *i ))
	return 1;
    
    while (i != end( ) && isdigit( *i )) {
	number += number * 10 + *i - '0';
	i++;
    }

    if (i != end( )) 
        erase( 0, find_first_not_of( separator, i - begin( ) ) );

    return number;
}

int DLString::getMultArgument( )
{
    return splitFirstNumber( '*' );
}

int DLString::getNumberArgument( )
{
    return splitFirstNumber( '.' );
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

bool DLString::equalLess( const DLString &str ) const
{
    if (length( ) != str.length( ))
	return false;

    for( size_type pos = 0; pos < length( ); pos++ ) 
	if( dl_tolower( at( pos ) ) != dl_tolower( str.at( pos ) ) )
		return false;

    return true;
}

DLString &DLString::cutSize( size_t s )
{
    if (length( ) > s)
	erase( s );

    return *this;
}

