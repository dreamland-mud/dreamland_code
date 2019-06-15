/* $Id$
 *
 * ruffina, 2004
 */
#ifndef QUESTMODELS_H
#define QUESTMODELS_H

#include "quest.h"

class VnumList;
struct obj_index_data;
struct mob_index_data;
struct area_data;

typedef vector<area_data *> AreaList;
typedef vector<Room *> RoomList;

/*
 * RoomQuestModel
 */
class RoomQuestModel : public virtual Quest {
friend struct DoorFunc;
friend struct ExtraExitFunc;
friend struct PortalFunc;
public:

protected:
    virtual bool checkRoom( PCharacter *, Room * );
    virtual bool checkRoomVictim( PCharacter *, Room *, NPCharacter * );
    virtual bool checkRoomClient( PCharacter *, Room * );
    bool checkRoomForTraverse(PCharacter *, Room *);
    void findClientRooms( PCharacter *, RoomList & );
    void findClientRooms( PCharacter *, RoomList &, const VnumList & );
    Room * getDistantRoom( PCharacter *, RoomList &, Room *, int, int );
    Room * getRandomRoomClient( PCharacter * );
    bool mobileCanAggress(PCharacter *, NPCharacter *);
    bool targetRoomAccessible(PCharacter *, Room *);
    static Room * getRandomRoom( RoomList & );
    AreaList findAreas(PCharacter *);
    RoomList findClientRooms(PCharacter *pch, struct area_data *targetArea);
    RoomList findVictimRooms(PCharacter *pch, struct area_data *targetArea);
};

/*
 * ItemQuestModel
 */
class ItemQuestModel : public virtual RoomQuestModel {
public:
    typedef vector<Object *> ItemList;
    
protected:
    virtual void destroy( Object * );
    virtual void clear( Object * );
    virtual bool checkItem( PCharacter *, Object * );
    virtual bool isItemVisible( Object *, PCharacter * );
    Object * getRandomItem( PCharacter * );

    template<typename T> inline Object * createItem( int );
    template<typename T> inline Object * createItem( obj_index_data * );
    template<typename T> inline void assign( Object * );
    template<typename T> inline Object * getItemWorld( );
    template<typename T> inline Object * getItemList( Object * );
    template<typename T> inline ItemList getItemsList( Object * );
    template<typename T> inline void clearItems( );
    template<typename T> inline void clearItem( );
    template<typename T> inline void destroyItems( );
    template<typename T> inline void destroyItem( );
};

/*
 * MobileQuestModel
 */
class MobileQuestModel : public virtual RoomQuestModel {
public:
    typedef vector<NPCharacter *> MobileList;
    typedef map<mob_index_data *, MobileList> MobIndexMap;

protected:
    virtual void destroy( NPCharacter * );
    virtual void clear( NPCharacter * );
    virtual bool checkMobile( PCharacter *, NPCharacter * );
    virtual bool isMobileVisible( NPCharacter *, PCharacter * );
    static NPCharacter * getRandomMobile( MobileList & );
    static mob_index_data * getRandomMobIndex( MobIndexMap & );

    template<typename T> inline NPCharacter * createMobile( int );
    template<typename T> inline NPCharacter * createMobile( mob_index_data * );
    template<typename T> inline void assign( NPCharacter * );
    template<typename T> inline void clearMobiles( );
    template<typename T> inline void clearMobile( );
    template<typename T> inline void destroyMobiles( );
    template<typename T> inline void destroyMobile( );
    template<typename T> inline NPCharacter * getMobileWorld( );
    template<typename T> inline NPCharacter * getMobileRoom( Room * );
    template<typename T> inline Room * findMobileRoom( );
};

/*
 * ClientQuestModel
 */
class ClientQuestModel : public virtual MobileQuestModel {
protected:
    virtual bool checkMobileClient( PCharacter *, NPCharacter * );
    void findClients( PCharacter *, MobileList & );
    void findClients( PCharacter *, MobIndexMap & );
    NPCharacter * getRandomClient( PCharacter * );
};

/*
 * VictimQuestModel
 */
class VictimQuestModel : public virtual MobileQuestModel {
protected:
    virtual bool checkMobileVictim( PCharacter *, NPCharacter * );
    void findVictims( PCharacter *, MobileList & );
    void findVictims( PCharacter *, MobIndexMap & );
    NPCharacter * getRandomVictim( PCharacter * );
};

#endif

