/* $Id: schedulerqueue.cpp,v 1.1.2.2.10.1 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/***************************************************************************
                          schedulerqueue.cpp  -  description
                             -------------------
    begin                : Wed May 30 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include "schedulerprioritymap.h"
#include "schedulerqueue.h"
#include "schedulertask.h"
#include "schedulerlist.h"

void SchedulerQueue::put( long time, SchedulerTaskPointer& task )
{
    iterator pos = find( time );
    
    if (pos == end( ))
    {
        SchedulerPriorityMapPointer pv( NEW );
        task->putInto( pv );
        ( *this )[time] = pv;
    }
    else
    {
        task->putInto( pos->second );
    }
}

SchedulerPriorityMap::Pointer SchedulerQueue::get( long time )
{
    iterator pos = find( time );
    
    if (pos != end( )) {
        SchedulerPriorityMap::Pointer pv = pos->second;
        erase( pos );
        return pv;
    }
    
    return SchedulerPriorityMap::Pointer( );
}

void SchedulerQueue::slay( SchedulerTask::Pointer& task )
{
    for (iterator ipos = begin( ); ipos != end( ); ipos++)
            ipos->second->slay( task );
}

void SchedulerQueue::slayInstance( SchedulerTask::Pointer& task )
{
    for (iterator ipos = begin( ); ipos != end( ); ipos++)
            ipos->second->slayInstance( task );
}
