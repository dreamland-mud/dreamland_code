/* $Id$
 *
 * ruffina, 2004
 */
#ifndef PAGERHANDLER_H
#define PAGERHANDLER_H

#include "descriptor.h"
#include "xmlinteger.h"
#include "xmlstring.h"

class PagerHandler : public InputHandler {
XML_OBJECT
public:
    PagerHandler();
    PagerHandler(const char *str);

    virtual int handle(Descriptor *d, char *arg);
    virtual void prompt(Descriptor *d);

    XML_VARIABLE XMLString str;
    XML_VARIABLE XMLInteger ptr;
};

#endif
