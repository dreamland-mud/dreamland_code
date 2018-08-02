/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "bufferhandler.h"
#include "descriptor.h"

template class XMLStub<BufferHandler>;

/*-------------------------------------------------------------------
 * BufferHandler
 *-------------------------------------------------------------------*/
bool
BufferHandler::read(Descriptor *d)
{
    LogStream::sendError() << "no buffer handler for descriptor" << endl;
    return false;
}

void
BufferHandler::write(Descriptor *d, const char *txt)
{
    LogStream::sendError() << "no buffer handler for descriptor" << endl;
}


