/* $Id: regexp.cpp,v 1.1.2.8.10.4 2011-04-19 01:20:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#include "regexp.h"

#include "logstream.h"

RegExp::RegExp( const char *r )
{
    prepare( r, false );
}

RegExp::RegExp( const char *r, bool fCase ) 
{
    prepare( r, fCase );
}

void RegExp::prepare( const char *r, bool fCase )
{
    int errcode, flags;
    char errbuf[512];
    
    flags = REG_EXTENDED /* | REG_NOSUB */;

    if (!fCase)
        flags |= REG_ICASE;

    errcode = regcomp(&preg, r, flags);
    
    if (errcode) {
        regerror(errcode, &preg, errbuf, sizeof(errbuf));
        throw Exception(errbuf);
    }
}

RegExp::~RegExp( ) 
{
    regfree(&preg);
}

bool RegExp::match( const DLString &str ) const
{
    return match( str.c_str( ) );
}

bool RegExp::match(  const char *str)  const
{
    regmatch_t pmatch[256];
    
    return !regexec(&preg, str, 0, pmatch, 0);
}

RegExp::MatchVector RegExp::subexpr( const char *str ) const
{
    int errcode;
    size_t nmatch = preg.re_nsub + 1;
    regmatch_t pmatch[nmatch];
    MatchVector result;

    errcode = regexec (&preg, str, nmatch, pmatch, 0);
    
    if (!errcode) {
        for (size_t i = 1; i < nmatch; i++) {
            char buf[256];
            regoff_t j;
            
            if (pmatch[i].rm_so != -1 && pmatch[i].rm_eo != -1) {
                for (j = 0; j < pmatch[i].rm_eo - pmatch[i].rm_so; j++)
                    buf[j] = str[pmatch[i].rm_so + j];

                buf[j] = 0;
                result.push_back( buf );
            }  
        }
    }

    return result;
}

