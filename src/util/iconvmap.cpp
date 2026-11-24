/* $Id$
 *
 * ruffina, Dream Land, 2018
 */


#include <errno.h>
#include <stdio.h>
#include <sstream>
#include <unordered_map>

#include "logstream.h"
#include "iconvmap.h"
#include "exception.h"

// KOI8 has none of these: game text uses their ASCII forms.
static const struct { unsigned int code; const char *to; } typography[] = {
    { 0x2026, "..." },  // ellipsis
    { 0x2014, "--" },   // em dash
    { 0x2013, "-" },    // en dash
    { 0x2012, "-" },    // figure dash
    { 0x2010, "-" },    // hyphen
    { 0x2011, "-" },    // non-breaking hyphen
    { 0x2212, "-" },    // minus sign
    { 0x02BC, "'" },    // modifier letter apostrophe (Ukrainian apostrophe)
    { 0x2019, "'" },    // right single quotation mark
    { 0x2018, "'" },    // left single quotation mark
    { 0x201A, "'" },    // single low-9 quotation mark
    { 0x2039, "'" },    // single left angle quotation mark
    { 0x203A, "'" },    // single right angle quotation mark
    { 0x201C, "\"" },   // left double quotation mark
    { 0x201D, "\"" },   // right double quotation mark
    { 0x201E, "\"" },   // double low-9 quotation mark
    { 0x00AB, "\"" },   // left angle quotation mark
    { 0x00BB, "\"" },   // right angle quotation mark
    { 0, 0 }
};

static const char *typographyFor(unsigned int code)
{
    for (int i = 0; typography[i].to; i++)
        if (typography[i].code == code)
            return typography[i].to;
    return 0;
}

/*
 * Decode one UTF-8 character at s[i]. Returns its length in bytes and sets
 * code, or returns 0 for a malformed sequence (bad lead byte, missing or
 * bad continuation byte, overlong form, surrogate, beyond U+10FFFF).
 */
static size_t decodeUtf8(const std::string &s, size_t i, unsigned int &code)
{
    unsigned char b = s[i];
    size_t len;
    unsigned int min;

    if (b < 0x80) {
        code = b;
        return 1;
    }
    if ((b & 0xE0) == 0xC0) {
        len = 2; min = 0x80; code = b & 0x1F;
    } else if ((b & 0xF0) == 0xE0) {
        len = 3; min = 0x800; code = b & 0x0F;
    } else if ((b & 0xF8) == 0xF0) {
        len = 4; min = 0x10000; code = b & 0x07;
    } else {
        return 0;
    }

    if (i + len > s.size())
        return 0;

    for (size_t k = 1; k < len; k++) {
        unsigned char c = s[i + k];
        if ((c & 0xC0) != 0x80)
            return 0;
        code = (code << 6) | (c & 0x3F);
    }

    if (code < min || code > 0x10FFFF || (code >= 0xD800 && code <= 0xDFFF))
        return 0;

    return len;
}

// Bytes to skip at a malformed sequence: the lead byte and the continuation
// bytes after it (at most three), so one broken character is one loss.
static size_t malformedLength(const std::string &s, size_t i)
{
    size_t len = 1;
    while (len < 4 && i + len < s.size() && ((unsigned char)s[i + len] & 0xC0) == 0x80)
        len++;
    return len;
}

// Code point -> KOI8-U byte for the upper half, built once from iconv.
static const std::unordered_map<unsigned int, char> &koiTable()
{
    static const std::unordered_map<unsigned int, char> table = []() {
        std::unordered_map<unsigned int, char> t;
        iconv_t cd = iconv_open("utf-8", "koi8-u");

        if (cd == (iconv_t)(-1))
            throw Exception("Failed to iconv_open koi8-u -> utf-8");

        for (int b = 0x80; b <= 0xFF; b++) {
            char in[1] = { (char)b };
            char out[8];
            char *pin = in, *pout = out;
            size_t in_len = 1, out_len = sizeof(out);
            unsigned int code;

            if (iconv(cd, &pin, &in_len, &pout, &out_len) == (size_t)(-1))
                continue;

            std::string utf(out, pout - out);
            if (decodeUtf8(utf, 0, code) == utf.size())
                t[code] = (char)b;
        }

        iconv_close(cd);
        return t;
    }();

    return table;
}

