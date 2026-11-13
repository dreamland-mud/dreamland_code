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
#include "dreamland.h"
#include "dlfilestream.h"

#include "descriptor.h"
#include "pcharacter.h"
#include "npcharacter.h"
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

    // A takeover may already have handed the body to a newer descriptor:
    // only detach it if it is still ours.
    if (character && character->desc == this)
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

bool char_unswitch( PCharacter *pch )
{
    NPCharacter *mob = pch->switchedTo;
    bool moved = false;

    if (!mob)
        return false;

    if (mob->desc && !pch->desc) {
        mob->desc->associate( pch );
        moved = true;
    }

    mob->desc = 0;
    mob->switchedFrom = 0;
    pch->switchedTo = 0;
    return moved;
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
        // Payload >= 64 KiB: RFC 6455 requires the 7-bit length field == 127,
        // signalling an 8-byte extended length. Omitting it left hdr[1] as
        // garbage (uninitialised, possibly with the mask bit set), so the client
        // mis-framed the message and dropped the connection -- any frame this
        // large (e.g. a big `fedit` webedit payload) disconnected the client.
        hdr[1] = 127;
        *(uint64_t*)(hdr+2) = htonll((uint64_t)length);
        hdrLen = 10;
    }
    // One writeFd call per frame: the output queue takes or refuses a write
    // whole, so a header can never reach the wire without its payload.
    std::string frame((const char *)hdr, hdrLen);
    frame.append((const char *)txt, length);

    if (writeFd((const unsigned char *)frame.data( ), frame.size( )) < 0)
        return -1;

    return length;
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

// A link that stays stalled this long is dead for practical purposes: drop it
// rather than let the queue grow without bound. Same order of magnitude as the
// outbuf cap in write_to_buffer.
static const size_t PENDING_OUT_MAX = 256 * 1024;

// One non-blocking pass of send(): bytes accepted by the kernel, or -1 when the
// socket is really broken. A full kernel buffer is not a dead peer: the caller
// queues the rest and retries next pulse.
int
Descriptor::sendSome(const unsigned char *txt, int length)
{
    int iStart;
    int nWrite = 0;

    for ( iStart = 0; iStart < length; iStart += nWrite ) {
        int nBlock = min( length - iStart, 4096 );
        nWrite = ::send( descriptor, (const char*)txt + iStart, nBlock, 0 );

        if ( nWrite < 0 ) {
            if ( errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR )
                return iStart;

            LogStream::sendWarning( ) << "Descriptor::writeFd(" << descriptor << "):" << strerror( errno ) << endl;
            return -1;
        }

        // No progress: stop instead of spinning the whole game loop.
        if ( nWrite == 0 )
            return iStart;
    }

    return iStart;
}

// Every byte bound for the socket goes through here, so a partial send never
// loses or reorders anything: whatever the kernel does not take is queued and
// flushed first on the next pulse. A write is taken or refused whole: the cap
// applies only when appending to an existing backlog, and then nothing of the
// write is sent. Callers must hand over a protocol unit (a whole WebSocket
// frame) in one call. capped=false skips the cap for pieces of a unit the
// caller already admitted (writeMccp's compressed chunks). Returns length, or
// -1 on a dead socket or overflow.
int
Descriptor::writeFd(const unsigned char *txt, int length, bool capped)
{
    if (length <= 0)
        return 0;

    int sent = 0;

    if (pendingOut.empty( )) {
        sent = sendSome(txt, length);
        if (sent < 0)
            return -1;
    }
    else if (capped && pendingOut.size( ) + length > PENDING_OUT_MAX) {
        // Refuse this write and keep the queue intact: dropping its middle
        // would corrupt the stream. process_output kicks on -1; callers that
        // ignore it just lose this one whole unit.
        LogStream::sendWarning( ) << "Descriptor::writeFd(" << descriptor << "): output queue overflow, dropping" << endl;
        return -1;
    }

    if (sent < length)
        pendingOut.append((const char *)txt + sent, length - sent);

    return length;
}

int
Descriptor::flushPending( )
{
    if (pendingOut.empty( ))
        return 0;

    int sent = sendSome((const unsigned char *)pendingOut.data( ), pendingOut.size( ));
    if (sent < 0)
        return -1;

    pendingOut.erase(0, sent);
    return pendingOut.size( );
}

