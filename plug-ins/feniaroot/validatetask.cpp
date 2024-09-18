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

            freeList.push_back( Register( &*oi ) );
        }

    for(si = Scripting::CodeSource::manager->begin(); si != Scripting::CodeSource::manager->end(); si++)
        for(fi = si->functions.begin(); fi != si->functions.end(); fi++)
            if (fi->refcnt <= 0)  {
                LogStream::sendWarning( ) 
                    << "fenia fsck:  unreferenced function " 
                    << " cs: " << fi->source.source->getId() << " (" << fi->source.source->name << ")"
                    << " line: " << fi->source.line << " (fn:" << fi->getId() << ")" << endl;
                freeList.push_back( Register( new Closure(NULL, &*fi )) );
            }

    if (!freeList.empty( ))
        LogStream::sendWarning( ) 
            << "fenia fsck: " << freeList.size( ) 
            << " unref objects/functions cleared" << endl;
    
    freeList.clear( );

    for(si = Scripting::CodeSource::manager->begin(); si != Scripting::CodeSource::manager->end(); si++)
        if (si->refcnt == 0) 
            csList.push_back( &*si );
    
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

