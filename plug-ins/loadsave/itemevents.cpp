#include "itemevents.h"

ItemReadEvent::ItemReadEvent(Object *obj) 
{
    this->obj = obj;
}

ItemReadEvent::~ItemReadEvent() 
{
    
}

ItemResetEvent::ItemResetEvent(Object *obj, int level, struct reset_data *pReset) 
{
    this->obj = obj;
    this->level = level;
    this->pReset = pReset;
}

ItemResetEvent::~ItemResetEvent() 
{
    
}

ItemEditedEvent::ItemEditedEvent(Object *obj) 
{
    this->obj = obj;
}

ItemEditedEvent::~ItemEditedEvent() 
{
    
}

ItemCreatedEvent::ItemCreatedEvent(Object *obj, bool count) 
{
    this->obj = obj;
    this->count = count;
}

ItemCreatedEvent::~ItemCreatedEvent() 
{
    
}

