/* $Id: exceptionbadtype.h,v 1.12.2.1.28.3 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#ifndef EXCEPTIONBADTYPE_H
#define EXCEPTIONBADTYPE_H

#include "exception.h"
#include <string>

/**
 * @author Igor S. Petrenko
 * @short Возникает при попытке присвоить XML переменной значение
 * с недопустимым типом
 * @see XMLObject XMLVariable
 */
class ExceptionBadType : public Exception
{
public:
    inline ExceptionBadType( string str ) 
            : Exception( str )
    {
    }
    
    inline ExceptionBadType( string parentType, string nodeType ) 
            : Exception( "Unparsed node <" + parentType + "> <" + nodeType +">" )
    {
    }

    virtual ~ExceptionBadType( ) ;
};

#endif
