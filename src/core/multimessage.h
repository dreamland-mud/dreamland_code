/* Dreamland trilinguality — a format string that resolves per recipient.
 * Wave 3 / Trello 2594.
 */
#ifndef MULTIMESSAGE_H
#define MULTIMESSAGE_H

#include "dlstring.h"

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

    const DLString & getRu() const { return ru; }
    const DLString & getFile() const { return file; }
    bool empty() const { return ru.empty(); }

private:
    DLString ru;
    DLString file;
};

#endif
