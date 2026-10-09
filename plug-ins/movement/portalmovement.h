/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __PORTALMOVEMENT_H__
#define __PORTALMOVEMENT_H__

#include "walkment.h"

class Object;

class PortalMovement : public Walkment {
public:
    PortalMovement( Character *, Object *, Room *leaderRoom = 0 );
    
    virtual int move( );

protected:
    virtual bool findTargetRoom( );
    virtual bool canLeaveMaster( Character * );
    virtual int moveOneFollower( Character *, Character * );
    virtual  int getMoveCost( Character * );
    virtual bool moveAtomic( );
    virtual void place( Character * );
    
    virtual void msgOnMove( Character *, bool fLeaving );
    virtual void msgEcho( Character *, Character *, const char * );

    virtual bool canMove( Character * );
    virtual bool checkSafe( Character * );
    virtual bool checkClosedDoor( Character * );
    virtual int getDoorStatus(Character *);    
    virtual bool checkWater( Character * );
    virtual bool checkAir( Character * );
            bool checkScripts( Character * );
            bool checkCurse( Character * );
            bool checkCharges( );

    virtual bool tryMove( Character * );
    virtual bool applyWeb( Character * );
    virtual bool applyMovepoints( Character * );
            bool applySpellbane( Character * );
            bool checkOath( Character * );

    bool isNormalExit( );
    Room * pickRandomRoom( );
    
    Object *portal;
    Room *leaderRoom;
};

#endif
