/* $Id: stealquest.h,v 1.1.2.11.6.3 2008/03/06 17:48:36 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef STEALQUEST_H
#define STEALQUEST_H

#include "questmodels.h"
#include "questmodels-impl.h"
#include "questscenario.h"
#include "questregistrator.h"

#include "xmlmap.h"
#include "xmlvector.h"

#include "wearlocation.h"

struct obj_index_data;

enum {
    QSTAT_HUNT_ROBBER = 1,
};


class StealQuest : public ItemQuestModel, 
                   public VictimQuestModel, 
                   public ClientQuestModel 
{
friend class RobbedVictim;
friend class HiddenChest;
XML_OBJECT
public:
    typedef ::Pointer<StealQuest> Pointer;
    
    StealQuest( );

    virtual void create( PCharacter *, NPCharacter * );
    virtual Reward::Pointer reward( PCharacter *, NPCharacter * );
    virtual bool isComplete( );
    virtual Room * helpLocation( );
    virtual void helpMessage( ostringstream & );
    virtual void info( std::ostream &, PCharacter * );
    virtual void shortInfo( std::ostream &, PCharacter * );
    virtual void destroy( );
    
    XML_VARIABLE XMLInteger mode;
    XML_VARIABLE XMLString victimName;
    XML_VARIABLE XMLString victimArea;
    XML_VARIABLE XMLString victimRoom;
    XML_VARIABLE XMLString thiefName;
    XML_VARIABLE XMLString thiefArea;
    XML_VARIABLE XMLString thiefRoom;
    XML_VARIABLE XMLInteger thiefSex;
    XML_VARIABLE XMLString chestRoom;
    XML_VARIABLE XMLString itemName;
    XML_VARIABLE XMLWearlocationReference itemWear;
    
protected:
    virtual bool checkMobileClient( PCharacter *, NPCharacter * );
    virtual bool checkMobileVictim( PCharacter *, NPCharacter * );
    virtual bool checkItem( PCharacter *, Object * );
    virtual void clear( Object * );
    
private:    
    bool isThief( NPCharacter * );
    void fillChest( PCharacter *, Object * );
    bool isBonus( obj_index_data *, PCharacter * );
    Room * findHideaway( PCharacter *, NPCharacter * );
    DLString getRoomHint( Room * room, Room *from = NULL, int depth = 0 );

    Object *item;
};

class StealQuestRegistrator : public QuestRegistrator<StealQuest> {
XML_OBJECT
public:
    StealQuestRegistrator( );
    virtual ~StealQuestRegistrator( );

    static inline StealQuestRegistrator * getThis( ) {
        return thisClass;
    }
    
    XML_VARIABLE NameList thiefs;
    XML_VARIABLE VnumList bonuses;
    XML_VARIABLE VnumList chests;

private:
    static StealQuestRegistrator * thisClass;
};

#endif
