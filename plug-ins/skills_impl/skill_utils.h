#ifndef SKILL_UTILS_H
#define SKILL_UTILS_H

class Skill;
class Character;
class PCSkillData;

/**
 * Return true if this skill is a temporary one for the character
 * and is active at current day of the year.
 */
bool temporary_skill_active( const Skill *skill, Character *ch );

/**
 * Return true if this skill data corresponds to an active temporary skill.
 */
bool temporary_skill_active( const PCSkillData & );

#endif
