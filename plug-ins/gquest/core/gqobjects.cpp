#include "gqobjects.h"
#include "globalquestmanager.h"
#include "globalquest.h"
#include "core/object.h"
#include "character.h"
#include "loadsave.h"
#include "merc.h"
#include "def.h"

// Extract items from expired quests.
bool GlobalQuestObject::save( )
{
    if (!hasQuest())
        return false;

    if (myQuestIsRunning())
        return false;

    if (obj->carried_by)
        obj->carried_by->pecho("%^O1 исчезает.", obj);
        
    extract_obj(obj);
    return true;
}

bool GlobalQuestObject::myQuestIsRunning() const
{
    if (!hasQuest())
        return false;

    // Find an active quest with the given ID and see if it's the same instance or not.
    GlobalQuest::Pointer gq = GlobalQuestManager::getThis( )->findGlobalQuest(questID);
    if (!gq)
        return false;

    if (gq->getStartTime() != questStartTime)
        return false;

    return true;
}

void GlobalQuestObject::setQuest(const GlobalQuest &gquest)
{
    questID = gquest.getQuestID();
    questStartTime = gquest.getStartTime();
}

bool GlobalQuestObject::hasQuest() const
{
    if (questID.empty() || questStartTime <= 0)
        return false;

    return true;
}