#ifndef ITEMUTILS_H
#define ITEMUTILS_H

#include "bitstring.h"

class Object;
class Character;

namespace Item {
    bitstring_t furnitureFlags(Object *obj);
    bitstring_t furnitureMaxPeople(Object *obj);

    int countUsers(Object *obj);

    bool canDrop( Character *ch, Object *obj, bool verbose = false);
}


#endif
