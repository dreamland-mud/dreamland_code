/* $Id: exceptions.cpp,v 1.1.2.2.18.2 2009/10/11 18:35:35 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: exceptions.cpp,v 1.1.2.2.18.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#include <sstream>
#include <typeinfo>

#include "exceptions.h"
#include "nodes.h"
#include "context.h"

using namespace std;

namespace Scripting {
    
Exception::~Exception( ) throw() 
{
}

// Same text as the tail of info(): prefix, then where it happened, then the
// backtrace. The prefix doubles as reverse()'s continuation line, as before.
void
Exception::where(ostream &out, const string &prefix)
{
    NodeTrace *nt = Context::current ? Context::current->nodeTrace : 0;
    ostringstream buf;

    buf << prefix << "    in ";
    
    if(nt) {
        buf << nt->node->source << ": ";
        nt->node->reverse(buf, buf.str());
    } else
        buf << "native code";

    buf << endl;
    if (Context::current)
        BackTrace::report(buf);

    out << buf.str();
}

string 
Exception::info(string s) 
{
    ostringstream buf;
    where(buf, "Runtime exception " + s + "\n");
    return buf.str( );
}

// The last native exception seen unwinding through a node, kept as copies.
// Never hold the object itself: its destructor may live in a plugin that
// `plug reload` unloads before the next exception would release it.
static const void *nativeAddr = 0;
static string nativeType;
static string nativeMessage;
static string nativeLocation;

static bool nativeMatches(const ::Exception &e)
{
    return &e == nativeAddr
        && nativeType == typeid(e).name()
        && nativeMessage == e.getMessage();
}

void
Exception::recordNative(const ::Exception &e)
{
    // Unwinding through outer nodes: the innermost one already recorded it.
    if (nativeMatches(e))
        return;

    // Runs inside a catch handler: anything thrown here would replace the
    // exception being reported.
    try {
        ostringstream buf;
        where(buf, "");
        nativeLocation = buf.str();
        nativeType = typeid(e).name();
        nativeMessage = e.getMessage();
        nativeAddr = &e;
    } catch (...) {
        nativeAddr = 0;
    }
}

void
Exception::forgetNative()
{
    nativeAddr = 0;
}

string
Exception::nativeWhere(const ::Exception &e)
{
    if (nativeMatches(e))
        return nativeLocation;

    return string();
}

CustomException::~CustomException() throw() 
{
}

NullPointerException::~NullPointerException( ) throw() 
{
}

NotImplementedException::NotImplementedException()  throw()
    : Exception("Operation not implemented") 
{ 
}

NotImplementedException::NotImplementedException(const DLString &msg)  throw()
    : Exception(msg) 
{ 
}

NotImplementedException::~NotImplementedException( ) throw() 
{
}

NotEnoughArgumentsException::NotEnoughArgumentsException(const DLString &expected, const DLString & actual) throw()
           : Exception("Not enough arguments: expected " + expected + ", but got only " + actual)
{
}

NotEnoughArgumentsException::~NotEnoughArgumentsException( ) throw() 
{
}

TooManyArgumentsException::TooManyArgumentsException(const DLString &expected, const DLString & actual) throw()
           : Exception("Too many arguments: expected only " + expected + ", but got " + actual)
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

