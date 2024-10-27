#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include "dlstring.h"
#include "lang.h"

class XMLMultiString;

namespace String {
    /** Caseless comparison of two strings. */
    bool equalLess(const DLString &a, const DLString &b);

    /** Truncates the string to given size. */
    DLString truncate(const DLString &str, size_t size);

    /** Truncate the string, remove trailing newline and add ellipsis. */
    DLString ellipsis(const DLString &str, size_t size);

    /** Remove line feed characters from the end of string. */
    DLString stripEOL(const DLString &str);

    /** True if string contains ї or similar. */
    bool hasUaSymbol(const DLString &str);

    /** True if string contains ы or similar. */
    bool hasRuSymbol(const DLString &str);

    /** True if has at least one Cyrillic character. */
    bool hasCyrillic(const DLString &str);

    bool lessCase( const DLString &a, const DLString& b );

    /** True if arg is empty ignoring colours. */
    bool isEmpty(const char *arg);

    /** Returns first non-empty string for given language. */
    const DLString & firstNonEmpty(const XMLMultiString &a, const XMLMultiString &b, lang_t lang);

    /** Adds new line to the existing text and return result. */
    DLString addLine(const DLString &text, const DLString &line);

    /** Remove last line from the text. */
    DLString delLine(const DLString &text);

    /** Shorthand to see if this substring contained within the string. */
    bool contains(const DLString &bigString, const DLString &smallString);

    /** Split string into lines. */
    std::list<DLString> toLines(const DLString &text);

    /** Combine string from lines. */
    DLString fromLines(const std::list<DLString> &lines);
}

#endif