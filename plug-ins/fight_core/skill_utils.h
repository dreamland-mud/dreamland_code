#ifndef SKILL_UTILS_H
#define SKILL_UTILS_H

#include "dlstring.h"

class Skill;
class CharacterMemoryInterface;
class Character;
class PCharacter;
class PCSkillData;
class XMLSkillReference;
class Object;

/**
 * Return true if this skill is a temporary one for the character
 * and is active at current day of the year.
 */
bool temporary_skill_active( const Skill *skill, CharacterMemoryInterface *ch );

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

/** Return rus and eng names for the skill, ordered according to 'config ruskill'. */
DLString print_names_for(const Skill *skill, Character *ch);

/** Return "Skill" or "Spell" string for the help header. */
DLString print_what(const Skill *skill);

/** Return skill group name with a hyper-link. */
DLString print_group_for(const Skill *skill, Character *ch);

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


// Skill help formatting colours.
extern const char SKILL_HEADER_BG;
extern const char SKILL_HEADER_FG;
extern const char *SKILL_INFO_PAD;

// Return min level the item can be worn or used, considering all bonuses.
// TODO: find a better header.
short get_wear_level( Character *ch, Object *obj );

#endif
