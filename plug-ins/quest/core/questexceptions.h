/* $Id: questexceptions.h,v 1.1.4.1.18.1 2007/09/11 00:34:23 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef QUESTEXCEPTIONS_H
#define QUESTEXCEPTIONS_H

#include "exception.h"
#include "dlstring.h"

class QuestCannotStartException : public Exception {
public:
        QuestCannotStartException( );
        QuestCannotStartException( const DLString & );
        virtual ~QuestCannotStartException( ) ;
};

class QuestRuntimeException : public Exception {
public:
        QuestRuntimeException( const DLString & );
        virtual ~QuestRuntimeException( ) ;
};

#endif
