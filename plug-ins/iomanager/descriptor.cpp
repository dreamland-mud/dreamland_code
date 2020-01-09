/* $Id$
 *
 * ruffina, 2004
 */
#include <errno.h>
#include <fcntl.h>

#include <algorithm>
#include <openssl/sha.h>

#ifndef __MINGW32__
#include <arpa/inet.h>
#include <sys/socket.h>
#else
#include <winsock.h>

#ifndef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif
#endif

#include "json/json.h"

#include "rpccommandmanager.h"
#include "iconvmap.h"
#include "logstream.h"
#include "descriptor.h"
#include "serversocketcontainer.h"
#include "wrapperhandler.h"
#include "nannyhandler.h"
#include "defaultbufferhandler.h"
#include "pagerhandler.h"
#include "codepage.h"
#include "comm.h"
#include "outofband.h"

#include "char.h"
#include "dreamland.h"
#include "ban.h"
#include "descriptor.h"
#include "pcharacter.h"
#include "objectbehavior.h"
#include "room.h"
#include "object.h"
#include "mercdb.h"
#include "loadsave.h"
#include "interp.h"
#include "telnet.h"
#include "colour.h"
#include "wiznet.h"
#include "act.h"
#include "vnum.h"
#include "def.h"
#include "base64.h"
#include "webeditor.h"

using namespace Scripting;

template class EventHandler<WebEditorSaveArguments>;

static IconvMap utf2koi("utf-8", "koi8-r//IGNORE");

static const char *MSG_FLUSH_BUF = "Buffer flushed.\r\n";
const char *lid = "\n\r*** PUT A LID ON IT!!! ***\n\r";

/*
 * Negotiated client terminal type.
 */
enum {
    TTYPE_NONE = 0,
    TTYPE_MUDLET,
    TTYPE_MAX
};
const char *TTYPE_NAMES[TTYPE_MAX] = { "none", "Mudlet" };
int ttype_lookup( const char *received )
{
    for (int i = 0; i < TTYPE_MAX; i++) {
        const char *ttype = TTYPE_NAMES[i]; 
        if (strncmp(received, ttype, strlen(ttype)) == 0) {
            LogStream::sendNotice() << "telnet: received " << ttype << " terminal type" << endl;
            return i;
        }
    }
    return TTYPE_NONE;
}
const char *ttype_name( int ttype )
{
    return TTYPE_NAMES[URANGE(TTYPE_NONE, ttype, TTYPE_MAX-1)];
}

bool Descriptor::checkStopSymbol( )
{
    if (!character)
        return false;

    if (!character->is_npc() 
        && character->getPC( )->getAttributes( ).isAvailable( "speedwalk" ))
        return false;
    
    if(inptr > 0)
        switch(inbuf[inptr-1]) {
            case '\r':
            case '\n':
                break;
            default:
                return false;
        }
    
    inptr = 0;
    *incomm = 0;

    writeRaw((const unsigned char *)MSG_FLUSH_BUF, strlen(MSG_FLUSH_BUF));
    return true;
}

int Descriptor::inputChar( unsigned char i )
{
    if(i == '\\' && checkStopSymbol( ))
        return i;
    
    if(inptr < sizeof(inbuf)) {
        inbuf[inptr++] = i;
        return i;
    } 

    inbuf[sizeof(inbuf) - 1] = 0;
    LogStream::sendWarning( ) << host << " input overflow: " << inbuf << endl;
    writeRaw((const unsigned char*)lid, strlen(lid));
    return -1;
}


