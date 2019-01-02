#include "logstream.h"
#include "calendar_utils.h"
#include "skill_utils.h"
#include "dreamskill.h"
#include "skillmanager.h"
#include "skillgroup.h"
#include "pcharacter.h"
#include "dreamland.h"
#include "dlscheduler.h"
#include "genericskill.h"
#include "wiznet.h"
#include "merc.h"
#include "mercdb.h"
#include "itemflags.h"
#include "def.h"

CLAN(battlerager);
PROF(universal);
GROUP(fightmaster);
GROUP(weaponsmaster);
GROUP(defensive);
GROUP(necromancy);
GROUP(combat);
GROUP(attack);
GROUP(healing);
GROUP(vampiric);
GROUP(maladictions);
GROUP(protective);
GROUP(benedictions);
GROUP(curative);
GROUP(harmful);
GROUP(beguiling);
GROUP(transportation);
GROUP(creation);
RELIG(none);

bitnumber_t get_weapon_for_skill(Skill *skill);
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
 */
void DreamSkillManager::describeDream(PCharacter *ch, Skill *skill) const
{
    ostringstream buf;
    DLString sname = skill->getNameFor(ch);
    bool isSpell = skill->getSpell() && skill->getSpell()->isCasted();

    if (skill->getGroup() == group_weaponsmaster) {
        buf << "Тебе снится битва с Лаеркаи Мастером." << endl;
        bitnumber_t weapon = get_weapon_for_skill(skill);       
        if (weapon != -1)  
            buf << "Внезапно у тебя в руке оказывается {c" << weapon_class.message(weapon) << "{x, и ты одним движением уничтожаешь противника!" << endl;
        else
            buf << "Ловко применив умение {c" << sname << ", ты одним движением разделываешься с противником!" << endl; 
    }
    else if (skill->getGroup() == group_defensive) {
        if (ch->death > 0) 
            buf << "Во сне ты видишь свое недавнее неудачное сражение." << endl
                << "В самый ответственный момент ты уверенно пользуешься умением {c" << sname << "{x," << endl
                << "разворачивая исход сражения в выгодную для тебя сторону." << endl;
        else
            buf << "Во сне ты видишь свое недавнее сражение." << endl
                << "Ты пользуешься невесть откуда взявшимся навыком {c" << sname << "{x," << endl
                << "завершая битву еще быстрее и эффектнее, чем это было на самом деле." << endl;
    }
    else if (skill->getGroup() == group_vampiric || skill->getGroup() == group_necromancy) {
        buf << "В леденящем душу замогильном шепоте к тебе приходит тайна " << (isSpell ? "заклинания" : "умения") << " {c" << sname << "{x." << endl;
    }
    else if (skill->getGroup() == group_fightmaster) {
        buf << "Тебе снится, как ловко ты разделываешься с Даркеном Ралом, применив на него умение {c" << sname << "{x." << endl;
    }
    else if (skill->getGroup() == group_healing || skill->getGroup() == group_curative) {
        buf << "Во сне ты бродишь по городу, исцеляя всех, кто попадется тебе под руку, молитвой {c" << sname << "{x." << endl;
    }
    else if (skill->getGroup() == group_maladictions) {
        buf << "Перед тобой как будто парит темное и прекрасное лицо женщины-дроу." << endl
            << "Она шепчет тебе: '{c" << spell_utterance(skill) << "'{x. Что бы это могло означать?" << endl;
    }
    else if (skill->getGroup() == group_benedictions) {
        buf << "Монашеская жизнь в приснившемся тебе сюжете кажется очень заманчивой." << endl
            << "Ты вряд ли вспомнишь детали, когда проснешься, за исключением одной молитвы:" << endl
            << "                   {c" << sname << "{x." << endl; 
    }
    else if (skill->getGroup() == group_harmful) {
        DLString relig = ch->getReligion() == god_none ? "неизвестное божество" : ch->getReligion()->getRussianName().ruscase('1');
        buf << "Во сне " << relig << " как будто направляет твою руку, и ты повергаешь врагов молитвой {c" << sname << "{x." << endl;
    }
    else if (skill->getGroup() == group_protective) {
        buf << "Прошептав во сне заклинание {c" << sname << "{x, ты отправляешься на битву с Лагом и, конечно же, побеждаешь его." << endl;
    }
    else if (skill->getGroup() == group_combat) {
        buf << "Ты видишь себя как будто со стороны: ты склоняешься над толстым фолиантом " << endl
            << "где-то на седьмом этаже Башни Высшего Волшебства. На открытой странице написана формула:" << endl
            << "                    {c" << spell_utterance(skill) << "{x." << endl;
    }
    else if (skill->getGroup() == group_attack) {
        buf << "Сила твоей веры во сне так сильна, что позволяет тебе сражать противников молитвой {c" << sname << "{x." << endl;
    }
    else if (skill->getGroup() == group_transportation) {
        buf << "И уносят тебя, и уносят тебя... три белых кентавра!" << endl
            << "Во сне магия перемещения не выглядит такой уж загадочной, оставляя в твоей памяти одно из заклинаний:" << endl
            << "                      {c" << sname << "{x." << endl;
    }
    else if (skill->getGroup() == group_creation) {
        buf << "В довольно запутанном сне ты постоянно превращаешь вино в воду, а большие камни - в летающие диски." << endl
            << "Одно из заклинаний все же задерживается в твоей памяти: это {c" << sname << "{x." << endl;
    }
    else if (skill->getGroup() == group_beguiling) {
        buf << "Никто не может устоять перед тобой - ты в этом сне так" << (ch->getSex() == SEX_MALE ? "ой" : "ая") << " милашка!" << endl
            << "Соблазнительно хлопая своими длинными ресницами, ты применяешь заклинание {c" << sname << "{x." << endl;
    }
    else if (isSpell && chance(50))
        buf << "Во сне тебе открываются тайны ранее недоступного заклинания {c" << sname << "{x." << endl;
    else if (chance(50))
        buf << "Тебе снится странный сон, будто бы ты владеешь " << (isSpell ? "заклинанием" : "умением") << " {c" << sname << "{x." << endl;
    else
        buf << "Тебя переполняет неизвестно откуда взявшаяся уверенность, что, проснувшись," << endl
            << "ты будешь знать умение {c" << sname << "{x не хуже любого профессионала!" << endl;

    ch->send_to(buf);
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

