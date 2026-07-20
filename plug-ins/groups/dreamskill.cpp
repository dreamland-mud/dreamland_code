#include "logstream.h"
#include "calendar_utils.h"
#include "skill_utils.h"
#include "dreamskill.h"
#include "skillmanager.h"
#include "skillgroup.h"
#include "pcharacter.h"
#include "room.h"
#include "dreamland.h"
#include "dlscheduler.h"
#include "genericskill.h"
#include "wiznet.h"
#include "merc.h"

#include "itemflags.h"
#include "def.h"
#include "act.h"
#include "l10n.h"

CLAN(battlerager);
GROUP(fightmaster);
GROUP(weaponsmaster);
GROUP(defensive);
GROUP(necromancy);
GROUP(combat);
GROUP(healing);
GROUP(maladictions);
GROUP(protective);
GROUP(benedictions);
GROUP(illusion);
GROUP(transportation);
GROUP(enchantment);
GROUP(vampiric);

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

int DreamSkillManager::findActiveDreamSkill(PCharacter *ch) const
{
    PCSkills &skills = ch->getSkills();    
    for (PCSkills::size_type sn = 0; sn < skills.size(); sn++) {
        PCSkillData &data = skills.at(sn);
        // It also means no new skills are dreamt while a skill gained from wearing an item is active.
        if (temporary_skill_active(data)) 
            return sn;
    }

    return -1;
}

