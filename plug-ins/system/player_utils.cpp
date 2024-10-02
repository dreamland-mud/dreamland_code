#include "player_utils.h"
#include "xmlattributestatistic.h"
#include "pcharacter.h"

/** 
 * Someone who lives their first life and haven't done enough successful quests
 * is considered a 'newbie'.
 */
bool PlayerUtils::isNewbie(PCMemoryInterface *pcm)
{
    if (pcm->getRemorts().size() > 0)   
        return false;
    
    XMLAttributeStatistic::Pointer stats = pcm->getAttributes( ).findAttr<XMLAttributeStatistic>("questdata");
    if (stats && stats->getAllVictoriesCount() > 50)
        return false;
    
    return true;
}

