#include "gmcp.h"
#include "telnet.h"
#include "descriptor.h"
#include "logstream.h"

const char C_IAC = static_cast<char>(IAC);
const char C_SB = static_cast<char>(SB);
const char C_GMCP  = static_cast<char>(GMCP);
const char C_SE = static_cast<char>(SE);

void GMCPHandler::sendVersion(Descriptor *d)
{
    LogStream::sendNotice() << "telnet: sending GMCP version" << endl;
    send(d, "Client", "GUI", "0.3\nhttps://dreamland.rocks/img/dl.zip");
}


void GMCPHandler::send(Descriptor *d, const string &package, const string &message, const string &data)
{
    ostringstream buf;

    buf << C_IAC << C_SB << C_GMCP
        << package << "." << message << " " << data
        << C_IAC << C_SE;

    string str = buf.str();
    d->writeFd(str.c_str());
}

