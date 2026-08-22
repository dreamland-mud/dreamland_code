#include "logstream.h"
#include "wrappedcommand.h"
#include "plugininitializer.h"
#include "feniamanager.h"
#include "fenia/exceptions.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "idcontainer.h"

using namespace Scripting;

// Stable, stdlib-independent 64-bit FNV-1a over the command's English name.
// Mirrors word_effect_hash in languages/core/wordeffect.cpp: used so a command
// that shares another command's help article (and therefore has no help id of
// its own) still gets a wrapper id that stays constant across reboots and
// rebuilds, keeping any persisted Fenia runFunc override bound.
static unsigned long long command_name_hash( const DLString &s )
{
    unsigned long long h = 1469598103934665603ULL;
    for (size_t i = 0; i < s.size( ); i++) {
        h ^= (unsigned char)s[i];
        h *= 1099511628211ULL;
    }
    return h;
}

void WrappedCommand::linkWrapper()
{
    if (FeniaManager::wrapperManager) {
        // getID() no longer throws: a command with no help id of its own falls
        // back to a stable name-hash id (see getID()), so commands that share a
        // help article (pourout/fill under pour's 1075) link their wrapper too.
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

    if (myId > 0)
        return (myId << 4) | 8;

    // The command has no help id of its own because it shares another command's
    // help article -- e.g. 'pourout' and 'fill' sit under 'pour's help (id 1075)
    // so players see all the liquid subcommands in one place. Fall back to a
    // stable hash of the English command name so the command still gets a unique
    // Fenia wrapper id without splitting the shared help. Mirrors
    // WordEffect::getID(). Low nibble stays 8 (the command tag, disjoint from
    // every other wrapper type); the 56-bit hash keeps the value positive and,
    // being large, stays clear of the small help-id ids ((help_id<<4)|8, help_id
    // < 2^13). Any residual cross-command collision is detected and logged at
    // boot in WrappersPlugin::linkTargets.
    unsigned long long h = command_name_hash(getName()) & 0x00FFFFFFFFFFFFFFULL;
    return (long long)((h << 4) | 8);
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
