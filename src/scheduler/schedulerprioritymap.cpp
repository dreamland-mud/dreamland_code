/* $Id: schedulerprioritymap.cpp,v 1.1.4.1.10.3 2009/10/11 18:35:37 rufina Exp $
 * 
 * ruffina, Dream Land, 2003
 */

#include "schedulerprioritymap.h"
#include "schedulerlist.h"
#include "schedulertask.h"

SchedulerPriorityMap::SchedulerPriorityMap( ) : priority( 0 )
{
}

void SchedulerPriorityMap::tick( )
{
    for (iterator i = begin( ); i != end( ); i++) {
	priority = i->first;
	i->second.tick( );
    }
    
    priority = 0;
}

void SchedulerPriorityMap::slay( SchedulerTaskPointer& task )
{
    for (iterator i = begin( ); i != end( ); i++)
	i->second.slay( task );
}

void SchedulerPriorityMap::slayInstance( SchedulerTaskPointer& task )
{
    for (iterator i = begin( ); i != end( ); i++)
	i->second.slayInstance( task );
}

int SchedulerPriorityMap::getPriority( ) const
{
    return priority;
}
