#include "skill_utils.h"
#include "pcharacter.h"
#include "calendar_utils.h"
#include "skill.h"
#include "merc.h"

bool temporary_skill_active( const Skill *skill, Character *ch )
{
    if (ch->is_npc())
        return false;
    
    PCSkillData &data = ch->getPC()->getSkillData(skill->getIndex());
    return temporary_skill_active(data);
}

bool temporary_skill_active( const PCSkillData &data )
{
    if (!data.temporary)
        return false;

    long today = day_of_epoch(time_info);
    long start = data.start.getValue();
    long end = data.end.getValue();
    return (start <= today && today <= end);
}

