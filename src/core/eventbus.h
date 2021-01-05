#ifndef __EVENTBUS_H__
#define __EVENTBUS_H__

#include <typeinfo>
#include <typeindex>
#include <map>
#include <list>
#include "oneallocate.h"

using namespace std;

/** 
 * Base class for all global events. Has to be of a polymorphic type in order
 * for typeid to work correctly, hence virtual destructor.
 */
class Event {
public:    
    virtual ~Event();
};

/**
 * Base class for anyone capable of handling events of a certain type.
 */
class EventHandler : public virtual DLObject {
public:
    typedef ::Pointer<EventHandler> Pointer;

    virtual void handleEvent(const type_index &eventType, const Event &event) const = 0;
};

/**
 * Singleton that keeps a list of event handlers and invokes them once an event is published.
 */
class EventBus : public OneAllocate {
public:
    EventBus();
    virtual ~EventBus();

    void subscribe(const type_index &eventType, EventHandler::Pointer eventHandler);
    void unsubscribe(const type_index &eventType, EventHandler::Pointer eventHandler);
    void publish(const Event &event) const;

protected:
    map<string, list<EventHandler::Pointer> > handlers;
};

extern EventBus *eventBus;

#endif // __EVENTBUS_H__