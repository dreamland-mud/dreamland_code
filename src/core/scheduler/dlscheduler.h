/* $Id: dlscheduler.h,v 1.1.2.3 2005/07/30 14:50:16 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef __DLSCHEDULER_H__
#define __DLSCHEDULER_H__

#include "scheduler.h"
#include "oneallocate.h"

enum {
    SCDP_BOOT         =  100,
    SCDP_INITIAL = 1000,
    SCDP_IOINIT         = 2000,
    SCDP_IOPOLL         = 3000,
    SCDP_IOREAD         = 4000,
    SCDP_AUTO         = 5000,
    SCDP_ROUND         = 6500,
    SCDP_IOWRITE = 7000,
    SCDP_FINAL         = 10000,
};

class SchedulerTask;

class DLScheduler : public Scheduler, public OneAllocate {
public:        
        typedef ::Pointer<DLScheduler> Pointer;

        DLScheduler( );
        virtual ~DLScheduler( );
        
        /** Выполнить задачу через секунду */
        void putTaskInSecond( SchedulerTaskPointer task );
        /** Выполнить задачу через time секунд */
        void putTaskInSecond( long time, SchedulerTaskPointer task );
        
        static inline DLScheduler* getThis( ) 
        {
            return thisClass;
        }

private:
        static DLScheduler* thisClass;
};

#endif
