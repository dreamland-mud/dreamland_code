/* $Id: xmlpolymorphvariable.h,v 1.1.2.4.6.5 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#ifndef __XMLPOLYMORPHVARIABLE_H__
#define __XMLPOLYMORPHVARIABLE_H__

#include <typeinfo>

#include "allocateclass.h"
#include "xmlvariable.h"

#include "pointer.h"
#include "class.h"
#include "exceptionclassnotfound.h"

class XMLPolymorphVariable : public AllocateClass, public virtual XMLVariable {
public:

    virtual const DLString & getType( ) const = 0;
    virtual DLObject::Pointer set( DLObject::Pointer arg1, DLObject::Pointer arg2 );
};

class ExceptionXMLClassAllocate : public ExceptionBadType {
public:
    ExceptionXMLClassAllocate( const DLString &msg )
            : ExceptionBadType( "Polymorph pointer exception: " + msg )
    {
    }

    virtual ~ExceptionXMLClassAllocate( ) throw ( );
};

class ExceptionXMLClassNotRegistered : public ExceptionXMLClassAllocate {
public:
    ExceptionXMLClassNotRegistered( const DLString &myType, const DLString &type ) 
            : ExceptionXMLClassAllocate( "while allocating " + myType + ": class " + type + " not registered" ) 
    { 
    }

    virtual ~ExceptionXMLClassNotRegistered( ) throw ( );
};

class ExceptionXMLClassNotDerived : public ExceptionXMLClassAllocate {
public:
    ExceptionXMLClassNotDerived( const DLString &myType, const DLString &type ) 
            : ExceptionXMLClassAllocate( "class " + type + " not derived from " + myType ) 
    { 
    }

    virtual ~ExceptionXMLClassNotDerived( ) throw ( );
};


template <typename T>
class XMLPolymorphPointer : public Pointer<T> {
    typedef ::Pointer<T> Ptr;

public:
    using Ptr::clear;
    using Ptr::isEmpty;
    using Ptr::getPointer;

    XMLPolymorphPointer() : Ptr() { }
    XMLPolymorphPointer(const Ptr &p) : Ptr(p) { }
    
    bool toXML( XMLNode::Pointer& parent ) const
    {
        if (isEmpty( )) {
            parent->setType( XMLNode::XML_LEAF );
            return true;
        }
        else if(getPointer( )->toXML(parent)) {
            parent->insertAttribute( XMLNode::ATTRIBUTE_TYPE, getPointer( )->getType( ) );
            return true;
        } 
        return false;
    }
    
    void fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType )
    {
        if (parent->getType( ) == XMLNode::XML_LEAF) {
            clear( );
        }
        else {
            const DLString & type = parent->getAttribute( XMLNode::ATTRIBUTE_TYPE );

            try {
                AllocateClass::Pointer p = Class::allocateClass( type );
                        
                this->setPointer( dynamic_cast<T *>( p.getPointer( ) ) );

                if (isEmpty( ))
                    throw ExceptionXMLClassNotDerived( typeid( this ).name( ), type );
                        
                getPointer( )->fromXML( parent );
            
            } catch (const ExceptionClassNotFound &e) {
                throw ExceptionXMLClassNotRegistered( typeid( this ).name( ), type );
            }
        }
    }
};

#endif
