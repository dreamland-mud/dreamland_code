/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __TRANSFERMOVEMENT_H__
#define __TRANSFERMOVEMENT_H__

#include <stdio.h>

#include "descriptorstatelistener.h"
#include "jumpmovement.h"
#include "multimessage.h"

class TransferMovement : public JumpMovement {
friend class TransferListener;
public:
    TransferMovement( Character *, Character *, Room *,
                      const char * = NULL, const char * = NULL,
                      const char * = NULL, const char * = NULL);
    // Trilingual variant: each message is a MultiMessage (built with _() or the
    // explicit en/ru/ua ctor in the caller), rendered per recipient so transfer
    // flavour stops leaking Russian to EN/UA clients. First three are required so
    // this never collides with the message-less 3-arg const-char* callers.
    TransferMovement( Character *, Character *, Room *,
                      const MultiMessage &mrl, const MultiMessage &msl,
                      const MultiMessage &mre, const MultiMessage &mse = MultiMessage() );

protected:
    virtual bool tryMove( Character * );
    virtual void msgEcho( Character *, Character *, const char * );
    virtual void msgOnMove( Character *, bool );

    const char *msgRoomLeave, *msgSelfLeave;
    const char *msgRoomEnter, *msgSelfEnter;

    bool multiLang;
    MultiMessage mmRoomLeave, mmSelfLeave, mmRoomEnter, mmSelfEnter;
};

class SilentTransferMovement : public TransferMovement {
public:
    SilentTransferMovement(Character *, Room *);
};


#endif
