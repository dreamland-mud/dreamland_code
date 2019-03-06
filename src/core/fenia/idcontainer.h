/* $Id: idcontainer.h,v 1.1.2.2.18.2 2009/11/04 03:24:31 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef __IDCONTAINER_H__
#define __IDCONTAINER_H__

#include <map>

#include <xmlnode.h>
#include <xmlcontainer.h>

#include "fenia/handler.h"
#include "xmlregister.h"
#include "lex.h"
#include "native.h"

using namespace std;
using Scripting::XMLRegister;
using Scripting::Register;
using Scripting::RegisterList;
using Scripting::NativeTraits;

class IdContainer : public Scripting::Handler, 
                    public virtual XMLContainer 
{
NMI_OBJECT
    typedef NativeTraits<IdContainer> Traits;
    typedef map<Scripting::Lex::id_t, XMLRegister> Idmap;
    Idmap idmap;
public:
    IdContainer() { }

    virtual Register getField(const Register &key);
    virtual void setField(const Register &key, const Register &val);
    virtual Register callMethod(const Register &key, const RegisterList &args);

    virtual bool nodeFromXML( const XMLNode::Pointer& node );
    virtual void fromXML( const XMLNode::Pointer& node ) throw ( ExceptionBadType );
    virtual bool toXML( XMLNode::Pointer& node ) const;

    virtual const DLString &getType() const {
        return TYPE;
    }
    
    virtual DLObject::Pointer set(DLObject::Pointer o1, DLObject::Pointer o2) {
        return DLObject::Pointer();
    }

    virtual void setSelf(Scripting::Object *s) {
        self = s;
    }

    static const DLString TYPE;

private:
    Scripting::Object *self;
};

#endif
