/* Dream Land, 2026 */
#ifndef FENIAQUEST_H
#define FENIAQUEST_H

#include "quest.h"
#include "questmodels.h"
#include "questselectparams.h"
#include "xmlmap.h"
#include "xmlstring.h"
#include "register-decl.h"

class WrapperBase;
class MobQuestTarget;
class ObjQuestTarget;

/** One autoquest whose logic lives in Fenia rather than in C++.
 *
 *  A single class serves every type: what distinguishes a Fenia KillQuest from
 *  a Fenia StealQuest is 'typeName', which names the .AutoQuest() wrapper each
 *  virtual below delegates to. The eight concrete C++ classes stay compiled and
 *  registered meanwhile, so a quest already in flight keeps running on the class
 *  it was created with -- flipping a type is about what gets GENERATED next, not
 *  about rewriting what players already hold.
 *
 *  'vars' is where a Fenia scenario keeps its own state. It has to be a C++
 *  attribute and not a Fenia field on the character: attributes are what the
 *  pfile stores, and the pfile is the only store that survives a reboot AND is
 *  reachable for a player who is offline.
 *
 *  It inherits all three selection models because the search functions are quest
 *  MEMBERS that dispatch through per-quest check* virtuals -- there is no way to
 *  offer target selection to a scenario without being the thing that searches.
 *  A concrete type inherits only the models it uses; this one cannot know which
 *  it will be asked for, so it takes all three. They all derive Quest virtually,
 *  so there is exactly one Quest in the object.
 */
