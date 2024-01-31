#include "areaquest.h"
#include "fenia/exceptions.h"
#include "merc.h"
#include "def.h"

map<int, AreaQuest *> areaQuests;

AreaQuest::AreaQuest()
    : flags(0, &areaquest_flags), 
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
