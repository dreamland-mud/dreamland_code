/* $Id: exceptions.cpp,v 1.1.2.2.18.2 2009/10/11 18:35:35 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: exceptions.cpp,v 1.1.2.2.18.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#include <sstream>

#include "exceptions.h"
#include "nodes.h"
#include "context.h"

using namespace std;

namespace Scripting {
    
Exception::~Exception( ) 
{
}

string 
Exception::info(string s) 
{
    NodeTrace *nt = Context::current->nodeTrace;
    ostringstream buf;

    buf << "Runtime exception " << s << endl
        << "    in ";
    
    if(nt) {
        buf << nt->node->source << ": ";
        nt->node->reverse(buf, buf.str());
    } else
        buf << "native code";

    buf << endl;
    BackTrace::report(buf);

    return buf.str( );
}

CustomException::~CustomException() 
{
}

NullPointerException::~NullPointerException( ) 
{
}

NotImplementedException::~NotImplementedException( ) 
{
}

NotEnoughArgumentsException::~NotEnoughArgumentsException( ) 
{
}

TooManyArgumentsException::~TooManyArgumentsException( ) 
{
}

IllegalArgumentException::~IllegalArgumentException( ) 
{
}

MissplacedBreakException::~MissplacedBreakException( ) 
{
}

MissplacedContinueException::~MissplacedContinueException( ) 
{
}

InvalidIndexingModeException::~InvalidIndexingModeException( ) 
{
}

NotAReferenceException::~NotAReferenceException( ) 
{
}

WrongNativeThisException::~WrongNativeThisException( ) 
{
}

IdentifierComparitionException::~IdentifierComparitionException( ) 
{
}

UnknownNativeMethodException::~UnknownNativeMethodException( ) 
{
}

IdentifierExpectedException::~IdentifierExpectedException( ) 
{
}

FunctionNotDefinedException::~FunctionNotDefinedException( ) 
{
}

ObjectNotDefinedException::~ObjectNotDefinedException( ) 
{
}

InvalidCastException::~InvalidCastException( ) 
{
}

ParseError::~ParseError( ) 
{
}

IndexOutOfBoundsException::~IndexOutOfBoundsException( ) 
{
}

DivisionByZero::~DivisionByZero( ) 
{
}

}

