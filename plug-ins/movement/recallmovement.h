/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __RECALLMOVEMENT_H__
#define __RECALLMOVEMENT_H__

#include "skillreference.h"
#include "jumpmovement.h"

class NPCharacter;

class RecallMovement : public JumpMovement {
public:
    RecallMovement( Character * );
    RecallMovement( Character *, Character *, Room * );

protected:
    virtual bool moveAtomic( );
    virtual void moveFollowers( Character * ); 
    virtual void msgOnStart( );
    virtual void movePet( NPCharacter * ) = 0;

    virtual bool checkBloody( Character * );
            bool checkMount( );
            bool checkShadow( );
            bool checkForsaken( Character * );
            bool checkNorecall( Character * );
            bool checkCurse( Character * );
            bool checkSameRoom( );
            bool checkPumped( );

            bool applyFightingSkill( Character *, SkillReference & );
            bool applyInvis( Character * );
            bool applyMovepoints( );
            bool applyWaitstate( );
};

#endif
