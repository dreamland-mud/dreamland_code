/* $Id: globalreference.h,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * ruffina, Dream Land, 2005
 */
#ifndef __GLOBALREFERENCE_H__
#define __GLOBALREFERENCE_H__

#include "globalregistry.h"

class GlobalReferenceBase {
public:
    virtual ~GlobalReferenceBase( );

    inline operator const int & ( )
    {
	resolve( );	
	return index;
    }
    
    void setName( const DLString &nome )
    {
	init( nome );
	resolve( );
    }

    const DLString & getName( ) const
    {
	return name;
    }

protected:
    GlobalReferenceBase( );
    
    inline void init( )
    {
	init( "none" );
    }
    inline void init( const DLString &nome )
    {
	init( nome.c_str( ) );
    }
    inline void init( const char *nome )
    {
	name = nome;
	index = -1;
    }
    
    virtual void resolve( ) = 0;

    DLString name;
    int index;
};

/*
 * Reference on an element of global registry
 */
template <typename Registry, typename Element>
class GlobalReference : public virtual GlobalReferenceBase {
public:
    typedef GlobalReference Reference;
    
    inline Element * operator -> ( )
    {
	resolve( );
	return Registry::getThis( )->find( index );
    }
    inline Element & operator * ( )
    {
	resolve( );
	return *Registry::getThis( )->find( index );
    }

    void assign( const Reference &ref )
    {
	setName( ref.name );
    }
    void assign( const Element &elem )
    {
	setName( elem.getName( ) );
    }
    void assign( const Element *elem )
    {
	setName( elem->getName( ) );
    }
    void assign( int index )
    {
	if (Registry::getThis( )->goodIndex( index )) {
	    this->index = index;
	    name = Registry::getThis( )->find( index )->getName( );
	}
	else
	    init( );
    }

    inline bool operator == ( const Element &elem )
    {
	resolve( );
	return index == elem.getIndex( );
    }
    inline bool operator == ( const Element *elem )
    {
	return operator == ( *elem );
    }
    inline bool operator != ( const Element *elem )
    {
	return ! operator == ( elem );
    }
    inline bool operator != ( const Element &elem )
    {
	return ! operator == ( elem );
    }
    
protected:
    GlobalReference( ) 
    {
    }

    virtual void resolve( )
    {
	if (index == -1)
	    index = Registry::getThis( )->lookup( name );
    }
};

#define GLOBALREF_DECL(Type) \
    struct Type##Reference : public GlobalReference<Type##Manager,Type> { \
	typedef GlobalReference<Type##Manager, Type> Base; \
	Type##Reference( const DLString & ); \
	Type##Reference( const char * ); \
	virtual ~Type##Reference( ); \
    protected: \
	Type##Reference( ); \
    };

#define GLOBALREF_IMPL(Type, nounderscore) \
    Type##Reference::Type##Reference( const DLString &nome ) { \
	init( nome ); \
	name.substitute( '_', nounderscore); \
    } \
    Type##Reference::Type##Reference( const char *nome ) { \
	init( nome ); \
	name.substitute( '_', nounderscore); \
    } \
    Type##Reference::Type##Reference( ) { } \
    Type##Reference::~Type##Reference( ) { }

#endif
