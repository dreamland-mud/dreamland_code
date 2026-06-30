#ifndef PLAYER_UTILS_SYSTEM_H
#define PLAYER_UTILS_SYSTEM_H

#include "lang.h"
#include "dlstring.h"

class PCharacter;
class PCMemoryInterface;
class Character;

namespace Player {
    bool isNewbie(PCMemoryInterface *pcm);

    lang_t lang(Character *ch);

    /**
     * Effective language for rendering names/content to this viewer.
     * Precedence: an explicit 'config lang' choice (ua/ru/en) wins; when the
     * player has never set it, fall back to the legacy 'rucommands' flag
     * (RU if set, EN otherwise). Use this -- not lang() -- for display so that
     * English players who never touched 'config lang' keep seeing English.
     */
    lang_t displayLang(Character *ch);

    DLString title(PCMemoryInterface *pcm);
}

#endif