/* $Id$
 *
 * ruffina, 2004
 */
#include "questexceptions.h"
#include "logstream.h"

QuestCannotStartException::QuestCannotStartException( const DLString &msg ) : Exception( msg )
{        
    LogStream::sendWarning( ) << "Quest: " << msg << endl;
}

QuestCannotStartException::QuestCannotStartException( )
{
}

QuestCannotStartException::~QuestCannotStartException( ) throw( ) 
{
}

        
QuestRuntimeException::QuestRuntimeException( const DLString & msg ) : Exception( msg )
{        
    LogStream::sendError( ) << "Quest: " << msg << endl;
}

QuestRuntimeException::~QuestRuntimeException( ) throw( )
{
}

