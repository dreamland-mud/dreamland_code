/* $Id: dl_strings.cpp,v 1.1.2.5 2009/11/02 13:48:11 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#include <string.h>
#include <stdlib.h>
#include "dl_strings.h"
#include "dl_ctype.h"
#include "flexer.h"
#include "grammar_entities_impl.h"
#include "inflectedstring.h"

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return true if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
    if (!astr || !bstr)
        return true;

    for ( ; *astr; astr++, bstr++ )
    {
        if (dl_tolower(*astr) != dl_tolower(*bstr))
            return true;
    }

    return false;
}




/*
 * Return true if an argument is completely numeric.
 */
bool is_number( const char *arg )
{
    if (*arg == '\0')
        return false;

    if (*arg == '+' || *arg == '-')
        arg++;

    for ( ; *arg != '\0'; arg++ )
        if (!isdigit( *arg ))
            return false;

    return true;
}


/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
        char *pdot;
        int number;

        for ( pdot = argument; *pdot != '\0'; pdot++ )
        {
                if ( *pdot == '.' )
                {
                        *pdot = '\0';
                        number = atoi( argument );
                        *pdot = '.';
                        strcpy( arg, pdot+1 );
                        return number;
                }
        }

        strcpy( arg, argument );

        return 1;
}

/*
 * Given a string like 14*foo, return 14 and 'foo'
*/
int mult_argument(char *argument, char *arg)
{
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
        if ( *pdot == '*' )
        {
            *pdot = '\0';
            number = atoi( argument );
            *pdot = '*';
            strcpy( arg, pdot+1 );
            return number;
        }
    }

    strcpy( arg, argument );
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
        char cEnd;

        while ( dl_isspace(*argument) )
                argument++;

        cEnd = ' ';

        if ( *argument == '\'' || *argument == '"' )
                cEnd = *argument++;

        while ( *argument != '\0' )
        {
                if ( *argument == cEnd )
                {
                        argument++;
                        break;
                }

                *arg_first = dl_tolower(*argument);
                arg_first++;
                argument++;
        }

        *arg_first = '\0';

        while ( dl_isspace(*argument) )
                argument++;

        return argument;
}

     
/*
 * See if a string is one of the names of an object.
 */
bool is_name( const char *carg1, const char *carg2 )
{        
        char *list;
        char *string;
        
        if (!carg1 || !carg2)
            return false;

        DLString arg1str = DLString(carg1).colourStrip();
        DLString arg2str = DLString(carg2).colourStrip();
        const char *arg1 = arg1str.c_str();
        const char *arg2 = arg2str.c_str();

        int len1 = strlen(arg1) + 1, len2 = strlen(arg2) + 1;
        char name[len2], part[len1];
        char buf_str[len1], buf_namelist[len2];
        char *str = buf_str, *namelist = buf_namelist;

        strcpy( buf_str, arg1 );
        strcpy( buf_namelist, arg2 );

        if ( namelist == 0 || namelist[0] == '\0' )
                return false;

        if ( str[0] == '\0'
                || ( str[0] == '\''
                        && ( str[1] == '\''
                                || str[1] == '\0' ) ) )
                return false;
                
        string = str;
        /* we need ALL parts of string to match part of namelist */
        for ( ; ; )  /* start parsing string */
        {
                str = one_argument(str,part);

                if ( part[0] == '\0' )
                        return true;

                /* check to see if this is part of namelist */
                list = namelist;
                for ( ; ; )  /* start parsing namelist */
                {
                        list = one_argument(list,name);
                        if ( name[0] == '\0' )  /* this name was not found */
                                return false;

                        if ( !str_prefix(string,name) )
                                return true; /* full pattern match */

                        if ( !str_prefix(part,name) )
                                break;
                }
        }
}

/*--------------------------------------------------------------------------
 * russian case 
 *--------------------------------------------------------------------------*/
DLString russian_case( const DLString &description, char gram_case )
{
    return Flexer::flex( description, Grammar::Case( gram_case ) + 1 );
}

DLString russian_case_all_forms(const DLString &string)
{
    InflectedString rs(string);
    return rs.decline(Grammar::Case::MAX).colourStrip();
}

std::list<DLString> russian_cases(const DLString &str)
{
    InflectedString inflected(str);
    std::list<DLString> cases;

    for (int c = Case::NOMINATIVE; c < Case::MAX; c++) {
        cases.push_back(inflected.decline(c));
    }

    return cases;
}

