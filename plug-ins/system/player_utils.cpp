#include "player_utils.h"
#include "xmlattributestatistic.h"
#include "pcharacter.h"
#include "commonattributes.h"

/** 
 * Someone who lives their first life and haven't done enough successful quests
 * is considered a 'newbie'.
 */
bool Player::isNewbie(PCMemoryInterface *pcm)
{
    if (pcm->getRemorts().size() > 0)   
        return false;
    
    XMLAttributeStatistic::Pointer stats = pcm->getAttributes( ).findAttr<XMLAttributeStatistic>("questdata");
    if (stats && stats->getAllVictoriesCount() > 50)
        return false;
    
    return true;
}

lang_t Player::lang(PCMemoryInterface *pcm)
{
    auto langAttr = pcm->getAttributes().findAttr<XMLStringAttribute>("lang");
    if (!langAttr)
        return LANG_DEFAULT;

    // TODO create a lookup function
    if (langAttr->getValue() == "ua")
        return LANG_UA;
    if (langAttr->getValue() == "ru")
        return LANG_RU;
    if (langAttr->getValue() == "en")
        return LANG_EN;

    return LANG_DEFAULT;
}
