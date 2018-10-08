/* $Id$
 *
 * ruffina, 2004
 */
#pragma interface

#include "schedulertask.h"

class Object;

/**
 * @short Задача для планировщика, которой на вход подаются обьекты
 * @author Igor S. Petrenko
 * @see Scheduler
 * @see SchedulerTask
 */
struct SchedulerTaskRoundObject : public virtual SchedulerTask
{
        typedef ::Pointer<SchedulerTaskRoundObject> Pointer;
        
        virtual void run( );
        /** Обработать обьект */
        virtual void run( Object* object ) = 0;
        virtual int getPriority( ) const;
};
