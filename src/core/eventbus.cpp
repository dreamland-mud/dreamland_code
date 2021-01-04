#include "eventbus.h"
#include "logstream.h"

EventBus *eventBus = 0;

EventBus::EventBus() 
{
    checkDuplicate(eventBus);
    eventBus = this;
}

EventBus::~EventBus() 
{
    eventBus = 0;
}

void EventBus::subscribe(const type_index &eventType, EventHandler::Pointer eventHandler) 
{
    handlers[eventType].push_back(eventHandler);
    notice("EventBus: handler %s [%d total] subscribed to %s events",
            typeid(**eventHandler).name(), handlers[eventType].size(), 
            eventType.name());
}

void EventBus::unsubscribe(const type_index &eventType, EventHandler::Pointer eventHandler) 
{
    auto h = handlers.find(eventType);
    if (h == handlers.end())
        return;

    h->second.remove(eventHandler);
    notice("EventBus: handler %s unsubscribed from %s events",
            typeid(**eventHandler).name(), eventType.name());
}

void EventBus::publish(const Event &event) const
{
    const type_index &eventType = typeid(event);
    auto h = handlers.find(eventType);
    if (h == handlers.end())
        return;

    for (auto &eventHandler: h->second)
        eventHandler->handleEvent(eventType, event);
}

Event::~Event() 
{
    
}
