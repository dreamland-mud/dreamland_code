/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DEFAULTBUFFERHANDLER_H
#define DEFAULTBUFFERHANDLER_H

#include "descriptor.h"
#include "xmlinteger.h"
#include "xmllonglong.h"
#include "xmlstring.h"
#include "xmllist.h"

class OutputEntry : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<OutputEntry> Pointer;

    XML_VARIABLE XMLLongLong seq;
    XML_VARIABLE XMLString text;
};

class DefaultBufferHandler : public BufferHandler {
XML_OBJECT
public:
    typedef ::Pointer<DefaultBufferHandler> Pointer;

    DefaultBufferHandler();
    DefaultBufferHandler(int i);

    virtual void write(Descriptor *d, const char *txt); 
    virtual bool read( Descriptor *d );
    virtual DLString convert(const char *txt);

    const XMLListBase<OutputEntry> & getOutputSince(long long startSeq) const;
    long long getCurrentSeq() const;

    XML_VARIABLE XMLInteger codepage;
    XML_VARIABLE XMLLongLong seqCounter;
    XML_VARIABLE XMLListBase<OutputEntry> outputLog;

    static const int MAX_OUTPUT_LINES = 200;

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
