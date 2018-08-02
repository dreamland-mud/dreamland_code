/* $Id: marriageexception.h,v 1.1.2.2 2004/02/24 18:33:21 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef MARRIAGEEXCEPTIONS_H
#define MARRIAGEEXCEPTIONS_H

#include "exception.h"

class MarriageException : public Exception {
public:
	MarriageException( DLString str ) : Exception( str )
	{	
	}
};

#endif
