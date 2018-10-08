/* $Id: xmlregister.cpp,v 1.1.2.5.6.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: xmlregister.cpp,v 1.1.2.5.6.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#include <xmlnode.h>
#include <integer.h>
#include <dlstring.h>

#include <logstream.h>

#include "object.h"
#include "function.h"
#include "codesource.h"
#include "xmlregister.h"
#include "register-impl.h"

namespace Scripting {

const DLString XMLRegister::ATTRIBUTE_REGTYPE = "regtype";
const DLString XMLRegister::REGTYPE_NONE = "none";
const DLString XMLRegister::REGTYPE_STRING = "string";
const DLString XMLRegister::REGTYPE_IDENTIFIER = "identifier";
const DLString XMLRegister::REGTYPE_NUMBER = "number";
const DLString XMLRegister::REGTYPE_FUNCTION = "function";
const DLString XMLRegister::REGTYPE_OBJECT = "object";

XMLRegister::XMLRegister() 
{
}

XMLRegister::XMLRegister(const Register &r) : Register(r) 
{
}

bool
XMLRegister::toXML( XMLNode::Pointer& parent) const
{
    if(type == NONE) {
        parent->setType(XMLNode::XML_LEAF);
        parent->insertAttribute(ATTRIBUTE_REGTYPE, REGTYPE_NONE);
        return true;
    } else if(type == FUNCTION) {
        XMLFunctionRef ref;
        
        parent->setType(XMLNode::XML_NODE);
        parent->insertAttribute(ATTRIBUTE_REGTYPE, REGTYPE_FUNCTION);
        value.function->toXMLFunctionRef(ref);

        ref.toXML(parent);
        return true;
    }
    
    XMLNode::Pointer node(NEW);
    node->setType(XMLNode::XML_TEXT);
    switch(type) {
        case STRING:
            node->setName("'" + *strPtr( ) + "'");
            parent->insertAttribute(ATTRIBUTE_REGTYPE, REGTYPE_STRING);
            break;
        case IDENTIFIER:
            node->setName(Lex::getThis()->getName(value.identifier));
            parent->insertAttribute(ATTRIBUTE_REGTYPE, REGTYPE_IDENTIFIER);
            break;
        case NUMBER:
        case OBJECT:
            {
                int number = -99;
                switch(type) {
                    case NUMBER:
                        number = value.number;
                        parent->insertAttribute(ATTRIBUTE_REGTYPE, REGTYPE_NUMBER);
                        break;
                    case OBJECT:
                        number = value.object->getId();
                        parent->insertAttribute(ATTRIBUTE_REGTYPE, REGTYPE_OBJECT);
                        break;
                    default:
                        ;
                }
                node->setName(Integer(number).toString());
            }
            break;
        default:
            ;
    }
    parent->appendChild(node);
    return true;
}

void 
XMLRegister::fromXML( const XMLNode::Pointer& parent) throw( ExceptionBadType )
{
    const DLString &regtype = parent->getAttribute(ATTRIBUTE_REGTYPE);

    if(regtype == REGTYPE_NONE) {
        *this = Register();
        return;
    } else if(regtype == REGTYPE_FUNCTION) {
        XMLFunctionRef ref;
        ref.fromXML(parent);
        Register::set(new Closure(ref));
        return;
    }
    
    XMLNode::Pointer node = parent->getFirstNode();

    if(node->getType() != XMLNode::XML_TEXT 
            && node->getType() != XMLNode::XML_CDATA)
        throw ExceptionBadType(parent->getName(), "text node expected");
    
    const DLString &cd = node->getCData();
    
    if(regtype == REGTYPE_STRING) {
        if(cd[0] == '\'')
            Register::set(FeniaString(cd.substr(1, cd.length() - 2)));
        else
            Register::set(FeniaString(cd));
    } else if(regtype == REGTYPE_IDENTIFIER) {
        Register::set(Lex::getThis()->resolve(cd));
    } else if(regtype == REGTYPE_NUMBER) {
        Register::set(cd.toInt());
    } else if(regtype == REGTYPE_OBJECT) {
        Register::set(&Object::manager->at(cd.toInt()));
    } else {
        throw ExceptionBadType(parent->getName(), "unknown regtype in XMLRegister element");
    }
}

}
