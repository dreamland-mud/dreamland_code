/* $Id: process.cpp,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#include "process.h"
#include "logstream.h"
#include "lastlogstream.h"

#include "scheduler.h"
#include "schedulertask.h"

//#define PDEBUG

Mutex hm;

ProcessManager *ProcessManager::thisClass = 0;

class ProcessManagerTask : public SchedulerTask {
public:
    typedef ::Pointer<ProcessManagerTask> Pointer;

    virtual void run( );
    virtual void after( );
    virtual int getPriority( ) const;
};

void 
ProcessManagerTask::run( )
{
    LastLogStream::send( ) <<  "Processes pulse"  << endl;
    if(ProcessManager::getThis())
        ProcessManager::getThis()->yield( );
}

void 
ProcessManagerTask::after( )
{
    Scheduler::getThis( )->putTaskInitiate( Pointer(this) );
}

int 
ProcessManagerTask::getPriority( ) const
{
    return SCDP_PROCESS;
}


ProcessManager::RoundRobinElement::RoundRobinElement( ) : mux( ), sync( &mux ) 
{
    next = prev = this;
}

ProcessManager::RoundRobinElement::~RoundRobinElement( ) 
{
    fromlist( );
}

void
ProcessManager::RoundRobinElement::fromlist( )
{
    hm.lock( );
    next->prev = prev;
    prev->next = next;
    next = prev = this;
    hm.unlock( );
}

void
ProcessManager::RoundRobinElement::tolist( RoundRobinElement &l )
{
    hm.lock( );
    next = &l;
    prev = l.prev;
    prev->next = this;
    next->prev = this;
    hm.unlock( );
}

void
ProcessManager::RoundRobinElement::speenup( )
{
#ifdef PDEBUG
    LogStream::sendError( ) << "speenup" << endl;
#endif
    mux.lock( ); // stuck in here until parent enters sync.wait();
    fromlist( );
    tolist(getThis( )->running);
    sync.notify( ); // notify the parent thread that we have aquired the lock and ready
    sync.wait( ); // give up the mux and wait for the first yield. mux will be re-aquired when yield received
#ifdef PDEBUG
    LogStream::sendError( ) << "speenup - done" << endl;
#endif
}

void
ProcessManager::RoundRobinElement::speendown( )
{
#ifdef PDEBUG
    LogStream::sendError( ) << "speedown" << endl;
#endif
    RoundRobinElement *n = next;
    fromlist( );
    tolist(getThis( )->stopped);
    n->mux.lock( );
    n->sync.notify( );
    n->mux.unlock( );
    mux.unlock( );
#ifdef PDEBUG
    LogStream::sendError( ) << "speedown - done" << endl;
#endif
}

void
ProcessManager::RoundRobinElement::yield( ) 
{
#ifdef PDEBUG
    LogStream::sendError( ) << "yield" << endl;
#endif
    next->mux.lock( );
    next->sync.notify( );
    next->mux.unlock( );
    sync.wait( );
#ifdef PDEBUG
    LogStream::sendError( ) << "yield - done" << endl;
#endif
}

void 
ProcessManager::RoundRobinElement::getInfo(ostream &os) 
{
    os << "main thread" << endl;
}

ProcessManager::ProcessManager( )
{
    running.mux.lock( );
    thisClass = this;

    if(Scheduler::getThis( ))
        Scheduler::getThis( )->putTaskInitiate(ProcessManagerTask::Pointer(NEW));
}

ProcessManager::~ProcessManager( )
{
    if(Scheduler::getThis( ))
        Scheduler::getThis( )->slay(ProcessManagerTask::Pointer(NEW));

    if(running.next != &running)
        LogStream::sendError( ) << "not all threads finished!" << endl;

    thisClass = 0;
    running.mux.unlock( );
}

void
ProcessManager::yield() 
{
    if(running.next != &running) 
        running.yield( );
}

