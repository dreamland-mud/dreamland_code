/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          exceptionxsl.h  -  description
                             -------------------
    begin                : Mon Oct 15 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef EXCEPTIONXSL_H
#define EXCEPTIONXSL_H

#include "exception.h"

/**
 * @short Для генерации ощибок при разборе patterns
 * @author Igor S. Petrenko
 */
class ExceptionXSL : public Exception
{
public: 
    /**
     * @arg type - сообщение о ошибке
     * @arg position - в какой позиции
     */
    ExceptionXSL( const string type, int position  );
    
    /**
     * @arg type - сообщение о ошибке
     * @arg symbol - неизвестная строка
     * @arg position - в какой позиции
     */
    ExceptionXSL( const string type, const string  symbol, int position  );
    
    /**
     * @arg symbol - неизвестный символ
     * @arg position - в какой позиции
     */
    ExceptionXSL( char symbol, int position  );

    virtual ~ExceptionXSL( ) throw( );
};

#endif
