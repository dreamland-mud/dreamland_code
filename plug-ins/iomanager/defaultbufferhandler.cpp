/* $Id$
 *
 * ruffina, 2004
 */
#include <iconv.h>
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
#include "def.h"

extern const char *lid;

const char *MSG_LINE_TOO_LONG = C_YELLOW "Слишком длинная строка была обрезана. Эта часть не влезла:\n\r" C_WHITE;
const char *MSG_EOL = "\r\n";

void
DefaultBufferHandlerPlugin::initialization()
{
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
        static const char *fromcode = "utf-8";
        static const char *tocode = "koi8-u";
        iconv_t icnv_desc = iconv_open(tocode, fromcode);

        if (icnv_desc == (iconv_t)(-1)) {
            syserr("Failed to iconv_open %s to %s.", fromcode, tocode);
            return -1;
        }

        size_t rest = to_length;
        
        if (iconv(icnv_desc, &from, &from_length, &to, &rest) < 0) {
            syserr("Failed to convert characters from %s to %s.", fromcode, tocode);
            return -1;
        }

        *to = '\0';
        iconv_close(icnv_desc);
        return to_length - rest;
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

    if(line_length < 0) {
        d->close( );
        return false;
    }

    // Cut off too long lines and notify the user.
    if(line_length >= MAX_INPUT_LENGTH - 2) {
        d->writeConverted(MSG_LINE_TOO_LONG);

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
    for(i = 0, k = 0; i < line_length;i++) {
        if(line[i] == '\b' && k > 0)
            --k;
        else if ( (unsigned)line[i] >= ' ' )
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

                    d->writeConverted(lid);
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

// TODO use convert method.
void 
DefaultBufferHandler::write( Descriptor *d, const char *txt ) 
{
    int size, length;
    int i;

    if( d->snoop_by )
        d->snoop_by->send( txt );

    length = strlen(txt);

    char txt_buf[length];
    strcpy(txt_buf, txt);
    char *txt_ptr = txt_buf;

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
        static const char *tocode = "utf-8";
        static const char *fromcode = "koi8-u";
        iconv_t icnv_desc = iconv_open(tocode, fromcode);

        if (icnv_desc == (iconv_t)(-1)) {
            syserr("Failed to iconv_open %s to %s.", fromcode, tocode);
            d->close( );
            return;
        }
        
        size_t icnv_insize = length;
        char icnv_outbuf[length * 6];
        char *icnv_outptr = icnv_outbuf;
        size_t icnv_outsize = sizeof(icnv_outbuf);

        if (iconv(icnv_desc, &txt_ptr, &icnv_insize, &icnv_outptr, &icnv_outsize) < 0) {
            syserr("Failed to convert characters from %s to %s, txt[%s].", fromcode, tocode, txt);
            d->close( );
            return;
        }
        
        memmove(d->outbuf + d->outtop, icnv_outbuf, icnv_outptr - icnv_outbuf );
        d->outtop += icnv_outptr - icnv_outbuf ;

        iconv_close(icnv_desc);
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

    char txt_buf[length];
    strcpy(txt_buf, txt);
    char *txt_ptr = txt_buf;

    /*
     * utf-8 write
     */
    if (!to) {
        DLString result;
        static const char *tocode = "utf-8";
        static const char *fromcode = "koi8-u";
        iconv_t icnv_desc = iconv_open(tocode, fromcode);

        if (icnv_desc == (iconv_t)(-1)) {
            syserr("Failed to iconv_open %s to %s.", fromcode, tocode);
            return result;
        }
    
        size_t icnv_insize = length;
        char icnv_outbuf[length * 6];
        char *icnv_outptr = icnv_outbuf;
        size_t icnv_outsize = sizeof(icnv_outbuf);

        if (iconv(icnv_desc, &txt_ptr, &icnv_insize, &icnv_outptr, &icnv_outsize) < 0) {
            syserr("Failed to convert characters from %s to %s, txt[%s].", fromcode, tocode, txt);
            return result;
        }

        icnv_outbuf[icnv_outptr-icnv_outbuf] = 0;
        result.assign(icnv_outbuf);    
        iconv_close(icnv_desc);
        return result;
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


