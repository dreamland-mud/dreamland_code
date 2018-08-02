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
    PortalMovement( Character *, Object * );
    
    virtual int move( );

protected:
    virtual bool findTargetRoom( );
    virtual bool canLeaveMaster( Character * );
    virtual void moveOneFollower( Character *, Character * );
    virtual  int getMoveCost( Character * );
    virtual bool moveAtomic( );
    
    virtual void msgOnMove( Character *, bool fLeaving );
    virtual void msgEcho( Character *, Character *, const char * );

    virtual bool canMove( Character * );
    virtual bool checkSafe( Character * );
    virtual bool checkClosedDoor( Character * );
    virtual bool checkWater( Character * );
    virtual bool checkAir( Character * );
            bool checkCurse( Character * );
            bool checkCharges( );

    virtual bool tryMove( Character * );
    virtual bool applyWeb( Character * );
    virtual bool applyMovepoints( Character * );
            bool applySpellbane( Character * );

    bool isNormalExit( );
    
    Object *portal;
};

#endif
