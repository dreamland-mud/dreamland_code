/* $Id: exceptionclassnotfound.h,v 1.1.2.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * ruffina, DreamLand, 2003
 */

/***************************************************************************
                          exceptionclassnotfound.h  -  description
                             -------------------
    begin                : Mon Oct 1 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef EXCEPTIONCLASSNOTFOUND_H
#define EXCEPTIONCLASSNOTFOUND_H

#include "exception.h"

/**
 * @author Igor S. Petrenko
 */
class ExceptionClassNotFound : public Exception
{
public: 
    inline ExceptionClassNotFound( string name )
            : Exception( string( "Class '" ) + name + "' not found" )
    {
    }

    virtual ~ExceptionClassNotFound( ) ;
};

#endif
