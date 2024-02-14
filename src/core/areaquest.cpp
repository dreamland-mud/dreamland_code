#include "areaquest.h"
#include "fenia/exceptions.h"
#include "idcontainer.h"
#include "hometown.h"
#include "profession.h"
#include "merc.h"
#include "def.h"

map<int, AreaQuest *> areaQuests;

using namespace Scripting;

AreaQuest::AreaQuest()
    : flags(0, &areaquest_flags), 
      align(0, &align_flags),
      hometowns(hometownManager),
      classes(professionManager),
      pAreaIndex(0)
{

}
    
AreaQuest::~AreaQuest()
{

}

long long AreaQuest::getID() const
{
    if (vnum <= 0)
        throw Scripting::Exception("Area quest VNUM not set");

    return (vnum.getValue() << 4) | 9;
}

QuestStep::QuestStep()
{

}

QuestStep::~QuestStep()
{
    
}

Scripting::Register QuestStep::toRegister() const
{
    Register stepReg = Register::handler<IdContainer>();
    IdContainer *step = stepReg.toHandler().getDynamicPointer<IdContainer>();
    step->setField(IdRef("rewardQp"), (int)rewardQp);
    step->setField(IdRef("rewardGold"), (int)rewardGold);
    step->setField(IdRef("rewardExp"), (int)rewardExp);
    step->setField(IdRef("rewardVnum"), (int)rewardVnum);
    step->setField(IdRef("info"), info);
    step->setField(IdRef("beginTrigger"), beginTrigger); 
    step->setField(IdRef("endTrigger"), endTrigger); 
    return stepReg;    

}
