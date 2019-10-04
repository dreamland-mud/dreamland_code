/* $Id: descriptor.h,v 1.1.4.3.6.3 2008/03/26 10:57:27 rufina Exp $
 *
 * ruffina, 2005
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/

#ifndef _DESCRIPTOR_H_
#define _DESCRIPTOR_H_

#include "config.h"

#ifndef __MINGW32__
#include <netinet/in.h>
#else
#include <winsock.h>
#endif

#ifdef MCCP
#include <zlib.h>
#endif

#include "inputhandler.h"
#include "bufferhandler.h"
#include "merc.h"
#include "dlmap.h"

class Character;

namespace Json {
class Value;
}

/*
 * Connected state for a channel.
 */
enum {
    CON_CODEPAGE,
    CON_NANNY,
    CON_CREATE_DONE,     // for notify only
    CON_READ_MOTD,       // for notify only
    CON_PLAYING,
    CON_BREAK_CONNECT,   // for notify only
    CON_CLOSED,
    CON_QUIT,
};


#define TNS_NORMAL        0
#define TNS_SUBNEG        1
#define TNS_SUBNEG_IAC        2

struct telnet {
    int state;
    unsigned char subneg[2048];
    unsigned int sn_ptr;
    int ttype;
};

typedef std::pair<in_addr, std::string> ViaRecord;
typedef std::vector<ViaRecord> ViaVector;

#define WS_DISABLED     0
#define WS_NEGOTIATING  1
#define WS_ESTABLISHED  2

/*
 * Out-of-band protocols support
 */
#define OOB_GMCP (A)

struct WebSockState {
    int state;
    std::vector<unsigned char> frame;
    std::vector<unsigned char> payload;
    DLString cmd;
    std::map<std::string, std::string> headers;
    DLString nonce;
};

/*
 * Descriptor (channel) structure.
 */
class Descriptor 
{
public:
    Descriptor();
    virtual ~Descriptor();
    
    void send(const char *); 
    void printf( const char *, ... );
    void close( );             
    void slay( );
    void associate( Character * );

    int writeFd(const unsigned char *buf, int len);
    int writeFd(const char *buf);
    int writeWebSock(unsigned char opcode, const unsigned char *txt, int length);
    int writeWSCommand(const Json::Value &val);
    int writeWSCommand(const DLString &name, const std::vector<DLString> &args);
    int writeSock(const unsigned char *buf, int len);
    int writeRaw(const unsigned char *txt, int len);
#ifdef MCCP
    int writeMccp(const unsigned char *buf, int len);
    int processMccp( );
    bool startMccp(unsigned char telopt);
    bool stopMccp( );
#endif
    const char * getRealHost( ) const;

    void echoOn( );
    void echoOff( );
    
    /* implemented in iomanager */
    bool noIAC();
    bool noTelnet();
    bool readInput();
private:    
    int inputChar(unsigned char);
    int inputTelnet(unsigned char);
    bool checkStopSymbol();
     
public:
    Descriptor *        next;
    Descriptor *        snoop_by;
    Character *                character;
    int                        control;
    bool                echo;
    char *                host;
    char *                realip;
    int                descriptor;
    int                connected;
    bool                fcommand;
    struct telnet        telnet;
    unsigned int        inptr;
    char                inbuf                [4 * MAX_INPUT_LENGTH];
    char                incomm                [MAX_INPUT_LENGTH];
    char                inlast                [MAX_INPUT_LENGTH];
    int                        repeat;
    char *                outbuf;
    int                        outsize;
    int                        outtop;
    int                 oob_proto;

#ifdef MCCP
    unsigned char       compressing;
    z_stream *          out_compress;
#define COMPRESS_BUF_SIZE 1024
    unsigned char *     out_compress_buf;
#endif

    handle_input_t handle_input;
    XMLPersistent<BufferHandler> buffer_handler;

    ViaVector via;

    WebSockState        websock;
    bool wsHandleNeg(unsigned char *buf, int rc);
    bool wsHandleFrame(unsigned char *buf, int rc);
    bool wsHandlePassThrough(unsigned char *buf, int rc);
    bool wsHandlePayload(const Json::Value&);

private:
    static int max_online;
public:
    static void updateMaxOnline( );
    static int getMaxOnline( );
};

extern Descriptor *descriptor_list;

#endif
