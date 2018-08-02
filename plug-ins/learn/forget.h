/* $Id: forget.h,v 1.1.2.3.6.2 2008/02/23 13:41:24 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef FORGET_H
#define FORGET_H

#include "schedulertaskroundpcharacter.h"
#include "schedulertaskroundplugin.h"

class SkillTimerUpdate : public SchedulerTaskRoundPlugin,
                         public virtual SchedulerTaskRoundPCharacter 
{
public:
    typedef ::Pointer<SkillTimerUpdate> Pointer;

    virtual void run( PCharacter * );
    virtual void after( );

private:
    void forget( PCharacter *, int );
};

#endif
