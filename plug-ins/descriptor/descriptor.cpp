/* $Id$
 *
 * ruffina, 2004
 */

#include <errno.h>
#include <byteswap.h>
#include <iconv.h>
#include <string.h>
#include <sys/socket.h>

#include "json/json.h"

#include "logstream.h"
#include "plugin.h"
#include "character.h"
#include "pcharactermanager.h"
#include "so.h"

#include "descriptor.h"
#include "telnet.h"


#include "def.h"

#include "iconvmap.h"

static IconvMap koi2utf("koi8-u", "utf-8");

/*-------------------------------------------------------------------
 * Descriptor
 *-------------------------------------------------------------------*/

static Descriptor d_zero;

Descriptor::Descriptor()
            : echo( true )
{
    *this = d_zero;
    websock.state = WS_DISABLED;
}

Descriptor::~Descriptor()
{
    if(outbuf)
        free(outbuf);
}

void 
Descriptor::send(const char *buf)
{
    if(buffer_handler)
        buffer_handler->write(this, buf);
}


void
Descriptor::close( )
{
    Descriptor *d;

    if (snoop_by)
        snoop_by->send("Your victim has left the game.\n\r");

    for (d = descriptor_list; d; d = d->next)
        if (d->snoop_by == this)
            d->snoop_by = 0;
    
    for (handle_input_t::iterator h = handle_input.begin( ); h != handle_input.end( ); h++)
        (*h)->close( this );

    if(character)
        character->desc = NULL;

    character = NULL;
    connected = CON_CLOSED;
}

void
Descriptor::slay( )
{
    stopMccp( );

    ::close( descriptor );

    if ( descriptor_list == this )
        descriptor_list = next;
    else {
        Descriptor *d;

        for ( d = descriptor_list; d ; d = d->next )
            if(d->next == this) {
                d->next = next;
                break;
            }

        if (!d)
            throw Exception( "Close_socket: dclose not found." );
    }

    delete this;
}

void Descriptor::associate( Character *ch )
{
    ch->desc = this;
    character = ch;
}

#define htonll(p) bswap_64(p)

int
Descriptor::writeWebSock(unsigned char opcode, const unsigned char *txt, int length)
{
    int hdrLen;
    unsigned char hdr[10];

    hdr[0] = 0x80 | opcode; // fin

    if(length < 126) {
        hdr[1] = (unsigned char)length;
        hdrLen = 2;
    } else if(length < 0x10000) {
        hdr[1] = 126;
        *(unsigned short*)(hdr+2) = htons((unsigned short)length);
        hdrLen = 4;
    } else {
        *(uint64_t*)(hdr+2) = htonll(length);
        hdrLen = 10;
    }
    if(writeFd(hdr, hdrLen) < 0)
        return -1;

    return writeFd(txt, length);
}


static void
translate(Json::Value &val)
{
    if(val.isString()) {
        val = koi2utf(val.asString());
        return;
    }

    for(Json::Value::iterator i=val.begin();i!=val.end();i++) {
        translate(*i);
    }
}

int
Descriptor::writeWSCommand(const Json::Value &val)
{
    if(websock.state != WS_ESTABLISHED) {
        return 0;
    }

    Json::FastWriter fast;
    Json::Value clone = val;
    translate(clone);
    std::string str = fast.write(clone);

    return writeWebSock(2, (unsigned const char*)str.c_str(), str.size()); // 2 - binary
}

int 
Descriptor::writeWSCommand(const DLString &name, const std::vector<DLString> &args)
{
    std::vector<DLString>::const_iterator i;
    int j;
    Json::Value val;

    val["command"] = name;

    for(i=args.begin(), j=0;i!=args.end();i++, j++)
        val["args"][j] = *i;

    return writeWSCommand(val);
}


static int
writeConsoleMessage(Descriptor *d, const unsigned char *txt, int length)
{
    Json::Value val;

    val["command"] = "console_out";
    val["args"][0] = std::string((const char *)txt, (const char *)txt+length);

    return d->writeWSCommand(val);
}

int
Descriptor::writeSock(const unsigned char *txt, int length)
{
    if(websock.state == WS_ESTABLISHED) {
        return writeConsoleMessage(this, txt, length);
    } else {
        return writeFd(txt, length);
    }
}

int
Descriptor::writeFd(const char *txt)
{
    return writeFd((const unsigned char *)txt, strlen(txt));
}

int
Descriptor::writeFd(const unsigned char *txt, int length)
{
    int iStart;
    int nWrite = 0;

    for ( iStart = 0; iStart < length; iStart += nWrite ) {
        int nBlock = min( length - iStart, 4096 );
        nWrite = ::send( descriptor, (const char*)txt + iStart, nBlock, 0 );

        if ( nWrite < 0 ) {
            LogStream::sendWarning( ) << "Descriptor::writeFd(" << descriptor << "):" << strerror( errno ) << endl;
            return -1;
        }
    }

    return iStart;
}

