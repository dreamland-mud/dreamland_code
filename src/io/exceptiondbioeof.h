/* $Id: exceptiondbioeof.h,v 1.4.34.1 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
// exceptiondbioeof.h: interface for the ExceptionDBIOEOF class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXCEPTIONDBIOEOF_H__D3154446_87DA_4B52_B061_5AFC6CEA98AD__INCLUDED_)
#define AFX_EXCEPTIONDBIOEOF_H__D3154446_87DA_4B52_B061_5AFC6CEA98AD__INCLUDED_

#include "exception.h"

class ExceptionDBIOEOF : public Exception  
{
public:
        inline ExceptionDBIOEOF( ) throw() 
                : Exception( "EOF" )
        {
        }

};

#endif
