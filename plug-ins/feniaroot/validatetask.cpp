/* $Id: validatetask.cpp,v 1.1.4.7.6.3 2009/11/04 03:24:33 rufina Exp $
 *
 * ruffina, 2004
 */

#include "fenia/register-impl.h"
#include "fenia/manager-impl.h"
#include "fenia/object.h"
#include "fenia/function.h"
#include "fenia/codesource.h"
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

            // DO NOT free. See the comment on freeList below.
        }

    for(si = Scripting::CodeSource::manager->begin(); si != Scripting::CodeSource::manager->end(); si++)
        for(fi = si->functions.begin(); fi != si->functions.end(); fi++)
            if (fi->refcnt <= 0)  {
                LogStream::sendWarning( ) 
                    << "fenia fsck:  unreferenced function " 
                    << " cs: " << fi->source.source->getId() << " (" << fi->source.source->name << ")"
                    << " line: " << fi->source.line << " (fn:" << fi->getId() << ")" << endl;
                // DO NOT free. See the comment on freeList below.
            }

    // freeList and csList are deliberately left EMPTY, so this task reports and
    // frees nothing. It used to collect every refcnt<=0 object and every
    // refcnt<=0 function, then destroy them all at once by clearing the list.
    //
    // That destruction is not safe. Freeing an unreferenced object runs
    // ~IdContainer -> ~XMLRegister -> ~Closure -> Function::unlink ->
    // Function::finalize, which calls manager->erase(id) on the function
    // manager owned by the function's CodeSource. When a hot reload has already
    // replaced that CodeSource, the manager pointer is dangling and the boot
    // dies with SIGSEGV inside the fsck (observed 2026-08-08: manager at 0x68,
    // four consecutive crash-looping boots, game down until the binary changed).
    //
    // Repeatedly hot-reloading a large shared file -- utils/object: destruction
    // and public/utils/mob were each posted several times in one day -- leaves
    // exactly this shape behind, because .tmp.object.* / .tmp.mob.* maps hold
    // closures into the CodeSource that was replaced. The orphans are harmless
    // where they sit: they are unreferenced, they already persist in the DB
    // across boots, and nothing dereferences them. Only the attempt to collect
    // them is fatal.
    //
    // So: keep the diagnostics, drop the sweep. A real collector has to prove
    // the owning CodeSource is still alive before unlinking a function, which
    // is a bigger change than an outage should carry.
    if (!freeList.empty( ))
        LogStream::sendWarning( ) 
            << "fenia fsck: " << freeList.size( ) 
            << " unref objects/functions cleared" << endl;
    
    freeList.clear( );

    for(si = Scripting::CodeSource::manager->begin(); si != Scripting::CodeSource::manager->end(); si++)
        if (si->refcnt == 0)
            LogStream::sendWarning( )
                << "fenia fsck:  unreferenced source " << si->getId()
                << " (" << si->name << ") -- reported, not freed" << endl;
    
    if (!csList.empty( ))
        LogStream::sendWarning( ) 
            << "fenia fsck: " << csList.size( ) 
            << " unref sources cleared" << endl;

    csList.clear( );

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

