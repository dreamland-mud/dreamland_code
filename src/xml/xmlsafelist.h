/* $Id: xmlsafelist.h,v 1.1.4.3 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#ifndef __XMLSAFELIST__
#define __XMLSAFELIST__

#include "safelist.h"
#include "exceptionskipvariable.h"
#include "xmlcontainer.h"
#include "xmlpointer.h"

template <typename ElementType, typename ListType>
class XMLSafeList : public SafeList<ElementType, ListType>, 
                    public virtual XMLContainer
{
    typedef SafeList<ElementType, ListType> List;
public:
    using List::begin;
    using List::end;
    using List::push_front;
    using List::empty;

    virtual bool toXML( XMLNode::Pointer& parent ) const
    {
	if (empty( ))
	    return false;

	for (typename XMLSafeList::const_iterator it = begin( ); it != end( ); ++it) {
	    XMLNode::Pointer child( NEW );

	    try {
		if (it->toXML( child )) {
                    child->insertAttribute( XMLNode::ATTRIBUTE_TYPE, it->getType( ) );
                    child->setName( XMLNode::ATTRIBUTE_NODE );
		    parent->appendChild( child );
                }
	    }
	    catch (const ExceptionSkipVariable &) {
	    }
	}

	return true;
    }

    virtual bool nodeFromXML( const XMLNode::Pointer &child )
    {
	if (child->getName( ) == XMLNode::ATTRIBUTE_NODE) {
	    const DLString & type = child->getAttribute( XMLNode::ATTRIBUTE_TYPE );
	    AllocateClass::Pointer ptr = Class::allocateClass( type );
	    ElementType *element = dynamic_cast<ElementType *>( ptr.getPointer( ) );
	    
	    ptr.strip( );
	    element->fromXML( child );
	    push_front( element );
	    return true;
	}

	return false;
    }

};

#endif
