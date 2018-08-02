/* $Id: class.cpp,v 1.1.2.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * ruffina, DreamLand, 2003
 */
/***************************************************************************
                          class.cpp  -  description
                             -------------------
    begin                : Mon Oct 1 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include <map>

#include "dlstring.h"
#include "class.h"
#include "logstream.h"

struct ClassMap : public map<DLString, Class::ClassRegistrator::Pointer> 
{
    void reg( const DLString& name, Class::ClassRegistrator::Pointer classPointer )
    {
	(*this)[name] = classPointer;
    }

    void unreg( const DLString& name )
    {
	iterator pos = find( name );

	if (pos != end( ))
	    erase( pos );
    }

    AllocateClass::Pointer allocate( const DLString& name ) throw( ExceptionClassNotFound )
    {
	if (name.empty( ))
	    throw Exception( "Attempt to allocate a class with empty name!" );

	iterator pos = find( name );

	if (pos != end( ))
	    return pos->second->clone( );
	
	LogStream::sendError( ) << name << " not found for allocate!" << endl;
	throw ExceptionClassNotFound( name );
    }

    AllocateClass * allocateRaw( const DLString& name ) throw( ExceptionClassNotFound )
    {
	if (name.empty( ))
	    throw Exception( "Attempt to allocate a class with empty name!" );

	iterator pos = find( name );

	if (pos != end( ))
	    return pos->second->cloneRaw( );
	
	LogStream::sendError( ) << name << " not found for 'raw' allocate!" << endl;
	throw ExceptionClassNotFound( name );
    }

};

static ClassMap classMap;

void Class::regClass( const DLString& name, ClassRegistrator::Pointer classPointer )
{
    classMap.reg( name, classPointer );
}

void Class::unRegClass( const DLString& name )
{
    classMap.unreg( name );
}

AllocateClass::Pointer Class::allocateClass( const DLString& name ) throw( ExceptionClassNotFound )
{
    return classMap.allocate( name );
}

AllocateClass * Class::allocateClassRaw( const DLString& name ) throw( ExceptionClassNotFound )
{
    return classMap.allocateRaw( name );
}

Class::ClassRegistrator::~ClassRegistrator( )
{
}

