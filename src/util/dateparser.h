/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          dateparser.h  -  description
                             -------------------
    begin                : Thu Sep 27 2001
    copyright            : (C) 2001 by nofate
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef DATEPARSER_H
#define DATEPARSER_H

#include <sstream>

#include "dlstring.h"
#include "dlobject.h"
#include "exceptionbaddatestring.h"

/**
 * @author Igor S. Petrenko
 * @short Чтение времени с помощью flex
 */
class DateParser : public yyFlexLexer, public virtual DLObject
{
public: 
        DateParser( const DLString& date, istream * );
        int dateLex( ) ;
        inline int getSecond( ) 
        {
                dateLex( );
                return second * modifier;
        }
        
private:
        char tokenStatus;
        DLString date;
        int second;
        int modifier;
        int position;
        int lastNumber;
        std::basic_ostringstream<char> ostr;
        std::basic_istringstream<char> istr;
};


#endif
