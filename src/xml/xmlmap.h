/* $Id: xmlmap.h,v 1.9.2.5.6.3 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 * based on XMLMap by NoFate, 2001
 */
#ifndef XMLMAP_H
#define XMLMAP_H

#include <map>

#include "dlstring.h"
#include "exceptionskipvariable.h"
#include "xmlvariablecontainer.h"
#include "xmlnode.h"
#include "xmlcontainer.h"
#include "xmlpointer.h"

template <typename T>
class XMLMapBase : public std::map<DLString, T>,
	           public virtual XMLContainer 
{
    typedef std::map<DLString, T> Map;

public:
    using Map::end;
    using Map::begin;
    using Map::find;
    using Map::empty;
    
    XMLMapBase( bool flag = true ) : saveEmpty( flag )
    {
    }

    virtual bool nodeFromXML( const XMLNode::Pointer& child ) 
    {
	if (child->getName( ) == XMLNode::ATTRIBUTE_NODE) {
	    const DLString &name = child->getAttribute( XMLNode::ATTRIBUTE_NAME );

	    (*this)[name].fromXML( child );
	    return true;
	}
	
	return false;
    }

    virtual bool toXML( XMLNode::Pointer& parent ) const
    {
	if (empty( ) && !saveEmpty)
	    return false;

	for (typename XMLMapBase::const_iterator ipos = begin( ); ipos != end( ); ipos++ ) {
	    try {
		XMLNode::Pointer child( NEW );
		
		if (ipos->second.toXML( child )) {
                    child->insertAttribute( XMLNode::ATTRIBUTE_NAME, ipos->first );
                    child->setName( XMLNode::ATTRIBUTE_NODE );
		    parent->appendChild( child );
                }
	    }
	    catch (const ExceptionSkipVariable &) {
	    }
	}

	return true;
    }

    bool isAvailable( const DLString &key ) const
    {
	return find( key ) != end( );
    }

protected:
    bool saveEmpty;
};

template <typename T>
class XMLMapContainer : public XMLMapBase<T>, public XMLVariableContainer 
{
public:
    virtual bool nodeFromXML( const XMLNode::Pointer& child )
    {
	return XMLMapBase<T>::nodeFromXML( child )
		|| XMLVariableContainer::nodeFromXML( child );
    }
    virtual bool toXML( XMLNode::Pointer& parent ) const
    {
	bool ret1, ret2;

	ret1 = XMLMapBase<T>::toXML( parent );
	ret2 = XMLVariableContainer::toXML( parent );
	return ret1 || ret2;
    }
};

#endif
