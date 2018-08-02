/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __MOVEMENT_H__
#define __MOVEMENT_H__

class Character;
class Room;

enum {
    RC_MOVE_UNDEF = -1,
    RC_MOVE_OK = 0,
    RC_MOVE_FAIL,
};

class Movement {
public:
    Movement( Character * );
    
    virtual int move( );

protected:
    virtual bool moveRecursive( );
    virtual bool findTargetRoom( ) = 0;
    virtual bool moveAtomic( ) = 0;
    virtual bool canLeave( Character * );

    virtual void callProgs( Character * );
    virtual void callProgsFinish( Character * );

    virtual void moveFollowers( Character * ) = 0;

    virtual bool canOrderHorse( );

    virtual void place( Character * );
    virtual void msgOnMove( Character *, bool ) = 0;
    
    virtual bool canHear( Character *, Character * );
    virtual void msgEcho( Character *, Character *, const char * );
    void msgSelfParty( Character *, const char *, const char * );
    void msgSelfRoom( Character *, const char *, const char * );
    void msgRoomNoParty( Character *, const char * );
    void msgRoomNoParty( Character *, const char *, const char * );
    void msgSelfMaster( Character *, const char *, const char * );
    void msgSelf( Character *, const char * );
    void msgRoom( Character *, const char * );

    Character * ch;
    Character * horse;
    Character * rider;

    Room * from_room;
    Room * to_room;
    int movetype;
    int rc;
};

#endif
