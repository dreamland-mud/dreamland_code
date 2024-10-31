#include <jsoncpp/json/json.h>

#include "areaquestcleanupplugin.h"
#include "xmlattributeareaquest.h"
#include "pcharactermanager.h"
#include "pcharacter.h"
#include "areaquest.h"
#include "areaquestutils.h"
#include "wiznet.h"
#include "configurable.h"
#include "descriptor.h"
#include "interp.h"
#include "dreamland.h"
#include "merc.h"
#include "def.h"

using namespace Scripting;

// A set of area quest settings defined in config/areaquest.json.
Json::Value aquestConfig;
CONFIGURABLE_LOADED(config, areaquest)
{
    aquestConfig = value;
}

void AreaQuestCleanupPlugin::run( int oldState, int newState, Descriptor *d )
{
    Character *ch = d->character;

    if (!ch)
        return;
    
    if (newState != CON_PLAYING) 
        return;
   
    PCharacter *pch = ch->getPC(); 
    auto areaQuestAttr = pch->getAttributes().findAttr<XMLAttributeAreaQuest>("areaquest");
    
    if (!areaQuestAttr)
        return;

    int lifetime = aquestConfig["lifetime"].asInt();
    int cutoffTime = dreamland->getCurrentTime() - lifetime * Date::SECOND_IN_DAY;
    bool changed = false;

    for (auto &aquestDataPair: **areaQuestAttr) {
        const DLString &questId = aquestDataPair.first;
        AreaQuestData &aquestData = aquestDataPair.second;

        if (aquestData.questActive() && aquestData.timestart < cutoffTime) {

            AreaQuest *aquest = get_area_quest(questId);

            if (!aquest) {
                // Some weird non-existing quest in the attributes, just clean it.
                aquestData.cancel();
                wiznet(WIZ_QUEST, 0, 0, "Auto-cancelled obsolete area quest %s for %s", questId.c_str(), pch->getNameC());

            } else if (!aquest->flags.isSet(AQUEST_ONBOARDING|AQUEST_NOEXPIRE)) {
                // Make the player type 'quest cancel <num>', to allow onCancel triggers to run.
                pch->pecho("\r\n{yЗадание {Y%s{y будет отменено из-за неактивности.{x", aquest->title.get(LANG_DEFAULT).c_str());

                interpret_raw(ch, "quest", "cancel %d", aquest->vnum.getValue());

                wiznet(WIZ_QUEST, 0, 0, "Auto-cancelled area quest %s for %s", questId.c_str(), pch->getNameC());
            }
            
            changed = true;      
        }
    }

    if (changed)
        pch->save();
}
