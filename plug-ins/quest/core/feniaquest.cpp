/* Dream Land, 2026 */
#include "feniaquest.h"
#include "questwrapper.h"
#include "questtargets.h"
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
#include "object.h"
#include "room.h"
#include "profflags.h"
#include "behavior.h"
#include "char_weight.h"
#include "loadsave.h"
#include "save.h"

#include "logstream.h"
#include "fenia/exceptions.h"
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

    // OBJECT is what toString() throws on. FUNCTION it would happily accept, and
    // that is worse than a throw: Closure::toString decompiles the thing, so a
    // scenario that assigns a function where it meant to call one would print its
    // own source code to the player.
    if (rc.type == Register::OBJECT || rc.type == Register::FUNCTION) {
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

    // After the script, so onDestroy can still find its own targets. Every C++
    // type does this in its own destroy(); doing it here means no scenario can
    // forget and leave a marked mob wandering the world forever.
    clearMarked( );
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
    // Worded so it stays true when answerString has already complained about a
    // bad type -- "answered nothing" would have contradicted that first message.
    if (text.empty( ))
        throw QuestRuntimeException( typeName + ": onInfo produced no text" );

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

    if (!tryCallType( "onHelpLocation", args, rc ))
        return 0;

    // A room or its vnum, whichever the script finds natural. The unwrap can
    // throw when the answer is some other object, and it happens after
    // tryCallType has returned, i.e. outside its containment -- the same trap
    // that made the step 2 blocker, so it gets its own catch.
    if (rc.type == Register::NUMBER)
        return get_room_instance( rc.toNumber( ) );

    if (rc.type == Register::OBJECT) {
        Room *room = 0;

        try {
            FeniaManager::wrapperManager->getTarget( rc, room );
        } catch (const ::Exception &e) {
            complain( "onHelpLocation", e );
            return 0;
        }

        return room;
    }

    return 0;
}

/*--------------------------------------------------------------------------
 * Target selection
 *
 * Thin wrappers over the C++ quest models, which stay in C++ because a single
 * pass walks the whole mob or object list -- order 10^4 entries with ten checks
 * each. Interpreted, that is a visible stall on a player-driven command; native
 * it is milliseconds. What moves into Fenia is WHICH candidates count, not the
 * walking.
 *
 * Each answers NULL where the model throws, so a scenario can react instead of
 * having its onCreate torn down mid-sentence.
 *------------------------------------------------------------------------*/
NPCharacter * FeniaQuest::selectVictim( PCharacter *pch, const QuestSelectParams &params )
{
    QuestSelectScope scope( this, params );

    try {
        return getRandomVictim( pch );
    } catch (const QuestCannotStartException &) {
        return 0;
    }
}

NPCharacter * FeniaQuest::selectClient( PCharacter *pch, const QuestSelectParams &params )
{
    QuestSelectScope scope( this, params );

    try {
        return getRandomClient( pch );
    } catch (const QuestCannotStartException &) {
        return 0;
    }
}

::Object * FeniaQuest::selectItem( PCharacter *pch, const QuestSelectParams &params )
{
    QuestSelectScope scope( this, params );

    try {
        return getRandomItem( pch );
    } catch (const QuestCannotStartException &) {
        return 0;
    }
}

Room * FeniaQuest::selectClientRoom( PCharacter *pch )
{
    return getRandomRoomClient( pch );
}

Room * FeniaQuest::selectDistantRoom( PCharacter *pch, Room *from, int range )
{
    RoomList rooms;

    findClientRooms( pch, rooms );

    // 20 attempts is what the C++ types that use this pass; it bounds a
    // room_distance BFS per candidate, so it is a real cost, not a formality.
    return getDistantRoom( pch, rooms, from, range, 20 );
}

bool FeniaQuest::isRoomReachable( PCharacter *pch, Room *room )
{
    if (!room)
        return false;

    return targetRoomAccessible( pch, room );
}

/*--------------------------------------------------------------------------
 * Selection filters
 *
 * These run inside the world walk above, once per candidate, so they stay cheap
 * and they are the ONLY place the scenario's knobs are consulted.
 *------------------------------------------------------------------------*/
