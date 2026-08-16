/* Dream Land, 2026 */
#include "questtargets.h"
#include "feniaquest.h"

#include "feniamanager.h"
#include "wrappermanagerbase.h"
#include "register-impl.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "object.h"
#include "room.h"

#include "merc.h"
#include "act.h"
#include "l10n.h"
#include "def.h"

using namespace Scripting;
using namespace std;

/*--------------------------------------------------------------------------
 * Shared helpers
 *------------------------------------------------------------------------*/
static Register wrapChar( Character *ch )
{
    if (!ch)
        return Register( );

    return FeniaManager::wrapperManager->getWrapper( ch );
}

static Register wrapObj( ::Object *obj )
{
    if (!obj)
        return Register( );

    return FeniaManager::wrapperManager->getWrapper( obj );
}

/*--------------------------------------------------------------------------
 * MobQuestTarget
 *------------------------------------------------------------------------*/
MobQuestTarget::MobQuestTarget( )
{
}

void MobQuestTarget::setRole( const DLString &r )
{
    role.setValue( r );
}

void MobQuestTarget::setQuestType( const DLString &t )
{
    questType.setValue( t );
}

/** The hero's current quest, but only if it is still the one that marked this
 *  mob. heroName alone is not enough: a player who finished a kill quest and
 *  took a steal quest would otherwise have their new quest answering for a mob
 *  the old one left standing. */
::Pointer<FeniaQuest> MobQuestTarget::getFeniaQuest( ) const
{
    ::Pointer<FeniaQuest> quest = getMyQuest<FeniaQuest>( );

    if (quest && quest->getName( ) != questType.getValue( ))
        return ::Pointer<FeniaQuest>( );

    return quest;
}

bool MobQuestTarget::deathAsVictim( Character *killer )
{
    PCMemoryInterface *pcm = getHeroMemory( );
    ::Pointer<FeniaQuest> quest = getFeniaQuest( );

    if (!pcm || !quest)
        return false;

    // Unwrap charm, mirrors and switch before deciding who did this. Without it
    // a hero who sent a charmed pet gets no credit for their own kill.
    killer = quest->getActor( killer );

    DLString how;

    if (ourHero( killer )) {
        quest->state = QSTAT_FINISHED;
        how = "hero";

    } else if (ourHeroGroup( killer )) {
        quest->state = QSTAT_FINISHED;
        how = "group";

    } else {
        if (killer && killer->getNPC( ) != ch)
            how = "other";
        else
            how = "suicide";

        quest->state = QSTAT_BROKEN_BY_OTHERS;
        quest->setTime( pcm, quest->getAccidentTime( pcm ) );
        quest->scheduleDestroy( );
    }

    RegisterList args;
    args.push_back( wrapChar( ch ) );
    args.push_back( wrapChar( killer ) );
    args.push_back( Register( how ) );
    args.push_back( Register( role.getValue( ) ) );

    Register rc;
    quest->callTargetTrigger( "onTargetDeath", args, rc );
    return false;
}

bool MobQuestTarget::deathAsClient( Character *killer )
{
    PCMemoryInterface *pcm = getHeroMemory( );
    ::Pointer<FeniaQuest> quest = getFeniaQuest( );

    if (!pcm || !quest)
        return false;

    if (quest->isComplete( ))
        return false;

    killer = quest->getActor( killer );

    DLString how;

    // A hero who kills the person they were sent to help waits longer than one
    // who merely stood by while somebody else did it. That asymmetry is the
    // whole point of the client role and predates this class by twenty years.
    if (ourHero( killer )) {
        quest->setTime( pcm, quest->getPunishTime( pcm ) );
        quest->state = QSTAT_BROKEN_BY_HERO;
        how = "hero";

    } else {
        quest->setTime( pcm, quest->getAccidentTime( pcm ) );
        quest->state = QSTAT_BROKEN_BY_OTHERS;
        how = "other";
    }

    quest->scheduleDestroy( );

    RegisterList args;
    args.push_back( wrapChar( ch ) );
    args.push_back( wrapChar( killer ) );
    args.push_back( Register( how ) );
    args.push_back( Register( role.getValue( ) ) );

    Register rc;
    quest->callTargetTrigger( "onTargetDeath", args, rc );
    return false;
}

bool MobQuestTarget::death( Character *killer )
{
    if (role.getValue( ) == "victim")
        return deathAsVictim( killer );

    return deathAsClient( killer );
}

void MobQuestTarget::give( Character *victim, ::Object *obj )
{
    ::Pointer<FeniaQuest> quest = getFeniaQuest( );

    if (!quest)
        return;

    // Everyone's gifts reach the script, not only the hero's, with a flag saying
    // which it was. A scenario usually wants to refuse a stranger politely rather
    // than ignore them, and the C++ GreedyClient's silent drop is a large share
    // of the "the mob does not see my item" reports.
    RegisterList args;
    args.push_back( wrapChar( ch ) );
    args.push_back( wrapChar( victim ) );
    args.push_back( wrapObj( obj ) );
    args.push_back( Register( ourHero( victim ) ) );
    args.push_back( Register( role.getValue( ) ) );

    Register rc;
    quest->callTargetTrigger( "onTargetGive", args, rc );
}

