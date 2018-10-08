/* $Id: reglist.h,v 1.1.2.2.18.2 2009/11/04 03:24:31 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef __REGLIST_H__
#define __REGLIST_H__

#include <map>

#include <xmlnode.h>
#include <xmlvariablecontainer.h>

#include "handler.h"
#include "xmlregister.h"
#include "lex.h"
#include "native.h"
#include "register-impl.h"

using namespace std;
using Scripting::XMLRegister;
using Scripting::Register;
using Scripting::RegisterList;

class XMLRegisterList : public std::list<XMLRegister>,
                        public virtual XMLContainer 
{
public:
    virtual bool nodeFromXML( const XMLNode::Pointer& node );
    virtual void fromXML( const XMLNode::Pointer& node ) throw ( ExceptionBadType );
    virtual bool toXML( XMLNode::Pointer& node ) const;
};

class RegList : public XMLRegisterList,
                public Scripting::NativeHandler,
                public Scripting::NativeImpl<RegList>
{
NMI_OBJECT
public:
    typedef ::Pointer<RegList> Pointer;

    virtual void setSelf(Scripting::Object *s);
    virtual const DLString &getType() const;
    virtual DLObject::Pointer set(DLObject::Pointer o1, DLObject::Pointer o2);

    static const DLString TYPE;

private:
    Scripting::Object *self;
};

extern template class Scripting::NativeImpl<RegList>;

class RegListCall : public XMLVariableContainer, public Scripting::Handler {
    friend class RegList;
XML_OBJECT
public:
    typedef ::Pointer<RegListCall> Pointer;
    
    virtual Register getField(const Register &key);
    virtual void setField(const Register &key, const Register &val);
    virtual Register callMethod(const Register &key, const RegisterList &args);

    virtual void setSelf(Scripting::Object *s) { }

protected:
    XML_VARIABLE XMLRegister list;
};

#endif
