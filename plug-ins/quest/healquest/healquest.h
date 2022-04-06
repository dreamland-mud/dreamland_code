/* $Id: healquest.h,v 1.1.2.14.6.3 2008/03/04 09:48:50 rufina Exp $
 *
 * ruffina, 2003
 */
#ifndef HEALQUEST_H
#define HEALQUEST_H

#include "questmodels.h"
#include "questmodels-impl.h"
#include "questregistrator.h"
#include "scenarios.h"

enum {
    HSTAT_INIT,
    HSTAT_ATTEMPT,
    HSTAT_SUCCESS,
    HSTAT_SUCCESS_OTHER
};

class HealMaladies : public XMLMapBase<XMLInteger> {
public:
    void setElement( const SkillReference & );
    bool hasKey( int ) const;
    bool hasKey( const DLString & ) const;
    void setAttempts( int );
    bool checkSuccess( int, NPCharacter * );
    bool checkSuccessOther( int, NPCharacter * );
     int successHero( ) const;
     int successTotal( ) const;
    void cleanup( NPCharacter * );

protected:
    bool checkSuccessAny( int, NPCharacter *, int );
     int countStates( int ) const;
};

class HealQuest : public ClientQuestModel {
XML_OBJECT
public:
    typedef ::Pointer<HealQuest> Pointer;
    
    virtual void create( PCharacter *, NPCharacter * );
    virtual bool isComplete( );
    virtual QuestReward::Pointer reward( PCharacter *, NPCharacter * );
    virtual Room * helpLocation( );
    virtual void info( std::ostream &, PCharacter * );
    virtual void shortInfo( std::ostream &, PCharacter * );
    virtual void destroy( );
    virtual void clear( NPCharacter * );
    
    XML_VARIABLE XMLString  mobName;       
    XML_VARIABLE XMLString  roomName;       
    XML_VARIABLE XMLString  areaName;       
    XML_VARIABLE HealMaladies maladies;
    XML_VARIABLE XMLInteger mode;

protected:
    virtual bool checkRoomClient( PCharacter *, Room * );
    virtual bool checkMobileClient( PCharacter *, NPCharacter * );
};

class HealQuestRegistrator : public QuestRegistrator<HealQuest>,
                             public QuestScenariosContainer
{
XML_OBJECT
public:
    HealQuestRegistrator( );
    virtual ~HealQuestRegistrator( );
    virtual bool applicable( PCharacter *, bool ) const;
};

extern HealQuestRegistrator *registrator;

#endif
