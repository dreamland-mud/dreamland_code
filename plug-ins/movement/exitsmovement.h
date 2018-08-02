/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __EXITSMOVEMENT_H__
#define __EXITSMOVEMENT_H__

#include "walkment.h"

struct extra_exit_data;
struct exit_data;

enum {
    RC_MOVE_PASS_NEVER = 100,
    RC_MOVE_PASS_FAILED,
    RC_MOVE_PASS_NEEDED,
};

class ExitsMovement : public Walkment {
protected:
    ExitsMovement( Character *, int );
public:
    ExitsMovement( Character *, int, int );
    ExitsMovement( Character *, struct extra_exit_data *, int );

protected:
    virtual bool findTargetRoom( );
    virtual bool canMove( Character * );
    virtual void setWaitstate( );
    virtual  int getMoveCost( Character * );
    virtual void moveOneFollower( Character *, Character * );
    virtual void place( Character * );
    
    virtual void msgEcho( Character *, Character *, const char * );
    virtual void msgOnMove( Character *, bool );

    virtual bool checkVisibility( Character * );
    virtual bool checkClosedDoor( Character * );
            bool checkExtraExit( Character * );

    virtual bool tryMove( Character * );
            bool applyPassDoor( Character * );
	     int getPassDoorLevel( Character * );

    void randomizeExits( );
    int adjustMovetype( Character * );
    void init( );

    struct exit_data * pexit;
    struct extra_exit_data * peexit;
    int door;
    int exit_info;
};

#endif
