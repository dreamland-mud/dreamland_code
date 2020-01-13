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
    
Exception::~Exception( ) throw() 
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

CustomException::~CustomException() throw() 
{
}

NullPointerException::~NullPointerException( ) throw() 
{
}

NotImplementedException::~NotImplementedException( ) throw() 
{
}

NotEnoughArgumentsException::~NotEnoughArgumentsException( ) throw() 
{
}

TooManyArgumentsException::~TooManyArgumentsException( ) throw() 
{
}

IllegalArgumentException::~IllegalArgumentException( ) throw() 
{
}

MissplacedBreakException::~MissplacedBreakException( ) throw() 
{
}

MissplacedContinueException::~MissplacedContinueException( ) throw() 
{
}

InvalidIndexingModeException::~InvalidIndexingModeException( ) throw() 
{
}

NotAReferenceException::~NotAReferenceException( ) throw() 
{
}

WrongNativeThisException::~WrongNativeThisException( ) throw() 
{
}

IdentifierComparitionException::~IdentifierComparitionException( ) throw() 
{
}

UnknownNativeMethodException::~UnknownNativeMethodException( ) throw() 
{
}

IdentifierExpectedException::~IdentifierExpectedException( ) throw() 
{
}

FunctionNotDefinedException::~FunctionNotDefinedException( ) throw() 
{
}

ObjectNotDefinedException::~ObjectNotDefinedException( ) throw() 
{
}

InvalidCastException::~InvalidCastException( ) throw() 
{
}

ParseError::~ParseError( ) throw() 
{
}

IndexOutOfBoundsException::~IndexOutOfBoundsException( ) throw() 
{
}

DivisionByZero::~DivisionByZero( ) throw() 
{
}

}

