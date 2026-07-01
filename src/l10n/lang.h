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


#endif