/* Dreamland trilinguality — runtime-message translation catalog.
 * Wave 3 / Trello 2594. Built to Ruffina's design (Ruffina out indefinitely).
 */
#ifndef L10N_TRANSLATION_H
#define L10N_TRANSLATION_H

#include <map>
#include "dlstring.h"
#include "lang.h"

/** Runtime-message translation catalog, keyed by (FILE, ru_phrase) -> {en, ua}.
 *
 *  RU is BOTH the key and the fallback: any language whose translation is
 *  missing (unknown file, unknown phrase, or empty stored value) resolves to the
 *  ru phrase itself, so output is never blank and wrapping a call site before it
 *  is translated is always safe. Populated by TranslationLoader from the JSON
 *  tree under share/DL/l10n (relative path = FILE key). Single process-wide
 *  instance; reloadable at runtime. */
class TranslationManager {
public:
    static TranslationManager & getThis();

    /** Resolve `ru` within `file` for `lang`. RU (or any empty translation)
     *  falls back to `ru`. Returned reference is valid for the catalog's
     *  lifetime, or is the caller-owned `ru` on fallback. */
    const DLString & run(lang_t lang, const DLString &file, const DLString &ru) const;

    /** Loader API. */
    void put(const DLString &file, const DLString &ru, const DLString &en, const DLString &ua);
    void clear();

    size_t phraseCount() const;
    size_t fileCount() const;

private:
    TranslationManager() { }

    struct Entry {
        DLString en, ua;
    };
    typedef std::map<DLString, Entry> PhraseMap; // ru_phrase -> {en, ua}
    std::map<DLString, PhraseMap> catalog;       // FILE -> PhraseMap
};

#endif
