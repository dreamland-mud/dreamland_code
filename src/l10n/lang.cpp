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

const char * lmsg(lang_t lang, const char *en, const char *ru, const char *ua)
{
    switch (lang) {
        case LANG_EN: return en;
        case LANG_UA: return ua;
        default:      return ru;
    }
}
