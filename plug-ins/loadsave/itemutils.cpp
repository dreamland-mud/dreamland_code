#include "itemutils.h"
#include "core/object.h"

bitstring_t ItemUtils::furnitureFlags(Object *obj)
{
    switch (obj->item_type) {
    default:
        return 0;

    case ITEM_FURNITURE:
        return obj->value2();
    
    case ITEM_FOUNTAIN:
        return obj->value4();
    }
}    
 
bitstring_t ItemUtils::furnitureMaxPeople(Object *obj)
{
    switch (obj->item_type) {
    default:
        return 0;

    case ITEM_FURNITURE:
        return obj->value0();
    
    case ITEM_FOUNTAIN:
        return obj->value3();
    }
}