int Descriptor::inputTelnet( unsigned char i )
{
    switch(telnet.state) {
        case TNS_NORMAL:
            switch(i) {
                case IAC:
                    telnet.state = IAC;
                    break;
                default:
                    if(inputChar( i ) < 0)
                        return -1;
            }
            break;

        case IAC:
            switch(i) {
                case DO:
                case DONT:
                case WILL:
                case WONT:
                    telnet.state = i;
                    break;
                    
                case SB:
                    telnet.sn_ptr = 0;
                    telnet.state = TNS_SUBNEG;
                    break;
                    
                case IAC:
                    if(inputChar( i ) < 0)
                        return -1;
                    
                    /* FALL THROUGH */
                default:
                    telnet.state = TNS_NORMAL;
            }
            break;

        case TNS_SUBNEG:
            switch(i) {
                case IAC:
                    telnet.state = TNS_SUBNEG_IAC;
                    break;
                    
                default:
                    telnet.subneg[telnet.sn_ptr++] = i;
                    
                    if(telnet.sn_ptr > sizeof(telnet.subneg) - 5) {
                        LogStream::sendError() << "telnet: subneg overflow" << endl;
                        telnet.sn_ptr = 0;
                        telnet.state = TNS_NORMAL;
                    }
                    break;
            }
            break;
            
        case TNS_SUBNEG_IAC:
            switch(i) {
                case IAC:
                    telnet.subneg[telnet.sn_ptr++] = IAC;
                    
                    if(telnet.sn_ptr > sizeof(telnet.subneg) - 5) {
                        LogStream::sendError() << "telnet: subneg overflow" << endl;
                        telnet.sn_ptr = 0;
                        telnet.state = TNS_NORMAL;
                    } else
                        telnet.state = TNS_SUBNEG;
                    break;
                    
                case SE:
                    if(telnet.sn_ptr >= 5 && 
                            telnet.subneg[0] == TELOPT_VIA)
                    {
                        telnet.subneg[telnet.sn_ptr] = 0;
                        
                        ViaRecord v;
                        
                        v.first = *(in_addr*)(telnet.subneg + 1);
                        v.second = (char*)telnet.subneg + 1 + sizeof(in_addr);
                        
                        via.push_back(v);
                        
                        LogStream::sendError() 
                            << "got via: " 
                            << v.second << "(" << inet_ntoa(v.first) << ")" 
                            << endl;
                        
                        if (banManager->checkVerbose( this, BAN_ALL )) 
                            return -1;
                    }

                    if (telnet.sn_ptr >= 3 
                            && telnet.subneg[0] == TELOPT_TTYPE
                            && telnet.subneg[1] == TELQUAL_IS)
                    {
                        telnet.subneg[telnet.sn_ptr] = 0;
                        telnet.ttype = ttype_lookup((const char *)telnet.subneg + 2);
                    }
                    telnet.state = TNS_NORMAL;
                    break;
            }
            break;
            
        case DO:
            switch (i) {
#ifdef MCCP
                case TELOPT_COMPRESS:
                case TELOPT_COMPRESS2:
                    startMccp(i);
                    break;
#endif
                case GMCP:
                    outOfBandManager->run("protoInit", ProtoInitArgs(this, "GMCP"));
                    break;
            }
            telnet.state = TNS_NORMAL;
            break;

        case WILL:
            switch (i) {
                case TELOPT_TTYPE:
                    static const unsigned char ttype_qry_str[] = { 
                        IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE };
                    writeRaw(ttype_qry_str, sizeof(ttype_qry_str));
                    break;
            }
            telnet.state = TNS_NORMAL;
            break;

        case DONT:
            switch (i) {
#ifdef MCCP
                case TELOPT_COMPRESS:
                case TELOPT_COMPRESS2:
                    if (compressing == i)
                        stopMccp();
                    break;
#endif
            }
            telnet.state = TNS_NORMAL;
            break;

        default:
            LogStream::sendError() << "telnet: unknown state " << i << endl;
            telnet.state = TNS_NORMAL;
    }

    return i;
}


#ifdef __MINGW32__
static int
hasmore(int fd)
{ 
    fd_set fds;
    timeval tv = { 0, 0 };

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    return select(fd+1, &fds, 0, 0, &tv);
}
#endif

bool 
Descriptor::wsHandlePassThrough(unsigned char *buf, int rc)
{
    for(int i=0;i<rc;i++)
        if (!noTelnet()) {
            if(inputTelnet( buf[i] ) < 0)
                return false;
        } else {
            if(inputChar( buf[i] ) < 0)
                return false;
        }

    return true;
}


static void
translate(Json::Value &val)
{
    if(val.isString()) {
        val = utf2koi(val.asString());
        return;
    }

    for(Json::Value::iterator i=val.begin();i!=val.end();i++) {
        translate(*i);
    }
}

static int 
mprog_editorsave(Character *ch, const DLString &text)
{
    FENIA_CALL(ch, "EditorSave", "s", text.c_str());
    return 0;
}

RPCRUN(editor_save)
{
    DLString text = args[0];

    PCharacter *pch = ch->getPC();

    if(pch) {
        pch->getAttributes().handleEvent(WebEditorSaveArguments(pch, text));
        LogStream::sendError() << "editor_save: " << text << endl;
        mprog_editorsave(pch, text);
    }
}

