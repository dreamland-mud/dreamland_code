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
 *
 * A separate RPC serves the mudjs settings tab -- separate, because the
 * autobuff handler ignores its arguments, so 'autobuff list' sent to a server
 * without this code would START a buff run instead of being ignored:
 *   autobuff_prefs list                     -> 'autobuff_list' frame
 *   autobuff_prefs set <order> <off> <own> [seq]
 *                                           -> store, then 'autobuff_list' frame
 *                                              carrying seq back, so the tab can
 *                                              tell the answer to its own change
 *                                              from an answer to a plain list
 * Everything the client sends goes to Fenia as plain strings. No structure is
 * built from client JSON: a Fenia map keyed by client-chosen names is exactly
 * the record that corrupts the Fenia DB at boot.
 */
#include "wrapperbase.h"
#include "feniamanager.h"
#include "wrappermanager.h"
#include "reglist.h"
#include "regcontainer.h"
#include "json_utils_ext.h"
#include "descriptor.h"
#include "rpccommandmanager.h"
#include "commandtemplate.h"
#include "logstream.h"
#include "pcharacter.h"
#include "merc.h"
#include "def.h"

using namespace Scripting;

/** Call .tmp.autobuff.<name>(ch, extra...) and hand back what it returned. */
static bool autobuff_fenia(const char *name, PCharacter *pch,
                           const RegisterList &extra, Register &result)
{
    if (!FeniaManager::wrapperManager)
        return false;

    static IdRef ID_TMP( "tmp" ), ID_AUTOBUFF( "autobuff" );
    IdRef ID_FUNC( name );

    try {
        Register tmp = *Context::root[ID_TMP];
        Register ab  = *tmp[ID_AUTOBUFF];
        Register fn  = *ab[ID_FUNC];

        if (fn.type != Register::FUNCTION)
            return false;

        RegisterList fnArgs;
        fnArgs.push_back( FeniaManager::wrapperManager->getWrapper( (Character *)pch ) );
        for (auto &a: extra)
            fnArgs.push_back( a );

        result = fn.toFunction( )->invoke( ab, fnArgs );
        return true;

    } catch (const ::Exception &e) {
        FeniaManager::getThis( )->croak( 0, Register( DLString( "autobuff." ) + name ), e );
    }

    return false;
}

static void run_autobuff(Character *ch)
{
    PCharacter *pch = ch->getPC( );
    if (!pch)
        return;

    Register ignored;
    autobuff_fenia( "run", pch, RegisterList( ), ignored );
}

/** Send the settings tab its list: every candidate buff in the player's order
 *  with its on/off state, and the player's own lines. */
static void send_autobuff_list(PCharacter *pch, const DLString &seq)
{
    if (!pch->desc)
        return;

    Register answer;
    if (!autobuff_fenia( "list", pch, RegisterList( ), answer ))
        return;

    Json::Value body = JsonUtils::fromRegister( answer );
    if (!body.isObject( ))
        return;

    body["who"] = pch->getName( );
    if (!seq.empty( ))
        body["seq"] = seq;

    Json::Value frame;
    frame["command"] = "autobuff_list";
    frame["args"][0] = body;
    pch->desc->writeWSCommand( frame );
}

RPCRUN(autobuff)
{
    run_autobuff(ch);
}

RPCRUN(autobuff_prefs)
{
    PCharacter *pch = ch->getPC( );
    if (!pch || args.empty( ))
        return;

    if (args[0] == "list") {
        send_autobuff_list( pch, DLString::emptyString );
        return;
    }

    if (args[0] == "set" && args.size( ) >= 4) {
        RegisterList extra;
        extra.push_back( Register( args[1] ) );
        extra.push_back( Register( args[2] ) );
        extra.push_back( Register( args[3] ) );

        Register ignored;
        autobuff_fenia( "setPrefs", pch, extra, ignored );
        send_autobuff_list( pch, args.size( ) >= 5 ? args[4] : DLString::emptyString );
    }
}

CMDRUN(buff)
{
    run_autobuff(ch);
}
