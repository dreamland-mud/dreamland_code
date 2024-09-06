#include "behavior.h"
#include "fenia/exceptions.h"
#include "autoflags.h"

Behavior::Behavior()
            : target(INDEX_NONE, &index_data_table)
{
}

Behavior::Behavior(const DLString &n) 
        : target(INDEX_NONE, &index_data_table), name(n)
{
}

Behavior::~Behavior()
{
}

const DLString &Behavior::getName() const
{
    return name;
}

bool Behavior::isValid() const
{
    return false;
}

const DLString &Behavior::getRussianName() const
{
    return nameRus;
}

long long Behavior::getID() const
{
    if (id <= 0)
        throw Scripting::Exception("Behavior ID not set");

    return (id.getValue() << 4) | 10;
}


BehaviorManager* behaviorManager = 0;

BehaviorManager::BehaviorManager() 
{
    checkDuplicate(behaviorManager);
    behaviorManager = this;
    setRegistryName("behavior");    
    saveRegistryName();
}

BehaviorManager::~BehaviorManager()
{
    eraseRegistryName();
    behaviorManager = 0;
}

GlobalRegistryElement::Pointer BehaviorManager::getDumbElement(const DLString &name) const
{
    return Behavior::Pointer(NEW, name);
}

int BehaviorManager::getNextId() const
{
    int nextId = 0;

    for (int i = 0; i < size(); i++) {
        int id = const_cast<BehaviorManager *>(this)->find(i)->id;
        nextId = std::max(id, nextId);
    }
    
    return nextId + 1;
}

GLOBALREF_IMPL(Behavior, '-')
XMLGLOBALREF_IMPL(Behavior)

