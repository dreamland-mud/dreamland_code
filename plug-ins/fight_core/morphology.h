#ifndef MORPHOLOGY_H
#define MORPHOLOGY_H

#include "grammar_entities.h"

using namespace Grammar;
class DLString;

struct Morphology {
    static DLString adjective(const DLString &normalForm, const MultiGender &gender);

};

#endif