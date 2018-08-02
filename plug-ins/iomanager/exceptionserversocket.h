/* $Id$
 *
 * ruffina, 2004
 */
// exceptionserversocket.h: interface for the ExceptionServerSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXCEPTIONSERVERSOCKET_H__24BD8091_9805_4AEA_9C9D_DE1615845A8D__INCLUDED_)
#define AFX_EXCEPTIONSERVERSOCKET_H__24BD8091_9805_4AEA_9C9D_DE1615845A8D__INCLUDED_

#include "exception.h"

class ExceptionServerSocket : public Exception
{
public:
	inline ExceptionServerSocket( const char* msg, const char* error, int port, int listen )
		: Exception( DLString( msg ) << "::port:" << port << "::listen:" << listen << "::" << error )
	{
	}
};

#endif
