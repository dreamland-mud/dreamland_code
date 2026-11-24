/* $Id$
 *
 * ruffina, Dream Land, 2018
 */


#include <errno.h>
#include <sstream>

#include "logstream.h"
#include "iconvmap.h"
#include "exception.h"

// KOI8 has none of these: game text uses their ASCII forms.
static const struct { const char *from; const char *to; } typography[] = {
    { "\xE2\x80\xA6", "..." },  // U+2026 ellipsis
    { "\xE2\x80\x94", "--" },   // U+2014 em dash
    { "\xE2\x80\x93", "-" },    // U+2013 en dash
    { "\xCA\xBC", "'" },        // U+02BC modifier letter apostrophe (UA apostrophe)
    { "\xE2\x80\x99", "'" },    // U+2019 right single quotation mark
    { "\xE2\x80\x98", "'" },    // U+2018 left single quotation mark
    { "\xE2\x80\x9C", "\"" },   // U+201C left double quotation mark
    { "\xE2\x80\x9D", "\"" },   // U+201D right double quotation mark
    { "\xC2\xAB", "\"" },       // U+00AB left angle quotation mark
    { "\xC2\xBB", "\"" },       // U+00BB right angle quotation mark
    { 0, 0 }
};

// Closes the descriptor on every way out, exceptions included.
struct IconvDesc {
    iconv_t cd;
    IconvDesc(const char *to, const char *from) : cd(iconv_open(to, from)) { }
    ~IconvDesc() { if (cd != (iconv_t)(-1)) iconv_close(cd); }
};

IconvMap::IconvMap(const char *from, const char *to)
    : to(to), from(from)
{
    ignore = this->to.find("//IGNORE") != std::string::npos;
}

IconvMap::~IconvMap()
{
}

std::string IconvMap::typographyToAscii(const std::string &utf)
{
    std::string mapped = utf;

    for (int i = 0; typography[i].from; i++) {
        const std::string f = typography[i].from;
        const std::string t = typography[i].to;
        std::string::size_type pos = 0;

        while ((pos = mapped.find(f, pos)) != std::string::npos) {
            mapped.replace(pos, f.size(), t);
            pos += t.size();
        }
    }

    return mapped;
}

std::string
IconvMap::operator() (const std::string &constSrc)
{
    IconvDesc desc(to.c_str(), from.c_str());

    if (desc.cd == (iconv_t)(-1))
        throw Exception("Failed to iconv_open " + from + " -> " + to);

    bool fromUtf8 = (from == "utf-8");
    std::string src = fromUtf8 ? typographyToAscii(constSrc) : constSrc;

    char *in = const_cast<char *>(src.data());
    size_t in_len = src.size();
    std::ostringstream os;
    int lost = 0;

    while (in_len > 0) {
        char buf[1024];
        char *out = buf;
        size_t out_len = sizeof(buf);
        char *in_before = in;

        size_t rc = iconv(desc.cd, &in, &in_len, &out, &out_len);
        int err = errno;
        os.write(buf, out - buf);

        if (rc != (size_t)(-1))
            continue;

        if (err == E2BIG) {
            if (out == buf)
                throw Exception("iconv: output buffer too small for one character");
            continue;
        }

        if (err != EILSEQ && err != EINVAL)
            throw Exception("Failed to convert characters");

        // glibc //IGNORE reports EILSEQ after it skipped what it could not
        // convert; go on while it makes progress.
        if (ignore && in != in_before)
            continue;

        // Stopped at a character the target cannot hold (EILSEQ) or at a
        // sequence cut short by the end of the input (EINVAL): replace it
        // and go on with the rest.
        lost++;
        if (!ignore)
            os << '?';

        size_t skip = 1;
        if (err == EINVAL)
            skip = in_len;
        else if (fromUtf8)
            while (skip < in_len && skip < 4 && (in[skip] & 0xC0) == 0x80)
                skip++;

        in += skip;
        in_len -= skip;
        iconv(desc.cd, 0, 0, 0, 0);
    }

    if (lost > 0 && !ignore)
        LogStream::sendWarning() << "iconv " << from << " -> " << to << ": "
                                 << lost << " character(s) replaced by '?'" << endl;

    return os.str();
}
