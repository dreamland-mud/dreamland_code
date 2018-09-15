#include "craftattribute.h"
#include "subprofession.h"

XMLAttributeCraft::XMLAttributeCraft( )
{
}

XMLAttributeCraft::~XMLAttributeCraft( )
{
}


int XMLAttributeCraft::proficiencyLevel(const CraftProfession &prof) const
{
    return proficiencyLevel(prof.getName());
}

int XMLAttributeCraft::proficiencyLevel(const DLString &profName) const
{
    Proficiency::const_iterator p = proficiency.find(profName);
    if (p == proficiency.end())
        return 0;
        
    return p->second.level;
}

bool XMLAttributeCraft::learned(const DLString &profName) const
{
    Proficiency::const_iterator p = proficiency.find(profName);
    return p != proficiency.end();
}

