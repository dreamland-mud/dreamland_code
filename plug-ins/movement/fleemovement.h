/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __FLEEMOVEMENT_H__
#define __FLEEMOVEMENT_H__

#include "exitsmovement.h"
#include "skillreference.h"

class FleeMovement : public ExitsMovement {
public:
    FleeMovement( Character * );

protected:
    virtual bool findTargetRoom( );
    virtual void callProgs( Character * );
    virtual void msgOnMove( Character *, bool );

    virtual bool canMove( Character * );
    virtual bool checkPositionHorse( );
    virtual bool checkPositionRider( );
    virtual bool checkPositionWalkman( );

    bool applySkill( SkillReference & );

    bool canFlee( Character * );
};

#endif
