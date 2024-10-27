/* $Id: schedulertaskroundcharacter.cpp,v 1.1.4.1.6.2 2009/09/24 14:09:13 rufina Exp $
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
#include "schedulertaskroundcharacter.h"
#include "schedulerlist.h"
#include "profiler.h"
#include "character.h"
#include "merc.h"

int SchedulerTaskRoundCharacter::getPriority( ) const
{
    return SCDP_ROUND + 30;
}

void SchedulerTaskRoundCharacter::run( )
{
    ProfilerBlock profiler("SchedulerTaskRoundCharacter", 100);
    
    Character *ch, *ch_next;
    
    for (ch = char_list; ch != 0 ; ch = ch_next) {
        ch_next = ch->next;
        run( ch );
    }
}

