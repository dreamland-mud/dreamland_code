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
    virtual void extract( bool );

    bool call( Register id, const char *fmt, ... );
    void postpone( Register id, const char *fmt, ... );
    DLString stringCall( Register id, const char *fmt, ... );
    bool numberCall( Register id, int &result, const char *fmt, ... );
    bool hasTrigger( const DLString &name ) const;
    void collectTriggers(StringSet &triggers, StringSet &misc) const;

    inline bool isAlive() const;
    inline bool isZombie() const;
    inline void setAlive();
    inline long long getID() const;
    
protected:
    bool vcall( Register &rc, const Register &key, const char *fmt, va_list ap);
    virtual void croak(const Register &key, const Exception &e) const;
    bool triggerFunction(const Register &key, Register &prog) const;
    void triggerArgs( RegisterList &regList, const char *fmt, va_list ap );

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
