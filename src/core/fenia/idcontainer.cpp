/* $Id: idcontainer.cpp,v 1.1.2.4.6.2 2009/11/04 03:24:31 rufina Exp $
 *
 * ruffina, 2004
 */

#include "idcontainer.h"
#include "register-impl.h"
#include "schedulerwrapper.h"

using namespace Scripting;

const DLString IdContainer::TYPE = "IdContainer";
    
NMI_INIT(IdContainer, "структура")

Register 
IdContainer::getField(const Register &key)
{
    Traits::Get::Entry *e = Traits::Get::List::lookup(key);
    
    if(e && e->method) {
	PlugLock plDummy;
	BTPushNative dummy(this, key.toIdentifier());
	
	return (this->*(e->method))( );
    }
	
    Lex::id_t id = key.toIdentifier();
    Idmap::iterator i = idmap.find(id);
    
    if(i == idmap.end())
	return Register();
    else
	return i->second;
}

void 
IdContainer::setField(const Register &key, const Register &val)
{
    Traits::Set::Entry *e = Traits::Set::List::lookup(key);
    
    if(e && e->method) {
	PlugLock plDummy;
	BTPushNative dummy(this, key.toIdentifier());
	
	(this->*(e->method))( val );
	return;
    }
	
    Lex::id_t id = key.toIdentifier();
    
    if(val.type == Register::NONE) {
	Idmap::iterator i = idmap.find(id);
	if(i != idmap.end()) {
	    idmap.erase(i);
	    self->changed();
	}
    } else {
	idmap[id] = val;
	self->changed();
    }
}

Register 
IdContainer::callMethod(const Register &key, const RegisterList &args)
{
    Traits::Invoke::Entry *e = Traits::Invoke::List::lookup(key);
    
    if(e && e->method) {
	PlugLock plDummy;
	BTPushNative dummy(this, key.toIdentifier());
	
	return (this->*(e->method))( args );
    }

    return getField(key).toFunction()->invoke(Register(self), args);
}

bool
IdContainer::toXML( XMLNode::Pointer& parent ) const
{
    Idmap::const_iterator i;
    
    for(i = idmap.begin(); i != idmap.end(); i++) {
	XMLNode::Pointer node(NEW);
	
	i->second.toXML(node);
	node->setName(Lex::getThis()->getName(i->first));
	
	parent->appendChild(node);
    }

    return true;
}

void 
IdContainer::fromXML( const XMLNode::Pointer& parent ) throw ( ExceptionBadType )
{
    idmap.clear();
    XMLContainer::fromXML(parent);
}

bool
IdContainer::nodeFromXML( const XMLNode::Pointer& child)
{
    const DLString &name = child->getName();
    Lex::id_t id = Lex::getThis()->resolve(name);
    
    idmap[id].fromXML(child);
    return true;
}

