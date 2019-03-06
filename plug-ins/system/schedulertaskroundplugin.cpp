/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          schedulertaskroundplugin.cpp  -  description
                             -------------------
    begin                : Fri Oct 19 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include "schedulertaskroundplugin.h"
#include "dlscheduler.h"
#include "so.h"

void SchedulerTaskRoundPlugin::initialization( )
{
        DLScheduler::getThis( )->putTaskNOW( SchedulerTaskRoundPlugin::Pointer( this ) );
}

void SchedulerTaskRoundPlugin::destruction( )
{
        DLScheduler::getThis( )->slay( SchedulerTaskRoundPlugin::Pointer( this ) );
}

extern "C"
{
        SO::PluginList initialize_scheduler_task_round( )
        {
                SO::PluginList ppl;
                return ppl;
        }
}
