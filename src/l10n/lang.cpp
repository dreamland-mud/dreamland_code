#include "lang.h"

lang_t attr2lang(const DLString &langAttr)
{
    if (langAttr == "en")
        return EN;

    if (langAttr == "ua")
        return UA;

    if (langAttr == "ru")
        return RU;

    return LANG_DEFAULT;
}

DLString lang2attr(lang_t lang)
{
    switch (lang) {
        case EN: return "en";
        case UA: return "ua";
        case RU: return "ru";
        default: return "en";
    }
}
