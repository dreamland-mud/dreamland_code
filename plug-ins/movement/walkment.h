/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __WALKMENT_H__
#define __WALKMENT_H__

#include "movement.h"

class Object;

enum {
    RC_MOVE_CLOSED = 50,
    RC_MOVE_AIR,
    RC_MOVE_WATER,
    RC_MOVE_FIGHTING,
    RC_MOVE_RESTING,
    RC_MOVE_WEB,
    RC_MOVE_LAWZONE,
};

class Walkment : public Movement {
public:
    Walkment( Character * );
    
protected:
    virtual bool moveAtomic( );
    virtual bool canLeave( Character * );
    virtual bool canLeaveMaster( Character * );
    virtual void setWaitstate( );

    virtual bool canControlHorse( );
    
    virtual bool canMove( Character * );
    virtual bool checkPosition( Character * );
    virtual bool checkPositionHorse( );
    virtual bool checkPositionRider( );
    virtual bool checkPositionWalkman( );
    virtual bool checkCyclicRooms( Character * );
    virtual bool checkTrap( Character * );
    virtual bool checkVisibility( Character * );
    virtual bool checkLawzone( Character * );
    virtual bool checkClosedDoor( Character * ) = 0;
    virtual int getDoorStatus(Character *) = 0;
    virtual bool checkRoomCapacity( Character * );
    virtual bool checkGuild( Character * );
    virtual bool checkSafe( Character * );
    virtual bool checkAir( Character * );
    virtual bool checkWater( Character * );
    virtual bool checkMovepoints( Character * );

    virtual bool tryMove( Character * );
    virtual bool applyWeb( Character * );
    virtual bool applyCamouflage( Character * );
    virtual bool applyMovepoints( Character * );

    virtual  int getMoveCost( Character * ) = 0;

    virtual void moveFollowers( Character * );
    virtual void moveOneFollower( Character *, Character * ) = 0;
    
    bool autoDismount( Character * );
    void visualize( Character * );

    virtual bool canHear( Character *, Character * );


    bool silence;
    Object *boat;
    int boat_type;
};


#endif
