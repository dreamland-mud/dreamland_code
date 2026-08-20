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
MobQuestTarget::MobQuestTarget( ) : dealtWith( false )
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
        dealtWith = true;
        how = "hero";

    } else if (ourHeroGroup( killer )) {
        quest->state = QSTAT_FINISHED;
        dealtWith = true;
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

        // Same state and timer either way -- the C++ ProtectedClient made no
        // difference there -- but the scenario is told which, because "the man
        // you were guarding died on his own" and "someone cut him down" want
        // different words.
        if (killer && killer->getNPC( ) != ch)
            how = "other";
        else
            how = "suicide";
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

bool MobQuestTarget::deathAsNeutral( Character *killer )
{
    ::Pointer<FeniaQuest> quest = getFeniaQuest( );

    if (!quest)
        return false;

    if (quest->isComplete( ))
        return false;

    killer = quest->getActor( killer );

    DLString how;

    if (ourHero( killer ))
        how = "hero";
    else if (ourHeroGroup( killer ))
        how = "group";
    else if (killer && killer->getNPC( ) != ch)
        how = "other";
    else
        how = "suicide";

    // Mark the death as accounted for BEFORE the corpse is extracted this same
    // pulse: extract() runs mandatoryExtract() whenever !dealtWith, which would
    // break the quest ("задание уже невозможно выполнить") on the very kill the
    // hero was sent to make. A neutral target that vanishes WITHOUT dying (purge,
    // area cleanup) never reaches here, so dealtWith stays false and that path
    // still correctly declares the quest broken.
    dealtWith = true;

    // No state, timer or scheduleDestroy: this target was meant to be killed.
    // The scenario's onTargetDeath decides what the death means -- count it
    // towards a total, drop a key, or say nothing.
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

    if (role.getValue( ) == "client")
        return deathAsClient( killer );

    // gangster, thief, and any future killable-by-design role.
    return deathAsNeutral( killer );
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
    // A target that vanishes with the quest still wanting it leaves the player
    // chasing nothing, so mandatoryExtract declares the quest broken -- the same
    // contract MandatoryMobile has always had.
    //
    // But NOT after the hero's own kill. mandatoryExtract asks the quest whether
    // it is complete, and for a Fenia quest that question goes to the scenario's
    // onIsComplete. A multi-stage scenario -- kill the beast, then bring the
    // trophy back, which is the first thing anyone will write on this API --
    // answers no, and the state is FINISHED rather than BROKEN_*, so the guard
    // below it would fire and tell the player their quest is impossible during
    // the very kill they were sent to make. The C++ types never saw this because
    // their isComplete IS the state.
    if (!dealtWith)
        mandatoryExtract( );

    return MobQuestBehavior::extract( count );
}

// Roles that carry a visible kill/steal tag. Client/king/prince and the other
// served roles are quest mobs too, but were never tagged and stay untagged.
static bool taggedRole( const DLString &r )
{
    return r == "victim" || r == "gangster" || r == "thief";
}

void MobQuestTarget::show( Character *viewer, std::basic_ostringstream<char> &buf )
{
    bool mine = ourHero( viewer );
    // A groupmate standing with the hero sees the same target, tagged with the
    // hero's name so the whole group knows whose quest it is (Zodda's request).
    bool group = !mine && ourHeroGroup( viewer );
    if (!mine && !group)
        return;

    const DLString &r = role.getValue( );

    // Despite look.cpp's parameter name this is the VIEWER, so the tag renders
    // in their language rather than always in Russian.
    if (r == "victim" || r == "gangster") {
        if (mine)
            buf << fmt( viewer, _("{R[ЦЕЛЬ] {x") );
        else
            buf << fmt( viewer, _("{y[ЦЕЛЬ %1$s] {x"), getHeroName( ).c_str( ) );
    } else if (r == "thief") {
        if (mine)
            buf << fmt( viewer, _("{R[ВОР] {x") );
        else
            buf << fmt( viewer, _("{y[ВОР %1$s] {x"), getHeroName( ).c_str( ) );
    } else if (r == "client") {
        // The awaiting-client hint (Report 7587). The C++ concrete quests tagged
        // their waiting mob per type (RobbedVictim/LocateCustomer/SteakCustomer
        // ::show); the Fenia port marks role "client" and this branch never
        // existed, so the hint vanished for every generated item-return quest.
        // Hero-only, matching the legacy gating. HealQuest also marks "client"
        // but had no legacy marker, so it stays unmarked here. The !isComplete()
        // clause matches legacy too: once the hero hands the item over the mob
        // is no longer waiting, so it falls silent before the walk to turn-in.
        ::Pointer<FeniaQuest> q = getFeniaQuest( );
        if (mine && q && !q->isComplete( )) {
            const DLString &t = questType.getValue( );
            if (t == "StealQuest")
                buf << fmt( viewer, _("{x({YХныкает{x) ") );
            else if (t == "LocateQuest")
                buf << fmt( viewer, _("{x({YЖдет кого-то{x) ") );
            else if (t == "ButcherQuest")
                buf << fmt( viewer, _("{x({YТерпеливо ждет{x) ") );
        }
    }
}

bool MobQuestTarget::isQuestTarget( Character *viewer ) const
{
    return (ourHero( viewer ) || ourHeroGroup( viewer ))
           && taggedRole( role.getValue( ) );
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
    bool mine = ourHero( viewer );
    bool group = !mine && ourHeroGroup( viewer );
    if (!mine && !group)
        return;

    if (role.getValue( ) == "loot") {
        if (mine)
            buf << fmt( viewer, _("{R[ЦЕЛЬ] {x") );
        else
            buf << fmt( viewer, _("{y[ЦЕЛЬ %1$s] {x"), getHeroName( ).c_str( ) );
    }
}
