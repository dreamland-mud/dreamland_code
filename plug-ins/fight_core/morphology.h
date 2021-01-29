#ifndef MORPHOLOGY_H
#define MORPHOLOGY_H

#include "grammar_entities.h"

using namespace Grammar;
class DLString;

struct Morphology {
    // Substitute ending placeholder "смертельн(ое,ый,ая2,ые)" for
    // given gender with 6 grammar cases, based on grammar/rules.json file.
    static DLString adjective(const DLString &normalForm, const MultiGender &gender);
};

struct Syntax {
    // Remove all modifiers from a phrase, returning a noun: "меч" for "большой ворпальный меч".
    // Performs very basic checks, only really useful for pet names.
    static DLString noun(const DLString &phrase);
};

#endif