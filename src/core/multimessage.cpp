/* Dreamland trilinguality — a format string that resolves per recipient.
 * Wave 3 / Trello 2594.
 */
#include "multimessage.h"
#include "translation.h"
#include "lang.h"

const DLString & MultiMessage::getMessage(const Character *ch) const
{
    // viewerLang(ch) is the same resolver Object/PCharacter::toNoun use (core,
    // no libsystem dependency). TranslationManager falls back to RU on any miss.
    return TranslationManager::getThis().run(viewerLang(ch), file, ru);
}

const DLString & MultiMessage::getMessage(lang_t lang) const
{
    return TranslationManager::getThis().run(lang, file, ru);
}
