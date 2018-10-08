/* $Id: schedulertaskroundobject.cpp,v 1.1.4.1.6.1 2009/09/24 14:09:13 rufina Exp $
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
#include "schedulertaskroundobject.h"
#include "schedulerlist.h"
#include "object.h"

int SchedulerTaskRoundObject::getPriority( ) const
{
    return SCDP_ROUND + 40;
}

void SchedulerTaskRoundObject::run( )
{
    for( Object* obj = object_list; obj != 0; obj = obj->next )
        run( obj );
}

