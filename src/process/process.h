/* $Id: process.h,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <ostream>

#include "mutex.h"
#include "monitor.h"
#include "thread.h"
#include "schedulertask.h"

using namespace std;

#define SCDP_PROCESS       11000

class ProcessManager : public SchedulerTask
{
public:
    typedef ::Pointer<ProcessManager> Pointer;

    struct RoundRobinElement {
        RoundRobinElement( );
        virtual ~RoundRobinElement( );

        void fromlist( );
        void tolist(RoundRobinElement &l);

        void speenup( );
        void yield( );
        void speendown( );

        virtual void getInfo(ostream &os);

        RoundRobinElement *next, *prev;
        Mutex mux;
        Monitor sync;
    };

public:
    ProcessManager( );
    virtual ~ProcessManager( );

    void yield();

    static inline ProcessManager *getThis() {
        return thisClass;
    }

    virtual void run( );
    virtual void after( );
    virtual int getPriority( );

    RoundRobinElement running, stopped;
private:
    static ProcessManager *thisClass;
};

#endif /* __PROCESS_H__ */
