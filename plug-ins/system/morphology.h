#ifndef MORPHOLOGY_H
#define MORPHOLOGY_H

#include "grammar_entities.h"
#include "xmlmultistring.h"

using namespace Grammar;
class DLString;
namespace Morphology {
    // Substitute ending placeholder "смертельн(ое,ый,ая2,ые)" for
    // given gender with 6 grammar cases, based on grammar/rules.json file.
    DLString adjective(const DLString &normalForm, const MultiGender &gender);

    // Decide which form of "с/со" preposition to use in front of this noun.
    DLString preposition_with(const DLString &noun);

    // Same, in the reader's language: English has one invariant form, Russian
    // and Ukrainian both lengthen the preposition in front of a consonant
    // cluster. Pass the noun already picked in that same language, or the
    // euphony rule judges a word the reader will never see.
    DLString preposition_with(lang_t lang, const DLString &noun);

    // Decline a Ukrainian word into a game Flexer pad (root + per-case delta
    // endings, e.g. "меч||а|еві||ем|еві") by asking the local pymorphy3 sidecar
    // (127.0.0.1:5299). pos = "NOUN"|"ADJF"|"-", gender = "masc"|"femn"|"neut"|"-".
    // Authoring-time only; caches, times out at 500ms, and falls back to the
    // nominative in every case if the sidecar is unreachable (never blocks long).
    DLString declineUa(const DLString &word, const DLString &pos = "-", const DLString &gender = "-");

    // Same as declineUa but for Russian (the sidecar's ru analyzer, loaded on
    // first use). gender = "masc"|"femn"|"neut"|"-".
    DLString declineRu(const DLString &word, const DLString &pos = "-", const DLString &gender = "-");
};

namespace Syntax {
    // Remove all modifiers from a phrase, returning a noun: "меч" for "большой ворпальный меч".
    // Performs very basic checks, only really useful for pet names.
    DLString noun(const DLString &phrase);

    // Return first EN and RU labels from a list of names, surrounded by lang tags.
    DLString label(const DLString &names);

    // Return first EN label from a list of names.
    DLString label_en(const DLString &names);

    // Return first RU label from a list of names.
    DLString label_ru(const DLString &names);

    DLString label_en(const XMLMultiString &); 
    DLString label_ru(const XMLMultiString &); 

};

#endif