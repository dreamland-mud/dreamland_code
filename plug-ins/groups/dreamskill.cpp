#include "logstream.h"
#include "calendar_utils.h"
#include "skill_utils.h"
#include "dreamskill.h"
#include "skillmanager.h"
#include "pcharacter.h"
#include "dreamland.h"
#include "dlscheduler.h"
#include "genericskill.h"
#include "wiznet.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

CLAN(battlerager);
PROF(universal);

DreamSkillManager *dreamSkillManager = 0;
    
DreamSkillManager::DreamSkillManager()
{
    checkDuplicate(dreamSkillManager);
    dreamSkillManager = this;
}

DreamSkillManager::~DreamSkillManager()
{
    dreamSkillManager = 0;
}

int DreamSkillManager::findActiveTemporarySkill(PCharacter *ch) const
{
    PCSkills &skills = ch->getSkills();    
    for (PCSkills::size_type sn = 0; sn < skills.size(); sn++) {
        PCSkillData &data = skills.at(sn);
        if (temporary_skill_active(data)) 
            return sn;
    }

    return -1;
}

long DreamSkillManager::findLatestTemporarySkill(PCharacter *ch) const
{
    PCSkills &skills = ch->getSkills();    
    long latest = -1;

    for (PCSkills::size_type sn = 0; sn < skills.size(); sn++) {
        PCSkillData &data = skills.at(sn);
        if (data.temporary && data.end > latest) {
            latest = data.end;
        }
    }

    return latest;
}

/**
 * Find random 'professional' skill that is not visible to this character.
 */
Skill * DreamSkillManager::findRandomProfSkill(PCharacter *ch) const
{
    int totalFound = 0;
    Skill *result = 0;

    for (int sn = 0; sn < skillManager->size(); sn++) {
        Skill *skill = skillManager->find( sn );

        // Already visible, i.e. part of player's profession or clan.
        if (skill->visible(ch))
            continue;

        // Dreamed about this skill once already in this life.
        if (ch->getSkillData(skill->getIndex()).temporary)
            continue;
        
        // Reduce spell probability for Battleragers.
        if (skill->getSpell() && skill->getSpell()->isCasted())
            if (ch->getClan() == clan_battlerager && chance(70))
                continue;

        // This is not a professional/class skill.
        GenericSkill *genSkill= dynamic_cast<GenericSkill *>(skill);
        if (!genSkill)
            continue;
        if (!genSkill->isProfessional())
            continue;
       
        // Return one skill from all found, with equal probability. 
        if (number_range(0, totalFound++) == 0)
            result = skill;
    }

    return result;
}

/**
 * Show special effects when player is dreaming up a skill/spell.
 * Dream about a battle for group_defense, group_fightmaster etc.
 * Dream about magic/holy books for group_attack, combat, protective.
 * group_vampiric/group_necromancy deserve some effects too.
 */
void DreamSkillManager::describeDream(PCharacter *ch, Skill *skill) const
{
    // TODO
    ch->pecho("Тебе снится странный сон, будто бы ты владеешь %s {c%s{x.", 
              (skill->getSpell() && skill->getSpell()->isCasted() ? "заклинанием" : "умением"),
              skill->getNameFor(ch).c_str());
}

void DreamSkillManager::run( PCharacter *ch ) 
{
    // Exclude awake players, universals and newbies.

    if (ch->position != POS_SLEEPING)
        return;
    
    if (ch->getProfession() == prof_universal)
        return;

    if (ch->getRealLevel() < 20 && ch->getRemorts().size() == 0)
        return;

    // Exclude those with an active dreamt skill.
    int dreamtSkillNumber = findActiveTemporarySkill(ch);
    if (dreamtSkillNumber >= 0) 
        return;

    // Find suitable candidate.
    Skill *skill = findRandomProfSkill(ch);
    if (!skill) {
        wiznet(WIZ_SKILLS, 0, 0, "Для %^C2 нету умений, которые могли бы присниться.", ch);
        return;
    }

    // The less time since last dream - the smaller probability to see another one.
    long latest = findLatestTemporarySkill(ch);
    long today = day_of_epoch(time_info);
    long diff = today - max(latest, 0L);
    if (number_range(0, 200) > diff) 
        return;

    // Set up temporary skill for one "month", learned at 75%.
    PCSkillData &data = ch->getSkillData(skill->getIndex());
    data.temporary = true;
    data.start = today;
    data.end = today + 35;
    data.learned = ch->getProfession()->getSkillAdept();
    ch->save();

    describeDream(ch, skill);
    wiznet(WIZ_SKILLS, 0, 0, "%^C1 (%s) видит во сне умение %s, начало %l, конец %l.", 
           ch, ch->getProfession()->getName().c_str(), skill->getName().c_str(), data.start, data.end);
}

void DreamSkillManager::after( )
{
    DLScheduler::getThis( )->putTaskInSecond( 4, Pointer( this ) );
}

