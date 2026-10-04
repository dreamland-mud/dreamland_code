/* $Id$
 *
 * ruffina, 2004
 */
#ifndef INTERPRETHANDLER_H
#define INTERPRETHANDLER_H

#include "descriptor.h"
#include "xmlinteger.h"
#include "xmlstring.h"

class InterpretHandler : public InputHandler {
XML_OBJECT
public:

    static void init( Descriptor * );
    /* Room echo + wiznet answering the net-death line close() prints, for C++
     * paths that hand a body already in the world to a new descriptor.
     * takeover: the body still had a live descriptor that was just evicted. */
    static void announceReconnect( Character *, bool takeover );
    
    virtual int handle(Descriptor *d, char *arg);
    virtual void prompt(Descriptor *d);
    virtual void close( Descriptor * );

protected:
    void webPrompt(Descriptor *d, Character *ch);
    void normalPrompt( Character * );
    void battlePrompt( Character * );
};

#endif