long DreamSkillManager::findLatestDreamSkill(PCharacter *ch) const
{
    PCSkills &skills = ch->getSkills();    
    long latest = -1;

    for (PCSkills::size_type sn = 0; sn < skills.size(); sn++) {
        PCSkillData &data = skills.at(sn);
        if (data.origin == SKILL_DREAM && data.end > latest) {
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
        if (ch->getSkillData(skill->getIndex()).origin == SKILL_DREAM)
            continue;
        
        // Reduce spell probability for Battleragers to zero.
        if (skill->getSpell() && skill->getSpell()->isCasted())
            if (ch->getClan() == clan_battlerager)
                continue;

        // This is not a professional/class skill.
        GenericSkill *genSkill= dynamic_cast<GenericSkill *>(skill);
        if (!genSkill)
            continue;
        if (!genSkill->isProfessional())
            continue;
        if (!genSkill->checkAlignEthos(ch))
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
    bool isSpell = skill_is_spell(skill);

    if (skill->hasGroup(group_weaponsmaster)) {
        buf << l(ch, "Тебе снится битва с Лаеркаи Мастером.") << endl;
        bitnumber_t weapon = get_weapon_for_skill(skill);
        if (weapon != -1)
            buf << fmt(ch, _("Внезапно у тебя в руке оказывается {c%1$s{x, и ты одним движением уничтожаешь противника!"),
                       weapon_class.message(weapon, '1', viewerLang(ch)).c_str()) << endl;
        else
            buf << fmt(ch, _("Ловко применив навык {c%1$s{x, ты одним движением разделываешься с противником!"), sname.c_str()) << endl;
    }
    else if (skill->hasGroup(group_defensive)) {
        if (ch->death > 0)
            buf << fmt(ch, _("Во сне ты видишь свое недавнее неудачное сражение.\n"
                             "В самый ответственный момент ты уверенно пользуешься навыком {c%1$s{x,\n"
                             "разворачивая исход сражения в выгодную для тебя сторону."), sname.c_str()) << endl;
        else
            buf << fmt(ch, _("Во сне ты видишь свое недавнее сражение.\n"
                             "Ты пользуешься невесть откуда взявшимся навыком {c%1$s{x,\n"
                             "завершая битву еще быстрее и эффектнее, чем это было на самом деле."), sname.c_str()) << endl;
    }
    else if (skill->hasGroup(group_necromancy) || skill->hasGroup(group_vampiric)) {
        // Two whole sentences instead of an inflected skill_what(): only Russian
        // can decline the noun into the frame, English and Ukrainian cannot.
        if (isSpell)
            buf << fmt(ch, _("В леденящем душу замогильном шепоте к тебе приходит тайна заклинания {c%1$s{x."), sname.c_str()) << endl;
        else
            buf << fmt(ch, _("В леденящем душу замогильном шепоте к тебе приходит тайна умения {c%1$s{x."), sname.c_str()) << endl;
    }
    else if (skill->hasGroup(group_fightmaster)) {
        buf << fmt(ch, _("Тебе снится, как ловко ты разделываешься со Здрени Мстителем, применив на него навык {c%1$s{x."), sname.c_str()) << endl;
    }
    else if (skill->hasGroup(group_healing)) {
        buf << fmt(ch, _("Во сне ты бродишь по городу, исцеляя всех, кто попадется тебе под руку, с помощью {c%1$s{x."), sname.c_str()) << endl;
    }
    else if (skill->hasGroup(group_maladictions)) {
        buf << fmt(ch, _("Перед тобой как будто парит темное и прекрасное лицо женщины-дроу.\n"
                         "Она шепчет тебе: '{c%1$s'{x. Что бы это могло означать?"), spell_utterance(skill).c_str()) << endl;
    }
    else if (skill->hasGroup(group_benedictions)) {
        buf << fmt(ch, _("Монашеская жизнь в приснившемся тебе сюжете кажется очень заманчивой.\n"
                         "Ты вряд ли вспомнишь детали, когда проснешься, за исключением одной молитвы:\n"
                         "                   {c%1$s{x."), sname.c_str()) << endl;
    }
    else if (skill->hasGroup(group_protective)) {
        buf << fmt(ch, _("Прошептав во сне заклинание {c%1$s{x, ты отправляешься на битву с Лагом и, конечно же, побеждаешь его."), sname.c_str()) << endl;
    }
    else if (skill->hasGroup(group_combat)) {
        buf << fmt(ch, _("Ты видишь себя как будто со стороны: ты склоняешься над толстым фолиантом \n"
                         "где-то на седьмом этаже Башни Высшего Волшебства. На открытой странице написана формула:\n"
                         "                    {c%1$s{x."), spell_utterance(skill).c_str()) << endl;
    }
    else if (skill->hasGroup(group_transportation)) {
        buf << fmt(ch, _("И уносят тебя, и уносят тебя... три белых кентавра!\n"
                         "Во сне магия перемещения не выглядит такой уж загадочной, оставляя в твоей памяти одно из заклинаний:\n"
                         "                      {c%1$s{x."), sname.c_str()) << endl;
    }
    else if (skill->hasGroup(group_enchantment)) {
        buf << fmt(ch, _("В довольно запутанном сне ты постоянно превращаешь вино в воду, а большие камни -- в летающие диски.\n"
                         "Одно из заклинаний все же задерживается в твоей памяти: это {c%1$s{x."), sname.c_str()) << endl;
    }
    else if (skill->hasGroup(group_illusion)) {
        // The Russian and Ukrainian adjective agrees with the dreamer's sex, so
        // each sex gets its own phrase rather than a glued-on ending.
        if (ch->getSex() == SEX_MALE)
            buf << fmt(ch, _("Никто не может устоять перед тобой -- ты в этом сне такой милашка!\n"
                             "Соблазнительно хлопая своими длинными ресницами, ты применяешь заклинание {c%1$s{x."), sname.c_str()) << endl;
        else
            buf << fmt(ch, _("Никто не может устоять перед тобой -- ты в этом сне такая милашка!\n"
                             "Соблазнительно хлопая своими длинными ресницами, ты применяешь заклинание {c%1$s{x."), sname.c_str()) << endl;
    }
    else if (isSpell && chance(50))
        buf << fmt(ch, _("Во сне тебе открываются тайны ранее недоступного заклинания {c%1$s{x."), sname.c_str()) << endl;
    else if (chance(50)) {
        if (isSpell)
            buf << fmt(ch, _("Тебе снится странный сон, будто бы ты владеешь заклинанием {c%1$s{x."), sname.c_str()) << endl;
        else
            buf << fmt(ch, _("Тебе снится странный сон, будто бы ты владеешь умением {c%1$s{x."), sname.c_str()) << endl;
    }
    else
        buf << fmt(ch, _("Тебя переполняет неизвестно откуда взявшаяся уверенность, что, проснувшись,\n"
                         "ты будешь знать умение {c%1$s{x не хуже любого профессионала!"), sname.c_str()) << endl;

    ch->send_to(buf);
}

void DreamSkillManager::run( PCharacter *ch ) 
{
    // Exclude awake players, newbies and player in mansions.

    if (ch->position != POS_SLEEPING)
        return;
    
    if (ch->getRealLevel() < 20 && ch->getRemorts().size() == 0)
        return;

    // Exclude those with an active dreamt skill (or a skill gained from equipment, obj prog etc).
    int dreamtSkillNumber = findActiveDreamSkill(ch);
    if (dreamtSkillNumber >= 0) 
        return;

    // Find suitable candidate.
    Skill *skill = findRandomProfSkill(ch);
    if (!skill) {
//        wiznet(WIZ_SKILLS, 0, 0, "Для %^C2 нету умений, которые могли бы присниться.", ch);
        return;
    }

    // The less time since last dream - the smaller probability to see another one.
    long latest = findLatestDreamSkill(ch);
    long today = day_of_epoch(time_info);
    long diff = today - max(latest, 0L);
    if (number_range(0, 200) > diff) 
        return;

    // Set up temporary skill for one "month", learned at 75%.
    PCSkillData &data = ch->getSkillData(skill->getIndex());
    data.origin = SKILL_DREAM;
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