bool FeniaQuest::passesParams( PCharacter *pch, NPCharacter *mob )
{
    const QuestSelectParams &p = selectParams;

    if (p.levelDiffSet) {
        int diff = mob->getRealLevel( ) - pch->getModifyLevel( );

        if (diff < p.levelDiffMin || diff > p.levelDiffMax)
            return false;
    }

    if (p.maxLevel > 0 && mob->getRealLevel( ) > p.maxLevel)
        return false;

    if (p.noCaster && mob->getProfession( )->getFlags( mob ).isSet( PROF_CASTER ))
        return false;

    if (p.visible && !isMobileVisible( mob, pch ))
        return false;

    if (!p.noBehaviorInHometown.empty( )
        && mob->in_room
        && IS_SET( mob->in_room->area->area_flag, AREA_HOMETOWN ))
    {
        Behavior *bhv = behaviorManager->findExisting( p.noBehaviorInHometown );

        if (bhv && mob->pIndexData->behaviors.isSet( bhv ))
            return false;
    }

    if (!p.vnums.empty( ) && p.vnums.count( mob->pIndexData->vnum ) == 0)
        return false;

    // ANY of the require* that are set is enough, which is StealQuest::isThief's
    // shape: act flag OR behavior OR a name the scenario keeps for itself.
    if (p.requireActFlag != 0 || !p.requireBehavior.empty( )) {
        bool matched = false;

        if (p.requireActFlag != 0 && IS_SET( mob->act, p.requireActFlag ))
            matched = true;

        if (!matched && !p.requireBehavior.empty( )) {
            Behavior *bhv = behaviorManager->findExisting( p.requireBehavior );

            if (bhv && mob->pIndexData->behaviors.isSet( bhv ))
                matched = true;
        }

        if (!matched)
            return false;
    }

    return true;
}

bool FeniaQuest::checkMobileVictim( PCharacter *pch, NPCharacter *mob )
{
    if (!VictimQuestModel::checkMobileVictim( pch, mob ))
        return false;

    return passesParams( pch, mob );
}

bool FeniaQuest::checkMobileClient( PCharacter *pch, NPCharacter *mob )
{
    if (!ClientQuestModel::checkMobileClient( pch, mob ))
        return false;

    return passesParams( pch, mob );
}

bool FeniaQuest::checkItem( PCharacter *pch, ::Object *obj )
{
    static const DLString basicName( "BasicObjectBehavior" );

    if (!ItemQuestModel::checkItem( pch, obj ))
        return false;

    // Never offer something that already has a behavior of its own -- a recipe
    // tome, a generated weapon. markObject refuses those outright, so handing one
    // back would strand the scenario in the middle of building its quest. Not a
    // knob: there is no case for wanting one.
    if (obj->behavior && obj->behavior->getType( ) != basicName)
        return false;

    if (selectParams.visible && !isItemVisible( obj, pch ))
        return false;

    // maxLevel is deliberately NOT applied to the item's own level. No C++ type
    // ever capped it, and with carriedByNpc set below the mob knobs already judge
    // the CARRIER -- one field meaning two different levels depending on another
    // field is how a scenario author ends up debugging the engine.
    //
    // Below: the whole "an NPC could hand this straight back to you" condition,
    // which is what makes a steal-style quest finishable at all. Note the carrier
    // is judged by the client rules, so the mob knobs -- level window, vnum list,
    // require* -- describe the CARRIER, exactly as StealQuest::checkItem does.
    if (selectParams.carriedByNpc) {
        if (!obj->carried_by || !obj->carried_by->is_npc( ))
            return false;

        NPCharacter *carrier = obj->carried_by->getNPC( );

        if (!checkMobileClient( pch, carrier ))
            return false;

        if (!carrier->can_see( obj ))
            return false;

        // Hands almost full: it may not be able to give the item back.
        if (carrier->carry_number >= Char::canCarryNumber( carrier ))
            return false;

        if (Char::getCarryWeight( carrier ) >= Char::canCarryWeight( carrier ))
            return false;
    }

    return true;
}

bool FeniaQuest::checkRoomClient( PCharacter *pch, Room *room )
{
    if (!RoomQuestModel::checkRoomClient( pch, room ))
        return false;

    if (selectParams.roomNoCast && IS_SET( room->room_flags, ROOM_NO_CAST ))
        return false;

    if (!selectParams.excludeAreaName.empty( )
        && selectParams.excludeAreaName == room->areaName( ))
        return false;

    return true;
}

/*--------------------------------------------------------------------------
 * Target marking
 *------------------------------------------------------------------------*/
void FeniaQuest::markMobile( NPCharacter *mob, const DLString &role )
{
    if (!mob)
        return;

    // Refuse anything that already has a mind of its own. Selection can never
    // hand back such a mob -- checkMobile skips hasDestiny() -- but a scenario
    // can pass any character it likes, and marking a questor, a shopkeeper or
    // another hero's target would delete that behavior outright. Marking twice
    // is worse still: setChar snapshots imm_flags and act_flags before setting
    // IMM_SUMMON|IMM_CHARM|ACT_NOEYE, so a second snapshot records them as
    // already set and unsetChar never takes them off again -- permanently, and
    // saved to disk.
    if (mob->behavior && mob->behavior->hasDestiny( ))
        throw Scripting::Exception( "markMob: this mobile already carries a behavior of its own" );

    MobQuestTarget::Pointer target( NEW );

    target->setHeroName( charName );
    target->setQuestType( typeName );
    target->setRole( role );
    target->setChar( mob );
    mob->behavior.setPointer( *target );

    // The behavior is XML-serialized with the mob, so a marked target survives a
    // reboot -- but only if the room it stands in is written out now.
    if (mob->in_room)
        save_mobs( mob->in_room );
}