bool 
Descriptor::wsHandlePayload(const Json::Value &cmd)
{
    DLString name = cmd["command"].asString();
    std::vector<DLString> args;

    for(Json::Value::const_iterator i=cmd["args"].begin(); i != cmd["args"].end();i++) {
        args.push_back(DLString((*i).asString()));
    }

    // special case as we don't need Character to run console-in rpc
    if(name == "console_in") {
        string arg = cmd["args"][0].asString();
        for(string::iterator i = arg.begin();i != arg.end();i++)
            if(inputChar(*i) < 0)
                return false;
    } else if(character) {
        RpcCommandManager::getThis()->run(character, name, args);
    } else {
        LogStream::sendError() << "no character for non-console-in RPC" << endl;
    }
        
    return true;
}


static int 
read_uint(std::vector<unsigned char>::iterator begin, std::vector<unsigned char>::iterator end, int l)
{
    int rc = 0;

    if(l == 0)
        return rc;

    for(;l > 0;l--) {
        if(begin == end)
            return -1;

        rc <<= 8;
        rc |= *begin++;
    }

    return rc;
}

bool 
Descriptor::wsHandleFrame(unsigned char *buf, int rc)
{

    websock.frame.insert(websock.frame.end(), buf, buf + rc);

    for(;;) {
        if(websock.frame.size() < 2) {
            return true;
        }

        int hlen = 2, len = websock.frame[1] & 0x7f;

        if(len == 126) {
            len = read_uint(websock.frame.begin() + 2, websock.frame.end(), 2);
            hlen += 2;
        } else if(len == 127) {
            len = read_uint(websock.frame.begin() + 2, websock.frame.end(), 8);
            hlen += 8;
        }

        if(websock.frame[1] & 0x80) {
            hlen += 4;
        }

        if(len < 0)
            return true;
        
        if(websock.frame.size() < (size_t)(len + hlen))
            return true;
     
        switch(websock.frame[0] & 0xf) {
            case 0: // continuation
            case 1: // text?! (we never negotiated text)
            case 2: // binary
                if(websock.frame[1] & 0x80) // Apply mask
                    for(int i = hlen, j = 0; i < hlen + len;i++, j=((j+1) % 4))
                        websock.frame[i] ^= websock.frame[hlen - 4 + j];

                websock.payload.insert(websock.payload.end(), websock.frame.begin() + hlen, websock.frame.begin() + hlen + len);
                
                if(websock.frame[0] & 0x80) { // fin
                    string str(websock.payload.begin(), websock.payload.end());
                    std::vector<unsigned char>().swap(websock.payload);

                    Json::Value val;
                    Json::Reader reader;
                    
                    if(!reader.parse(str, val)) {
                        LogStream::sendError() << "Failed to parse WebSocket payload: " << reader.getFormattedErrorMessages() << endl 
                                                << str << endl;
                        return false;
                    }

                    if(!val.isObject()) {
                        LogStream::sendError() << "WebSocket RPC: expected object:" << endl
                                                 << str << endl;
                        return false;
                    }

                    translate(val);

                    if(!wsHandlePayload(val))
                        return false;
                }
                break;
            case 8: // close
                writeWebSock(0x8, &websock.payload[0], websock.payload.size()); // close
                LogStream::sendNotice( ) << "WebSock: received close from " << host << endl;
                return false;
            case 9: // ping
                writeWebSock(0xa, &websock.payload[0], websock.payload.size()); // pong
                std::vector<unsigned char>().swap(websock.payload);
                LogStream::sendNotice( ) << "WebSock: responding to ping from " << host << endl;
                break;
            default:
                LogStream::sendError( ) << "WebSock: opcode " << (websock.frame[0] & 0xf) << " is not supported" << endl;
                return false;
        }

        std::vector<unsigned char>(websock.frame.begin() + hlen + len, websock.frame.end()).swap(websock.frame);
    }

}

static void
sendVersion(Descriptor *d)
{
    Json::Value val;
    val["command"] = "version";
    val["args"][0] = "DreamLand Web Client/1.10";
    val["args"][1] = d->websock.nonce;
    LogStream::sendError( ) << "WebSock: sending server version" << endl;
    d->writeWSCommand(val);
}

