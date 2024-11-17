/* $Id: exceptions.h,v 1.4.2.6.18.2 2009/10/11 18:35:35 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: exceptions.h,v 1.4.2.6.18.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__

#include <dlstring.h>

#include "exception.h"

namespace Scripting {

// MOC_SKIP_BEGIN
class Exception : public ::Exception { 
public:
    Exception(DLString s)  throw(): ::Exception(info(s)) { }
    virtual ~Exception( ) throw() ;

    static string info(string s);
};
// MOC_SKIP_END

class NullPointerException : public Exception {
public:
    NullPointerException() throw() : Exception("Null pointer dereference") { }

    virtual ~NullPointerException( )  throw();
};

class NotImplementedException : public Exception {
public:
    NotImplementedException()  throw();
    NotImplementedException(const DLString &msg)  throw();

    virtual ~NotImplementedException( ) throw() ;
};

class NotEnoughArgumentsException : public Exception {
public:
    NotEnoughArgumentsException() throw() : Exception("Not enough arguments") { }
    NotEnoughArgumentsException(const DLString &expected, const DLString & actual) throw();

    virtual ~NotEnoughArgumentsException( ) throw() ;
};

class TooManyArgumentsException : public Exception { 
public:
    TooManyArgumentsException() throw() : Exception("Too many arguments") { }
    TooManyArgumentsException(const DLString &expected, const DLString & actual) throw();

    virtual ~TooManyArgumentsException( ) throw() ;
};

class IllegalArgumentException : public Exception { 
public:
    IllegalArgumentException() throw() : Exception("Illegal argument") { }

    virtual ~IllegalArgumentException( ) throw() ;
};

class MissplacedBreakException : public Exception {
public:
    MissplacedBreakException() throw() : Exception("Break without loop or switch") { }

    virtual ~MissplacedBreakException( ) throw() ;
};

class MissplacedContinueException : public Exception {
public:
    MissplacedContinueException() throw() : Exception("Continue without loop") { }

    virtual ~MissplacedContinueException( ) throw() ;
};

class InvalidIndexingModeException : public Exception {
public:
    InvalidIndexingModeException()  throw(): Exception("Invalid indexing mode") { }

    virtual ~InvalidIndexingModeException( )  throw();
};

class NotAReferenceException : public Exception { 
public:
    NotAReferenceException()  throw(): Exception("Not a reference") { }

    virtual ~NotAReferenceException( )  throw();
};

class WrongNativeThisException : public Exception { 
public:
    WrongNativeThisException()  throw(): Exception("Wrong `this' for native method call") { }

    virtual ~WrongNativeThisException( )  throw();
};

class IdentifierComparitionException : public Exception {
public:
    IdentifierComparitionException()  throw(): Exception("Identifier compared with something else") { }

    virtual ~IdentifierComparitionException( )  throw();
};

class UnknownNativeMethodException : public Exception {
public:
    UnknownNativeMethodException()  throw(): Exception("Method not found") { }

    virtual ~UnknownNativeMethodException( )  throw();
};

class IdentifierExpectedException : public Exception {
public:
    IdentifierExpectedException()  throw(): Exception("Identifier expected") { }

    virtual ~IdentifierExpectedException( )  throw();
};

class FunctionNotDefinedException : public Exception {
public:
    FunctionNotDefinedException()  throw(): Exception("Function with requested id was not defined") { }

    virtual ~FunctionNotDefinedException( )  throw();
};

class ObjectNotDefinedException : public Exception {
public:
    ObjectNotDefinedException()  throw(): Exception("Object with requested id was not defined") { }

    virtual ~ObjectNotDefinedException( )  throw();
};

class InvalidCastException : public Exception {
public:
    InvalidCastException()  throw(): Exception("Invalid cast") { 
    }
    InvalidCastException(DLString exp, DLString found) throw() 
        : Exception( DLString( ) +
                "Invalid cast: expected '" + exp + "' got '" + found + "'") 
    { 
    }

    virtual ~InvalidCastException( )  throw();
};

class ParseError : public Exception {
public:
    ParseError( string msg )  throw(): Exception("Parse error: " + msg) { }

    virtual ~ParseError( )  throw();
};

class CustomException : public Exception {
public:
    CustomException( string msg )  throw(): Exception( msg ), message(msg) { 
    }
    virtual ~CustomException()  throw();

    string message;
};

class IndexOutOfBoundsException : public Exception {
public:
    IndexOutOfBoundsException( )  throw(): Exception( "Index out of bounds" ) { }

    virtual ~IndexOutOfBoundsException( )  throw();
};

class DivisionByZero : public Exception {
public:
    DivisionByZero( )  throw(): Exception( "division by zero" ) { }

    virtual ~DivisionByZero( )  throw();
};

}

#endif /* __EXCEPTIONS_H__ */
