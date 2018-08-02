/* $Id: dl_ctype.cpp,v 1.1.2.3 2009/11/02 13:48:11 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#include <string.h>
#include <ctype.h>

#include "dl_ctype.h"

bool dl_isdelim( char ch )
{
    return (strchr( " ,.!;'\":\t`" , ch ) != NULL);
}

bool dl_isspace( char ch )
{
    switch(ch) {
	case ' ':
//	case '\0':
	case '\t':
	case '\r':
	case '\n':
	    return true;

	default:
	    return false;
    }
}

bool dl_isrusalpha( char c )
{
    return (c >= 'à' && c < 'ÿ') 
	   || (c >= 'À' && c <= 'ß') 
	   || c == '£' 
	   || c == '³';
}

bool dl_isalpha( char c )
{
    return isalpha(c) || dl_isrusalpha( c );
}

bool dl_isupper( char c )
{
    return isupper( c ) 
	    || (c >= 'à' && c < 'ÿ') 
	    || c == '³';
}

bool dl_islower( char c )
{
    return islower( c )
	   || (c >= 'À' && c <= 'ß') 
	   || c == '£';
}

bool dl_isalnum( char c )
{
    return dl_isalpha( c ) || isdigit( c );
}


