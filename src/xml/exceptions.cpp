/* $Id: exceptions.cpp,v 1.1.4.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#include "exceptionskipvariable.h"
#include "exceptionxsl.h"
#include "exceptionxmlerror.h"
#include "exceptionvariablenotfound.h"

#include <sstream>

using namespace std;

ExceptionSkipVariable::~ExceptionSkipVariable( ) 
{
}

ExceptionVariableNotFound::~ExceptionVariableNotFound( ) 
{
}

ExceptionXMLError::~ExceptionXMLError( ) 
{
}

ExceptionXSL::ExceptionXSL( const string type, int position  )
{
    basic_ostringstream<char> ostr;
    
    ostr << type << " at position " << position;
    setStr( ostr.str( ) );
}

ExceptionXSL::ExceptionXSL( char symbol, int position  )
{
    basic_ostringstream<char> ostr;
    
    ostr << " Unknown symbol '" << symbol << "' at at position " << position;
    setStr( ostr.str( ) );
}

ExceptionXSL::ExceptionXSL( const string type, const string symbol, int position  )
{
    basic_ostringstream<char> ostr;
    
    ostr << type << " '" << symbol << "' at at position " << position;
    setStr( ostr.str( ) );
}

ExceptionXSL::~ExceptionXSL( ) 
{
}

