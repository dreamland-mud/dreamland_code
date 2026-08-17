/* Dream Land, 2026 */
#ifndef QUESTTARGETS_H
#define QUESTTARGETS_H

#include "mobquestbehavior.h"
#include "objquestbehavior.h"
#include "xmlstring.h"
#include "xmlboolean.h"
#include "register-decl.h"

class FeniaQuest;

/** The one mob-side quest target, in place of the seventeen specialized
 *  behaviors the eight C++ types register between them.
 *
 *  What stays here rather than moving to the script is everything that is the
 *  same for every quest ever written: who counts as the killer once charm,
 *  mirrors and switch are unwound, whether that killer was the hero, one of the
 *  hero's group, a stranger or nobody at all, and what each of those does to the
 *  quest timer and state. Eight C++ types already agreed on those rules; making
 *  each scenario restate them in Fenia would be eight chances to get them wrong.
 *  The scenario gets told what happened and writes the prose.
 *
 *  'role' is what a template parameter used to be. 'questType' is checked
 *  alongside heroName so a stale target left on a mob by a previous quest cannot
 *  answer for the current one.
 */
class MobQuestTarget : public virtual MobQuestBehavior {
XML_OBJECT
public:
    typedef ::Pointer<MobQuestTarget> Pointer;

    MobQuestTarget( );

    virtual bool death( Character *killer );
    virtual void give( Character *victim, ::Object *obj );
    virtual void greet( Character *victim );
    virtual void speech( Character *victim, const char *speech );
    virtual bool extract( bool );

    /** The `[ЦЕЛЬ]` / `[ВОР]` tag on the look line.
     *
     *  Deliberately NOT a Fenia hook. look.cpp calls this for every mob in the
     *  room on every look, there is no catch anywhere above it, and a script
     *  throwing here would end the process from the most ordinary command in the
     *  game -- the same shape as the blocker found in step 2. The role picks from
     *  a fixed table instead, rendered in the viewer's own language. */
    virtual void show( Character *viewer, std::basic_ostringstream<char> &buf );

    void setRole( const DLString & );
    void setQuestType( const DLString & );

    XML_VARIABLE XMLString role;
    XML_VARIABLE XMLString questType;

    /** True once this target's own death has given the quest what it was for, or
     *  it was killable by design (gangster/thief) and the death has been reported.
     *
     *  Not persisted: the mob is extracted in the same tick that sets it. It
     *  exists because extract() must not declare the quest broken after a kill
     *  the hero was sent to make -- see the comment on extract(). */
    bool dealtWith;

protected:
    /** The idle half of the AI tick, and ONLY the idle half.
     *
     *  spec() is the dispatcher: in combat it runs the mob's fighting brain, out
     *  of combat its adrenaline, and only a quiet awake mob reaches specIdle.
     *  Overriding spec() instead -- which this class did for one draft -- takes
     *  away every marked target's ability to cast, flee or call for help. */
    virtual bool specIdle( );

    ::Pointer<FeniaQuest> getFeniaQuest( ) const;

    /** How this mob died, as the scenario sees it: "hero", "group", "other" or
     *  "suicide". Also applies the timer and state change each case has always
     *  carried, before the scenario is told. */
    bool deathAsVictim( Character *killer );

    /** A client, i.e. someone the hero was supposed to protect or serve. Killing
     *  one is a failure, and it matters whether the hero did it. */
    bool deathAsClient( Character *killer );

    /** Any other marked role (a gang member to wipe out, a thief to rob): the
     *  engine changes NO quest state or timer and simply reports the death, with
     *  its "hero"/"group"/"other"/"suicide" flavour, to onTargetDeath. The
     *  scenario owns the outcome -- count towards a total, drop loot, or ignore.
     *  Without this a killable-by-design target (gangster, thief) would route
     *  through deathAsClient and break the quest the moment the hero touched it. */
    bool deathAsNeutral( Character *killer );
};

/** The one object-side quest target. Same reasoning as MobQuestTarget.
 *
 *  'mandatory' is the old MandatoryItem: the item cannot quietly vanish while
 *  the quest still needs it, and if it does the quest is declared broken rather
 *  than left unfinishable. Report 31038 is exactly this -- a quest wand
 *  destroyed by a stray area attack while it sat in the target's inventory.
 */
class ObjQuestTarget : public virtual ObjQuestBehavior {
XML_OBJECT
public:
    typedef ::Pointer<ObjQuestTarget> Pointer;

    ObjQuestTarget( );

    virtual void get( Character *victim );
    virtual bool extract( bool );
    virtual void show( Character *viewer, ostringstream &buf );

    void setRole( const DLString & );
    void setQuestType( const DLString & );
    void setMandatory( bool );

    XML_VARIABLE XMLString role;
    XML_VARIABLE XMLString questType;
    XML_VARIABLE XMLBoolean mandatory;

protected:
    ::Pointer<FeniaQuest> getFeniaQuest( ) const;
};

#endif
