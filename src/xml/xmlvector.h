/* $Id: xmlvector.h,v 1.13.2.4.18.5 2009/10/11 18:35:39 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 * based on XMLVector by NoFate, 2001
 */

#ifndef XMLVECTOR_H
#define XMLVECTOR_H

#include <vector>
#include "xmlvariablecontainer.h"
#include "xmlcontainer.h"
#include "xmlpointer.h"
#include "exceptionskipvariable.h"

template <typename T>
class XMLVectorBase : public std::vector<T>, public virtual XMLContainer {
    typedef std::vector<T> Vector;

public:
    using Vector::clear;
    using Vector::back;
    using Vector::end;
    using Vector::begin;
    using Vector::resize;
    using Vector::empty;
    using Vector::insert;
    
    XMLVectorBase( bool flag = false, int size = 0 ) 
                     : saveEmpty( flag ), defaultSize( size )
    {
        if (defaultSize != 0)
            resize( defaultSize );
    }

    virtual void fromXML( const XMLNode::Pointer& node ) throw( ExceptionBadType )
    {
        clear( );
        
        if (defaultSize != 0)
            resize( defaultSize );

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

        for (typename XMLVectorBase::const_iterator ipos = begin( ); ipos != end( ); ipos++) {
            try {
                XMLNode::Pointer child( NEW );
                
                if (nodeToXML( *ipos, child ))
                    parent->appendChild( child );
            }
            catch (const ExceptionSkipVariable &) {
            }
        }

        return true;
    }

    virtual bool nodeToXML( const T &element, XMLNode::Pointer &child ) const
    {
        if(element.toXML(child)) {
            child->setName( XMLNode::ATTRIBUTE_NODE );
            return true;
        }
        return false;
    }

protected:
    bool saveEmpty;
    int defaultSize;
};

template <typename T>
class XMLVectorContainer : public XMLVectorBase<T>, 
                           public XMLVariableContainer 
{
public:
    virtual bool nodeFromXML( const XMLNode::Pointer& child )
    {
        return XMLVectorBase<T>::nodeFromXML( child )
                || XMLVariableContainer::nodeFromXML( child );
    }
    virtual bool toXML( XMLNode::Pointer& parent ) const
    {
        bool ret1, ret2;

        ret1 = XMLVectorBase<T>::toXML( parent );
        ret2 = XMLVariableContainer::toXML( parent );
        return ret1 || ret2;
    }
};

#endif
