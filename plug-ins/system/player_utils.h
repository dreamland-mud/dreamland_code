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

    DLString title(PCMemoryInterface *pcm, lang_t lang = LANG_DEFAULT);

    /**
     * The stock status line in the reader's own language -- what a character is
     * created with and what 'prompt all' resets to. Labels are kept in step with
     * dreamland_fenia/newbie/nanny (initCreated), which stamps them at creation;
     * the RU form is byte-identical to the string the engine used before there
     * were three. Also lets the command log tell a stock prompt from a custom
     * one without every reader's default counting as "custom".
     */
    DLString defaultPrompt(lang_t lang, bool battle);

    /** True if `prompt` is the stock status line of ANY language. */
    bool isDefaultPrompt(const DLString &prompt, bool battle);
}

#endif