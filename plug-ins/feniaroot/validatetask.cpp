/* $Id: validatetask.cpp,v 1.1.4.7.6.3 2009/11/04 03:24:33 rufina Exp $
 *
 * ruffina, 2004
 */

#include "fenia/register-impl.h"
#include "fenia/manager-impl.h"
#include "fenia/object.h"
#include "fenia/function.h"
#include "fenia/codesource.h"
#include "fenia/closure.h"
#include "fenia/handler.h"
#include "fenia/context.h"
#include "fenia/phase.h"

#include "logstream.h"
#include "core/object.h"
#include "dlscheduler.h"
#include "merc.h"

#include "feniamanager.h"

#include "objectwrapper.h"
#include "validatetask.h"
#include "roomwrapper.h"
#include "root.h"

using namespace Scripting;

void 
ValidateTask::run( )
{
    std::list<Register> freeList;
    std::list< ::Pointer<CodeSource> > csList;
    Scripting::Object::Manager::iterator oi;
    Scripting::FunctionManager::iterator fi;
    Scripting::CodeSource::Manager::iterator si;

    for(oi = Scripting::Object::manager->begin(); oi != Scripting::Object::manager->end(); oi++)
        if (oi->refcnt <= 0 && oi->hasHandler( )) {
            ostream &os = LogStream::sendWarning( ) 
                << "fenia fsck:  unreferenced ("<< oi->refcnt << ") object id:" 
                << oi->getId();
            
            try {
                os << " type: " << oi->getHandler()->getType();
            } catch(...) {
                os << " no type";
            }
            
            os << endl;

            freeList.push_back( Register( &*oi ) );
        }

    for(si = Scripting::CodeSource::manager->begin(); si != Scripting::CodeSource::manager->end(); si++)
        for(fi = si->functions.begin(); fi != si->functions.end(); fi++)
            if (fi->refcnt <= 0)  {
                ostream &os = LogStream::sendWarning( )
                    << "fenia fsck:  unreferenced function ";

                // A function whose code source is missing is exactly what this
                // report is for, so it must not be the thing that crashes it.
                if (fi->source.source)
                    os << " cs: " << fi->source.source->getId()
                       << " (" << fi->source.source->name << ")";
                else
                    os << " cs: GONE";

                os << " line: " << fi->source.line << " (fn:" << fi->getId() << ")" << endl;

                // Wrap the function in a throwaway Closure so its teardown runs
                // through Closure::~Closure, whose (csId, fnId) re-resolve keeps
                // the unlink safe even if the owning CodeSource is freed earlier
                // in this same sweep. The Closure links the function, pinning it
                // (and, through it, its CodeSource) until this list is cleared.
                freeList.push_back( Register( new Closure(NULL, &*fi) ) );
            }

    // Collect first, free second. Both loops above snapshot each orphan behind
    // its own reference (a Register that links the object; a temporary Closure
    // that links the function) BEFORE anything is freed, so the managers are
    // never mutated while still being iterated, and clearing a list then frees
    // each orphan independently of the others.
    //
    // This is the sweep that crash-looped four boots on 2026-08-08 (manager at
    // 0x68). Freeing an unreferenced object cascades ~Closure ->
    // Function::unlink -> Function::finalize; back then ~Closure dereferenced a
    // raw Function* whose CodeSource a hot reload had already replaced, and
    // finalize walked that freed function manager. It is safe to re-enable now
    // because two guards landed first: Function::finalize looks the CodeSource
    // up by a stored id and confirms identity instead of dereferencing
    // source.source (P1, #1105), and Closure::~Closure re-resolves its function
    // by (csId, fnId) and skips the unlink when the owner is already gone (P2a).
    // See the Fenia GC plan, Trello #2857.
    //
    // Any exception is swallowed with a loud log rather than propagated: a
    // half-freed graph must not take the boot down. The orphans left behind are
    // harmless where they sit -- exactly as they were while the sweep was off.
    size_t freeCount = freeList.size( );

    try {
        freeList.clear( );
        if (freeCount)
            LogStream::sendWarning( )
                << "fenia fsck: " << freeCount
                << " unref objects/functions cleared" << endl;
    } catch (...) {
        LogStream::sendError( )
            << "fenia fsck: sweep of unref objects/functions aborted mid-free"
            << endl;
    }

    for(si = Scripting::CodeSource::manager->begin(); si != Scripting::CodeSource::manager->end(); si++)
        if (si->refcnt == 0)  {
            LogStream::sendWarning( )
                << "fenia fsck:  unreferenced source " << si->getId()
                << " (" << si->name << ")" << endl;
            csList.push_back( &*si );
        }

    // A refcnt==0 CodeSource is referenced by nothing -- in particular no live
    // Function names it (each would link it), so its function manager is empty
    // and erasing it cascades into nothing. This loop never crashed; it was the
    // object/function sweep above that needed P1+P2a to become safe.
    size_t csCount = csList.size( );

    try {
        csList.clear( );
        if (csCount)
            LogStream::sendWarning( )
                << "fenia fsck: " << csCount
                << " unref sources cleared" << endl;
    } catch (...) {
        LogStream::sendError( )
            << "fenia fsck: sweep of unref sources aborted mid-free" << endl;
    }

    /*can't fail*/
    Scripting::Object *root = Context::current->root.toObject( );

    if(!root->hasHandler( )) {
        root->setHandler( ::Pointer<Root>(NEW) );
        LogStream::sendWarning( ) << "fenia: root object was not recovered. creating new" << endl;
        root->changed( );
    }
}

int ValidateTask::getPriority( ) const
{
    return SCDP_INITIAL + 0;
}

