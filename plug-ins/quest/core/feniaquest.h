/* Dream Land, 2026 */
#ifndef FENIAQUEST_H
#define FENIAQUEST_H

#include "quest.h"
#include "xmlmap.h"
#include "xmlstring.h"
#include "register-decl.h"

class WrapperBase;

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
 */
class FeniaQuest : public virtual Quest {
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

    XML_VARIABLE XMLString typeName;
    XML_VARIABLE XMLMapBase<XMLString> vars;

protected:
    virtual void destroy( );

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
};

#endif
