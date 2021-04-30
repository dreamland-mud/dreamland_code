/* $Id: schedulerwrapper.h,v 1.1.2.2.18.3 2009/11/04 03:24:31 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef __SCHEDULERWRAPPER_H__
#define __SCHEDULERWRAPPER_H__

#include "xmlvariablecontainer.h"
#include "xmlregister.h"
#include "xmlstring.h"
#include "native.h"
#include "thread.h"
#include "reglist.h"
#include "process.h"

using namespace std;
using Scripting::Register;
using Scripting::RegisterList;

struct PlugLock {
    PlugLock() {
        cnt++;
    }
    ~PlugLock() {
        cnt--;
    }
    static int cnt;
};

struct YieldException : public Scripting::Exception {
    YieldException(const DLString &s) throw(): 
        Scripting::Exception(s)
    {
    }
};

// MOC_SKIP_BEGIN
class MocHack : 
        public Scripting::Context, 
        public ProcessManager::RoundRobinElement
{
};
// MOC_SKIP_END

#if 0
class MocHack { };
#endif


class FeniaProcess : public Thread, 
                     public Scripting::NativeImpl<FeniaProcess>,
                     public Scripting::NativeHandler,
                     public XMLVariableContainer,
                     public MocHack
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<FeniaProcess> Pointer;
    FeniaProcess();
    virtual ~FeniaProcess();

    virtual void before();
    virtual void process();
    virtual void after();

    virtual void getInfo(ostream &os);

    virtual void setSelf(Scripting::Object *s);
    virtual Scripting::Object *getSelf() const { return self; }

    void start();
    void yield(const DLString &msg);
    void stop(const DLString &r);
    static FeniaProcess *currentProcess( );

    XML_VARIABLE XMLString name;
    XML_VARIABLE XMLRegister fun;
    XML_VARIABLE XMLRegister thiz;
    XML_VARIABLE XMLRegisterList args;

    bool running;
    DLString cancel, yielding;
    Scripting::Object *self;
    Register selfRef;
};


class SchedulerWrapper : public Scripting::NativeImpl<SchedulerWrapper>, 
                         public Scripting::NativeHandler,
                         public XMLVariableContainer 
{
    /*XML_OBJECT defenitions hold key method*/
XML_OBJECT
NMI_OBJECT
public:

    virtual void setSelf(Scripting::Object*) { }
    virtual Scripting::Object *getSelf() const { return 0; }

protected:    
    void yield(const DLString &msg);
};

extern template class Scripting::NativeImpl<SchedulerWrapper>;

#endif /* __SCHEDULERWRAPPER_H__ */
