/* $Id: wrapperbase.h,v 1.1.2.5.18.3 2009/11/08 17:35:28 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __WRAPPERBASE_H__
#define __WRAPPERBASE_H__

#include <stdarg.h>

#include "fenia/handler.h"
#include "fenia/native.h"
#include "xmlvariablecontainer.h"
#include "xmllonglong.h"
#include "xmlboolean.h"
#include "guts.h"

class StringSet;

namespace Scripting {
class Object;
};
using Scripting::Register;
using Scripting::RegisterList;

class WrapperBase : public virtual Scripting::Handler, 
                    public virtual Scripting::Native, 
                    public XMLVariableContainer 
{
XML_OBJECT    
public:
    typedef ::Pointer<WrapperBase> Pointer;

    WrapperBase( );

    virtual void setField(const Register &key, const Register &val);
    virtual Register getField(const Register &key);
    virtual Register callMethod(const Register &key, const RegisterList &args );

    virtual void setSelf( Scripting::Object * );
    virtual Scripting::Object *getSelf() const { return self; }
    virtual void extract( bool );
    virtual bool targetExists() const;

    bool call( Register id, const char *fmt, ... );
    bool vcall( Register &rc, const Register &key, const char *fmt, va_list ap);
    bool call( Register &rc, const Register &progName, const Register &progFun, const RegisterList &progArgs);
    void postpone( Register id, const char *fmt, ... );
    bool vpostpone( Register id, const char *fmt, va_list ap);
    void postpone(const Register &progName, const Register &progFun, const RegisterList &progArgs);
    DLString stringCall( Register id, const char *fmt, ... );
    bool numberCall( Register id, int &result, const char *fmt, ... );

    inline bool isAlive() const;
    inline bool isZombie() const;
    inline void setAlive();
    inline long long getID() const;

    bool hasTrigger( const DLString &name ) const;
    bool hasField(const DLString &name) const;
    void collectTriggers(StringSet &triggers, StringSet &misc) const;
    bool triggerFunction(const Register &key, Register &prog) const;
    static void triggerArgs( RegisterList &regList, const char *fmt, va_list ap );

protected:

    Scripting::Object * self;
    bool alive;

    XML_VARIABLE XMLLongLong id;
    XML_VARIABLE XMLBoolean zombie;
    XML_VARIABLE Guts guts;
};

inline bool WrapperBase::isAlive() const 
{
    return alive;
}
inline bool WrapperBase::isZombie() const 
{
    return zombie.getValue();
}
inline void WrapperBase::setAlive()
{
    alive = true;
}


inline long long WrapperBase::getID() const
{
    return id.getValue();
}

WrapperBase * get_wrapper(Scripting::Object *obj);

#endif
