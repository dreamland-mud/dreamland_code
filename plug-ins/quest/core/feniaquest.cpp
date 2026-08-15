/* Dream Land, 2026 */
#include "feniaquest.h"
#include "questwrapper.h"
#include "questmanager.h"
#include "questregistrator.h"
#include "questexceptions.h"

#include "feniamanager.h"
#include "wrapperbase.h"
#include "wrappermanagerbase.h"
#include "register-impl.h"
#include "lex.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "room.h"

#include "logstream.h"
#include "merc.h"
#include "def.h"

using namespace Scripting;
using namespace std;

FeniaQuest::FeniaQuest( )
{
}

const DLString & FeniaQuest::getName( ) const
{
    return typeName;
}

void FeniaQuest::setTypeName( const DLString &name )
{
    typeName.setValue( name );
}

/*--------------------------------------------------------------------------
 * Delegation
 *------------------------------------------------------------------------*/
WrapperBase * FeniaQuest::getTypeWrapper( ) const
{
    QuestRegistratorBase::Pointer reg
        = QuestManager::getThis( )->findQuestRegistrator( typeName );

    if (!reg)
        return 0;

    return reg->getWrapper( );
}

bool FeniaQuest::callType( const DLString &methodName, const RegisterList &extraArgs, Register &rc )
{
    WrapperBase *wrapper = getTypeWrapper( );

    if (!wrapper)
        return false;

    IdRef methodId( methodName );
    Register method;

    if (!wrapper->triggerFunction( methodId, method ))
        return false;

    // Built only once the method is known to exist. shortInfo and isComplete run
    // on every command of every player holding a quest, and a type that defines
    // neither should not be allocating a wrapper object each time round.
    RegisterList args;
    args.push_back( QuestWrapper::wrap( this ) );

    for (RegisterList::const_iterator a = extraArgs.begin( ); a != extraArgs.end( ); a++)
        args.push_back( *a );

    try {
        rc = method.toFunction( )->invoke( Register( wrapper->getSelf( ) ), args );

    } catch (const ::Exception &e) {
        // Croak first, so the immortals get the Fenia backtrace, then let the
        // exception out. This is where we part company with
        // FeniaSkillActionHelper::executeMethod, which stops at the croak: a
        // swallowed error leaves the caller believing the method worked, and a
        // quest that silently does nothing looks exactly like one that works.
        FeniaManager::getThis( )->croak( wrapper, methodId, e );
        throw QuestRuntimeException( typeName + "." + methodName + ": " + e.what( ) );
    }

    return true;
}

bool FeniaQuest::tryCallType( const DLString &methodName, const RegisterList &extraArgs, Register &rc )
{
    try {
        return callType( methodName, extraArgs, rc );

    } catch (const ::Exception &e) {
        complain( methodName, e );
        return false;
    }
}

void FeniaQuest::complain( const DLString &methodName, const DLString &reason )
{
    DLString message = methodName + ": " + reason;

    LogStream::sendError( ) << "Fenia quest " << typeName << " for " << charName
                            << ": " << message << endl;
    wiznet( "error", "%s", message.c_str( ) );
}

void FeniaQuest::complain( const DLString &methodName, const ::Exception &e )
{
    complain( methodName, DLString( e.what( ) ) );
}

/*--------------------------------------------------------------------------
 * Reading what a script answered
 *
 * Register::toBoolean() throws for anything that is not a number or a string,
 * and toString() throws for an object. Calling either straight on a return value
 * would undo the containment below, because the conversion happens AFTER
 * tryCallType has returned: a scenario whose onIsComplete ends up answering a
 * List -- the most ordinary mistake there is in a language with no types -- would
 * throw out of a method whose callers do not catch, and the process would end.
 * Worse, the quest is in the pfile, so the player's next command after the
 * restart would do it again.
 *------------------------------------------------------------------------*/
bool FeniaQuest::answerBoolean( const DLString &methodName, const Register &rc, bool fallback )
{
    switch (rc.type) {
    case Register::NONE:
        // No answer at all is not a mistake: it means "use the default".
        return fallback;

    case Register::NUMBER:
    case Register::STRING:
        return rc.toBoolean( );

    default: {
        // Named local, not the expression inline: Exception has an implicit
        // constructor from std::string, so a bare concatenation makes the two
        // complain() overloads ambiguous.
        DLString reason = DLString( "answered " ) + rc.getTypeName( )
                          + " where true or false was expected";

        complain( methodName, reason );
        return fallback;
    }
    }
}

DLString FeniaQuest::answerString( const DLString &methodName, const Register &rc )
{
    if (rc.type == Register::NONE)
        return DLString::emptyString;

    if (rc.type == Register::OBJECT) {
        DLString reason = DLString( "answered " ) + rc.getTypeName( )
                          + " where a line of text was expected";

        complain( methodName, reason );
        return DLString::emptyString;
    }

    return rc.toString( );
}

/*--------------------------------------------------------------------------
 * The virtuals
 *
 * Which of these may throw is not a matter of taste. An exception leaving a
 * quest method travels up through Scheduler::tick, which rethrows, to
 * DreamLand::run, which does not catch, and main() exits -- taking the game down
 * for everyone. So a method throws only where its caller has a catch:
 *
 *   create   -> both `quest request` paths catch QuestCannotStartException
 *   info     -> CQuest::autoQuestInfo catches and tells the player to cancel
 *   reward   -> Questor::doComplete catches and hands nothing out
 *
 * Everything else contains its own errors and returns something safe. Contained
 * is not the same as silent: complain() puts every one on wiznet and in the log.
 *------------------------------------------------------------------------*/
