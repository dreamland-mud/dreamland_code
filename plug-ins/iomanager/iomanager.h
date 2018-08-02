/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __IOMANAGER_H__
#define __IOMANAGER_H__

#ifndef __MINGW32__
#include <sys/select.h>
#else
#include <winsock.h>
#endif

#include "plugin.h"
#include "schedulertask.h"

class Descriptor;

class IOManager : public Plugin, public virtual DLObject {
    friend class IOInitTask;
    friend class IOPollTask;
    friend class IOReadTask;
    friend class IOWriteTask;
    friend class IOFinalyTask;
public:
    IOManager();
    virtual ~IOManager();

    virtual void initialization();
    virtual void destruction();

    static IOManager *getThis() {
	return thisClass;
    }
protected:
    void ioInit();
    void ioPoll();
    void ioRead();
    void ioWrite();
    void ioFinaly();

private:
    void kick(Descriptor *);

    fd_set in_set;
    fd_set out_set;
    fd_set exc_set;
    
    static IOManager *thisClass;
};

class IOTask : public SchedulerTask, public virtual DLObject { 
public:
    typedef ::Pointer<IOTask> Pointer;
    virtual void after();
};

class IOInitTask : public IOTask {
public:
    typedef ::Pointer<IOInitTask> Pointer;
    
    virtual void run();
    virtual int getPriority( ) const;
};

class IOPollTask : public IOTask {
public:
    typedef ::Pointer<IOPollTask> Pointer;
    
    virtual void run();
    virtual int getPriority( ) const;
};

class IOReadTask : public IOTask {
public:
    typedef ::Pointer<IOReadTask> Pointer;
    
    virtual void run();
    virtual int getPriority( ) const;
};

class IOWriteTask : public IOTask {
public:
    typedef ::Pointer<IOWriteTask> Pointer;
    
    virtual void run();
    virtual int getPriority( ) const;
};

class IOFinalyTask : public IOTask {
public:
    typedef ::Pointer<IOFinalyTask> Pointer;
    
    virtual void run();
    virtual int getPriority( ) const;
};

#endif
