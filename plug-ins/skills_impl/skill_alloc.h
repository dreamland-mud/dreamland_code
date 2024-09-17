#ifndef SKILL_ALLOC_H
#define SKILL_ALLOC_H

#include "basicskill.h"

namespace SkillAlloc
{
    BasicSkill::Pointer newClassSkill(const DLString &name);
    BasicSkill::Pointer newClanSkill(const DLString &name);
    BasicSkill::Pointer newOrdenSkill(const DLString &name);
    BasicSkill::Pointer newRaceSkill(const DLString &name);
    BasicSkill::Pointer newOtherSkill(const DLString &name);

    BasicSkill::Pointer newSkill(const DLString &skillName, Skill *refSkill, const DLString &className);

} 

#endif