class FeniaQuest : public virtual VictimQuestModel,
                   public virtual ClientQuestModel,
                   public virtual ItemQuestModel {
XML_OBJECT
public:
    typedef ::Pointer<FeniaQuest> Pointer;

    FeniaQuest( );

    /** The TYPE name (KillQuest, StealQuest...), not the C++ class name.
     *
     *  Overridden because everything that counts quests counts them by this
     *  string -- victories, penalties, the "you like this one too much" limit,
     *  `quest set`. Left inherited it would answer "FeniaQuest" for every type
     *  and merge all their tallies into one. */
    virtual const DLString &getName( ) const;

    virtual void create( PCharacter *, NPCharacter * );
    virtual bool isComplete( );
    virtual bool hasPartialRewards( ) const;
    virtual void info( std::ostream &, PCharacter * );
    virtual void shortInfo( std::ostream &, PCharacter * );
    virtual QuestReward::Pointer reward( PCharacter *, NPCharacter * );
    virtual bool help( PCharacter *, NPCharacter * );
    virtual Room *helpLocation( );

    void setTypeName( const DLString & );

    /** Protected on Quest, widened here because the Fenia wrapper needs them.
     *  Either can answer NULL: a renamed or deleted character orphans their
     *  quest exactly as it does today. */
    using Quest::getHeroWorld;
    using Quest::getHeroMemory;

    /*----------------------------------------------------------------------
     * Target selection, as offered to a scenario
     *
     * Each answers NULL rather than throwing when nothing suitable exists, so
     * the script decides what to do about it -- widen the window, try a
     * different shape, or return false from onCreate and let another type have
     * the player. The C++ models signal the same thing by exception; that is
     * caught here and turned into NULL at the boundary.
     *--------------------------------------------------------------------*/
    NPCharacter *selectVictim( PCharacter *, const QuestSelectParams & );
    NPCharacter *selectClient( PCharacter *, const QuestSelectParams & );
    ::Object *selectItem( PCharacter *, const QuestSelectParams & );
    Room *selectClientRoom( PCharacter *, const QuestSelectParams & );
    Room *selectDistantRoom( PCharacter *, Room *from, int range, const QuestSelectParams & );
    bool isRoomReachable( PCharacter *, Room * );

    /*----------------------------------------------------------------------
     * Target marking
     *
     * One generic behavior carries a role instead of a class per target kind,
     * so these take the role as a string rather than a template parameter.
     *--------------------------------------------------------------------*/
    void markMobile( NPCharacter *, const DLString &role );
    /** mandatory: the quest breaks if this item is destroyed while it still
     *  matters. Report 31038 -- a quest wand blown up by a stray area attack
     *  inside the target's inventory, leaving the player chasing nothing. */
    void markObject( ::Object *, const DLString &role, bool mandatory );
    NPCharacter *findMarkedMobile( const DLString &role );
    ::Object *findMarkedObject( const DLString &role );
    void clearMarked( );

    /** The knobs in force for the search currently running. Public because
     *  QuestSelectScope sets them; nothing else should touch them. */
    QuestSelectParams selectParams;

    XML_VARIABLE XMLString typeName;
    XML_VARIABLE XMLMapBase<XMLString> vars;

protected:
    virtual void destroy( );

    /** Applied on top of whatever the C++ model already decided. The models call
     *  these while walking the world, so they must stay cheap. */
    virtual bool checkMobileVictim( PCharacter *, NPCharacter * );
    virtual bool checkMobileClient( PCharacter *, NPCharacter * );
    virtual bool checkItem( PCharacter *, ::Object * );
    virtual bool checkRoomClient( PCharacter *, Room * );

    bool passesParams( PCharacter *, NPCharacter * );

    /** This type's Fenia wrapper, or NULL when the type is unknown or carries
     *  no Fenia side at all. */
    WrapperBase *getTypeWrapper( ) const;

    /** Call methodName on the type wrapper. The script always receives this
     *  quest as its first argument, then extraArgs.
     *
     *  Returns false when no such method is defined, true when one ran. A method
     *  that throws is NOT swallowed: it croaks to the immortals and the exception
     *  comes back out. That is deliberately unlike
     *  FeniaSkillActionHelper::executeMethod, whose swallowing is how a skill can
     *  be dead for years without anyone noticing -- a quest that silently does
     *  nothing looks exactly like a working one. */
    bool callType( const DLString &methodName, const Scripting::RegisterList &extraArgs,
                   Scripting::Register &rc );

    /** callType with the throw contained: for the virtuals whose callers have no
     *  catch, where an exception would leave Scheduler::tick and end the process.
     *  Still loud -- croak plus wiznet plus the log. */
    bool tryCallType( const DLString &methodName, const Scripting::RegisterList &extraArgs,
                      Scripting::Register &rc );

    void complain( const DLString &methodName, const DLString &reason );
    void complain( const DLString &methodName, const ::Exception & );

    /** Read a script's answer without the InvalidCastException that
     *  Register::toBoolean/toString throw on a type they do not handle. The
     *  conversion happens after tryCallType has returned, i.e. outside its
     *  containment, so doing it raw would let a scenario that answers a List kill
     *  the process from a method that promised it could not. */
    bool answerBoolean( const DLString &methodName, const Scripting::Register &, bool fallback );
    DLString answerString( const DLString &methodName, const Scripting::Register & );

public:
    /** Public so MobQuestTarget and ObjQuestTarget can hand their events to the
     *  scenario. They are separate objects living on the mob, not part of this
     *  class, but they speak to the same type wrapper. */
    bool callTargetTrigger( const DLString &methodName, const Scripting::RegisterList &extraArgs,
                            Scripting::Register &rc );
};

/** Holds the selection knobs for exactly one search and takes them away again
 *  however the search ends, exception included.
 *
 *  The C++ model functions take no parameters -- they walk the world calling the
 *  quest's own check* virtuals -- so a member is the only way the knobs reach
 *  those overrides without rewriting questmodels for all eight existing types.
 *  Everything here is synchronous and single-threaded: a search runs to
 *  completion inside one command.
 */
class QuestSelectScope {
public:
    QuestSelectScope( FeniaQuest *q, const QuestSelectParams &p ) : quest( q )
    {
        quest->selectParams = p;
    }

    ~QuestSelectScope( )
    {
        quest->selectParams = QuestSelectParams( );
    }

private:
    FeniaQuest *quest;
};

#endif
