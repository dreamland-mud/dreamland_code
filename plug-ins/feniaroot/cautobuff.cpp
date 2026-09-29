/*
 * autobuff bridges: the mudjs button and the typeable 'buff' command.
 *
 * The mudjs autobuff button sends rpccmd('autobuff'); the websocket dispatch
 * (descriptor.cpp wsHandlePayload) routes any non-console_in verb through
 * RpcCommandManager, outside the command interpreter. Telnet and screen-reader
 * players have no button, so CMDRUN(buff) gives them the same entry point.
 *
 * The buff logic itself lives in Fenia (.tmp.autobuff.run) so it stays
 * hot-reloadable; both stubs only hand the character to it. Mirrors the
 * .tmp.questreward.modifier call in personalquestreward.cpp.
 */
#include "wrapperbase.h"
#include "feniamanager.h"
#include "wrappermanager.h"
#include "reglist.h"
#include "regcontainer.h"
#include "rpccommandmanager.h"
#include "commandtemplate.h"
#include "logstream.h"
#include "pcharacter.h"
#include "merc.h"
#include "def.h"

using namespace Scripting;

static void run_autobuff(Character *ch)
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

RPCRUN(autobuff)
{
    run_autobuff(ch);
}

CMDRUN(buff)
{
    run_autobuff(ch);
}
