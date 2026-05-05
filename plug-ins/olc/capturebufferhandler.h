#ifndef CAPTUREBUFFERHANDLER_H
#define CAPTUREBUFFERHANDLER_H

#include <sstream>
#include "bufferhandler.h"

/**
 * A BufferHandler that captures all output to an internal string buffer
 * instead of writing to a socket. Used by servlets to capture OLC command output.
 */
class CaptureBufferHandler : public BufferHandler {
public:
    typedef ::Pointer<CaptureBufferHandler> Pointer;

    virtual void write(Descriptor *d, const char *txt) {
        captured << txt;
    }

    DLString getString() const { return captured.str(); }

private:
    std::ostringstream captured;
};

#endif
