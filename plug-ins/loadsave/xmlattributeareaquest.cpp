#include <jsoncpp/json/json.h>

#include "xmlattributeareaquest.h"
#include "util/regexp.h"

#include "fenia/register-impl.h"
#include "idcontainer.h"
#include "regcontainer.h"
#include "wrapperbase.h"
#include "wrappertarget.h"

#include "areaquest.h"
#include "fenia_utils.h"
#include "areaquestutils.h"
#include "dreamland.h"

using namespace Scripting;

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

