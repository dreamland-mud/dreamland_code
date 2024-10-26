#include <algorithm>
#include "skill.h"
#include "skillreference.h"
#include "skillcommand.h"
#include "spell.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "commandtemplate.h"
#include "arg_utils.h"
#include "act.h"
#include "wearlocation.h"
#include "commandflags.h"
#include "wearloc_utils.h"
#include "move_utils.h"
#include "morphology.h"
#include "comm.h"
#include "interp.h"
#include "loadsave.h"
#include "damageflags.h"
#include "profflags.h"
#include "merc.h"
#include "def.h"

using namespace std;

GSN(sneak);
GSN(detect_hide);
GSN(second_weapon);
GSN(pick_lock);
GSN(bash_door);
GSN(hide);
GSN(concentrate);
GSN(recall);
GSN(lash);
GSN(axe);
GSN(bash);
GSN(hand_to_hand);
GSN(sword);
GSN(polearm);
GSN(dagger);
GSN(whip);
GSN(grip);
GSN(mace);
GSN(shield_block);
GSN(flail);
GSN(slice);

static void sortCommandsFor(vector<Skill::Pointer> &skills, Character *looker)
{
    sort(skills.begin(), skills.end(), [looker](const Skill::Pointer& left, const Skill::Pointer& right)
        {
            return left->getCommand()->getNameFor(looker)
                .compareRussian(right->getCommand()->getNameFor(looker)) < 0;
        });
}

static void sortSkillsFor(vector<Skill::Pointer> &skills, Character *looker)
{
    sort(skills.begin(), skills.end(), [looker](const Skill::Pointer& left, const Skill::Pointer& right)
        {
            return left->getNameFor(looker)
                .compareRussian(right->getNameFor(looker)) < 0;
        });
}

/**
 * Don't output specific skills in report function
 */
static bool skill_is_invalid(int sn, bool noCarry)
{
    if(noCarry){
        if(sn == gsn_lash
         || sn == gsn_bash
         || sn == gsn_hand_to_hand
         || sn == gsn_sword
         || sn == gsn_polearm
         || sn == gsn_dagger
         || sn == gsn_whip
         || sn == gsn_grip
         || sn == gsn_axe
         || sn == gsn_mace
         || sn == gsn_shield_block
         || sn == gsn_flail
         || sn == gsn_slice
         || sn == gsn_second_weapon
         || sn == gsn_pick_lock    
         ) return true;
    }
    return false;
}

/**
 * Don't output specific skills in report function
 */
static bool skill_is_invalid_in_fight(int sn)
{
    if(sn == gsn_recall
         || sn == gsn_concentrate
         || sn == gsn_hide
         || sn == gsn_sneak
         || sn == gsn_detect_hide
         || sn == gsn_pick_lock
         || sn == gsn_bash_door
         ) return true;
    
    return false;
}

