/* $Id: xmlref.h,v 1.1.4.1.6.3 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#ifndef __XMLREF_H__
#define __XMLREF_H__

#include <map>

#include "xmlpolymorphvariable.h"

using namespace std;

class XMLRefVariable;

class XMLRefBase {
protected:
    typedef long long refid_t;
    typedef map<refid_t, XMLRefVariable*> refmap_t;

    static refid_t nextReferenceId();

    static const DLString ATTRIBUTE_REFID;
    static refmap_t refmap;

private:
    static refid_t lastRefId;
};

class XMLRefVariable : public virtual XMLPolymorphVariable, XMLRefBase
{
    template <typename T> friend class XMLRefPointer;
    template <typename T> friend class XMLReference;
public:
    typedef ::Pointer<XMLRefVariable> Pointer;
    
    XMLRefVariable();
    virtual ~XMLRefVariable();

protected:
    refid_t getReferenceId() const;
    void setReferenceId(refid_t);

private:
    refid_t refid;
};

template <typename T>
class XMLReference : public Pointer<T>, XMLRefBase
{
    typedef ::Pointer<T> Ptr;

public:
    using Ptr::clear;
    using Ptr::isEmpty;
    using Ptr::getPointer;
    using Ptr::setPointer;

    XMLReference() : Ptr() { }
    XMLReference(const Ptr &p) : Ptr(p) { }
    
    bool toXML( XMLNode::Pointer& parent ) const
    {
	parent->setType( XMLNode::XML_LEAF );
	if(!isEmpty())
	    parent->insertAttribute( ATTRIBUTE_REFID, 
		    getPointer( )->getReferenceId( ) );
	return true;
    }
    void fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType )
    {
	const DLString &sid = parent->getAttribute( ATTRIBUTE_REFID );

	if(sid.empty( )) {
	    clear();
	    return;
	}

	refmap_t::iterator it;
	it = refmap.find(sid.toLongLong( ));

	if(it == refmap.end( ))
	    throw ExceptionBadType("id not assigned yet", sid);

	setPointer( dynamic_cast<T *>(it->second) );
    }

    template <typename V>
    const XMLReference &operator = (V &v) {
	setPointer( v.getPointer( ) );
	return *this;
    }
    template <typename V>
    const XMLReference &operator = (V *v) {
	setPointer( v );
	return *this;
    }
};

template <typename T>
class XMLRefPointer : public XMLPolymorphPointer<T>, XMLRefBase
{
    typedef XMLPolymorphPointer<T> Ptr;
    
public:
    using Ptr::clear;
    using Ptr::isEmpty;
    using Ptr::getPointer;
    using Ptr::setPointer;

    XMLRefPointer() : Ptr() { }
    XMLRefPointer(const Ptr &p) : Ptr(p) { }

    bool toXML( XMLNode::Pointer& parent ) const
    {
	if (Ptr::toXML(parent)) {
	    if(!isEmpty())
		parent->insertAttribute( ATTRIBUTE_REFID, 
			getPointer( )->getReferenceId( ) );
	    return true;
	}
	else
	    return false;
    }
    
    void fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType )
    {
	Ptr::fromXML(parent);

	if(isEmpty( ))
	    return;
	
	const DLString &sid = parent->getAttribute( ATTRIBUTE_REFID );
	
	if(sid.empty( ))
	    return;
	
	getPointer( )->setReferenceId(sid.toLongLong( ));
    }
};

#endif
