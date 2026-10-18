#ifndef EXECCAPTURE_H
#define EXECCAPTURE_H

#include <sstream>
#include "bufferhandler.h"
#include "dlstring.h"

/* A BufferHandler that captures output to a string instead of a socket, for the
 * synthetic actors of /api/exec and /api/sandbox. A self-contained copy of
 * olc/CaptureBufferHandler (whose .cpp lives in the olc plugin) so comm needs no
 * link against olc. */
class ExecCaptureBuffer : public BufferHandler {
public:
    virtual void write(Descriptor *, const char *txt) { captured << txt; }
    DLString getString() const { return captured.str(); }

    /* Everything written since the last take(), then clear. */
    DLString take()
    {
        DLString s = captured.str();
        captured.str("");
        captured.clear();
        return s;
    }

private:
    std::ostringstream captured;
};

#endif
