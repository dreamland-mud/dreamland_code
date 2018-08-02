/* $Id$
 *
 * ruffina, 2004
 */
#ifndef BACKDOORHANDLER_H
#define BACKDOORHANDLER_H

#include "descriptor.h"
#include "xmlinteger.h"
#include "xmlstring.h"

class BackdoorHandler : public InputHandler {
XML_OBJECT
public:
    virtual int handle(Descriptor *d, char *arg);
    virtual void prompt(Descriptor *d);

    static void init( Descriptor * );
};

#endif
