/* $Id: schedulertaskroundroom.cpp,v 1.1.4.1.6.2 2009/09/24 14:09:13 rufina Exp $
 *
 * ruffina, 2003
 *
 * переписано заново, добавлены приоритеты
 */
/***************************************************************************
                          schedulertask.cpp  -  description
                             -------------------
    begin                : Fri Oct 19 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#pragma implementation

#include "dlscheduler.h"
#include "schedulertaskroundroom.h"
#include "schedulerlist.h"
#include "room.h"
#include "mercdb.h"


int SchedulerTaskRoundRoom::getPriority( ) const
{
    return SCDP_ROUND + 50;
}

void SchedulerTaskRoundRoom::run( )
{
    for( Room* room = room_list; room != 0; room = room->rnext )
	run( room );
}
