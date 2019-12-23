/* $Id: mallocexception.h,v 1.1.4.1.10.1 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2005
 */

#ifndef MALLOCEXCEPTION_H
#define MALLOCEXCEPTION_H

#include "exception.h"
#include "dlstring.h"

class MallocException : public Exception {
public:
        MallocException( const DLString &msg, int count )  
              : Exception( "Cannot allocate " + DLString(count) + " bytes for " + msg )
        {
        }
};
                        

#endif