CMDRUNP(report)
{
    DLString args = argument;
    DLString arg = args.getOneArgument();

    if (!ch->is_npc() || !IS_CHARMED(ch)) {
        say_fmt("У меня %3d/%4d жизни (hp) %5d/%6d энергии (mana) %7d/%8d движения (mv).", ch, 0,
                ch->hit.getValue(), ch->max_hit.getValue(),
                ch->mana.getValue(), ch->max_mana.getValue(),
                ch->move.getValue(), ch->max_move.getValue());
        return;
    }

    NPCharacter *pet = ch->getNPC();
    if (!pet->master || pet->master->is_npc())
        return;

    vector<Skill::Pointer> skills, skillsFight, spells, passives;
    ostringstream result;
    bool showAll = arg_is(arg, "all");
    bool shown = false;
    bool noCarry = Char::canCarryNumber(pet) == 0;
    noCarry |= !pet->wearloc.isSet(wear_wield);

    for (int sn = 0; sn < SkillManager::getThis()->size(); sn++) {
        Skill::Pointer skill = SkillManager::getThis()->find(sn);
        Spell::Pointer spell = skill->getSpell();
        Command::Pointer cmd = skill->getCommand().getDynamicPointer<Command>();
        bool passive = skill->isPassive();

        if (!skill->usable(pet, false))
            continue;

        if (cmd && !cmd->getExtra().isSet(CMD_NO_INTERPRET)) {
            bool canOrder = cmd->properOrder(pet) == RC_ORDER_OK;
            pet->fighting = ch;
            bool canOrderFight = cmd->properOrder(pet) == RC_ORDER_OK && !skill_is_invalid_in_fight(sn);
            pet->fighting = 0;

            if(skill_is_invalid(sn, noCarry))
                continue;
            
            if (sn == gsn_second_weapon) {
                if (canOrder && pet->wearloc.isSet(wear_second_wield))
                    skills.push_back(skill);
                continue;
            }

            if (canOrderFight)
                skillsFight.push_back(skill);
            else if (canOrder && showAll)
                skills.push_back(skill);

            continue;
        }

        if (spell && spell->isCasted()) {
            if (spell->properOrder(pet)) {
                if (showAll) {
                    spells.push_back(skill);
                    continue;
                } else if (IS_SET(spell->getTarget(), TAR_CHAR_ROOM)) {
                    spells.push_back(skill);
                    continue;
                }
            }
            continue;
        }

        if (showAll && passive && !skill_is_invalid(sn, noCarry) )
            passives.push_back(skill);
    }

    result << fmt(0, "%1$^C1 говорит тебе \'{G%N2, я %d уровня, у меня %d/%d жизни и %d/%d маны.{x\'",
               pet,
               GET_SEX(pet->master, "Хозяин", "Хозяин", "Хозяйка"),
               pet->getModifyLevel(),
               pet->hit.getValue(), pet->max_hit.getValue(),
               pet->mana.getValue(), pet->max_mana.getValue())
            << endl;

    if (!skills.empty()) {
        ostringstream buf;
        sortCommandsFor(skills, pet->master);
        for (auto it = skills.begin(); it != skills.end();) {
            buf << "{G"
                << "{hh" << (*it)->getSkillHelp()->getID()
                << (*it)->getCommand()->getNameFor(pet->master)
                << "{x";
            if (++it != skills.end())
                buf << ", ";
            else
                buf << "{x" << endl;
        }

        result << "Небоевые умения: " << buf.str();
        shown = true;
    }

    if (!skillsFight.empty()) {
        ostringstream buf;
        sortCommandsFor(skillsFight, pet->master);
        for (auto it = skillsFight.begin(); it != skillsFight.end();) {
            buf << "{Y"
                << "{hh" << (*it)->getSkillHelp()->getID()
                << (*it)->getCommand()->getNameFor(pet->master)
                << "{x";
            if (++it != skillsFight.end())
                buf << ", ";
            else
                buf << "{x" << endl;
        }

        result << "В бою мне можно приказать: " << buf.str();
        shown = true;
    }

    if (!spells.empty() && pet->getProfession()->getFlags(pet).isSet(PROF_CASTER)) {
        ostringstream buf;
        sortSkillsFor(spells, pet->master);
        for (auto it = spells.begin(); it != spells.end();) {
            buf << "{g"
                << "{hh" << (*it)->getSkillHelp()->getID()
                << (*it)->getNameFor(pet->master)
                << "{x";
            if (++it != spells.end())
                buf << ", ";
            else
                buf << "{x" << endl;
        }

        if (showAll)
            result << "Я владею такими заклинаниями: " ;
        else 
            result << "Я могу наложить на тебя такие заклинания: ";
        result << buf.str();
        shown = true;
    }

    if (showAll && !passives.empty()) {
        ostringstream buf;
        sortSkillsFor(passives, pet->master);
        for (auto it = passives.begin(); it != passives.end();) {
            buf << "{W"
                << "{hh" << (*it)->getSkillHelp()->getID()
                << (*it)->getNameFor(pet->master)
                << "{x";
            if (++it != passives.end())
                buf << ", ";
            else
                buf << "{x" << endl;
        }

        result << "Мои пассивные умения: " << buf.str();
        shown = true;
    }

    if (IS_SET(pet->act, ACT_RIDEABLE))
        result << "На мне можно {y{hh1376ездить верхом{x! ";
    if (can_fly(pet))
        result << "Я умею {hh1018летать{x. ";
    if (!pet->master->getPC()->pet || pet->master->getPC()->pet != pet)
        result << "Мне можно {hh1024дать{x вещи и приказать их {hh990надеть{x. ";
    result << "Мне можно приказать {hh1020спать{x и другие стандартные команды." << endl;

    if (shown) {

        if (!showAll) {
            DLString petName = Syntax::noun(pet->getShortDescr(LANG_DEFAULT));
            result << fmt(0, "Напиши {y{hcприказать %1$N3 рапорт все{x, и я расскажу, что ещё я умею делать.", 
                             petName.c_str())
                   << endl;
        }

        result << endl << "См. также {y{hh1091? приказать{x и {y{hh1005? рапорт{x" 
               << endl;

        page_to_char(result.str().c_str(), pet->master);

    }
    else {
        page_to_char(result.str().c_str(), pet->master);
        tell_raw(pet->master, pet, "%s, больше я ничегошеньки не умею!", GET_SEX(pet->master, "Хозяин", "Хозяин", "Хозяйка"));
        interpret_raw(pet, "abat", "");
    }
}
