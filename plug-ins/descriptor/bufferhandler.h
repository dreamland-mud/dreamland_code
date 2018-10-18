/* $Id$
 *
 * ruffina, 2004
 */
#ifndef BUFFERHANDLER_H
#define BUFFERHANDLER_H

#include "xmlpersistent.h"
#include "xmlvariablecontainer.h"

class Descriptor;

class BufferHandler : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<BufferHandler> Pointer;

    virtual void write(Descriptor *d, const char *txt); 
    virtual bool read(Descriptor *d );
    virtual DLString convert(const char *txt);
};

extern template class XMLStub<BufferHandler>;

#endif
