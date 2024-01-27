#include "logstream.h"
#include "wrappedcommand.h"
#include "plugininitializer.h"
#include "feniamanager.h"
#include "fenia/exceptions.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "idcontainer.h"

using namespace Scripting;

void WrappedCommand::linkWrapper()
{
    if (FeniaManager::wrapperManager) {
        FeniaManager::wrapperManager->linkWrapper(this);
        if (wrapper)
            LogStream::sendNotice() << "Fenia command: linked wrapper for " << getName() << endl;
    }
}

void WrappedCommand::unlinkWrapper()
{
    if (FeniaManager::wrapperManager)
        if (wrapper)
            extractWrapper(false);    
}

long long WrappedCommand::getID() const
{
    int myId = 0;

    if (getHelp())
        myId =getHelp()->getID();

    if (myId <= 0)
        throw Scripting::Exception(getName() + ": command ID not found or zero");

    return (myId << 4) | 8;
}

void WrappedCommand::entryPoint( Character *ch, const DLString &constArgs )
{
    // See if there is 'runFunc' method override in Fenia. 
    bool rc = feniaOverride(ch, constArgs);

    // Fall back to the old implementation.
    if (!rc)
        run(ch, constArgs);
}

bool WrappedCommand::feniaOverride(Character *ch, const DLString &constArgs) 
{
    // Find method defined on the wrapper.
    WrapperBase *wrapperBase = getWrapper();
    if (!wrapperBase)
        return false;

    IdRef methodId("runFunc");
    Register method;
    if (!wrapperBase->triggerFunction(methodId, method))
        return false;

    // Invoke the 'run' function
    try {
        RegisterList args;
        args.push_back(FeniaManager::wrapperManager->getWrapper(ch));
        args.push_back(constArgs);

        method.toFunction()->invoke(Register(wrapperBase->getSelf()), args);

    } catch (const ::Exception &e) {
        // On error, complain to the logs and to all immortals in the game.
        FeniaManager::getThis()->croak(0, methodId, e);
    }

    return true;
}
