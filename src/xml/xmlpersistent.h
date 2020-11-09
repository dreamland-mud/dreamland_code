/* $Id: xmlpersistent.h,v 1.1.2.4.6.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#ifndef __XMLPERSISTENT_H__
#define __XMLPERSISTENT_H__

#include "xmlpointer.h"
#include "xmlpolymorphvariable.h"

template <typename StubBase>
class XMLStub : public StubBase, public virtual XMLPolymorphVariable {
public:
    typedef ::Pointer<XMLStub> Pointer;
    
    XMLStub( XMLNode::Pointer n ) : node( n )
    {
    }

    virtual bool toXML( XMLNode::Pointer& parent ) const
    {
        parent = node;
        return true;
    }
    virtual void fromXML( const XMLNode::Pointer& parent ) 
    {
        node = parent;
    }
    virtual const DLString &getType( ) const
    {
        return node->getAttribute( XMLNode::ATTRIBUTE_TYPE );
    }

private:        
    XMLNode::Pointer node;
};

template <typename T, typename StubBase = T>
class XMLPersistent : public XMLPointer<T> {
    typedef XMLPointer<T> Ptr;
    typedef XMLStub<StubBase> Stub;
public:
    using Ptr::toXML;
    using Ptr::setPointer;
    
    XMLPersistent() {
    }

    XMLPersistent(::Pointer<T> &p) {
        setPointer(p.getPointer( ));
    }
    XMLPersistent(T *p) {
        setPointer(p);
    }
    
    void fromXML( const XMLNode::Pointer& parent ) 
    {
        try {
            Ptr::fromXML( parent );
        
        } catch (const ExceptionXMLClassAllocate &e) {
            setPointer( new Stub( parent ) );
        }
    }
    inline void backup( )
    {
        XMLNode::Pointer node( NEW );
        
        toXML( node );
        setPointer( new Stub( node ) );
    }

    inline void recover( ) 
    {
        XMLNode::Pointer node( NEW );

        toXML( node );
        fromXML( node );
    }
};

#endif
