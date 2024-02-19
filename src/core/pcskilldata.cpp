/* $Id: pcskilldata.cpp,v 1.1.2.4.10.2 2010-09-05 13:57:11 rufina Exp $
 * 
 * ruffina, 2004
 */
#include "pcskilldata.h"

#include "logstream.h"
#include "xmlmap.h"

#include "skillmanager.h"
#include "skill.h"

#include "pcharacter.h"
#include "autoflags.h"
#include "def.h"

PCSkillData PCSkillData::empty;

long PCSkillData::END_NEVER = -1;

PCSkillData::PCSkillData( )
                    : forgetting( false ), temporary( false ), 
                      origin( SKILL_PRACTICE, &skill_origin_table )
{
}

bool PCSkillData::isValid() const
{
    return learned > 0;
}

bool PCSkillData::isTemporary() const
{
    return origin.getValue() != SKILL_PRACTICE;
}

void PCSkillData::clear()
{
    origin = 0;
    start = 0;
    end = 0;
    learned = 0;
}

PCSkills::PCSkills() 
            : Base(skillManager)
{
}

