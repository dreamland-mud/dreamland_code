/* $Id$
 *
 * ruffina, Dream Land, 2018
 */


#include <sstream>

#include "logstream.h"
#include "iconvmap.h"
#include "exception.h"

IconvMap::IconvMap(const char *from, const char *to) 
{
    this->from = from;
    this->to = to;
}

IconvMap::~IconvMap() 
{
    
}

std::string 
IconvMap::operator() (const std::string &constSrc)
{
    string src = constSrc;
    icnv_desc = iconv_open(to, from);

    if (icnv_desc == (iconv_t)(-1)) {
        throw new Exception("Failed to iconv_open");
    }

    // Substitute UA apostrophe sign (U+2019) with single quotation mark,
    // because there's no corresponding character in KOI8-U or CP1251 encoding.
    if (from == string("utf-8")) {
        const string modifiedLetterApostrophe = "\xCA\xBC";
        const string rightSingleQuotationMark = "'";
        size_t index = 0;
        
        while (true) {
            /* Locate the substring to replace. */
            index = src.find(modifiedLetterApostrophe, index);
            if (index == string::npos) 
                break;

            /* Make the replacement. */
            src = src.replace(index, modifiedLetterApostrophe.length(), rightSingleQuotationMark);
        }
    }

    char *in = (char*) src.c_str();
    size_t in_len = src.size();
    char buf[1024];
    std::ostringstream os;

    while(in_len > 0) {
        char *out = buf;
        size_t len = sizeof(buf);

        if (iconv(icnv_desc, &in, &in_len, &out, &len) < 0) {
            throw new Exception("Failed to convert characters");
        }

        if(out == buf) {
            LogStream::sendError() << "Incomplete UTF sequence?" << src << endl;
            break;
        }
        os.write(buf, out-buf);
    }

    iconv_close(icnv_desc);
    return os.str();
}

