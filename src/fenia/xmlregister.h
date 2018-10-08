/* $Id: xmlregister.h,v 1.1.2.4.6.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: xmlregister.h,v 1.1.2.4.6.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __XMLREGISTER_H__
#define __XMLREGISTER_H__

#include <xmlpolymorphvariable.h>
#include <xmlinteger.h>
#include <xmlmap.h>
#include <xmlvariablecontainer.h>

// MOC_SKIP_BEGIN
#include "register-decl.h"
#include "register-impl.h"
// MOC_SKIP_END

namespace Scripting {

/*
 * <varName regtype="number">5</varName>
 * <varName regtype="string">'free text'</varName>
 * <varName regtype="function"><codesource>76534</codesource><function>1243</function></varName> - function ID
 * <varName regtype="object">76534</varName> - object ID
 * <varName regtype="none"/> - null or undef
 */
class XMLRegister : public Register
{
public:
    XMLRegister();
    XMLRegister(const Register &r);
        
    void fromXML(const XMLNode::Pointer& parent) throw( ExceptionBadType );
    bool toXML(XMLNode::Pointer& parent) const;

private:
    static const DLString ATTRIBUTE_REGTYPE;
    static const DLString REGTYPE_NONE;
    static const DLString REGTYPE_STRING;
    static const DLString REGTYPE_IDENTIFIER;
    static const DLString REGTYPE_NUMBER;
    static const DLString REGTYPE_FUNCTION;
    static const DLString REGTYPE_OBJECT;
};

class XMLFunctionRef : public XMLVariableContainer {
XML_OBJECT
public:
    XML_VARIABLE XMLInteger codesource, function;
    XML_VARIABLE XMLMapBase<XMLRegister> environment;
};

}

#endif
