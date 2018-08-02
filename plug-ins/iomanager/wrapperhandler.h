/* $Id$
 *
 * ruffina, 2004
 */
#ifndef WRAPPERHANDLER_H
#define WRAPPERHANDLER_H

#include "descriptor.h"
#include "xmlinteger.h"
#include "xmlstring.h"

class WrapperHandler : public InputHandler {
XML_OBJECT
public:
    virtual int handle(Descriptor *d, char *arg);
    virtual void prompt(Descriptor *d);

    static void init( Descriptor * );
};

#endif
