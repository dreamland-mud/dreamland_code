/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __TRANSFERMOVEMENT_H__
#define __TRANSFERMOVEMENT_H__

#include <stdio.h>

#include "descriptorstatelistener.h"
#include "jumpmovement.h"

class TransferMovement : public JumpMovement {
friend class TransferListener;
public:
    TransferMovement( Character *, Character *, Room *,
		      const char * = NULL, const char * = NULL, 
		      const char * = NULL, const char * = NULL);

protected:
    virtual bool tryMove( Character * );
    virtual void msgEcho( Character *, Character *, const char * );
    virtual void msgOnMove( Character *, bool );
    
    const char *msgRoomLeave, *msgSelfLeave;
    const char *msgRoomEnter, *msgSelfEnter;
};


#endif
