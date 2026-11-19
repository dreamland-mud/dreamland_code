#ifndef ITEMUTILS_H
#define ITEMUTILS_H

#include "bitstring.h"
#include "dlstring.h"

class Object;
class Character;
struct obj_index_data;

namespace Item {
    bitstring_t furnitureFlags(Object *obj);
    bitstring_t furnitureMaxPeople(Object *obj);

    int countUsers(Object *obj);

    /** Furniture messages (sit, rest, sleep, stand, wake) a builder sets with
     *  oedit 'msg' live in the prototype props as props[FURNITURE_PROPS][tag].
     *  A Fenia field with the same tag name on the instance or the prototype
     *  wins over them (bodyposition oprog_msg). */
    extern const char *FURNITURE_PROPS;
    /** Every tag bodyposition looks up, null-terminated. */
    extern const char *furnitureMsgTags[];
    /** The prototype props message for a tag, empty if none. */
    DLString furnitureMsgProp(const obj_index_data *pObj, const char *tag);

    bool canDrop( Character *ch, Object *obj, bool verbose = false);
}


#endif