IconvMap::IconvMap(const char *from, const char *to)
    : to(to), from(from)
{
    ignore = this->to.find("//IGNORE") != std::string::npos;
    toKoi = this->from == "utf-8" && this->to.compare(0, 6, "koi8-u") == 0;
}

IconvMap::~IconvMap()
{
}

std::string IconvMap::typographyToAscii(const std::string &utf)
{
    std::string out;
    out.reserve(utf.size());

    for (size_t i = 0; i < utf.size(); ) {
        unsigned int code;
        size_t len = decodeUtf8(utf, i, code);
        const char *ascii = (len > 1) ? typographyFor(code) : 0;

        if (len == 0)
            len = 1;

        if (ascii)
            out += ascii;
        else
            out.append(utf, i, len);
        i += len;
    }

    return out;
}

std::string
IconvMap::operator() (const std::string &src)
{
    if (toKoi)
        return utf8ToKoi(src);
    return viaIconv(src);
}

std::string IconvMap::utf8ToKoi(const std::string &src)
{
    const std::unordered_map<unsigned int, char> &table = koiTable();
    std::string out;
    int lost = 0;
    unsigned int firstLost = 0;
    bool firstMalformed = false;

    out.reserve(src.size());

    for (size_t i = 0; i < src.size(); ) {
        unsigned int code;
        size_t len = decodeUtf8(src, i, code);

        if (len == 1) {
            out += src[i++];
            continue;
        }

        if (len == 0) {
            if (lost++ == 0)
                firstMalformed = true;
            if (!ignore)
                out += '?';
            i += malformedLength(src, i);
            continue;
        }

        i += len;

        const char *ascii = ignore ? (code == 0x02BC ? "'" : 0) : typographyFor(code);
        if (ascii) {
            out += ascii;
            continue;
        }

        std::unordered_map<unsigned int, char>::const_iterator k = table.find(code);
        if (k != table.end()) {
            out += k->second;
            continue;
        }

        if (lost++ == 0)
            firstLost = code;
        if (!ignore)
            out += '?';
    }

    if (lost > 0 && !ignore) {
        char first[32];
        if (firstMalformed)
            snprintf(first, sizeof(first), "malformed UTF-8");
        else
            snprintf(first, sizeof(first), "U+%04X", firstLost);
        LogStream::sendWarning() << "iconv utf-8 -> koi8-u: " << lost
                                 << " character(s) replaced by '?', first " << first << endl;
    }

    return out;
}

// Generic path (KOI8-U to UTF-8 and anything else): every input byte of
// KOI8-U has a UTF-8 form, so this never meets an unmappable character.
std::string IconvMap::viaIconv(const std::string &src)
{
    iconv_t cd = iconv_open(to.c_str(), from.c_str());

    if (cd == (iconv_t)(-1))
        throw Exception("Failed to iconv_open " + from + " -> " + to);

    std::string in_copy = src;
    char *in = const_cast<char *>(in_copy.data());
    size_t in_len = in_copy.size();
    std::ostringstream os;
    bool failed = false;

    while (in_len > 0) {
        char buf[1024];
        char *out = buf;
        size_t out_len = sizeof(buf);

        size_t rc = iconv(cd, &in, &in_len, &out, &out_len);
        int err = errno;
        os.write(buf, out - buf);

        if (rc != (size_t)(-1) || (err == E2BIG && out != buf))
            continue;

        failed = true;
        break;
    }

    iconv_close(cd);

    if (failed)
        LogStream::sendWarning() << "iconv " << from << " -> " << to
                                 << ": conversion stopped, " << in_len << " byte(s) left" << endl;

    return os.str();
}
