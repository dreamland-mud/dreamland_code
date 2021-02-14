/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __JUMPMOVEMENT_H__
#define __JUMPMOVEMENT_H__

#include "movement.h"

class JumpMovement : public Movement {
public:
    JumpMovement( Character * );
    JumpMovement( Character *, Character *, Room * );

protected:
    virtual bool moveAtomic( );
    virtual bool findTargetRoom( );
    virtual void moveFollowers( Character * );

    void msgRoomNoActor( Character *wch, const char *msg );

    virtual bool canMove( Character * );
    virtual bool tryMove( Character * );

    Character *actor;
};


#endif
