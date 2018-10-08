/* $Id: schedulerwrapper.cpp,v 1.1.2.5.6.3 2009/11/04 03:24:31 rufina Exp $
 *
 * ruffina, 2004
 */

#include <sstream>

#include "schedulerwrapper.h"
#include "logstream.h"
#include "process.h"
#include "feniamanager.h"
#include "dreamland.h"
#include "native.h"
#include "register-impl.h"

int PlugLock::cnt = 0;

using namespace Scripting;
using namespace std;

NMI_INIT(FeniaProcess, "поток")

FeniaProcess::FeniaProcess() : running(false)
{
}

FeniaProcess::~FeniaProcess() 
{
    if(current == this)
        current = FeniaManager::getThis( );

    if(running) {
        LogStream::sendError() 
            << "canceling runnable fenia process! (should not happend)" << endl;
        Thread::cancel();
    } 
}

void
FeniaProcess::before() 
{
    detach();
    speenup( );

    scope = NULL;
    current = this;
}

void
FeniaProcess::after()
{        
    Pointer dummy(this);
    
    if(current == this)
        current = FeniaManager::getThis( );

    running = false;
    fun = Register( );
    thiz = Register( );
    args.clear( );
    selfRef = Register( );

    speendown( );
}

void
FeniaProcess::process() 
{
    try {
        RegisterList rl;

        rl.assign(args.begin(), args.end());
        fun.toFunction()->invoke(thiz, rl);
    } catch(::Exception e) {
        LogStream::sendWarning() << "oops in FeniaProcess::process(): " << e.what( ) << endl;
    }
}

void 
FeniaProcess::setSelf(Scripting::Object *s)
{
    self = s;

    if(self)
        self->dynamicHandler = false;
}

void 
FeniaProcess::getInfo(ostream &os)
{
    os        << "Fenia thread " << (void *)this 
        << " name: " << name.getValue( );
    
    if(running)
        os << " started ";
    else
        os << " stoped ";

    if(currentProcess( ) == this)
        os << "RUNNING";
    else
        os << "yielding: " << yielding;
            
    if(!cancel.empty())
        os << " cancelled: " << cancel;
        
    os << endl;
}

void 
FeniaProcess::yield(const DLString &msg)
{
    yielding = msg;

    current = FeniaManager::getThis( );
    RoundRobinElement::yield( );
    current = this;

    if(dreamland->isShutdown( ))
        throw YieldException("terminate");
    
    if(!cancel.empty()) {
        DLString c = cancel;
        cancel = "";
        throw CustomException( c );
    }
}

void 
FeniaProcess::start() 
{
    selfRef = self;
    if(!running) {
        running = true; // don't start me twice
        mux.lock(); // aquire the lock, so that the child thread wont do anything until we're in wait()
        run(); // spawn a new thread
        sync.wait(); // wait for a notification from the child
        mux.unlock();
    }
}

void
FeniaProcess::stop(const DLString &r) 
{
    if(running)
        cancel = r;
}

FeniaProcess *
FeniaProcess::currentProcess()
{
    if(current == FeniaManager::getThis( ))
        return NULL;

    return (FeniaProcess *)current;
}

/* ---------------------------------------------------------- */
NMI_GET(FeniaProcess, running, "")
{
    return running;
}

NMI_GET(FeniaProcess, thiz, "")
{
    return thiz;
}

NMI_SET(FeniaProcess, thiz, "")
{
    thiz = arg;
}

NMI_GET(FeniaProcess, name, "")
{
    return Register( name );
}

NMI_SET(FeniaProcess, name, "")
{
    name = arg.toString( );
}

NMI_INVOKE(FeniaProcess, start, "")
{
    start();
    return Register( );
}

NMI_INVOKE(FeniaProcess, stop, "")
{
    if(args.empty())
        stop("cancelled");
    else
        stop(args.front().toString());

    return Register( );
}

/* ---------------------------------------------------------- */

template class Scripting::NativeImpl<SchedulerWrapper>;

NMI_INIT(SchedulerWrapper, "планировщик для потоков")

NMI_INVOKE(SchedulerWrapper, Thread, "") {
    if(args.empty( ))
        throw NotEnoughArgumentsException();
    
    FeniaProcess::Pointer fp(NEW);
    fp->args.assign(args.begin(), args.end());
    fp->fun = fp->args.front();
    fp->args.pop_front();
    
    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(fp);

    return Register( obj );
}

NMI_INVOKE(SchedulerWrapper, yield, "") {
    if(args.empty( ))
        yield("<default>");
    else
        yield(args.front( ).toString( ));

    return Register( );
}

void SchedulerWrapper::yield(const DLString &msg) {
    FeniaProcess *pw = FeniaProcess::currentProcess( );

    if(pw == NULL)
        throw YieldException("yielding allowed from fenia process only");

    if(PlugLock::cnt > 0)
        throw YieldException("yielding not allowed from native code");
       
    pw->yield(msg);
}

NMI_INVOKE(SchedulerWrapper, sleep, "") {
    if(args.size( ) != 1)
        throw NotEnoughArgumentsException();

    if (args.front().type != Register::NUMBER)
        throw IllegalArgumentException();

    int delay = args.front().toNumber();

    DLString msg;
    msg << "sleeping " << delay << " pulses";

    for (int i = 0; i < delay; i++)
        yield(msg);
    
    return Register();
}

NMI_INVOKE(SchedulerWrapper, report, "") {
    ProcessManager *pm = ProcessManager::getThis( );
    ProcessManager::RoundRobinElement *i;
    ostringstream os;
    
    os << "------runnable threads---------" << endl;
    for(i=pm->running.next;i != &pm->running; i = i->next) {
        i->getInfo(os);
    }
    
    os << "------stopped threads---------" << endl;
    for(i=pm->stopped.next;i != &pm->stopped; i = i->next) {
        i->getInfo(os);
    }
    os << "------------------------------" << endl;

    return Register( os.str( ) );
}

