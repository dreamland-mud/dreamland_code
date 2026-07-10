/* Dreamland trilinguality — C++ message wrapping (Trello 2594, Phase 4). */
#include <cstring>
#include "l10n.h"
#include "translation.h"
#include "lang.h"

const char * l10n_file_key(const char *file)
{
    if (file == 0)
        return "";

    // Anchor on the LAST "src/" or "plug-ins/" component (component boundary =
    // start of string or right after a '/'), so any build prefix is stripped.
    const char *best = 0;
    for (const char *p = file; *p; ++p) {
        if (p != file && p[-1] != '/')
            continue;
        if (strncmp(p, "src/", 4) == 0 || strncmp(p, "plug-ins/", 9) == 0)
            best = p;
    }
    if (best != 0)
        return best;

    // No anchor: degrade to the bare basename (RU-fallback only — this only
    // happens for unusual build layouts, harmless).
    const char *slash = strrchr(file, '/');
    return slash ? slash + 1 : file;
}

const char * l10n_run(const Character *ch, const DLString &fileKey, const char *ru)
{
    lang_t lang = viewerLang(ch);
    if (lang == LANG_RU)
        return ru;                 // fast path: RU is the source, no lookup

    // run() returns either a catalog-owned value (stable) or the `ruStr`
    // reference we pass in (which would dangle once ruStr dies). Detect the
    // fallback by identity and hand back the caller's stable literal instead.
    DLString ruStr(ru);
    const DLString &res = TranslationManager::getThis().run(lang, fileKey, ruStr);
    if (&res == &ruStr)
        return ru;
    return res.c_str();
}
