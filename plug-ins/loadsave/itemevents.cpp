#include "itemevents.h"

ItemReadEvent::ItemReadEvent(Object *obj) 
{
    this->obj = obj;
}

ItemResetEvent::ItemResetEvent(Object *obj, int level, struct reset_data *pReset) 
{
    this->obj = obj;
    this->level = level;
    this->pReset = pReset;
}

ItemCreatedEvent::ItemCreatedEvent(Object *obj, bool count) 
{
    this->obj = obj;
    this->count = count;
}

