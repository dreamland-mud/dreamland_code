/* Dreamland trilinguality — C++ message wrapping (Trello 2594, Phase 4).
 *
 * The C++ analog of Fenia's `._()`: wrap a hardcoded-RU output literal so the
 * translation catalog can resolve it in each viewer's display language, with
 * RU as source text AND universal fallback. Mirrors the proven Fenia path
 * (feniaroot: MultiMessageWrapper + regfmt) — same TranslationManager, same
 * (FILE, ru) keying, same RU fallback, so a wrapped-but-untranslated site is
 * byte-identical to the unwrapped original.
 *
 * Two spellings:
 *   _(msg)      — defers resolution: builds a MultiMessage(msg, FILE) that the
 *                 output overloads (pecho/recho/echo/oldact/fmt/ptc/Room::echo)
 *                 resolve per recipient. Use for any output call.
 *   l(ch, msg)  — resolves NOW for one known viewer, returns a stable const char*.
 *                 Use for string-building (ostringstream), table lookups (fight
 *                 dam-verbs), stc(), and anywhere a plain C-string is needed.
 *
 * Include this header ONLY in .cpp files that wrap literals — it defines the
 * function-like `_`/`l` macros. Headers that merely declare MultiMessage
 * overloads include "multimessage.h" (the type) instead, so the macros never
 * leak into every translation unit.
 */
#ifndef L10N_L10N_H
#define L10N_L10N_H

#include "dlstring.h"
#include "lang.h"
#include "translation.h"
#include "multimessage.h"

class Character;

/* Derive the catalog FILE key from a __FILE__ path, anchoring on the LAST
 * "src/" or "plug-ins/" path component so the key is build-layout independent:
 *   ../../../dreamland_code/plug-ins/comm/look.cpp  -> plug-ins/comm/look.cpp
 *   /abs/.../dreamland_code/src/core/character.cpp  -> src/core/character.cpp
 *   look.cpp (no anchor found)                      -> look.cpp  (degraded:
 *                                                      RU-fallback only)
 * Returns a pointer INTO `file`; __FILE__ has static storage, and L10N_FILE
 * copies the result into a per-site static DLString on first use. */
const char * l10n_file_key(const char *file);

/* Resolve `ru` (a literal, in `fileKey`) for viewer `ch`, returning a stable
 * const char*: the original literal on RU/fallback, or the catalog-owned value
 * otherwise. Never returns a dangling pointer. */
const char * l10n_run(const Character *ch, const DLString &fileKey, const char *ru);

/* Per-call-site cached FILE key: one scan + one DLString alloc per site, ever. */
#define L10N_FILE \
    ([]() -> const DLString & { \
        static const DLString k = l10n_file_key(__FILE__); \
        return k; \
    }())

#define _(msg)     MultiMessage((msg), L10N_FILE)
#define l(ch, msg) l10n_run((ch), L10N_FILE, (msg))

#endif
