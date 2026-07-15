/* Trilinguality (Trello 2594): a damage noun that declines in the recipient's
 * display language.
 *
 * The attack/skill damage noun ("разрезающий удар", "струя кислоты") is a
 * Grammar::Noun rendered by the act %O/%C/%T/%G machinery, which already passes
 * the recipient to NounHolder::toNoun(forWhom, ...) (act.cpp). A plain
 * InflectedString ignores forWhom and always declines the RU form; this drop-in
 * subclass instead picks the form by viewerLang(forWhom):
 *   RU, or a missing en/ua -> the RU InflectedString, byte-identical to before.
 *   EN -> a caseless single form ("=en": every Case yields `en`).
 *   UA -> the ua string as an InflectedString fullForm (Flexer-declined, or a
 *         leading "=a|b|.." for explicit 6-case UA irregulars).
 * Because it derives from InflectedString, the act arg machinery treats it
 * exactly like an ordinary noun. Zero-regression: RU and empty cells are
 * byte-identical.
 */
#ifndef FIGHT_MULTIINFLECTEDSTRING_H
#define FIGHT_MULTIINFLECTEDSTRING_H

#include "inflectedstring.h"
#include "lang.h"
#include "character.h"

class MultiInflectedString : public InflectedString {
public:
    MultiInflectedString(const DLString &ru, const DLString &en, const DLString &ua,
                         const Grammar::MultiGender &mg)
        : InflectedString(ru, mg), enForm(en), uaForm(ua)
    {
    }

    virtual Grammar::NounHolder::NounPointer toNoun(const DLObject *forWhom = 0, int flags = 0) const
    {
        lang_t lang = LANG_RU;
        const Character *wch = dynamic_cast<const Character *>(forWhom);
        if (wch)
            lang = viewerLang(wch);

        if (lang == LANG_EN && !enForm.empty())
            return InflectedString::Pointer(NEW, DLString("=") + enForm, getMultiGender());
        if (lang == LANG_UA && !uaForm.empty())
            return InflectedString::Pointer(NEW, uaForm, getMultiGender());

        return InflectedString::toNoun(forWhom, flags);
    }

private:
    DLString enForm, uaForm;
};

#endif
