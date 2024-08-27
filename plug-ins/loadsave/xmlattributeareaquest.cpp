#include <jsoncpp/json/json.h>

#include "xmlattributeareaquest.h"
#include "commonattributes.h"
#include "logstream.h"
#include "util/regexp.h"
#include "xmlmap.h"
#include "fenia/register-impl.h"
#include "idcontainer.h"
#include "regcontainer.h"
#include "wrapperbase.h"
#include "wrappertarget.h"
#include "pcharactermanager.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "schedulertaskroundplugin.h"
#include "plugininitializer.h"
#include "object.h"
#include "room.h"
#include "areaquest.h"
#include "fenia_utils.h"
#include "areaquestutils.h"
#include "wiznet.h"
#include "configurable.h"
#include "descriptor.h"
#include "dlscheduler.h"
#include "dreamland.h"
#include "def.h"

using namespace Scripting;

// A set of area quest settings defined in config/areaquest.json.
Json::Value aquestConfig;
CONFIGURABLE_LOADED(config, areaquest)
{
    aquestConfig = value;
}

const DLString XMLAttributeAreaQuest::TYPE = "XMLAttributeAreaQuest";

AreaQuestData::~AreaQuestData()
{

}

void AreaQuestData::handleRemort() 
{ 
    thisLife = 0; 
    step = -1; 
    timestart = 0; 
    timeend = 0; 
}

bool AreaQuestData::questActive() const 
{ 
    return step >= 0;
}

bool AreaQuestData::stepActive(int s) const 
{        
    return step == s;
}

void AreaQuestData::start() 
{
    step = 0;
    timestart = dreamland->getCurrentTime();
    timeend = 0;
}

void AreaQuestData::complete()
{
    step = -1;
    timeend = dreamland->getCurrentTime();
    thisLife++;
    total++;
}

void AreaQuestData::cancel()
{
    step = -1;
    timestart = 0;
}

void AreaQuestData::advance() 
{
    step++;
}		

void AreaQuestData::rollback() 
{
    if (step == 0)
        cancel();
    else
        step--;
}		

Scripting::Register AreaQuestData::toRegister() const
{
    Register stepReg = Register::handler<IdContainer>();
    IdContainer *step = stepReg.toHandler().getDynamicPointer<IdContainer>();
    step->setField(IdRef("step"), (int)this->step);
    step->setField(IdRef("thisLife"), (int)thisLife);
    step->setField(IdRef("timestart"), (int)timestart);
    step->setField(IdRef("timeend"), (int)timeend);
    step->setField(IdRef("total"), (int)total);
    return stepReg;    
}

Scripting::Register XMLAttributeAreaQuest::toRegister() const
{
    Register result = Register::handler<RegContainer>();
    RegContainer *map = result.toHandler().getDynamicPointer<RegContainer>();
    for (auto &aqd: *this) {
        map->setField(aqd.first, aqd.second.toRegister());
    }
    return result;
}

bool XMLAttributeAreaQuest::handle(const RemortArguments &args)
{
    for (auto &aqd: *this) {
        aqd.second.handleRemort();
    }

    return RemortAttribute::handle(args);
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
            aquestData.cancel();

            AreaQuest *aquest = get_area_quest(questId);

            pch->pecho("\r\n{yЗадание {Y%s{y отменено из-за неактивности.{x",
                        (aquest ? aquest->title.c_str() : questId.c_str()));

            changed = true;

            wiznet(WIZ_QUEST, 0, 0, "Auto-cancelled area quest %s for %s", questId.c_str(), pch->getNameC());
        }
    }

    if (changed)
        pch->save();
}
