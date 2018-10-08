/* $Id: regcontainer.cpp,v 1.1.2.3.6.2 2009/11/04 03:24:31 rufina Exp $
 *
 * ruffina, 2004
 */

#include "regcontainer.h"
#include "register-impl.h"
#include "schedulerwrapper.h"

using namespace Scripting;

const DLString RegContainer::TYPE = "RegContainer";
    
NMI_INIT(RegContainer, "массив")

Register 
RegContainer::getField(const Register &key)
{
    if(key.type == Register::IDENTIFIER) {
        Traits::Get::Entry *e = Traits::Get::List::lookup(key);
        
        if(e && e->method) {
            PlugLock plDummy;
            BTPushNative dummy(this, key.toIdentifier());
            
            return (this->*(e->method))( );
        }
    }
        
    Map::iterator i = map.find(key);
    
    if(i == map.end())
        return Register();
    else
        return i->second;
}

void 
RegContainer::setField(const Register &key, const Register &val)
{
    if(key.type == Register::IDENTIFIER) {
        Traits::Set::Entry *e = Traits::Set::List::lookup(key);
        
        if(e && e->method) {
            PlugLock plDummy;
            BTPushNative dummy(this, key.toIdentifier());
            
            (this->*(e->method))( val );
            return;
        }
    }
        
    if(val.type == Register::NONE) {
        Map::iterator i = map.find(key);
        if(i != map.end()) {
            map.erase(i);
            self->changed();
        }
    } else {
        map[key] = val;
        self->changed();
    }
}

Register 
RegContainer::callMethod(const Register &key, const RegisterList &args)
{
    if(key.type == Register::IDENTIFIER) {
        Traits::Invoke::Entry *e = Traits::Invoke::List::lookup(key);
        
        if(e && e->method) {
            PlugLock plDummy;
            BTPushNative dummy(this, key.toIdentifier());
            
            return (this->*(e->method))( args );
        }
    }

    return getField(key).toFunction()->invoke(Register(self), args);
}

bool
RegContainer::toXML( XMLNode::Pointer& parent ) const
{
    Map::const_iterator i;
    RegRecord rr;
    
    for(i = map.begin(); i != map.end(); i++) {
        XMLNode::Pointer node(NEW);
        
        rr.key = i->first;
        rr.value = i->second;
        
        rr.toXML(node);
        node->setName(XMLNode::ATTRIBUTE_NODE);
        
        parent->appendChild(node);
    }

    return true;
}

void 
RegContainer::fromXML( const XMLNode::Pointer& parent ) throw ( ExceptionBadType )
{
    map.clear();
    XMLContainer::fromXML(parent);
}

bool
RegContainer::nodeFromXML( const XMLNode::Pointer& child)
{
    const DLString &name = child->getName();
    RegRecord rr;

    if(name != "node")
        throw ExceptionBadType("node", name);
    
    rr.fromXML(child);

    map[rr.key] = rr.value;

    return true;
}