void FeniaQuest::markObject( ::Object *obj, const DLString &role, bool mandatory )
{
    static const DLString basicName( "BasicObjectBehavior" );

    if (!obj)
        return;

    // Same refusal as markMobile, and it matters more here: unlike mobs, item
    // selection does NOT skip candidates that already carry a behavior, so
    // randomItem can perfectly well hand back a recipe tome or a generated
    // weapon whose behavior this would silently destroy.
    if (obj->behavior && obj->behavior->getType( ) != basicName)
        throw Scripting::Exception( "markObj: this object already carries a behavior of its own" );

    ObjQuestTarget::Pointer target( NEW );

    target->setHeroName( charName );
    target->setQuestType( typeName );
    target->setRole( role );
    target->setMandatory( mandatory );
    target->setObj( obj );
    obj->behavior.setPointer( *target );

    // The object half of what save_mobs does for a marked mob, and it is NOT
    // optional. Objects are written out only when they MOVE (obj_to_room and
    // friends call this themselves), so changing a behavior in place saves
    // nothing: a scenario that places an item and then marks it would leave an
    // unmarked copy on disk, and after a reboot the mark, the [ЦЕЛЬ] tag and
    // every trigger would be gone with no sign anything had happened.
    // StealQuest has always done the equivalent by hand at stealquest.cpp:166.
    save_items_at_holder( obj );
}

NPCharacter * FeniaQuest::findMarkedMobile( const DLString &role )
{
    for (Character *wch = char_list; wch; wch = wch->next) {
        if (!wch->is_npc( ) || !wch->getNPC( )->behavior)
            continue;

        MobQuestTarget *target = dynamic_cast<MobQuestTarget *>( *wch->getNPC( )->behavior );

        if (target
            && target->getHeroName( ) == charName.getValue( )
            && target->questType.getValue( ) == typeName.getValue( )
            && (role.empty( ) || target->role.getValue( ) == role))
            return wch->getNPC( );
    }

    return 0;
}

::Object * FeniaQuest::findMarkedObject( const DLString &role )
{
    for (::Object *obj = object_list; obj; obj = obj->next) {
        if (!obj->behavior)
            continue;

        ObjQuestTarget *target = dynamic_cast<ObjQuestTarget *>( *obj->behavior );

        if (target
            && target->getHeroName( ) == charName.getValue( )
            && target->questType.getValue( ) == typeName.getValue( )
            && (role.empty( ) || target->role.getValue( ) == role))
            return obj;
    }

    return 0;
}

/** Take the marks off, leaving the mobs and objects themselves alone.
 *
 *  clear() on either model swaps the basic behavior back in; neither extracts,
 *  so a quest ending does not make the world it borrowed disappear. A scenario
 *  that wants its props gone extracts them itself in onDestroy, which runs
 *  first. */
void FeniaQuest::clearMarked( )
{
    for (Character *wch = char_list; wch; wch = wch->next) {
        if (!wch->is_npc( ) || !wch->getNPC( )->behavior)
            continue;

        MobQuestTarget *target = dynamic_cast<MobQuestTarget *>( *wch->getNPC( )->behavior );

        if (target
            && target->getHeroName( ) == charName.getValue( )
            && target->questType.getValue( ) == typeName.getValue( ))
            MobileQuestModel::clear( wch->getNPC( ) );
    }

    for (::Object *obj = object_list; obj; obj = obj->next) {
        if (!obj->behavior)
            continue;

        ObjQuestTarget *target = dynamic_cast<ObjQuestTarget *>( *obj->behavior );

        if (target
            && target->getHeroName( ) == charName.getValue( )
            && target->questType.getValue( ) == typeName.getValue( ))
        {
            ItemQuestModel::clear( obj );

            // Unlike the mobile clear, the item one does not save. Without this
            // the stale disk copy keeps the mark and brings it back on the next
            // reboot -- harmless for triggers, since the quest is gone by then,
            // but show() checks only ourHero, so the [ЦЕЛЬ] tag would haunt the
            // item forever.
            save_items_at_holder( obj );
        }
    }
}

bool FeniaQuest::callTargetTrigger( const DLString &methodName, const RegisterList &extraArgs,
                                    Register &rc )
{
    // Contained, always. Every caller is a mob or object event -- death, give,
    // greet, a spec tick -- and not one of them has a catch above it.
    return tryCallType( methodName, extraArgs, rc );
}
