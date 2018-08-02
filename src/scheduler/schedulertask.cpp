/* $Id: schedulertask.cpp,v 1.1.4.3 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#include "schedulertask.h"
#include "schedulerprioritymap.h"

void
SchedulerTask::before( )
{
}

void
SchedulerTask::run( ) 
{
}

void
SchedulerTask::after( ) 
{
}

void SchedulerTask::putInto( SchedulerPriorityMapPointer& pv )
{
    (**pv)[getPriority( )].put( Pointer( this ) );
}

