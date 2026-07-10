/* Dreamland trilinguality — a format string that resolves per recipient.
 * Wave 3 / Trello 2594.
 */
#ifndef MULTIMESSAGE_H
#define MULTIMESSAGE_H

#include "dlstring.h"
#include "lang.h"

class Character;

/** A runtime message carrying its RU source text plus the FILE it came from, so
 *  the same message can be rendered in each recipient's display language at
 *  format time (per-viewer). `_(msg)` in C++ and `._(msg)` in Fenia produce one;
 *  the act/vecho/regfmt format path resolves it against the recipient `to`.
 *  Resolution goes through TranslationManager, which falls back to RU, so a
 *  MultiMessage is always safe even before its catalog entry is translated. */
class MultiMessage {
public:
    MultiMessage() { }
    MultiMessage(const DLString &ru, const DLString &file) : ru(ru), file(file) { }

    /** The message in `ch`'s display language (RU fallback -> never blank). */
    const DLString & getMessage(const Character *ch) const;

    /** The message in an explicit language (RU fallback -> never blank). Used by
     *  the per-language resolver caches in the act/vecho output path. */
    const DLString & getMessage(lang_t lang) const;

    const DLString & getRu() const { return ru; }
    const DLString & getFile() const { return file; }
    bool empty() const { return ru.empty(); }

    /* Whether the message carries `$`-style act codes ($n/$e/...) that must be
     * run through act_to_fmt AFTER per-language resolution. Set only by the C++
     * oldact overloads; plain output (pecho/recho) and the Fenia path leave it
     * false. Kept on the message so a single vecho path handles both. */
    void setActCodes(bool v) { actCodes = v; }
    bool hasActCodes() const { return actCodes; }

private:
    DLString ru;
    DLString file;
    bool actCodes = false;
};

#endif
