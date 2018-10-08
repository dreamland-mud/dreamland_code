/* $Id: scheduler.cpp,v 1.1.2.4.6.5 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#include "scheduler.h"
#include "schedulertask.h"
#include "schedulerqueue.h"
#include "schedulerprioritymap.h"
#include "logstream.h"
#include "exception.h"

class SchedulerTask;
typedef Pointer<SchedulerTask> SchedulerTaskPointer;

Scheduler::Scheduler( ) : time( 0 )
{
}

Scheduler::~Scheduler( )
{
}

void Scheduler::putTask( long offset, SchedulerTaskPointer task )
{
        queue.put( time + offset, task );
}

void Scheduler::putTaskNOW( SchedulerTaskPointer task )
{
        queue.put( time, task );
}

void Scheduler::putTaskInitiate( SchedulerTaskPointer task )
{
        queue.put( time + 1, task );
}

void Scheduler::tick( )
{
    try {
        while (true) {
            working = queue.get( time );
            
            if(working.isEmpty( ))
                break;
            
            working->tick( );
        }
    } 
    catch (const Exception &e) {
        e.printStackTrace( LogStream::sendError( ) );
        LogStream::sendError( ) << "Scheduler: rethrow: " << e.what( ) << endl;
        working.clear( );
        throw e;
    } 

    time++;
}

void Scheduler::slay( SchedulerTask::Pointer task )
{
        queue.slay( task );
}

void Scheduler::slayInstance( SchedulerTask::Pointer task )
{
        queue.slayInstance( task );
}

int Scheduler::getPriority( ) const
{
    if (working.isEmpty( ))
        return 0;
    else
        return working->getPriority( );
}

long Scheduler::getCurrentTick( ) const
{
    return time;
}
