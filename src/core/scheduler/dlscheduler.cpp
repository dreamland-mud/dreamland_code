/* $Id: dlscheduler.cpp,v 1.1.2.5.6.1 2007/06/26 07:24:50 rufina Exp $
 * 
 * ruffina, 2004
 */

#include "class.h"
#include "dlscheduler.h"
#include "schedulertask.h"
#include "schedulerqueue.h"
#include "schedulerprioritymap.h"
#include "dreamland.h"

class SchedulerTask;
typedef Pointer<SchedulerTask> SchedulerTaskPointer;

DLScheduler* DLScheduler::thisClass = 0;

DLScheduler::DLScheduler( ) : Scheduler( ) 
{
    checkDuplicate( thisClass );
    thisClass = this;
}

DLScheduler::~DLScheduler( ) 
{
    thisClass = 0;
}

void DLScheduler::putTaskInSecond( SchedulerTaskPointer task ) 
{
    queue.put( time + dreamland->getPulsePerSecond( ), task );
}

void DLScheduler::putTaskInSecond( long offset, SchedulerTaskPointer task ) 
{
    queue.put( time + offset * dreamland->getPulsePerSecond( ), task );
}


