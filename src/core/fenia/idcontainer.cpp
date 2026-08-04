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

/**
 * A field key can arrive as a string rather than an identifier: 'props["random
 * weapon"]', 'map[someVar]'. Register::toIdentifier() throws on strings, which
 * used to make every props block keyed by a behavior name containing a space
 * unreachable from Fenia -- the name cannot be written as an identifier either.
 *
 * Pass intern=false for reads, so a lookup driven by runtime data does not grow
 * the Lex table on every miss; true for writes, which must create the name.
 * Returns false only when a non-interning lookup found nothing.
 */
static bool key2id(const Register &key, Lex::id_t &id, bool intern)
{
    if (key.type != Register::STRING) {
        id = key.toIdentifier();
        return true;
    }

    if (intern) {
        id = Lex::getThis()->resolve(key.toString());
        return true;
    }

    return Lex::getThis()->find(key.toString(), id);
}

Register
IdContainer::getField(const Register &key)
{
    Lex::id_t id;
    Traits::Get::Entry *e = Traits::Get::List::lookup(key);

    if(e && e->method) {
        PlugLock plDummy;
        // Matched a native getter by name, so that name is interned already.
        key2id(key, id, true);
        BTPushNative dummy(this, id);

        return (this->*(e->method))( );
    }

    // An unknown name cannot have a field, and reads must not intern.
    if(!key2id(key, id, false))
        return Register();

    Idmap::iterator i = idmap.find(id);

    if(i == idmap.end())
        return Register();
    else
        return i->second;
}

void
IdContainer::setField(const Register &key, const Register &val)
{
    Lex::id_t id;
    Traits::Set::Entry *e = Traits::Set::List::lookup(key);

    if(e && e->method) {
        PlugLock plDummy;
        key2id(key, id, true);
        BTPushNative dummy(this, id);

        (this->*(e->method))( val );
        return;
    }

    if(val.type == Register::NONE) {
        // Deleting a field: nothing to erase if the name was never seen.
        if(!key2id(key, id, false))
            return;

        Idmap::iterator i = idmap.find(id);
        if(i != idmap.end()) {
            idmap.erase(i);
            self->changed();
        }
    } else {
        key2id(key, id, true);
        idmap[id] = val;
        self->changed();
    }
}

Register
IdContainer::callMethod(const Register &key, const RegisterList &args)
{
    Lex::id_t id;
    Traits::Invoke::Entry *e = Traits::Invoke::List::lookup(key);

    if(e && e->method) {
        PlugLock plDummy;
        key2id(key, id, true);
        BTPushNative dummy(this, id);

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
IdContainer::fromXML( const XMLNode::Pointer& parent ) 
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

