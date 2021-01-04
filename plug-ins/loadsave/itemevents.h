#ifndef __ITEMEVENTS_H__
#define __ITEMEVENTS_H__

#include "eventbus.h"

class Object;
struct reset_data;

struct ItemCreatedEvent : public Event {
    ItemCreatedEvent(Object *obj, bool count);
    Object *obj;
    bool count;
};

struct ItemReadEvent : public Event {
    ItemReadEvent(Object *obj);
    Object *obj;
};

struct ItemResetEvent : public Event {
    ItemResetEvent(Object *obj, int level, struct reset_data *pReset);
    Object *obj;    
    reset_data *pReset;
    int level;
};


#endif // __ITEMEVENTS_H__