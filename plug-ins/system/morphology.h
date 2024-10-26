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