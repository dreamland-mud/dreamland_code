#ifndef SKILL_UTILS_H
#define SKILL_UTILS_H

#include "dlstring.h"

class Skill;
class Character;
class PCharacter;
class PCSkillData;
class XMLSkillReference;

/**
 * Return true if this skill is a temporary one for the character
 * and is active at current day of the year.
 */
bool temporary_skill_active( const Skill *skill, Character *ch );

/**
 * Return true if this skill data corresponds to an active temporary skill.
 */
bool temporary_skill_active( const PCSkillData & );

/** 
 * Return 'magical' phrase for a spell.
 */
DLString spell_utterance(Skill *skill);

Skill * skillref_to_pointer(const XMLSkillReference &);


/**
 * Return colour to display for 'learned' percent.
 */
char skill_learned_colour(const Skill *, PCharacter *ch);

/**
 * Print line 'See also help <skill>' to the buffer.
 */
void print_see_also(const Skill *skill, PCharacter *ch, ostream &buf);

/** Print wait state and mana cost for a skill. */
void print_wait_and_mana(const Skill *skill, Character *ch, ostream &buf);

bool skill_is_spell(const Skill *skill);
DLString skill_what(const Skill *skill);
DLString skill_what_plural(const Skill *skill);

/** Return a cumulative modifier this skill's 'learned' percentage
 *  receives from affects with TO_SKILLS and TO_SKILL_GROUPS 'where' fields.
 */
int skill_learned_from_affects(const Skill *skill, PCharacter *ch);

/**
 * Display effective skill percent if differs from learned.
 */
DLString skill_effective_bonus(const Skill *skill, PCharacter *ch);

/**
 *  Return skill level taking into account affects with APPLY_LEVEL and TO_SKILLS.
 */
int skill_level(Skill &skill, Character *ch);

/**
 *  Return skill level bonus from affects with APPLY_LEVEL and TO_SKILLS.
 */
int skill_level_bonus(Skill &skill, Character *ch);

#endif
