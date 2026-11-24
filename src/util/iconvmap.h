/* $Id$
 *
 * ruffina, Dream Land, 2018
 */


#ifndef ICONVMAP_H
#define ICONVMAP_H

#include <string>
#include <iconv.h>

/*
 * Charset conversion, one call per string, linear in its length.
 *
 * UTF-8 to KOI8-U is done by table, not by iconv: a character KOI8-U cannot
 * hold becomes '?' and the rest of the string is kept (a "//IGNORE" target
 * drops it instead). A strict target first writes typographic punctuation
 * (dashes, ellipsis, curly and angle quotes) in ASCII, as game text expects;
 * a "//IGNORE" target (config files, web client input) maps only the
 * Ukrainian apostrophe U+02BC, as it always did. Other directions go
 * through iconv.
 */
class IconvMap {
public:
    IconvMap(const char *from, const char *to);
    virtual ~IconvMap();

    std::string operator() (const std::string &src);

    /** UTF-8 typographic punctuation written in ASCII, in one pass. */
    static std::string typographyToAscii(const std::string &utf);

private:
    std::string utf8ToKoi(const std::string &src);
    std::string viaIconv(const std::string &src);

    std::string to;
    std::string from;
    bool ignore;
    bool toKoi;
};

#endif
