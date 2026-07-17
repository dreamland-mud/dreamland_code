/* Dreamland trilinguality — a format string that resolves per recipient.
 * Wave 3 / Trello 2594.
 */
#include "multimessage.h"
#include "translation.h"
#include "lang.h"

const DLString & MultiMessage::getMessage(const Character *ch) const
{
    // viewerLang(ch) is the same resolver Object/PCharacter::toNoun use (core,
    // no libsystem dependency). Delegates to the lang_t overload so the
    // explicit-langs branch is handled in one place.
    return getMessage(viewerLang(ch));
}

const DLString & MultiMessage::getMessage(lang_t lang) const
{
    // Explicit three-language message: return the stored text directly, RU as
    // the universal fallback when the en/ua slot is empty.
    if (explicitLangs) {
        switch (lang) {
            case LANG_EN: return en.empty() ? ru : en;
            case LANG_UA: return ua.empty() ? ru : ua;
            default:      return ru;
        }
    }

    // Catalog-backed message: resolve (file, ru) -> {en,ua}; TranslationManager
    // falls back to RU on any miss.
    return TranslationManager::getThis().run(lang, file, ru);
}
