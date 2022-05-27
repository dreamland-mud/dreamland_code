/* $Id: questor.h,v 1.1.2.1 2009/02/07 17:05:55 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef QUESTOR_H
#define QUESTOR_H

#include "wanderer.h"
#include "objectbehaviormanager.h"

class Quest;
class QuestRegistratorBase;
class QuestReward;

class Questor : public Wanderer {
XML_OBJECT
public:
        typedef ::Pointer<Questor> Pointer;        
        Questor( );
        
        virtual int getOccupation( );
        virtual void give( Character *victim, Object *obj );
        
        virtual bool canGiveQuest( Character * );
        void doRequest( PCharacter *, const DLString & );
        void doComplete( PCharacter *, DLString & );
        void doCancel( PCharacter * );
        void doFind( PCharacter * );
        
        virtual bool canWander( Room *const, EXIT_DATA * );
        virtual bool canWander( Room *const, EXTRA_EXIT_DATA * );
        virtual bool canWander( Room *const, Object * );

private:
        void giveReward(PCharacter *client, ::Pointer<Quest> &quest, ::Pointer<QuestReward> &reward);
        void rewardScroll( PCharacter * );
        void rewardWord( PCharacter * );
        ::Pointer<QuestRegistratorBase> parseQuestArgument(PCharacter *pch, const DLString &arg);
};

class QuestScrollBehavior : public BasicObjectBehavior {
XML_OBJECT
public:
    typedef ::Pointer<QuestScrollBehavior> Pointer;
    
    virtual bool examine( Character * );
    virtual bool hasTrigger( const DLString &  );

    void addSkill( int, int );
    void setOwner( PCharacter * );
    void createDescription( PCharacter * );

protected:
    bool isOwner( Character * ) const;

    XML_VARIABLE XMLMapBase<XMLInteger> skills;
    XML_VARIABLE XMLString ownerName;
    XML_VARIABLE XMLLongLong ownerID;
};

#endif

