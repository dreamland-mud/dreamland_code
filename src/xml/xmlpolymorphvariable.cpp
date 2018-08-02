/* $Id: xmlpolymorphvariable.cpp,v 1.1.2.2.18.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#include "xmlpolymorphvariable.h"

DLObject::Pointer 
XMLPolymorphVariable::set( DLObject::Pointer arg1, DLObject::Pointer arg2 )
{
    return DLObject::Pointer( );
}

ExceptionXMLClassNotDerived::~ExceptionXMLClassNotDerived( ) throw ( )
{
}

ExceptionXMLClassNotRegistered::~ExceptionXMLClassNotRegistered( ) throw ( )
{
}

ExceptionXMLClassAllocate::~ExceptionXMLClassAllocate( ) throw ( )
{
}

