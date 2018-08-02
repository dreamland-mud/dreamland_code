/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DEFAULTBUFFERHANDLER_H
#define DEFAULTBUFFERHANDLER_H

#include "descriptor.h"
#include "xmlinteger.h"
#include "xmlstring.h"

class DefaultBufferHandler : public BufferHandler {
XML_OBJECT
public:
    typedef ::Pointer<DefaultBufferHandler> Pointer;

    DefaultBufferHandler();
    DefaultBufferHandler(int i);

    virtual void write(Descriptor *d, const char *txt); 
    virtual bool read( Descriptor *d );

    XML_VARIABLE XMLInteger codepage;

private:
    int convertCodepage(char *from, size_t from_length, char *to, size_t to_length);
    int shiftOneLine(Descriptor *d, char *buf);
};

struct DefaultBufferHandlerPlugin : public Plugin {
    typedef ::Pointer<DefaultBufferHandlerPlugin> Pointer;

    virtual void initialization();
    virtual void destruction();
};

#endif
