/* $Id: xmllist.h,v 1.1.2.2.18.3 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2005
 */
#ifndef XMLLIST_H
#define XMLLIST_H

#include <list>
#include "xmlvariablecontainer.h"
#include "xmlcontainer.h"
#include "xmlpointer.h"
#include "exceptionskipvariable.h"

template <typename T>
class XMLListBase : public std::list<T>, public virtual XMLContainer {
    typedef std::list<T> List;

public:
    using List::clear;
    using List::back;
    using List::end;
    using List::begin;
    using List::empty;
    using List::insert;
    
    XMLListBase( bool flag = false ) : saveEmpty( flag )
    {
    }

    virtual void fromXML( const XMLNode::Pointer& node ) throw( ExceptionBadType )
    {
	clear( );
	XMLContainer::fromXML( node );
    }
    
    virtual bool nodeFromXML( const XMLNode::Pointer& child ) 
    {
	if (child->getName( ) == XMLNode::ATTRIBUTE_NODE) {
	    insert( end( ), T( ) );		
	    back( ).fromXML( child );
	    return true;
	}

	return false;
    }

    virtual bool toXML( XMLNode::Pointer& parent ) const
    {
	if (empty( ) && !saveEmpty)
	    return false;

        for (typename XMLListBase::const_iterator ipos = begin( ); ipos != end( ); ipos++) {
	    try {
		XMLNode::Pointer child( NEW );
		
		if (ipos->toXML( child )) {
                    child->setName( XMLNode::ATTRIBUTE_NODE );
		    parent->appendChild( child );
                }
	    }
	    catch (const ExceptionSkipVariable &) {
	    }
        }

	return true;
    }

protected:
    bool saveEmpty;
};


template <typename T>
class XMLListContainer : public XMLListBase<T>, 
                         public XMLVariableContainer 
{
public:
    virtual bool nodeFromXML( const XMLNode::Pointer& child )
    {
	return XMLListBase<T>::nodeFromXML( child )
		|| XMLVariableContainer::nodeFromXML( child );
    }
    virtual bool toXML( XMLNode::Pointer& parent ) const
    {
	bool ret1, ret2;

	ret1 = XMLListBase<T>::toXML( parent );
	ret2 = XMLVariableContainer::toXML( parent );
	return ret1 || ret2;
    }
};

#endif
