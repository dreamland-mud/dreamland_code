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


#endif