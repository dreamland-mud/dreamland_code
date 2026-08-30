/*
 * autobuff RPC bridge.
 *
 * The mudjs autobuff button sends rpccmd('autobuff'); the websocket dispatch
 * (descriptor.cpp wsHandlePayload) routes any non-console_in verb here through
 * RpcCommandManager -- OUTSIDE the command interpreter. So 'autobuff' is never a
 * typeable game command: a player typing it reaches console_in -> commandManager
 * and gets "no such command", while the button reaches this handler.
 *
 * The buff logic itself lives in Fenia (.tmp.autobuff.run) so it stays
 * hot-reloadable; this stub only bridges the button press to it, passing the
 * character. Mirrors the .tmp.questreward.modifier call in personalquestreward.cpp.
 */
#include "wrapperbase.h"
#include "feniamanager.h"
#include "wrappermanager.h"
#include "reglist.h"
#include "regcontainer.h"
#include "rpccommandmanager.h"
#include "logstream.h"
#include "pcharacter.h"
#include "merc.h"
#include "def.h"

using namespace Scripting;

RPCRUN(autobuff)
{
    if (!FeniaManager::wrapperManager)
        return;

    PCharacter *pch = ch->getPC( );
    if (!pch)
        return;

    static IdRef ID_TMP( "tmp" ), ID_AUTOBUFF( "autobuff" ), ID_RUN( "run" );

    try {
        Register tmp   = *Context::root[ID_TMP];
        Register ab    = *tmp[ID_AUTOBUFF];
        Register runFn = *ab[ID_RUN];

        if (runFn.type != Register::FUNCTION)
            return;

        RegisterList fnArgs;
        fnArgs.push_back( FeniaManager::wrapperManager->getWrapper( (Character *)ch ) );

        runFn.toFunction( )->invoke( ab, fnArgs );

    } catch (const ::Exception &e) {
        FeniaManager::getThis( )->croak( 0, Register( DLString( "autobuff.run" ) ), e );
    }
}
