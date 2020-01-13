/* $Id: fileformatexception.h,v 1.1.2.3.28.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#ifndef FILEFORMATEXCEPTION_H
#define FILEFORMATEXCEPTION_H

#include <stdarg.h>
#include <sstream>

#include "exception.h"

class FileFormatException : public Exception {
public:
    FileFormatException( const char * fmt, ... ) throw();
    virtual ~FileFormatException( ) throw();
};
                        

#endif
