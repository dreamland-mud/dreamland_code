/* $Id: guts.cpp,v 1.1.2.1 2009/11/04 03:24:31 rufina Exp $
 *
 * ruffina, 2004
 */
#include "guts.h"

#include "register-impl.h"
#include "xmlregister.h"

bool 
Guts::toXML( XMLNode::Pointer& parent ) const
{
    const_iterator i;
    
    for(i = begin(); i != end(); i++) {
        XMLNode::Pointer node(NEW);
        
        node->setName(Lex::getThis()->getName(i->first));

        if (i->second.toXML(node))
            parent->appendChild(node);
    }

    return true;
}

void 
Guts::fromXML( const XMLNode::Pointer& parent ) throw ( ExceptionBadType )
{
    clear();
    XMLContainer::fromXML(parent);
}

bool
Guts::nodeFromXML( const XMLNode::Pointer& child)
{
    const DLString &name = child->getName();
    Lex::id_t id = Lex::getThis()->resolve(name);
    
    (*this)[id].fromXML(child);
    return true;
}