int
Descriptor::processMccp( )
{
    int len = out_compress->next_out - out_compress_buf;

    if (len > 0) {
        int written = writeSock(out_compress_buf, len);

        if(written < 0)
            return written;

        if (!written)
            return 0;

        if (written < len)
            memmove(out_compress_buf, out_compress_buf + written, len - written);

        out_compress->next_out = out_compress_buf + len - written;
        
        return written;
    }
    return 0;
}

int
Descriptor::writeMccp(const unsigned char *txt, int length)
{
    if(!out_compress)
        return writeSock(txt, length);

    out_compress->next_in = (unsigned char *)txt;
    out_compress->avail_in = length;

    while (out_compress->avail_in) {
        do {
            out_compress->avail_out = COMPRESS_BUF_SIZE - 
                             (out_compress->next_out - out_compress_buf);

            if (out_compress->avail_out) {
                int status = deflate(out_compress, Z_SYNC_FLUSH);

                if (status != Z_OK)
                    return -1;
            }

            int rc = processMccp( );
            if (rc < 0)
                return -1;
            else if (rc == 0)
                break;
        } while(out_compress->avail_out == 0);
    }

    return length - out_compress->avail_in;
}

bool
Descriptor::startMccp(unsigned char telopt)
{
    static const unsigned char enable_compress[] = 
        { IAC, SB, TELOPT_COMPRESS, WILL, SE };
    static const unsigned char enable_compress2[] = 
        { IAC, SB, TELOPT_COMPRESS2, IAC, SE };

    if (out_compress)
        return true;

    LogStream::sendNotice( ) << "Starting compression for descriptor "
                            << descriptor << endl;

    z_stream *s;

    s = (z_stream *) malloc(sizeof(z_stream));
    out_compress_buf = (unsigned char *) malloc(COMPRESS_BUF_SIZE);

    s->next_in = NULL;
    s->avail_in = 0;

    s->next_out = out_compress_buf;
    s->avail_out = COMPRESS_BUF_SIZE;

    s->zalloc = NULL;
    s->zfree  = NULL;
    s->opaque = NULL;

    if (deflateInit(s, 9) != Z_OK) {
        free(out_compress_buf);
        free(s);
        return false;
    }

    if (telopt == TELOPT_COMPRESS)
        writeMccp(enable_compress, sizeof(enable_compress));
    else if (telopt == TELOPT_COMPRESS2)
        writeMccp(enable_compress2, sizeof(enable_compress2));
    else
        LogStream::sendError( ) << "compressStart: bad TELOPT passed" << endl;

    compressing = telopt;
    out_compress = s;

    return true;
}

bool 
Descriptor::stopMccp( )
{
    unsigned char dummy[1];

    if (!out_compress)
        return true;

    LogStream::sendNotice( ) << "Stopping compression for descriptor " 
                             << descriptor << endl;

    out_compress->avail_in = 0;
    out_compress->next_in = dummy;

    if (deflate(out_compress, Z_FINISH) != Z_STREAM_END)
        return false;

    processMccp( ); /* try to send any residual data. ignore errors */
    
    deflateEnd(out_compress);
    free(out_compress_buf);
    free(out_compress);
    out_compress = 0;
    out_compress_buf = 0;
    compressing = 0;

    return true;
}

int
Descriptor::writeRaw(const unsigned char *txt, int length)
{
    return writeMccp(txt, length);
}

const char * Descriptor::getRealHost( ) const
{
    if (via.empty( ))
        return host.c_str();
    else
        return via.back( ).second.c_str( );
}


Descriptor *descriptor_list;

int Descriptor::max_online = 0;
int Descriptor::max_offline = 0;

void Descriptor::updateMaxOnline( )
{
    Descriptor *d;
    int count = 0;

    for (d = descriptor_list; d != 0; d = d->next )
        if (d->connected == CON_PLAYING)
            count++;
    
    max_online = max( count, max_online );
}

int Descriptor::getMaxOnline( )
{
    return max_online;
}

void Descriptor::updateMaxOffline(int newValue)
{
    max_offline = max(max_offline, newValue);
}

int Descriptor::getMaxOffline()
{
    return max_offline;
}


void Descriptor::echoOff( )
{
    static const unsigned char echo_off_str [] = { IAC, WILL, TELOPT_ECHO };
    writeRaw( echo_off_str, sizeof(echo_off_str) );
    echo = false;
}

void Descriptor::echoOn( )
{
    static const unsigned char echo_on_str [] = { IAC, WONT, TELOPT_ECHO };
    writeRaw( echo_on_str, sizeof(echo_on_str) );
    echo = true;
}

