/* $Id$
 *
 * ruffina, 2004
 */
#include <algorithm>
#include <string.h>

#include "defaultbufferhandler.h"
#include "logstream.h"
#include "descriptor.h"
#include "character.h"
#include "ban.h"
#include "comm.h"
#include "wiznet.h"
#include "telnet.h"
#include "codepage.h"

#include "colour.h"
#include "l10n.h"
#include "def.h"
#include "iconvmap.h"

static IconvMap utf2koi("utf-8", "koi8-u//IGNORE");
static IconvMap koi2utf("koi8-u", "utf-8");

DLString msg_spam_warning( Character *ch );

const char *MSG_EOL = "\r\n";

void
DefaultBufferHandlerPlugin::initialization()
{
    Class::regMoc<OutputEntry>();
    Class::regMoc<DefaultBufferHandler>();

    Descriptor *d;
    
    for(d = descriptor_list; d; d = d->next)
        if(d->buffer_handler && d->buffer_handler->getType( ) == "DefaultBufferHandler")
            d->buffer_handler.recover( );
}

void
DefaultBufferHandlerPlugin::destruction()
{
    Descriptor *d;
    
    for(d = descriptor_list; d; d = d->next)
        if(d->buffer_handler && d->buffer_handler->getType( ) == "DefaultBufferHandler")
            d->buffer_handler.backup( );
    
    Class::unregMoc<DefaultBufferHandler>();
    Class::unregMoc<OutputEntry>();
}

DefaultBufferHandler::DefaultBufferHandler()
{
}

DefaultBufferHandler::DefaultBufferHandler(int i)
{
    codepage.setValue(i);
}


int
DefaultBufferHandler::convertCodepage(char *from, size_t from_length, char *to, size_t to_length)
{
    const unsigned char *cp = russian_codepages[codepage].from;
    
    if (cp) {
        size_t i;
        for(i=0;i < from_length;i++) {
            unsigned char c =(unsigned char)from[i];
            to[i] = c > ' ' ? (char)cp[c] : (char)c;
        }
        to[i] = 0;
        return from_length;
    } else {
        // Same conversion as web client input, so a line (a password too)
        // typed in a UTF-8 telnet client reaches the game as it would from
        // the web: unmappable characters are dropped, the rest of the line kept.
        // Every UTF-8 character becomes at most one KOI8 byte here, so the
        // result never outgrows 'from'.
        std::string koi = utf2koi(std::string(from, from_length));
        size_t len = std::min(koi.size(), to_length);

        memcpy(to, koi.data(), len);
        to[len] = '\0';
        return len;
    }
}

int
DefaultBufferHandler::shiftOneLine(Descriptor *d, char *buf)
{
    size_t i, j;

    for(i = 0;i < d->inptr; i++) {
        if( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' ) {
            int line_length = i;
            buf[i] = 0;

            // skip trailing new lines
            while(i < d->inptr && (d->inbuf[i] == '\n' || d->inbuf[i] == '\r') )
                i++;
            
            // shift the inbuf
            for(j = 0; i < d->inptr; i++, j++)
                d->inbuf[j] = d->inbuf[i];
            
            d->inptr = j;
            return line_length;
        }

        buf[i] = d->inbuf[i];
    }
    
    return -1;
}

/*
 * Transfer one line from input buffer to input line.
 * Returns false if 'd' is deallocated.
 */
bool 
DefaultBufferHandler::read( Descriptor *d )
{
    int i, k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
        return true;

    char line[d->inptr+1];
    int line_length;
    line_length = shiftOneLine(d, line);
    
    if(line_length < 0)
        return true;

    char koi8[line_length+2];
    line_length = convertCodepage(line, line_length, koi8, line_length);

    // Cut off too long lines and notify the user.
    if(line_length >= MAX_INPUT_LENGTH - 2) {
        DLString tooLong = DLString(C_YELLOW)
            + _("Слишком длинная строка была обрезана. Эта часть не влезла:").getMessage(d->character)
            + "\n\r" + C_WHITE;
        d->writeConverted(tooLong.c_str());

        char remainder[MAX_STRING_LENGTH];
        line_length = MAX_INPUT_LENGTH - 2;
        strncpy(remainder, koi8 + line_length, MAX_STRING_LENGTH);
        koi8[line_length] = 0;

        d->writeRaw((const unsigned char *)remainder, sizeof(remainder));
        d->writeConverted(MSG_EOL);
    }

    /*
     * handle backspaces
     */
    // Walk the converted line: after UTF-8 -> KOI8 it is shorter than 'line',
    // so indexing 'line' here let control bytes (and backspaces) slip through.
    for(i = 0, k = 0; i < line_length;i++) {
        if(koi8[i] == '\b') {
            if (k > 0)
                --k;
        }
        else if ( (unsigned char)koi8[i] >= ' ' )
            d->incomm[k++] = koi8[i];
    }

    if(k == 0) {
        d->incomm[k++] = ' ';
    }
    d->incomm[k] = 0;
    
    if (d->snoop_by) {
        d->snoop_by->send("# ");
        d->snoop_by->send(d->incomm);
        d->snoop_by->send("\r\n");
    }

    /*
     * Deal with bozos with #repeat 1000 ...
     */
    if ( k > 1 || d->incomm[0] == '!' ) {
        if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) ) {
            d->repeat = 0;
        } else {
            if ( ++d->repeat >= 100 
                    && d->character
                    && !d->character->isCoder( )
                    && d->connected == CON_PLAYING )
            {
                LogStream::sendWarning( ) << d->getRealHost( ) << " input spamming!" << endl;

                if ( d->character != 0 ) {
                    wiznet( WIZ_SPAM, 0, d->character->get_trust( ), 
                            "SPAM SPAM SPAM %C1 spamming, and OUT! Inlast[%s] Incomm[%s]", 
                            d->character, d->inlast, d->incomm );

                    d->repeat = 0;

                    d->writeConverted(msg_spam_warning( d->character ).c_str( ));
                    d->close( );
                    return false;
                }
            }
        }
    }

    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
        strcpy( d->incomm, d->inlast );
    else
        strcpy( d->inlast, d->incomm );

    return true;
}

