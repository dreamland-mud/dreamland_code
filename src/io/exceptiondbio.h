/* $Id: exceptiondbio.h,v 1.4.34.3 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
// exceptiondbio.h: interface for the ExceptionDBIO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXCEPTIONDBIO_H__CDBC51B3_C088_470A_AA4E_87DE5891C347__INCLUDED_)
#define AFX_EXCEPTIONDBIO_H__CDBC51B3_C088_470A_AA4E_87DE5891C347__INCLUDED_

#include "exception.h"
#include "dlstring.h"

class ExceptionDBIO : public Exception
{
public:
    ExceptionDBIO( const DLString &str ) throw( );

    virtual ~ExceptionDBIO( ) throw( );
};


#endif // !defined(AFX_EXCEPTIONDBIO_H__CDBC51B3_C088_470A_AA4E_87DE5891C347__INCLUDED_)
