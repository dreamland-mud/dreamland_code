/* $Id$
 *
 * ruffina, Dream Land, 2018
 */


#ifndef ICONVMAP_H
#define ICONVMAP_H

#include <string>
#include <iconv.h>

/*
 * Charset conversion, one call per string. A character the target charset
 * cannot hold becomes '?' and the rest of the string is kept; with a
 * "//IGNORE" target it is dropped instead. From UTF-8, typographic
 * punctuation (dashes, ellipsis, curly and angle quotes, apostrophes) is
 * first written in ASCII, as the KOI8 game text expects.
 */
class IconvMap {
public:
    IconvMap(const char *from, const char *to);
    virtual ~IconvMap();

    std::string operator() (const std::string &src);

    /** UTF-8 typographic punctuation written in ASCII. */
    static std::string typographyToAscii(const std::string &utf);

private:
    std::string to;
    std::string from;
    bool ignore;
};

#endif
