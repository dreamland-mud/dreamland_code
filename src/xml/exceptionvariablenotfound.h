/* $Id$
 *
 * ruffina, 2004
 */
// exceptionvariablenotfound.h: interface for the ExceptionVariableNotFound_ class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __EXCEPTIONVARIABLENOTFOUND_H__
#define __EXCEPTIONVARIABLENOTFOUND_H__

#include "exception.h"

class ExceptionVariableNotFound : public Exception {
public:
    inline ExceptionVariableNotFound( string name, string className )
            : Exception( string( "Variable '" ) + name + "' not found in class '" + className + '\'' )
    {
    }

    virtual ~ExceptionVariableNotFound( ) ;
};


#endif
