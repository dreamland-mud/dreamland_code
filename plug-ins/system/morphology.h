#ifndef MORPHOLOGY_H
#define MORPHOLOGY_H

#include "grammar_entities.h"

using namespace Grammar;
class DLString;
struct Morphology {
    // Substitute ending placeholder "смертельн(ое,ый,ая2,ые)" for
    // given gender with 6 grammar cases, based on grammar/rules.json file.
    static DLString adjective(const DLString &normalForm, const MultiGender &gender);

    // Decide which form of "с/со" preposition to use in front of this noun.
    static DLString preposition_with(const DLString &noun);
};

struct Syntax {
    // Remove all modifiers from a phrase, returning a noun: "меч" for "большой ворпальный меч".
    // Performs very basic checks, only really useful for pet names.
    static DLString noun(const DLString &phrase);

    // Return first EN and RU labels from a list of names, surrounded by lang tags.
    static DLString label(const DLString &names);

    // Return first EN label from a list of names.
    static DLString label_en(const DLString &names);

    // Return first RU label from a list of names.
    static DLString label_ru(const DLString &names);
};

#endif