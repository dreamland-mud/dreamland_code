#ifndef SANDBOXSERVLET_H
#define SANDBOXSERVLET_H

#include "schedulertaskroundplugin.h"

/* Ends an idle /api/sandbox session, and any live one when the comm plugin
 * unloads: the session's capture buffer lives in this .so. */
class SandboxReaper : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<SandboxReaper> Pointer;

    virtual void run( );
    virtual int getPriority( ) const;

protected:
    virtual void destruction( );
};

#endif