int
Descriptor::processMccp( )
{
    int len = out_compress->next_out - out_compress_buf;

    if (len > 0) {
        // Uncapped: writeMccp already admitted the whole unit, and refusing a
        // chunk of it would cut the decompressed stream mid-unit. The output
        // queue keeps what the socket cannot send yet.
        if (writeFd(out_compress_buf, len, false) < 0)
            return -1;

        out_compress->next_out = out_compress_buf;
        return len;
    }
    return 0;
}

int
Descriptor::writeMccp(const unsigned char *txt, int length)
{
    if(!out_compress)
        return writeSock(txt, length);

    // Admit or refuse the unit whole, before deflate consumes any of it.
    if (!pendingOut.empty( ) && pendingOut.size( ) + length > PENDING_OUT_MAX)
        return -1;

    out_compress->next_in = (unsigned char *)txt;
    out_compress->avail_in = length;

    while (out_compress->avail_in) {
        do {
            out_compress->avail_out = COMPRESS_BUF_SIZE - 
                             (out_compress->next_out - out_compress_buf);

            if (out_compress->avail_out) {
                int status = deflate(out_compress, Z_SYNC_FLUSH);

                // Z_BUF_ERROR is not a broken stream -- it is zlib reporting "no
                // progress", which happens benignly when the last input was fully
                // consumed and nothing is pending (a full-buffer episode that
                // landed exactly on COMPRESS_BUF_SIZE, then processMccp drained it).
                // avail_in is 0 in that case, so treating it as OK lets the loop
                // exit cleanly on its own. Before PR #1002 the caller ignored the
                // -1 and merely lost the tail; now -1 drops the descriptor, so this
                // would kick a live player on a rare buffer-boundary hit.
                if (status != Z_OK && status != Z_BUF_ERROR)
                    return -1;
            }

            int rc = processMccp( );
            if (rc < 0)
                return -1;
            else if (rc == 0)
                // Deflate emitted nothing. Guard against spinning the game
                // loop on an unchanged state should that ever coincide with
                // pending input.
                return length - out_compress->avail_in;
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

static const DLString PEAK_FILE = "who_peak.json";
static const int PEAK_DAYS = 7;

int Descriptor::offline_count = 0;

static int peak_today()
{
    return time(0) / (24 * 60 * 60);
}

// Day number -> peak for that day. Loaded lazily, so plugin reload re-reads the file.
static Json::Value &peak_days()
{
    static Json::Value days;
    static bool loaded = false;

    if (!loaded) {
        loaded = true;

        try {
            ostringstream buf;
            DLFileStream(dreamland->getMiscDir(), PEAK_FILE).toStream(buf);

            Json::Reader reader;
            if (!reader.parse(buf.str(), days))
                LogStream::sendError() << PEAK_FILE << ": " << reader.getFormattedErrorMessages() << endl;
        } catch (const std::exception &) {
            // No file yet: start counting from now.
        }

        if (!days.isObject())
            days = Json::Value(Json::objectValue);
    }

    return days;
}

// The file is hand-editable: a non-integer value must read as 0, not throw.
static int peak_value(const Json::Value &days, const std::string &key)
{
    const Json::Value &value = days[key];
    return value.isIntegral() ? value.asInt() : 0;
}

void Descriptor::recordPeak( )
{
    int count = offline_count;

    for (Descriptor *d = descriptor_list; d != 0; d = d->next)
        if (d->connected == CON_PLAYING)
            count++;

    Json::Value &days = peak_days();
    int today = peak_today();
    DLString key(today);

    if (peak_value(days, key) >= count)
        return;

    days[key] = count;

    for (const auto &name: days.getMemberNames())
        if (atoi(name.c_str()) <= today - PEAK_DAYS)
            days.removeMember(name);

    try {
        Json::FastWriter writer;
        DLFileStream(dreamland->getMiscDir(), PEAK_FILE).fromString(writer.write(days));
    } catch (const std::exception &ex) {
        LogStream::sendError() << PEAK_FILE << ": " << ex.what() << endl;
    }
}

void Descriptor::setOfflineCount(int count)
{
    offline_count = count;
    recordPeak();
}

int Descriptor::getWeekPeak()
{
    Json::Value &days = peak_days();
    int today = peak_today();
    int peak = 0;

    for (const auto &name: days.getMemberNames())
        if (atoi(name.c_str()) > today - PEAK_DAYS)
            peak = max(peak, peak_value(days, name));

    return peak;
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

