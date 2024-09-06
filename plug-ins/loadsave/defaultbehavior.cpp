#include "defaultbehavior.h"
#include "feniamanager.h"
#include "logstream.h"

DefaultBehavior::DefaultBehavior()
                    : Behavior()
{

}

void DefaultBehavior::loaded()
{
    // Once loaded from disk, register this behavior with the manager and resolve Fenia wrappers.
    behaviorManager->registrate(Pointer(this));

    if (FeniaManager::wrapperManager) {
        FeniaManager::wrapperManager->linkWrapper(this);
        if (wrapper)
            LogStream::sendNotice() << "Behavior: linked wrapper for " << getName() << endl;
    }    
}

void DefaultBehavior::unloaded()
{
    // Notify Fenia that the wrapper is extracted before unloading the behavior. De-register from the manager.
    if (FeniaManager::wrapperManager)
        if (wrapper)
            extractWrapper(false);    
    
    behaviorManager->unregistrate(Pointer(this));
}

const DLString & DefaultBehavior::getName() const
{
    return Behavior::getName();
}
     
void DefaultBehavior::setName(const DLString &name)
{
    this->name = name;
}