bool 
Descriptor::wsHandleNeg(unsigned char *buf, int rc)
{
    websock.frame.insert(websock.frame.end(), buf, buf + rc);

    for(std::vector<unsigned char>::iterator i = websock.frame.begin();i != websock.frame.end();i++)
        switch(*i) {
            case '\0':
                LogStream::sendError( ) << "WebSock: negotiation failed: found null character in the header" << endl;
                return false;
            case '\r':
                websock.frame.erase(i);
                return wsHandleNeg(NULL, 0);
            case '\n':
                if(websock.cmd.empty()) {
                    websock.cmd = std::string(websock.frame.begin(), i);

                    LogStream::sendError( ) << " cmd=" << websock.cmd << endl;
                } else if(websock.frame.begin() != i) {
                    std::vector<unsigned char>::iterator colon = std::find(websock.frame.begin(), i, (unsigned char)':');
                    if(colon == i-1 || *(colon+1) != ' ') {
                        LogStream::sendError( ) << "WebSock: negotiation failed: no colon in the header" << endl;
                        return false;
                    }
                    DLString key(std::string(websock.frame.begin(), colon)), val(std::string(colon+2, i));

                    LogStream::sendError( ) << "WebSock: hdr: " << key << ": '" << val << "'" << endl;
                    websock.headers[key] = val;
                } else {
                    unsigned char hash[SHA_DIGEST_LENGTH];
                    DLString msg(websock.headers["Sec-WebSocket-Key"] + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

                    SHA1((const unsigned char*)msg.c_str(), msg.size(), hash);
                    std::string accept = base64_encode(hash, SHA_DIGEST_LENGTH);

                    LogStream::sendError( ) << "WebSock: accept=" << accept << endl;

                    writeFd( "HTTP/1.1 101 Switching Protocols\r\n" );
                    writeFd( "Server: WebSockify Python/2.7.12\r\n" );
                    writeFd( "Date: Wed, 25 Apr 2018 02:52:48 GMT\r\n");
                    writeFd( "Upgrade: websocket\r\n" );
                    writeFd( "Connection: Upgrade\r\n" );
                    writeFd( (DLString("Sec-WebSocket-Accept: ") + accept + "\r\n").c_str() );
                    writeFd( "Sec-WebSocket-Protocol: binary\r\n" );
                    writeFd( "\r\n" );
                    websock.state = WS_ESTABLISHED;

                    DLString proxyFor(websock.headers["X-Forwarded-For"]);

                    if(!proxyFor.empty()) {
                        ViaRecord v;
                        in_addr addr;

                        if(inet_aton(proxyFor.c_str(), &addr))
                            v.first = addr;

                        v.second = proxyFor;
                        
                        via.push_back(v);

                        if (banManager->checkVerbose( this, BAN_ALL )) 
                            return -1;
                    }

                    // Generate unique client ID, to be used when creating secure clickable action links.
                    websock.nonce = create_nonce(8);

                    LogStream::sendError( ) << "WebSock: Success! ID " << websock.nonce << endl;
                    sendVersion(this);

                    NannyHandler::init(this);
                }

                std::vector<unsigned char>(i+1, websock.frame.end()).swap(websock.frame);
                
                if(websock.state == WS_ESTABLISHED) {
                    return wsHandleFrame(NULL, 0);
                }

                return wsHandleNeg(NULL, 0);
        }

    return true;
}

bool Descriptor::readInput( )
{
    unsigned char buf[256];

    for(;;) {
        int rc;

#ifdef __MINGW32__
        if(hasmore(descriptor) == 0)
            break;
        
        rc = ::recv(descriptor, (char *)buf, sizeof(buf), 0);
#else
        rc = read(descriptor, (char *)buf, sizeof(buf));
#endif

        if ( rc > 0 ) {
            switch(websock.state) {
                case WS_NEGOTIATING:
                    if(!wsHandleNeg(buf, rc))
                        return false;
                    break;
                case WS_ESTABLISHED:
                    if(!wsHandleFrame(buf, rc))
                        return false;
                    break;
                case WS_DISABLED:
                    if(!wsHandlePassThrough(buf, rc))
                        return false;
                    break;
                default:
                    LogStream::sendWarning( ) << "Unknown WebSock state." << endl;
                    return false;
            }
            
        } else if ( rc == 0 ) {
            LogStream::sendWarning( ) << "EOF encountered on read." << endl;
            return false;
        } else if ( errno == EWOULDBLOCK ) {
            break;
        } else {
            LogStream::sendError( ) << "Read_from_descriptor::" << strerror( errno ) << endl;
            return false;
        }
    }
    
    return true;
}



bool Descriptor::noIAC( )
{
    return character && IS_SET(character->add_comm, COMM_NOIAC); 
}

bool Descriptor::noTelnet( )
{
    return connected == CON_PLAYING 
            && character 
            && IS_SET(character->add_comm, COMM_NOTELNET);
}

