/* $Id: native.h,v 1.1.2.7.18.5 2009/11/04 03:24:31 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: native.h,v 1.1.2.7.18.5 2009/11/04 03:24:31 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __NATIVE_H__
#define __NATIVE_H__

#ifdef _never_defined_parsed_only_by_moc_

#include "handler.h"

namespace Scripting {

class Native { };
template <typename T> class NativeImpl : public virtual Native { };
class NativeHandler : public virtual Handler, public virtual Native { };

}
#endif

// MOC_SKIP_BEGIN
#include "register-decl.h"
#include "exceptions.h"
#include "staticlist.h"
#include "context.h"

namespace Scripting {

template <typename T>
struct NativeTraits {
    
    struct MTListComp {
        bool operator() (IdRef &k1, const Register &k2) {
            return (k2 == k1).toBoolean( );
        }
    };

    template <typename MethodType> 
    struct ListEntry {
        ListEntry( MethodType m, const char * h ) : method( m ), help( h ) { }
        MethodType method;
        const char * help;
    };
    
    struct Get {
        typedef Register (T::*Method)();
        typedef ListEntry<Method> Entry;
        typedef ::StaticList<IdRef, ListEntry<Method>, MTListComp> List;
        template <typename PropName> struct Holder { static List reg; };
    };
    struct Set {
        typedef void (T::*Method)(const Register &);
        typedef ListEntry<Method> Entry;
        typedef ::StaticList<IdRef, ListEntry<Method>, MTListComp> List;
        template <typename PropName> struct Holder { static List reg; };
    };
    struct Invoke {
        typedef Register (T::*Method)(const RegisterList &);
        typedef ListEntry<Method> Entry;
        typedef ::StaticList<IdRef, ListEntry<Method>, MTListComp> List;
        template <typename PropName> struct Holder { static List reg; };
    };

    static const DLString NAME;
    static const DLString HELP;
};
    

#define NMI_LIST_INIT(Cls, mode)   \
template <>                           \
NativeTraits<Cls>::mode::List *NativeTraits<Cls>::mode::List::first = 0

#define NMI_INIT(Cls, help)                     \
template <>                                        \
const DLString NativeTraits<Cls>::NAME = #Cls;  \
template <>                                        \
const DLString NativeTraits<Cls>::HELP = help;  \
NMI_LIST_INIT(Cls, Get);                        \
NMI_LIST_INIT(Cls, Set);                        \
NMI_LIST_INIT(Cls, Invoke);

#define NMI_OBJECT \
public: \
template <typename Property> Register nmiGet();             \
template <typename Property> void nmiSet(const Register &); \
template <typename Property> Register nmiInvoke(const RegisterList &); 

#define NMI_ENTRY(Cls, x, mode, help)                    \
template <> template <>                                    \
NativeTraits<Cls>::mode::List                            \
    NativeTraits<Cls>::mode::Holder<nmi::x>::reg    \
     ( #x, NativeTraits<Cls>::mode::Entry( &Cls::nmi ## mode< nmi::x >, help ) )

#define NMI_GET_MDECL(Cls, x)  template <> Register Cls::nmiGet< nmi::x >( )
#define NMI_GET(Cls, x, help)     \
namespace nmi { struct x; }       \
NMI_GET_MDECL(Cls, x);            \
NMI_ENTRY(Cls, x, Get, help);     \
NMI_GET_MDECL(Cls, x)

#define NMI_SET_MDECL(Cls, x)  template <> void Cls::nmiSet< nmi::x >( const Register &arg )
#define NMI_SET(Cls, x, help)     \
namespace nmi { struct x; }       \
NMI_SET_MDECL(Cls, x);            \
NMI_ENTRY(Cls, x, Set, help);     \
NMI_SET_MDECL(Cls, x)

#define NMI_INVOKE_MDECL(Cls, x)  template <> Register Cls::nmiInvoke< nmi::x >( const RegisterList &args )
#define NMI_INVOKE(Cls, x, help)  \
namespace nmi { struct x; }       \
NMI_INVOKE_MDECL(Cls, x);         \
NMI_ENTRY(Cls, x, Invoke, help);  \
NMI_INVOKE_MDECL(Cls, x)

class Native {
public:
    virtual bool setNativeField(const Register &key, const Register &val) = 0;
    virtual bool getNativeField(const Register &key, Register &retval) = 0;
    virtual bool callNativeMethod(const Register &key, const RegisterList &args, Register &retval) = 0;
};

template <typename T>
class NativeImpl : public virtual Native {
public:
    typedef NativeTraits<T> Traits;
    
    virtual bool setNativeField(const Register &key, const Register &val) {
        typename Traits::Set::Entry *e = Traits::Set::List::lookup(key);

        if(!e || e->method == 0)
            return false;

        (static_cast<T *>(this)->*(e->method))(val);
        return true;
    }

    virtual bool getNativeField(const Register &key, Register &retval) {
        typename Traits::Get::Entry *e = Traits::Get::List::lookup(key);
        
        if(!e || e->method == 0)
            return false;

        retval = (static_cast<T *>(this)->*(e->method))();
        return true;
    }

    virtual bool callNativeMethod(const Register &key, const RegisterList &args, Register &retval) {
        typename Traits::Invoke::Entry *e = Traits::Invoke::List::lookup(key);

        if(!e || e->method == 0)
            return false;

        retval = (static_cast<T *>(this)->*(e->method))(args);
        return true;
    }
};

class NativeHandler : public virtual Handler, public virtual Native {
public:
    virtual void setField(const Register &key, const Register &val);
    virtual Register getField(const Register &key);
    virtual Register callMethod(const Register &key, const RegisterList &args );
};
}
// MOC_SKIP_END

#endif
