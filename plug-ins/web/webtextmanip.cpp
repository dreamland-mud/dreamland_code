/* $Id$
 *
 * ruffina, 2018
 */
#include "webmanipcommandtemplate.h"

#include <sstream>

#include "logstream.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"


// Read in words between ( and ), remembering them in keyword.
// Returns position of the next ')' symbol.
static const char * consume_keyword(const char *desc, DLString &keyword)
{
    const char *d;
    for (d = desc; *d && *d != ')'; ++d) {
        if (!isspace(*d) && !isalpha(*d) && *d != '-') {
            // Doesn't look like part of a keyword anymore, interrupt.
            break;
        }

        // Still part of the keyword, remember it and move on.
        keyword += *d;
    }   

    // Stopped because we found matching ')' bracket - success.
    if (*d == ')') {
        return d;
    }

    // Stopped because end of string was reached - failure.
    // Return same position as where we started.
    return desc;
}


// Returns all extra description keywords matching given one.
// In the absense of unique numeric IDs for extra descriptions,
// this is used to identify an extra descr as exactly as possible.
static DLString unique_keyword_id(EXTRA_DESCR_DATA *ed, const DLString &keyword)
{
    for (; ed; ed = ed->next)
        if (is_name( keyword.c_str( ), ed->keyword ))
            return ed->keyword;

    return DLString::emptyString;
}

/**
 * Decorates room descriptions, object descriptions and various extra descrs
 * by replacing (sign) with [read=sign знак,see=sign] pseudo-tag for web-client.
 * Normal (sign) is still sent to telnet clients and invisible for web.
 * These tags will become clickable links in the web client, resulting in 
 * "read 'sign znak'" command being sent back to the server.
 */
WEBMANIP_RUN(decorateExtraDescr)
{
    const ExtraDescrManipArgs &myArgs = static_cast<const ExtraDescrManipArgs &>( args );
    const char *desc = myArgs.desc;
    extra_descr_data *ed = myArgs.ed;
    const char *d;

    for (d = desc; *d; ++d) {
        // Normal part of the description, copy as is.
        if (*d != '(') {
            buf << *d;
            continue;
        }
      
        // Potential extra-descr in brackets, try to parse it.
        DLString keyword;
        const char *next_bracket = consume_keyword(d+1, keyword);
        if (next_bracket == d+1) {
            // Nothing to see here, move along.
            buf << *d;
            continue;
        }

        // Found something that may be a description keyword. 
        // Look if matching extra description exists.
        DLString unique = unique_keyword_id(ed, keyword);
        if (unique.empty( )) {
            // False alarm, move along.
            buf << "(" << keyword << ")";
        } else {
            // Decorate with pseudo-tags, hide normal output from web.
            buf << "{Iw[read=" << unique << ",see=" << keyword << "]";
            buf << "{IW(" << keyword << "){Ix";
        }

        // Advance position to the next bracket.
        d = next_bracket;
    }

    return true;
}