void FeniaQuest::create( PCharacter *pch, NPCharacter *questman )
{
    // Before the call. QuestManager::generate only attaches this quest to the
    // player once create() has returned, so charName is the script's only way
    // back to its own hero while it is still being built.
    charName.setValue( pch->getName( ) );
    state.setValue( QSTAT_INIT );

    RegisterList args;
    args.push_back( FeniaManager::wrapperManager->getWrapper( (Character *)pch ) );
    args.push_back( FeniaManager::wrapperManager->getWrapper( (Character *)questman ) );

    Register rc;
    bool called = false;

    try {
        called = callType( "onCreate", args, rc );

    } catch (const ::Exception &e) {
        complain( "onCreate", e );
        throw QuestCannotStartException( typeName );
    }

    // The type says <engine>fenia</engine> but nobody has written the script.
    // Loud, because the alternative is a questor that mysteriously refuses.
    if (!called) {
        LogStream::sendError( ) << "Fenia quest " << typeName
                                << ": engine is fenia, but onCreate is not defined" << endl;
        wiznet( "failed", "onCreate is not defined for this type" );
        throw QuestCannotStartException( typeName );
    }

    // A script answering false has decided this player cannot have this quest
    // right now. Not an error: generate() drops the type and picks another, and
    // `quest request <name>` retries twice more. Anything that is not an answer
    // at all means proceed -- the quest is already built by this point, and the
    // `quest request <name>` path catches only QuestCannotStartException, so a
    // raw toBoolean() here would escape it.
    if (!answerBoolean( "onCreate", rc, true ))
        throw QuestCannotStartException( typeName );
}

void FeniaQuest::destroy( )
{
    RegisterList args;
    Register rc;

    // Contained: destroy runs from eraseAttribute, reached from the 60s timer
    // tick and from quest completion. Neither catches.
    tryCallType( "onDestroy", args, rc );
}

bool FeniaQuest::isComplete( )
{
    RegisterList args;
    Register rc;

    // Contained, and the fallback is what nearly every C++ type does anyway, so
    // a scenario that just sets state and defines no onIsComplete works.
    if (!tryCallType( "onIsComplete", args, rc ))
        return state == QSTAT_FINISHED;

    return answerBoolean( "onIsComplete", rc, state == QSTAT_FINISHED );
}

bool FeniaQuest::hasPartialRewards( ) const
{
    // const_cast because the whole delegation path hands this quest to Fenia,
    // which can write to it. Only BigQuest answers true today.
    FeniaQuest *self = const_cast<FeniaQuest *>( this );

    RegisterList args;
    Register rc;

    if (!self->tryCallType( "onHasPartialRewards", args, rc ))
        return false;

    return self->answerBoolean( "onHasPartialRewards", rc, false );
}

void FeniaQuest::info( std::ostream &buf, PCharacter *ch )
{
    if (isComplete( )) {
        infoComplete( buf, ch );
        return;
    }

    RegisterList args;
    args.push_back( FeniaManager::wrapperManager->getWrapper( (Character *)ch ) );

    Register rc;

    if (!callType( "onInfo", args, rc ))
        throw QuestRuntimeException( typeName + ": onInfo is not defined" );

    DLString text = answerString( "onInfo", rc );

    // Empty is broken, not quiet: this method IS the quest description, and the
    // caller's catch turns a throw into "your quest is broken, cancel it".
    if (text.empty( ))
        throw QuestRuntimeException( typeName + ": onInfo answered nothing" );

    buf << text << endl;
}

void FeniaQuest::shortInfo( std::ostream &buf, PCharacter *ch )
{
    if (isComplete( )) {
        shortInfoComplete( buf, ch );
        return;
    }

    RegisterList args;
    args.push_back( FeniaManager::wrapperManager->getWrapper( (Character *)ch ) );

    Register rc;

    // Contained: the only caller is the web prompt, built on every command.
    if (tryCallType( "onShortInfo", args, rc ))
        buf << answerString( "onShortInfo", rc );
}

QuestReward::Pointer FeniaQuest::reward( PCharacter *ch, NPCharacter *questman )
{
    QuestReward::Pointer r( NEW );

    RegisterList args;
    args.push_back( FeniaManager::wrapperManager->getWrapper( (Character *)ch ) );
    args.push_back( FeniaManager::wrapperManager->getWrapper( (Character *)questman ) );
    args.push_back( QuestRewardWrapper::wrap( r ) );

    Register rc;

    // Throws on purpose. Containing it here would pay an all-zero reward and
    // still take the quest away; doComplete's catch leaves the quest in place so
    // the player can try again once the script is fixed.
    if (!callType( "onReward", args, rc ))
        throw QuestRuntimeException( typeName + ": onReward is not defined" );

    return r;
}

bool FeniaQuest::help( PCharacter *ch, NPCharacter *questman )
{
    RegisterList args;
    args.push_back( FeniaManager::wrapperManager->getWrapper( (Character *)ch ) );
    args.push_back( FeniaManager::wrapperManager->getWrapper( (Character *)questman ) );

    Register rc;

    // True means the script handled `quest find` itself and doFind should stop;
    // false falls through to the standard speedwalk built from helpLocation().
    if (!tryCallType( "onHelp", args, rc ))
        return false;

    return answerBoolean( "onHelp", rc, false );
}

Room * FeniaQuest::helpLocation( )
{
    RegisterList args;
    Register rc;

    // Answers a room VNUM rather than a room. Turning a Fenia room wrapper back
    // into a Room* needs feniaroot, and feniaroot links this library, so a number
    // it is: `return .tmp.quest.victimRoom(quest).vnum;`.
    if (!tryCallType( "onHelpLocation", args, rc ) || rc.type != Register::NUMBER)
        return 0;

    return get_room_instance( rc.toNumber( ) );
}
