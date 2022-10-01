#ifndef ITEMUTILS_H
#define ITEMUTILS_H

#include "bitstring.h"

class Object;

namespace ItemUtils {
    bitstring_t furnitureFlags(Object *obj);
    bitstring_t furnitureMaxPeople(Object *obj);
}


#endif
