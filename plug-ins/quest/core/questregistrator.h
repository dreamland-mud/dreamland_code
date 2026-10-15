/* $Id: questregistrator.h,v 1.1.4.5.6.1 2007/09/29 19:33:59 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef QUESTREGISTRATOR_H
#define QUESTREGISTRATOR_H

#include "class.h"
#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlmultistring.h"
#include "xmlboolean.h"
#include "wrappertarget.h"
#include "lang.h"

#include "quest.h"
#include "questmanager.h"

class NPCharacter;
class PCharacter;

/** One autoquest type: its config, and the Fenia wrapper that type's scenario
 *  logic hangs off. There is exactly one instance per type, alive for the whole
 *  run, which is what makes it the right thing for Fenia to address -- the
 *  per-player quest instance comes and goes, the type does not.
 *
 *  A type is data: one quests/<Name>.xml file, loaded by QuestTypesPlugin. All
 *  scenario logic lives in dreamland_fenia/autoquest/<type>, so every quest of
 *  every type is a FeniaQuest.
 */
class QuestRegistratorBase : public virtual XMLVariableContainer, public WrapperTarget
{
XML_OBJECT
public:
    typedef ::Pointer<QuestRegistratorBase> Pointer;

    Quest::Pointer createQuest( PCharacter *, NPCharacter * ) const;

    /** The config file's name without .xml, e.g. KillQuest. Also the name
     *  .AutoQuest() looks the type up by and every victory tally is kept under. */
    const DLString& getName( ) const;
    void setName( const DLString & );

    /** Key of this type's entry in the Fenia DB.
     *
     *  Read from <feniaId> in the type's own config, never derived from load
     *  order or list position: the Fenia DB is keyed on it, so an id that moved
     *  when a quest type was added or removed would silently re-point every
     *  script written for one type at another. Low nibble 11 is this class's tag
     *  (1-10 are room, obj, mob, area, spell, affect, skill command, wrapped
     *  command, area quest, behavior). */
    long long getID( ) const;

    /** Whether this type may be offered to pch: the minAutoLevel gate, then
     *  .AutoQuest(name).onApplicable(ch, fAuto) when Fenia defines it. A Fenia
     *  answer of true/false is final. With no answer (hook missing, or a script
     *  error) the type is offered, unless <needsApplicable> is set: then it is
     *  withheld, so a lost binding hides the type instead of handing it to
     *  heroes who cannot finish it. Every caller goes through here. */
    bool isApplicable( PCharacter *, bool fAuto ) const;

    int getPriority( ) const;
    int getMinAutoLevel( ) const;
    const DLString& getShortDescr( lang_t ) const;
    const DLString& getDifficulty( lang_t ) const;

    /** True when the player typed a word from this quest's name in ANY language. */
    bool matchesShortDescr( const DLString & ) const;

    /** Re-attach this type's Fenia wrapper at load time, so triggers written in
     *  Fenia are live without anyone having touched .AutoQuest() first. */
    void linkFeniaWrapper( );

    /** Detach it again, if there is still a Fenia side to detach from. */
    void unlinkFeniaWrapper( );

    /** The raw configured id, 0 when unset. Unlike getID() this never throws,
     *  so it can be used to compare types during load. */
    int getFeniaId( ) const;

    /** True when <needsApplicable> is set: see isApplicable. */
    bool needsFeniaApplicable( ) const;

    /** True when the Fenia wrapper has an onApplicable method bound. */
    bool hasFeniaApplicable( ) const;

protected:
    /** Ask onApplicable. True when Fenia gave a usable answer, stored in answer. */
    bool askFeniaApplicable( PCharacter *, bool fAuto, bool &answer ) const;

    DLString name;

    XML_VARIABLE XMLMultiString shortDesc;
    XML_VARIABLE XMLMultiString difficulty;
    XML_VARIABLE XMLInteger priority;
    XML_VARIABLE XMLIntegerNoEmpty minAutoLevel;
    XML_VARIABLE XMLInteger feniaId;
    XML_VARIABLE XMLBooleanNoFalse needsApplicable;
};

#endif
