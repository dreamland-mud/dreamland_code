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
IconvMap::operator() (const std::string &src)
{
    icnv_desc = iconv_open(to, from);

    if (icnv_desc == (iconv_t)(-1)) {
        throw new Exception("Failed to iconv_open");
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