void MobQuestTarget::greet( Character *victim )
{
    ::Pointer<FeniaQuest> quest = getFeniaQuest( );

    if (!quest)
        return;

    RegisterList args;
    args.push_back( wrapChar( ch ) );
    args.push_back( wrapChar( victim ) );
    args.push_back( Register( role.getValue( ) ) );

    Register rc;
    quest->callTargetTrigger( "onTargetGreet", args, rc );
}

void MobQuestTarget::speech( Character *victim, const char *speech )
{
    ::Pointer<FeniaQuest> quest = getFeniaQuest( );

    if (!quest)
        return;

    RegisterList args;
    args.push_back( wrapChar( ch ) );
    args.push_back( wrapChar( victim ) );
    args.push_back( Register( DLString( speech ? speech : "" ) ) );
    args.push_back( Register( role.getValue( ) ) );

    Register rc;
    quest->callTargetTrigger( "onTargetSpeech", args, rc );
}

bool MobQuestTarget::specIdle( )
{
    ::Pointer<FeniaQuest> quest = getFeniaQuest( );

    if (!quest)
        return MobQuestBehavior::specIdle( );

    RegisterList args;
    args.push_back( wrapChar( ch ) );
    args.push_back( Register( role.getValue( ) ) );

    Register rc;

    // No handler means this role has no idle behaviour of its own, so the mob
    // keeps whatever wandering and idling it had before it was marked.
    if (!quest->callTargetTrigger( "onTargetSpec", args, rc ))
        return MobQuestBehavior::specIdle( );

    // True means the script did something with this tick and the ordinary idle
    // behaviour should be skipped, same contract as every other specIdle.
    if (rc.type == Register::NUMBER && rc.toBoolean( ))
        return true;

    return MobQuestBehavior::specIdle( );
}

bool MobQuestTarget::extract( bool count )
{
    // Same contract as MandatoryMobile: a target that disappears without being
    // dealt with leaves the quest unfinishable, so declare it broken rather than
    // let the player run out the clock chasing something that is gone.
    mandatoryExtract( );
    return MobQuestBehavior::extract( count );
}

void MobQuestTarget::show( Character *viewer, std::basic_ostringstream<char> &buf )
{
    if (!ourHero( viewer ))
        return;

    // Despite look.cpp's parameter name this is the VIEWER, so the tag renders
    // in their language rather than always in Russian.
    if (role.getValue( ) == "victim")
        buf << fmt( viewer, _("{R[ЦЕЛЬ] {x") );
    else if (role.getValue( ) == "thief")
        buf << fmt( viewer, _("{R[ВОР] {x") );
}

/*--------------------------------------------------------------------------
 * ObjQuestTarget
 *------------------------------------------------------------------------*/
ObjQuestTarget::ObjQuestTarget( )
{
}

void ObjQuestTarget::setRole( const DLString &r )
{
    role.setValue( r );
}

void ObjQuestTarget::setQuestType( const DLString &t )
{
    questType.setValue( t );
}

void ObjQuestTarget::setMandatory( bool value )
{
    mandatory.setValue( value );
}

::Pointer<FeniaQuest> ObjQuestTarget::getFeniaQuest( ) const
{
    ::Pointer<FeniaQuest> quest = getMyQuest<FeniaQuest>( );

    if (quest && quest->getName( ) != questType.getValue( ))
        return ::Pointer<FeniaQuest>( );

    return quest;
}

void ObjQuestTarget::get( Character *victim )
{
    ::Pointer<FeniaQuest> quest = getFeniaQuest( );

    if (!quest)
        return;

    RegisterList args;
    args.push_back( wrapObj( obj ) );
    args.push_back( wrapChar( victim ) );
    args.push_back( Register( ourHero( victim ) ) );
    args.push_back( Register( role.getValue( ) ) );

    Register rc;
    quest->callTargetTrigger( "onTargetGet", args, rc );
}

bool ObjQuestTarget::extract( bool count )
{
    // Only on a counted extract, matching MandatoryItem: the uncounted form is
    // bookkeeping (moving an object between containers), not destruction, and
    // treating it as a loss would break a quest every time its item changed
    // hands.
    if (count && mandatory.getValue( ))
        mandatoryExtract( );

    return ObjQuestBehavior::extract( count );
}

void ObjQuestTarget::show( Character *viewer, ostringstream &buf )
{
    if (!ourHero( viewer ))
        return;

    if (role.getValue( ) == "loot")
        buf << fmt( viewer, _("{R[ЦЕЛЬ] {x") );
}