long long DefaultBufferHandler::getCurrentSeq() const
{
    return seqCounter.getValue();
}

// TODO use convert method.
/* Set only for the duration of one widget_help RPC. */
Descriptor *widget_capture_desc = 0;
DLString *widget_capture_sink = 0;

void
DefaultBufferHandler::write( Descriptor *d, const char *txt )
{
    int size, length;
    int i;

    // widget_help RPC (descriptor.cpp): this descriptor's output goes to the
    // capture string, not to the socket, the snoop or the /api/snoop log.
    if (d == widget_capture_desc && widget_capture_sink) {
        *widget_capture_sink += txt;
        return;
    }

    // Capture raw output into the ring buffer with a monotonic sequence number.
    {
        OutputEntry entry;
        entry.seq = ++seqCounter;
        entry.text = txt;
        outputLog.push_back(entry);
        if ((int)outputLog.size() > MAX_OUTPUT_LINES)
            outputLog.erase(outputLog.begin(),
                            std::next(outputLog.begin(), outputLog.size() - MAX_OUTPUT_LINES));
    }

    if( d->snoop_by )
        d->snoop_by->send( txt );

    length = strlen(txt);


    const unsigned char *to = russian_codepages[codepage].to;

    /*
    * Abjust size in casr of IACs (they will be doubled)
    */
    size = length;
    if(to && !d->noTelnet() && !d->noIAC())
        for(i = 0; i < length; i++) {
            if(to[(unsigned char)txt[i]] == IAC)
                size++;

            if( to == koi8_tran ) {
                switch( txt[i] ) {
                case 'я': case 'Я': case 'ю': case 'Ю': 
                case 'ч': case 'Ч': case 'ж': case 'Ж': 
                case 'ш': case 'Ш': 
                    size++; 
                    break;
                case 'щ': case 'Щ': 
                    size+=2; 
                    break;
                }
            }                
        }

    /*
    * Initial \n\r if needed.
    */
    if ( d->outtop == 0 && !d->fcommand ) {
        d->outbuf[0]        = '\n';
        d->outbuf[1]        = '\r';
        d->outtop        = 2;
    }

    /*
    * Expand the buffer as needed.
    */
    while ( d->outtop + size * (1 + 5*(!to)) >= d->outsize ) {
        char *outbuf;

        if (d->outsize >= 6*32000) {
            LogStream::sendWarning( ) << "Buffer overflow for " << d->host << ". Closing." << endl;
            d->outtop = 0;
            d->close( );
            return;
        }
        
        outbuf = ( char* )malloc( 2 * d->outsize );
        strncpy( outbuf, d->outbuf, d->outtop );
        free( d->outbuf );
        d->outbuf = outbuf;
        d->outsize *= 2;
    }
    
    /*
     * utf-8 write
     */
    if (!to) {
        // The buffer was grown for 6 bytes per input byte above; KOI8 needs 3 at most.
        std::string utf = koi2utf(std::string(txt, length));

        memmove(d->outbuf + d->outtop, utf.data(), utf.size());
        d->outtop += utf.size();
        return;
    }

    /*
    * Copy.
    */
    while(length--) {
        unsigned char c;
        c = to[(unsigned char)*txt];
        d->outbuf[d->outtop] = c;
        if( to == koi8_tran ) {
            switch( *txt ) {
            case 'я': case 'Я': d->outbuf[++d->outtop] = 'a'; break;
            case 'ю': case 'Ю': d->outbuf[++d->outtop] = 'u'; break;
            case 'ч': case 'Ч': d->outbuf[++d->outtop] = 'h'; break;
            case 'ж': case 'Ж': d->outbuf[++d->outtop] = 'h'; break;
            case 'ш': case 'Ш': d->outbuf[++d->outtop] = 'h'; break;
            case 'щ': case 'Щ': d->outbuf[++d->outtop] = 'c';
                                d->outbuf[++d->outtop] = 'h';
                                break;
            }
        }
        txt++;
        if(c == IAC) {
            if(d->noIAC())
                d->outbuf[d->outtop] = IAC_REPL;
            else if(!d->noTelnet())
                d->outbuf[++d->outtop] = IAC;
        }
        d->outtop++;
    }
}

// Temporary here before descriptor read/write overhaul.
DLString DefaultBufferHandler::convert(const char *txt) 
{
    int length = strlen(txt);

    const unsigned char *to = russian_codepages[codepage].to;

    /*
     * utf-8 write
     */
    if (!to) {
        return koi2utf(std::string(txt, length));
    }

    /*
    * All other encodings.
    */
    ostringstream result;
    while(length--) {
        unsigned char c = to[(unsigned char)*txt];
        result << c;
        if( to == koi8_tran ) {
            switch( *txt ) {
            case 'я': case 'Я': result << 'a'; break;
            case 'ю': case 'Ю': result << 'u'; break;
            case 'ч': case 'Ч': result << 'h'; break;
            case 'ж': case 'Ж': result << 'h'; break;
            case 'ш': case 'Ш': result << 'h'; break;
            case 'щ': case 'Щ': result << "ch"; break;
            }
        }
        txt++;
        if(c == IAC) {
            result << (unsigned char)IAC;
        }
    }

    return result.str();
}


