/* $Id: exceptionskipvariable.h,v 1.1.2.2.18.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#ifndef EXCEPTIONSKIPVARIABLE_H
#define EXCEPTIONSKIPVARIABLE_H

#include "exception.h"

class ExceptionSkipVariable : public Exception {
public:
    inline ExceptionSkipVariable( ) throw(): Exception( "XML variable skipped while saving" )
    {
    }

    virtual ~ExceptionSkipVariable( ) throw();
};


#endif
