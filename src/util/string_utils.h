#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <list>
#include "lang.h"
#include "stringlist.h"

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

    /** Phonetic Cyrillic -> Latin transliteration (KOI8 chars). Used to
     *  surface a readable Latin form of a Cyrillic name to English viewers
     *  when no explicit English name form is set. Non-Cyrillic chars pass
     *  through; the first output letter is capitalized (single-token names).
     *  Lossy and one-directional -- meant for display defaults, not identity. */
    DLString translitToLatin(const DLString &str);

    /** Rewrite the four letters Ukrainian does not have (э ы ъ ё) into their
     *  Ukrainian spelling, so a Russian-spelled name can be read by Ukrainian
     *  morphology. NOT a full transliteration: и is left alone, because it is a
     *  normal Ukrainian letter and turning it into і is a per-name judgement,
     *  not a mechanical rule. Everything else passes through. */
    DLString ruLettersToUa(const DLString &str);

    /** True when a personal name belongs to a class that does not decline, so a
     *  pad repeating the nominative is the right answer rather than a failure:
     *  vowel-final foreign names (Кворо, Самуро) and feminine names ending in a
     *  consonant (Тайфоэн). Mirrors the rules the Russian decliner in fenia
     *  utils/inflect applies. Names in -а / -я decline and are not included. */
    bool nameIsIndeclinable(const DLString &name, bool female);

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

    DLString join(const std::list<DLString> &list, const DLString &delim);

    // Return a list of all entries in all cases
    StringList getAllForms(const XMLMultiString &);

    // Return a space-separated string of all language entries, in all cases
    DLString toString(const XMLMultiString &);

    // Return a list of all entries in nominative case and without colours
    std::list<DLString> toNormalizedList(const XMLMultiString &);
}

#endif