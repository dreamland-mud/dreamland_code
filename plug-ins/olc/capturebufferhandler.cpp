#include "capturebufferhandler.h"

CaptureBufferHandler::~CaptureBufferHandler()
{
}

void CaptureBufferHandler::write(Descriptor *d, const char *txt)
{
    captured << txt;
}
