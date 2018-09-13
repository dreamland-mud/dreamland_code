#include "craftskill.h"
#include "craftattribue.h"

#include "skillmanager.h"                                                       
#include "skillgroup.h"                                                       
#include "behavior_utils.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"

SkillGroupReference &CraftSkill::getGroup( )
{
    return group;
}

bool CraftSkill::visible( Character *ch ) const
{
    if (ch->is_npc())
        return false;

    
    XMLAttributeCraft::Pointer attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeCraft>("craft");
    if (!attr)
        return false;

    SubProfessions::const_iterator sp;
    for (sp = subprofessions.begin(); sp != subprofessions.end(); sp++) {
        const DLString &profName = sp->first;
        const Integer &minLevel = sp->second.level;

        if (attr->getProficiencyLevel(profName) >= minLevel)
            return true;
    }

    return false;
}

bool CraftSkill::available( Character *ch ) const
{
}

bool CraftSkill::usable( Character *ch, bool verbose ) const
{
} 

int CraftSkill::getLevel( Character *ch ) const
{
}

int CraftSkill::getLearned( Character *ch ) const
{
}

int CraftSkill::getWeight( Character *ch ) const
{
    return weight;
}

bool CraftSkill::canForget( PCharacter *ch ) const
{
    return false;
}

bool CraftSkill::canPractice( PCharacter *ch, std::ostream &buf ) const
{
}

bool CraftSkill::canTeach( NPCharacter *ch, PCharacter *, bool verbose )
{
}

void CraftSkill::show( PCharacter *ch, std::ostream &buf )
{
} 

