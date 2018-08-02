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
    virtual bool applicable( PCharacter * ) = 0;
    virtual bool applicable( PCharacter *, NPCharacter * );
};

class QuestScenariosContainer : public virtual XMLVariableContainer {
public:
    typedef XMLMapBase<XMLPointer<QuestScenario> > Scenarios;
    
    const DLString & getRandomScenario( PCharacter * );
    QuestScenario::Pointer getScenario( const DLString & );

    template<typename S> inline ::Pointer<S> getMyScenario( const DLString & );
    template<typename S> inline void getMyScenarios( PCharacter *, vector< ::Pointer<S> > & );
    template<typename S> inline void getMyScenarios( PCharacter *, NPCharacter *, vector< ::Pointer<S> > & );

protected:
    XML_VARIABLE Scenarios scenarios;
};

template<typename S> 
inline ::Pointer<S> QuestScenariosContainer::getMyScenario( const DLString &name )
{
    return static_cast<S*>(getScenario( name ).getPointer( ));
}

template<typename S> 
inline void QuestScenariosContainer::getMyScenarios( PCharacter *pch, vector< ::Pointer<S> > &list )
{
    Scenarios::iterator i;
    
    for (i = scenarios.begin( ); i != scenarios.end( ); i++)
	if (i->second->applicable( pch ))
	    list.push_back( static_cast<S*>( i->second.getPointer( ) ) );

    if (list.empty( ))
	throw QuestCannotStartException( );
}

template<typename S> 
inline void QuestScenariosContainer::getMyScenarios( PCharacter *pch, NPCharacter *victim, vector< ::Pointer<S> > &list )
{
    Scenarios::iterator i;
    
    for (i = scenarios.begin( ); i != scenarios.end( ); i++)
	if (i->second->applicable( pch ) && i->second->applicable( pch, victim ))
	    list.push_back( static_cast<S*>( i->second.getPointer( ) ) );

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

    void dress( Object * );
};

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
    
    void dress( NPCharacter * );
};

struct VnumList : public XMLReverseVector<XMLInteger> {
    int randomVnum( );
    Object * randomItem( );
};

typedef XMLReverseVector<XMLString> StringList;

struct NameList : public StringList {
    bool hasName( NPCharacter * );
};


#endif
