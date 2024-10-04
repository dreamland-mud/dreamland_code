#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include "dlstring.h"

namespace String {
    /** Caseless comparison of two strings. */
    bool equalLess(const DLString &a, const DLString &b);

    /** Truncates the string to give size. */
    DLString &truncate(DLString &str, size_t size);

    /** True if string contains ї or similar. */
    bool hasUaSymbol(const DLString &str);

    /** True if string contains ы or similar. */
    bool hasRuSymbol(const DLString &str);

    /** True if has at least one Cyrillic character. */
    bool hasCyrillic(const DLString &str);
}

#endif