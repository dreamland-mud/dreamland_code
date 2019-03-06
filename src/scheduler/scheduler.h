/* $Id: scheduler.h,v 1.1.2.3.6.1 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 * based on idea by NoFate, 2001
 */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "pointer.h"
#include "dlobject.h"
#include "schedulerqueue.h"

class SchedulerTask;
class SchedulerPriorityMap;

/**
 * @short Планировщик задач
 */
class Scheduler : public virtual DLObject { 
public:        
        typedef ::Pointer<Scheduler> Pointer;
        typedef ::Pointer<SchedulerTask> SchedulerTaskPointer;

public:
        Scheduler( );
        virtual ~Scheduler( );
        
        void putTask( long time, SchedulerTaskPointer task );
        /** Выполнить задачу немедленно */
        void putTaskNOW( SchedulerTaskPointer task );
        /** Выполнить задачу при инициализации */
        void putTaskInitiate( SchedulerTaskPointer task );
        
        /** Насильно убить все задачи заданного типа */
        void slay( SchedulerTaskPointer task );
        /** Насильно убить все задачи с этим указателем */
        void slayInstance( SchedulerTaskPointer task );
        
        /** Системный тик */
        void tick( );

        /** Текущий приоритет в обрабатываемом мэпе */
        int getPriority( ) const;
        long getCurrentTick( ) const;
        
protected:
        SchedulerQueue queue;
        long time;
        ::Pointer<SchedulerPriorityMap> working;
};

#endif
