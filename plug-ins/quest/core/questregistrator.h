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
#include "xmlattributeplugin.h"
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
 */
class QuestRegistratorBase : public XMLAttributePlugin, public virtual XMLVariableContainer,
                             public WrapperTarget
{
XML_OBJECT
public:
    typedef ::Pointer<QuestRegistratorBase> Pointer;

    virtual Quest::Pointer createQuest( PCharacter *, NPCharacter * ) const = 0;
    virtual const DLString& getName( ) const = 0;

    /** Key of this type's entry in the Fenia DB.
     *
     *  Read from <feniaId> in the type's own config, never derived from load
     *  order or list position: the Fenia DB is keyed on it, so an id that moved
     *  when a quest type was added or removed would silently re-point every
     *  script written for one type at another. Low nibble 11 is this class's tag
     *  (1-10 are room, obj, mob, area, spell, affect, skill command, wrapped
     *  command, area quest, behavior). */
    virtual long long getID( ) const;

    virtual bool applicable( PCharacter *, bool fAuto ) const;
    virtual int getPriority( ) const;
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

    /** True when <engine>fenia</engine> is set in this type's config, meaning
     *  new quests of it are generated as FeniaQuest and their logic is read from
     *  the .AutoQuest() wrapper instead of from C++.
     *
     *  Deliberately a value in the same file every other tunable lives in, so
     *  `quest set reload` flips a type in both directions without a reboot. The
     *  C++ class stays registered either way: quests already in a player's pfile
     *  keep loading and finishing on the class that made them.
     *
     *  🛑 To switch a type BACK, write <engine>cpp</engine> -- do not delete the
     *  node. Reload merges rather than replaces (QuestManager::reload explains
     *  why), so a node that is absent from the file is never read at all and the
     *  member keeps the value it already had. Deleting the line looks like a
     *  rollback and is not one until the next reboot. */
    bool isFeniaEngine( ) const;

protected:
    /** Build a Fenia-backed quest of this type. Out of line and out of the
     *  template so the header need not know about FeniaQuest. */
    Quest::Pointer createFeniaQuest( PCharacter *, NPCharacter * ) const;

    XML_VARIABLE XMLMultiString shortDesc;
    XML_VARIABLE XMLMultiString difficulty;
    XML_VARIABLE XMLInteger priority;
    XML_VARIABLE XMLIntegerNoEmpty minAutoLevel;
    XML_VARIABLE XMLInteger feniaId;
    XML_VARIABLE XMLString engine;
};

template<typename QT>
class QuestRegistrator : public QuestRegistratorBase {
public:
    typedef ::Pointer< QuestRegistrator<QT> > Pointer;

    virtual void initialization( )
    {
        Class::regMoc<QT>( );
        QuestManager::getThis( )->load( this );
        XMLAttributePlugin::initialization( );
        // After load(), because getID() reads feniaId out of the config file.
        // feniaroot is line 9 of plugin.xml and the quest plugins are 28-36, so
        // wrapperManager already exists here; the guard inside is for a reordered
        // profile, where the symptom would otherwise be Fenia triggers that
        // simply never fire.
        linkFeniaWrapper( );
    }

    virtual void destruction( )
    {
        // Before unLoad(), so the wrapper is detached while the type is still
        // registered. Left attached, a `plug reload` would strand it. The Fenia
        // side may already be gone by now -- see unlinkFeniaWrapper.
        unlinkFeniaWrapper( );
        XMLAttributePlugin::destruction( );
        QuestManager::getThis( )->unLoad( this );
        Class::unregMoc<QT>( );
    }

    virtual Quest::Pointer createQuest( PCharacter *pch, NPCharacter *questor ) const
    {
        if (isFeniaEngine( ))
            return createFeniaQuest( pch, questor );

        ::Pointer<QT> quest( NEW );
        quest->create( pch, questor );
        return quest;
    }

    virtual const DLString& getName( ) const 
    {
        return QT::MOC_TYPE;
    }
};

#endif
