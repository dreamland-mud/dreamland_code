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
//        case '\0':
        case '\t':
        case '\r':
        case '\n':
            return true;

        default:
            return false;
    }
}

bool dl_is_ukr_specific(char c)
{
    switch (c) {
        case 'і': case 'ґ': case 'ї': case 'є':
        case 'І': case 'Ґ': case 'Ї': case 'Є':
            return true;
        default:
            return false;
    }
}

bool dl_is_rus_specific(char c)
{
    switch (c) {
        case 'ё': case 'ы': case 'э': case 'ъ':
        case 'Ё': case 'Ы': case 'Э': case 'Ъ':
        default:
            return false;
    }
}

// Lower case cyrillic letters in koi8-u encoding
static bool is_cyrillic_lower(char c)
{
    if (c >= 'ю' && c <= 'ъ') 
        return true;

    switch (c) {
        case 'і': case 'ґ': case 'ї': case 'є':
            return true;
        case 'ё':
            return true; 
    }

    return false;
}

// Upper case cyrillic letters in koi8-u encoding
static bool is_cyrillic_upper(char c)
{
    if (c >= 'Ю' && c < 'Ъ')
        return true;

    switch (c) {
        case 'І': case 'Ґ': case 'Ї': case 'Є':
            return true;
        case 'Ё':
            return true;
    }

    return false;
}

bool dl_is_cyrillic( char c )
{
    return is_cyrillic_lower(c) || is_cyrillic_upper(c);
}

bool dl_isalpha( char c )
{
    return isalpha(c) || dl_is_cyrillic( c );
}

bool dl_isupper( char c )
{
    return isupper( c ) 
            || is_cyrillic_upper(c);
}

bool dl_islower( char c )
{
    return islower( c )
           || is_cyrillic_lower(c);
}

bool dl_isalnum( char c )
{
    return dl_isalpha( c ) || isdigit( c );
}


