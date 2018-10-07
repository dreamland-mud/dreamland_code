/* $Id: regexp.h,v 1.1.2.7.10.4 2011-04-19 01:20:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#ifndef REGEXP_H
#define REGEXP_H

#include "../regex/regex.h"
#include <vector>

#include "dlobject.h"
#include "dlstring.h"

#include "exception.h"

class RegExp : public virtual DLObject {
public:
    typedef std::vector<DLString> MatchVector;

    struct Exception : public ::Exception 
    { 
	Exception(const char *e) : ::Exception(e) { }
    };
    
    RegExp( const char * );
    RegExp( const char *, bool );
    ~RegExp( );

    bool match( const char * ) const;
    bool match( const DLString & ) const;
    MatchVector subexpr( const char * ) const;

private:
    void prepare( const char *, bool );

    regex_t preg;
};

#endif

