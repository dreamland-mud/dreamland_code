/* $Id: guts.h,v 1.1.2.1 2009/11/04 03:24:31 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __GUTS_H__
#define __GUTS_H__

#include <map>

#include "fenia/handler.h"
#include "xmlregister.h"
#include "xmlvariablecontainer.h"
#include "lex.h"

using Scripting::Lex;
using Scripting::Register;
using Scripting::RegisterList;
using Scripting::XMLRegister;

class GutsField : public XMLVariableContainer {
XML_OBJECT
public:
    XML_VARIABLE XMLRegister backref, target;
};

class Guts : public std::map<Lex::id_t, GutsField>, public virtual XMLContainer {
public:
    typedef ::Pointer<Guts> Pointer;

    virtual bool nodeFromXML( const XMLNode::Pointer& node );
    virtual void fromXML( const XMLNode::Pointer& node ) ;
    virtual bool toXML( XMLNode::Pointer& node ) const;
};

#endif
