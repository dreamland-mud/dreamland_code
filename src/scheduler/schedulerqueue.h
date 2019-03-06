/* $Id: schedulerqueue.h,v 1.1.2.2.10.1 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/***************************************************************************
                          schedulerqueue.h  -  description
                             -------------------
    begin                : Wed May 30 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef SCHEDULERQUEUE_H
#define SCHEDULERQUEUE_H

#include <map>

#include "dlobject.h"
#include "pointer.h"

class SchedulerPriorityMap;
class SchedulerTask;

/**
 * @short Сортированная очередь событий по времени
 * @author Igor S. Petrenko
 */

typedef Pointer<SchedulerPriorityMap> SchedulerPriorityMapPointer;

class SchedulerQueue : public std::map<long, SchedulerPriorityMapPointer>, public virtual DLObject
{
public:        
        typedef ::Pointer<SchedulerQueue> Pointer;
        typedef ::Pointer<SchedulerTask> SchedulerTaskPointer;
        
public:        
        /** Положить задачу с определенным временем срабатывания */
        void put( long time, SchedulerTaskPointer& task );
        /** Взять список задач для определенного времени срабатывания */
        SchedulerPriorityMapPointer get( long time );
        /** Насильно убить все задачи заданного типа */
        void slay( SchedulerTaskPointer& task );
        /** Насильно убить все задачи с этим указателем */
        void slayInstance( SchedulerTaskPointer& task );
};

#endif
