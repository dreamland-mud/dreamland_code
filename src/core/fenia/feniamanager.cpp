/* $Id: feniamanager.cpp,v 1.1.2.9.6.3 2009/11/04 03:24:31 rufina Exp $
 *
 * ruffina, 2004
 */
#include <fstream>

#include "logstream.h"
#include "class.h"
#include "dreamland.h"

#include "fenia/register-impl.h"
#include "fenia/context.h"

#include "feniamanager.h"
#include "schedulerwrapper.h"
#include "idcontainer.h"
#include "regcontainer.h"
#include "reglist.h"

using namespace Scripting;

/*----------------------------------------------------------------
 * FeniaManager
 *----------------------------------------------------------------*/
WrapperManagerBase::Pointer FeniaManager::wrapperManager;
FeniaCroakerBase::Pointer FeniaManager::feniaCroaker;
FeniaManager * FeniaManager::thisClass = 0;

FeniaManager::FeniaManager() 
{
    checkDuplicate( thisClass );
    thisClass = this;

    current = this;
    scope = NULL;

    Class::regMoc<SchedulerWrapper>( );
    Class::regMoc<FeniaProcess>( );
    Class::regXMLVar<IdContainer>( );
    Class::regXMLVar<RegContainer>( );
    Class::regXMLVar<RegList>( );
    Class::regMoc<RegListCall>( );
    Class::regMoc<RegListIterator>( );
}


FeniaManager::~FeniaManager() 
{
    Class::unregMoc<SchedulerWrapper>( );
    Class::unregMoc<FeniaProcess>( );
    Class::unregXMLVar<IdContainer>( );
    Class::unregXMLVar<RegContainer>( );
    Class::unregXMLVar<RegList>( );
    Class::unregMoc<RegListCall>( );
    Class::unregMoc<RegListIterator>( );

    if(current == this)
        current = NULL;

    thisClass = 0;
}

void FeniaManager::croak(const WrapperBase *wrapper, const Register &key, const ::Exception &e) const
{
    try {
        if (feniaCroaker)
            feniaCroaker->croak(wrapper, key, e);

        LogStream::sendError() 
            << "Exception calling Fenia prog " << key.toString() << ": " 
            << e.what() << endl;
        
    } catch(const ::Exception &x) {
        LogStream::sendError() 
            << "Exception trying to report exception " 
            << e.what() << ": " << x.what() << endl;
    }
}

void FeniaManager::croak(const FeniaProcess *process, const ::Exception &e) const
{
    try {
        if (feniaCroaker)
            feniaCroaker->croak(process, e);

        LogStream::sendError() 
            << "Exception in Fenia process " << process->name << ": "
            << e.what() << endl;
        
    } catch(const ::Exception &x) {
        LogStream::sendError() 
            << "Exception trying to report exception " 
            << e.what() << ": " << x.what() << endl;
    }
}


DbEnvContext *
FeniaManager::getDbEnv( ) const
{
    return dreamland->getFeniaDbEnv( );
}

