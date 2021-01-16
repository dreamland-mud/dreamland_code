#ifndef __ITEMEVENTS_H__
#define __ITEMEVENTS_H__

#include "eventbus.h"

class Object;
struct reset_data;

struct ItemCreatedEvent : public Event {
    ItemCreatedEvent(Object *obj, bool count);
    virtual ~ItemCreatedEvent();
    Object *obj;
    bool count;
};

struct ItemReadEvent : public Event {
    ItemReadEvent(Object *obj);
    virtual ~ItemReadEvent();
    Object *obj;
};

struct ItemResetEvent : public Event {
    ItemResetEvent(Object *obj, struct reset_data *pReset);
    virtual ~ItemResetEvent();
    Object *obj;    
    reset_data *pReset;
};

struct ItemEditedEvent : public Event {
    ItemEditedEvent(Object *obj);
    virtual ~ItemEditedEvent();
    Object *obj;    
};


#endif // __ITEMEVENTS_H__