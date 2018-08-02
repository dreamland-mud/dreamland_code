/* $Id$
 *
 * ruffina, 2004
 */
#ifndef PLUGINNATIVEIMPL_H
#define PLUGINNATIVEIMPL_H

#include "native.h"
#include "wrapperbase.h"
#include "schedulerwrapper.h"

#ifdef _never_defined_parsed_only_by_moc_
using namespace Scripting;

template <typename T>
class PluginNativeImpl : public NativeImpl<T> { }

#endif

// MOC_SKIP_BEGIN
template <typename T>
class PluginNativeImpl : public Scripting::NativeImpl<T>
{
public:
    typedef Scripting::NativeImpl<T> Super;

    virtual bool setNativeField(const Register &key, const Register &val) {
        PlugLock lock;
        return Super::setNativeField(key, val);
    }

    virtual bool getNativeField(const Register &key, Register &retval) {
        PlugLock lock;
        return Super::getNativeField(key, retval);
    }

    virtual bool callNativeMethod(const Register &key, const RegisterList &args, Register &retval) {
        PlugLock lock;
        return Super::callNativeMethod(key, args, retval);
    }
};
// MOC_SKIP_END

#endif
