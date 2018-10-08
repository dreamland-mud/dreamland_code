/* $Id: dl_match.cpp,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#include "dl_match.h"
#include "dl_ctype.h"

#define RANGE_MATCH     1
#define RANGE_NOMATCH   0
#define RANGE_ERROR     (-1)

static int
rangematch(const char *pattern, char test, bool casefold, char **newp)
{
    int negate, ok;
    char c, c2;
    const char *origpat;

    if (*pattern == '!' || *pattern == '^') {
        negate = 1;
        ++pattern;
    } else
        negate = 0;

    if (casefold)
        test = dl_tolower(test);

    ok = 0;
    origpat = pattern;

    for (;;) {
        if (*pattern == ']' && pattern > origpat) {
            pattern++;
            break;
            
        } else if (*pattern == 0) {
            return RANGE_ERROR;

        } else if (*pattern == '\\')
            pattern++;
        
        c = *pattern++;

        if (casefold)
            c = dl_tolower(c);

        if (*pattern == '-' && pattern[1] != 0 && pattern[1] != ']') {
            if (*++pattern == '\\')
                if (*pattern != 0)
                        pattern++;

            c2 = *pattern++;
            
            if (c2 == 0)
                return RANGE_ERROR;

            if (casefold)
                c2 = dl_tolower(c2);

            /*XXX - invalid for russian*/
            if ( c <= test && test <= c2 )
                ok = 1;

        } else if (c == test)
            ok = 1;
    }

    *newp = (char *)pattern;

    return ok == negate ? RANGE_NOMATCH : RANGE_MATCH;
}

bool
dl_match(const char *pattern, const char *string, bool casefold)
{
    const char *stringstart;
    char *newp;
    char c, pc, sc;

    for (stringstart = string;;) {
        pc = *pattern++;
        sc = *string;

        switch (pc) {
            case 0:
                return sc == 0;
                
            case '?':
                if (sc == 0)
                    return false;
                
                string++;
                break;

            case '*':
                for(c = *pattern; c == '*'; c = *++pattern)
                    ;

                if (c == 0)
                    return true;

                while (sc != 0) {
                    if (dl_match(pattern, string, casefold))
                        return true;
                    
                    sc = *string++;
                }
                return false;
                
            case '[':
                if (sc == 0)
                    return false;
                
                switch (rangematch(pattern, sc, casefold, &newp)) {
                    case RANGE_ERROR:
                        goto norm;
                        
                    case RANGE_MATCH:
                        pattern = newp;
                        break;
                        
                    case RANGE_NOMATCH:
                        return false;
                }
                string++;
                break;

            case '\\':
                pc = *pattern;
                
                if(pc == 0)
                    pc = '\\';
                else
                    pattern++;
                
            default:
            norm:
                string++;
                
                if (pc == sc)
                    break;
                
                if (casefold && dl_tolower(pc) == dl_tolower(sc))
                    break;
                
                return false;
        }
    }
}

