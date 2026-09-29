#ifndef SKILLLEVELHOOK_H
#define SKILLLEVELHOOK_H

class Skill;
class Character;

/**
 * Extra skill levels from a plugin that fight_core can't depend on (clan ranks).
 * skill_level_bonus() adds them. The plugin sets it on load and must reset it to
 * NULL on unload, or the pointer dangles into unloaded code.
 */
typedef int (*SkillLevelBonusHook)( Skill &, Character * );
extern SkillLevelBonusHook skill_level_bonus_hook;

#endif
