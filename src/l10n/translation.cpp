/* Dreamland trilinguality — runtime-message translation catalog.
 * Wave 3 / Trello 2594.
 */
#include "translation.h"

TranslationManager & TranslationManager::getThis()
{
    static TranslationManager instance;
    return instance;
}

const DLString & TranslationManager::run(lang_t lang, const DLString &file, const DLString &ru) const
{
    // RU is the key itself -- no lookup needed, and the universal fallback.
    if (lang == LANG_RU)
        return ru;

    std::map<DLString, PhraseMap>::const_iterator f = catalog.find(file);
    if (f == catalog.end())
        return ru;

    PhraseMap::const_iterator p = f->second.find(ru);
    if (p == f->second.end())
        return ru;

    const DLString &v = (lang == LANG_UA) ? p->second.ua : p->second.en;
    return v.empty() ? ru : v;
}

void TranslationManager::put(const DLString &file, const DLString &ru, const DLString &en, const DLString &ua)
{
    Entry &e = catalog[file][ru];
    e.en = en;
    e.ua = ua;
}

void TranslationManager::clear()
{
    catalog.clear();
}

size_t TranslationManager::phraseCount() const
{
    size_t n = 0;
    for (std::map<DLString, PhraseMap>::const_iterator i = catalog.begin(); i != catalog.end(); ++i)
        n += i->second.size();
    return n;
}

size_t TranslationManager::fileCount() const
{
    return catalog.size();
}
