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

protected:
    XML_VARIABLE XMLMultiString shortDesc;
    XML_VARIABLE XMLMultiString difficulty;
    XML_VARIABLE XMLInteger priority;
    XML_VARIABLE XMLIntegerNoEmpty minAutoLevel;
    XML_VARIABLE XMLInteger feniaId;
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
        // registered. Left attached, a `plug reload` would strand it.
        extractWrapper( false );
        XMLAttributePlugin::destruction( );
        QuestManager::getThis( )->unLoad( this );
        Class::unregMoc<QT>( );
    }

    virtual Quest::Pointer createQuest( PCharacter *pch, NPCharacter *questor ) const
    {
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
