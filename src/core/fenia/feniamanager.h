/* $Id: feniamanager.h,v 1.1.2.10.6.1 2007/06/26 07:24:43 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __FENIAMANAGER_H__
#define __FENIAMANAGER_H__

#include "dlobject.h"
#include "oneallocate.h"
#include "fenia/object.h"
#include "fenia/codesource.h"
#include "fenia/context.h"
#include "fenia/register-decl.h"

#include "wrappermanagerbase.h"

class WrapperBase;
class FeniaProcess;

/** Provide more complex logging functionality for exceptions; defined in plugins. */
class FeniaCroakerBase : public virtual DLObject {
public:
    typedef ::Pointer<FeniaCroakerBase> Pointer;

    virtual void croak(const FeniaProcess *process, const ::Exception &e) const = 0;
    virtual void croak(const WrapperBase *wrapper, const Scripting::Register &key, const ::Exception &e) const = 0;
};

class FeniaManager : public Scripting::CodeSource::Manager, // order is important: 
                     public Scripting::Object::Manager,     // the object manager must be destroyed before the codesource manager
                     public Scripting::Context,
                     public OneAllocate
{
public:
    typedef ::Pointer<FeniaManager> Pointer;

    FeniaManager( );
    virtual ~FeniaManager( );

    void open( ) {
        Scripting::Object::Manager::open( );
        Scripting::CodeSource::Manager::open( );
    }
    void close( ) {
        Scripting::Object::Manager::close( );
        Scripting::CodeSource::Manager::close( );
    }
    void load( ) {
        Scripting::Object::Manager::load( );
        Scripting::CodeSource::Manager::load( );
    }

    // Report exception thrown inside a thread.
    void croak(const FeniaProcess *process, const ::Exception &e) const;

    // Report exception thrown by a wrapper.
    void croak(const WrapperBase *wrapper, const Scripting::Register &key, const ::Exception &e) const;

    virtual DbEnvContext *getDbEnv( ) const;
    
    static WrapperManagerBase::Pointer wrapperManager;

    static FeniaCroakerBase::Pointer feniaCroaker;

    inline static FeniaManager * getThis( ) {
        return thisClass;
    }
private:
    static FeniaManager *thisClass;
};

#endif
