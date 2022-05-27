/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __QUESTSCENARIO_H__
#define __QUESTSCENARIO_H__

#include "xmlvariablecontainer.h"
#include "xmlmap.h"
#include "xmlstring.h"
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

class QuestScenariosContainer : public virtual XMLVariableContainer {
public:
    typedef XMLMapBase<XMLPointer<QuestScenario> > Scenarios;
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

class QuestItemAppearence : public XMLVariableContainer {
XML_OBJECT
public:
    QuestItemAppearence( );

    XML_VARIABLE XMLStringNoEmpty name;
    XML_VARIABLE XMLStringNoEmpty shortDesc;
    XML_VARIABLE XMLStringNoEmpty desc;
    XML_VARIABLE XMLStringNoEmpty extraDesc;
    XML_VARIABLE XMLFlagsNoEmpty wear;
    XML_VARIABLE XMLFlagsNoEmpty extra;
    XML_VARIABLE XMLStringNoEmpty gender;
    XML_VARIABLE XMLStringNoEmpty material;

    void dress( Object * ) const;
};

typedef XMLVectorBase<QuestItemAppearence> QuestItemAppearanceList;

class QuestMobileAppearence : public XMLVariableContainer {
XML_OBJECT
public:
    QuestMobileAppearence( );

    XML_VARIABLE XMLStringNoEmpty name;
    XML_VARIABLE XMLStringNoEmpty shortDesc;
    XML_VARIABLE XMLStringNoEmpty longDesc;
    XML_VARIABLE XMLStringNoEmpty desc;
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


#endif
