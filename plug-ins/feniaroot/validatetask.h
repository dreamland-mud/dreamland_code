/* $Id: validatetask.h,v 1.1.4.3.10.1 2009/09/24 14:09:12 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef __VALIDATETASK_H__
#define __VALIDATETASK_H__

#include <schedulertask.h>

struct ValidateTask : public SchedulerTask
{
    typedef ::Pointer<ValidateTask> Pointer;

    virtual void run( );
    virtual int getPriority( ) const;
};


#endif
