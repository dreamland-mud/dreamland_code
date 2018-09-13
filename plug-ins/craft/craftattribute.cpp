#include "craftattribute.h"
#include "subprofession.h"

XMLAttributeCraft::XMLAttributeCraft( )
{
}

XMLAttributeCraft::~XMLAttributeCraft( )
{
}


int XMLAttributeCraft::getProficiencyLevel(const SubProfession &prof) const
{
    return getProficiencyLevel(prof.getName());
}

int XMLAttributeCraft::getProficiencyLevel(const DLString &profName) const
{
    Proficiency::const_iterator p = proficiency.find(profName);
    if (p == proficiency.end())
        return 0;
        
    return p->second.level;
}

