#ifndef L10N_LANG_H
#define L10N_LANG_H

#include "dlstring.h"

typedef enum {
    LANG_MIN = 0,
    LANG_EN = 0,
    LANG_RU = 1,
    LANG_UA = 2,
    EN = LANG_EN,
    RU = LANG_RU,
    UA = LANG_UA,
    LANG_MAX = 3,
    LANG_DEFAULT = RU
} lang_t;

lang_t attr2lang(const DLString &langAttr);
DLString lang2attr(lang_t lang);

/** Pick a message template in the viewer's language.
 *  Returns a static string literal, safe to pass straight to act/pecho as the
 *  format string. Falls back to RU for any language without its own text (i.e.
 *  callers can pass ru==ua when a UA variant isn't ready yet). */
const char * lmsg(lang_t lang, const char *en, const char *ru, const char *ua);

/** A per-recipient text argument for act/fmt/echo. A plain char* arg is
 *  resolved once and shown to every recipient identically; a LangText* passed
 *  to the %w formatter code (act-code $w = arg1, $W = arg2) is resolved to each
 *  viewer's language INSIDE the formatter, so one TO_ROOM message shows every
 *  recipient the word in their own language. The en/ru/ua pointers must
 *  reference storage that outlives the (synchronous) act call; a null/empty en
 *  or ua falls back to ru. */
struct LangText {
    const char *en;
    const char *ru;
    const char *ua;
    const char *get(lang_t lang) const {
        switch (lang) {
        case LANG_EN: return (en && en[0]) ? en : ru;
        case LANG_UA: return (ua && ua[0]) ? ua : ru;
        default:      return ru;
        }
    }
};

class Character;

/** Resolve the display language (EN/RU/UA) of viewer 'wch': an explicit
 *  'config lang' wins, else the legacy rucommands flag; a switched immortal
 *  NPC uses its own language, plain NPCs/null use LANG_DEFAULT. Core-only (no
 *  libsystem / Player:: dependency), so both PCharacter::toNoun and
 *  Object::toNoun share this one implementation. Defined in core/pcharacter.cpp. */
lang_t viewerLang(const Character *wch);


#endif