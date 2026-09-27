/* Chat panel support for the web client: what the connection needs to know. */
#ifndef __CHATWEB_H__
#define __CHATWEB_H__

#include "descriptorstatelistener.h"

/** A character entering or leaving the world, told to a subscribed panel.
 *
 *  The subscription itself is the connection's and does not move: what changes
 *  is whether there is anybody to hear anything. A panel that is not told would
 *  keep the previous character's conversations on screen after a quit-and-login
 *  on the same socket, which is the one case a chat history must never get
 *  wrong. */
class ChatWebStateListener : public DescriptorStateListener {
public:
    typedef ::Pointer<ChatWebStateListener> Pointer;

    virtual void run( int, int, Descriptor * );
};

#endif
