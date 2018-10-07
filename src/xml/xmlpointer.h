/* $Id: xmlpointer.h,v 1.1.2.4.6.3 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#ifndef __XMLPOINTER_H__
#define __XMLPOINTER_H__

#include "xmlref.h"

// MOC_SKIP_BEGIN
#include "algo.h"
 
template <typename T>
struct non_polymorphic_type;

template <typename T>
class XMLPointer : public Choice<Conv<T, XMLPolymorphVariable>::exists, 
			    Choice<Conv<T, XMLRefVariable>::exists,
				XMLRefPointer<T>, XMLPolymorphPointer<T> >,
				    non_polymorphic_type<T> >
{
    typedef Pointer<T> Ptr;
public:
    using Pointer<T>::isEmpty;

    XMLPointer() { }
    
    template <typename T0>
    XMLPointer(T0 t0) {
	setPointer(Ptr(t0));
    }
    template <typename T0, typename T1>
    XMLPointer(T0 t0, T1 t1) {
	setPointer(Ptr(t0, t1));
    }
    template <typename T0, typename T1, typename T2>
    XMLPointer(T0 t0, T1 t1, T2 t2) {
	setPointer(Ptr(t0, t1, t2));
    }
    template <typename T0, typename T1, typename T2, typename T3>
    XMLPointer(T0 t0, T1 t1, T2 t2, T3 t3) {
	setPointer(Ptr(t0, t1, t2, t3));
    }
    template <typename T0, typename T1, typename T2, typename T3, typename T4>
    XMLPointer(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4) {
	setPointer(Ptr(t0, t1, t2, t3, t4));
    }

    template <typename NoT>
    void setPointer(NoT *p) {
	Ptr::setPointer(p);
    }
    template <typename NoT>
    void setPointer(NoT p) {
	Ptr::setPointer(p.getPointer( ));
    }
	
    template <typename V>
    const XMLPointer &operator = (V &v) {
	setPointer( v.getPointer( ) );
	return *this;
    }
    template <typename V>
    const XMLPointer &operator = (V *v) {
	setPointer( v );
	return *this;
    }
};

// MOC_SKIP_END

typedef XMLPointer<XMLPolymorphVariable> XMLPolymorphVariableXMLPointer;

template <typename T>
class XMLPointerNoEmpty : public XMLPointer<T> {
public:    
    typedef XMLPointer<T> Base;
    using Base::toXML;
    using Base::isEmpty;

    bool toXML( XMLNode::Pointer& parent ) const
    {
	if (isEmpty( ))
	    return false;
	else
	    return Base::toXML( parent );
    }
};


#endif
