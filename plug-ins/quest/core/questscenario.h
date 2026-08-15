/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __QUESTSCENARIO_H__
#define __QUESTSCENARIO_H__

#include "xmlvariablecontainer.h"
#include "xmlmap.h"
#include "xmlstring.h"
#include "xmlmultistring.h"
#include "xmlvector.h"
#include "multimessage.h"
#include "xmlflags.h"
#include "xmlinteger.h"
#include "xmlreversevector.h"
#include "xmlenumeration.h"
#include "race.h"
#include "questexceptions.h"

class Object;
class NPCharacter;
class PCharacter;

class QuestScenario : public virtual XMLPolymorphVariable {
public:
    typedef ::Pointer<QuestScenario> Pointer;

    virtual ~QuestScenario( );
    virtual bool applicable( PCharacter * ) const = 0;
    virtual bool applicable( PCharacter *, NPCharacter * ) const;
    virtual int getPriority() const;
};

/** Scenario table, keyed by scenario name.
 *
 * Exists only to drop its previous contents before reading. XMLMapBase merges
 * by key (xmlmap.h:38-45) instead of replacing, which is invisible at boot but
 * wrong for an in-game config reload: a scenario deleted from the data file
 * would keep being picked until the next reboot. Vectors already clear
 * themselves in XMLVectorBase::fromXML and a scenario pointer is reallocated
 * per read by XMLPolymorphPointer::fromXML, so the map is the one container in
 * this plugin that needed it.
 */
// MOC_SKIP_BEGIN
// Hidden from moc: its parser cannot read a templated base class and dies with
// "unexpected character: <" on the whole header. Nothing here needs moc anyway,
// the map is reached through the XML_VARIABLE member below.
class QuestScenarioMap : public XMLMapBase<XMLPointer<QuestScenario> > {
public:
    virtual void fromXML( const XMLNode::Pointer & );
};
// MOC_SKIP_END

class QuestScenariosContainer : public virtual XMLVariableContainer {
public:
    typedef QuestScenarioMap Scenarios;
    typedef vector<QuestScenario> ScenarioList;
    
    const DLString & getRandomScenario( PCharacter * ) const;
    const DLString & getWeightedRandomScenario( PCharacter * ) const;
    QuestScenario::Pointer getScenario( const DLString & ) const;

    template<typename S> inline const S * getMyScenario( const DLString & ) const;
    template<typename S> inline void getMyScenarios( PCharacter *, vector< ::Pointer<S> > & ) const;
    template<typename S> inline void getMyScenarios( PCharacter *, NPCharacter *, vector< ::Pointer<S> > & ) const;

protected:
    XML_VARIABLE Scenarios scenarios;
};

template<typename S> 
inline const S * QuestScenariosContainer::getMyScenario( const DLString &name ) const
{
    return getScenario(name).getConstPointer<S>( );
}

template<typename S> 
inline void QuestScenariosContainer::getMyScenarios( PCharacter *pch, vector< ::Pointer<S> > &list ) const
{
    Scenarios::const_iterator i;
    
    for (i = scenarios.begin( ); i != scenarios.end( ); i++)
        if (i->second->applicable( pch ))
            list.push_back( static_cast<const S*>( i->second.getPointer( ) ) );

    if (list.empty( ))
        throw QuestCannotStartException( );
}

template<typename S> 
inline void QuestScenariosContainer::getMyScenarios( PCharacter *pch, NPCharacter *victim, vector< ::Pointer<S> > &list ) const
{
    Scenarios::const_iterator i;
    
    for (i = scenarios.begin( ); i != scenarios.end( ); i++)
        if (i->second->applicable( pch ) && i->second->applicable( pch, victim ))
            list.push_back( static_cast<const S*>( i->second.getPointer( ) ) );

    if (list.empty( ))
        throw QuestCannotStartException( );
}

/** Appearance data a quest stamps onto a freshly created item.
 *
 * The text fields are XMLMultiString so a quest item reads in the viewer's
 * language. Data files may keep writing a bare <name>текст</name> with no 'l'
 * attribute: XMLMultiString::fromXML files an attribute-less Cyrillic node
 * under RU, so nothing has to be rewritten to keep working.
 */
class QuestItemAppearence : public XMLVariableContainer {
XML_OBJECT
public:
    QuestItemAppearence( );

    XML_VARIABLE XMLMultiString name;
    XML_VARIABLE XMLMultiString shortDesc;
    XML_VARIABLE XMLMultiString desc;
    XML_VARIABLE XMLMultiString extraDesc;
    XML_VARIABLE XMLFlagsNoEmpty wear;
    XML_VARIABLE XMLFlagsNoEmpty extra;
    XML_VARIABLE XMLStringNoEmpty gender;
    XML_VARIABLE XMLStringNoEmpty material;

    void dress( Object * ) const;
};

typedef XMLVectorBase<QuestItemAppearence> QuestItemAppearanceList;

/** Same contract as QuestItemAppearence, for a quest-spawned mobile. */
class QuestMobileAppearence : public XMLVariableContainer {
XML_OBJECT
public:
    QuestMobileAppearence( );

    XML_VARIABLE XMLMultiString name;
    XML_VARIABLE XMLMultiString shortDesc;
    XML_VARIABLE XMLMultiString longDesc;
    XML_VARIABLE XMLMultiString desc;
    XML_VARIABLE XMLEnumeration sex;
    XML_VARIABLE XMLEnumerationNoEmpty align;
    XML_VARIABLE XMLRaceReference race;
    
    void dress( NPCharacter * ) const;
};

typedef XMLVectorBase<QuestMobileAppearence> QuestMobileAppearanceList;


struct VnumList : public XMLReverseVector<XMLInteger> {
    int randomVnum( );
    Object * randomItem( );
};

typedef XMLReverseVector<XMLString> XMLStringVector;

struct NameList : public XMLStringVector {
    bool hasName( NPCharacter * );
};

/** One line of quest narration, in every language.
 *
 * A container wrapping an XMLMultiString rather than an XMLMultiString put
 * straight into a vector: XMLVectorBase builds a NEW element for every <node>
 * child it meets, while XMLMultiString is written to have fromXML called once
 * per sibling on ONE object (see the comment on XMLMultiString::fromXML), so
 * swapping a vector's element type would turn twenty lines into sixty, each two
 * thirds empty. Wrapped in a container, moc dispatches all three
 * <text l="..."> children onto the same member. Same shape and same reason as
 * PieceDescription in the rainbow gquest.
 *
 * NameList above must stay an XMLStringVector: it is matched against mob names,
 * not displayed, so it has no business being per-language.
 */
class QuestMessage : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<QuestMessage> Pointer;

    /** Read a legacy bare <node>текст</node> into the Russian slot, so existing
     *  data files keep working untouched and a rollback cannot blank them. */
    virtual void fromXML( const XMLNode::Pointer & );

    XML_VARIABLE XMLMultiString text;
};

typedef XMLVectorBase<QuestMessage> QuestMessageList;

/** Bridge a data-file XMLMultiString into a message that resolves per recipient.
 *
 * The three-language MultiMessage ctor stores the slots verbatim and does no
 * catalog lookup, which is what quest data wants: the translations live in the
 * data file, not in the phrase catalog. Same idiom as social.cpp:93. */
MultiMessage questMessage( const XMLMultiString & );


#endif
