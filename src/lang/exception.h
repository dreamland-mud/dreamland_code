/* $Id: exception.h,v 1.11.2.1.28.3 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

/**
 * @author Igor S. Petrenko
 * @short От этого класса порождаются все исключения
 */
class Exception : public std::exception
{
public:
    Exception( const std::string &  = "" ) throw ();
    virtual ~Exception( ) throw ();
    
    virtual const char* what( ) const throw ();
    inline const string & getMessage() const;
    void setStr( const string& str );
    void printStackTrace( std::ostream &os = std::cerr ) const;

private:
    void fillStackFrames( void * );
    std::vector<void *> callstack;

    string message;
};


inline const string & Exception::getMessage() const
{
    return message;
}

inline ostream& 
operator << ( ostream& ostr, Exception ex )
{
    ostr << ex.what( );
    return ostr;
}

#endif
