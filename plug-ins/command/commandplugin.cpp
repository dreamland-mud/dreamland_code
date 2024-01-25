/* $Id: commandplugin.cpp,v 1.1.2.3 2008/03/22 00:07:27 rufina Exp $
 *
 * ruffina, 2007
 */

#include "logstream.h"
#include "commandplugin.h"
#include "commandmanager.h"
#include "feniamanager.h"
#include "fenia/exceptions.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "idcontainer.h"

using namespace Scripting;


void CommandPlugin::initialization( )
{
    getLoader()->loadCommand( Pointer( this ) );
    commandManager->registrate( Pointer( this ) );

    if (FeniaManager::wrapperManager) {
        FeniaManager::wrapperManager->linkWrapper(this);
        if (wrapper)
            LogStream::sendNotice() << "Fenia command: linked wrapper for " << getName() << endl;
    }
}

void CommandPlugin::destruction( )
{
    if (FeniaManager::wrapperManager)
        if (wrapper)
            extractWrapper(false);
    
//    getLoader()->saveCommand( Pointer( this ) );
    commandManager->unregistrate( Pointer( this ) );
}

long long CommandPlugin::getID() const
{
    int myId = 0;

    if (getHelp())
        myId =getHelp()->getID();

    if (myId <= 0)
        throw Scripting::Exception(getName() + ": command ID not found or zero");

    return (myId << 4) | 8;
}

CommandLoader * CommandPlugin::getLoader( ) const
{
    return commandManager;
}

void CommandPlugin::entryPoint( Character *ch, const DLString &constArgs )
{
    // See if there is 'run' method override in Fenia. 
    bool rc = feniaOverride(ch, constArgs);

    // Fall back to the old implementation.
    if (!rc)
        run(ch, constArgs);
}

bool CommandPlugin::feniaOverride(Character *ch, const DLString &constArgs) 
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
