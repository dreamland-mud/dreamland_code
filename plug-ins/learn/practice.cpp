/* $Id: practice.cpp,v 1.1.2.15.6.10 2009/01/01 14:13:18 rufina Exp $
 *
 * ruffina, 2004
 */

#include <vector>
#include <map>

#include "practice.h"

#include "class.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "skill.h"
#include "skillmanager.h"
#include "spell.h"
#include "clanreference.h"
#include "pcharacter.h"
#include "room.h"
#include "../loadsave/behavior_utils.h"
#include "areaquestutils.h"
#include "occupations.h"
#include "skill_utils.h"
#include "comm.h"
#include "loadsave.h"
#include "msgformatter.h"

#include "merc.h"
#include "act.h"
#include "def.h"

CLAN(battlerager);

static bool mprog_cant_teach( PCharacter *client, NPCharacter *teacher, const char *skillName )
{
    if (behavior_trigger(teacher, "CantTeach", "CCs", teacher, client, skillName))
        return true;

    FENIA_CALL( teacher, "CantTeach", "Cs", client, skillName );
    FENIA_NDX_CALL( teacher, "CantTeach", "CCs", teacher, client, skillName );
    return false;
}

static void mprog_teach( PCharacter *client, NPCharacter *teacher, const char *skillName )
{
    aquest_trigger(teacher, client, "Teach", "CCs", teacher, client, skillName);
    behavior_trigger(teacher, "Teach", "CCs", teacher, client, skillName);
}

COMMAND(CPractice, "practice")
{
    DLString argument = constArguments;

    if (ch->is_npc( ))
        return;
        
    if (argument.empty( ) || arg_is_all( argument ))
        pracShow( ch->getPC( ) );
    else if (arg_is_strict(argument, "here"))
        pracHere( ch->getPC( ) );
    else
        pracLearn( ch->getPC( ), argument );
}

struct PracInfo {
    int percent;
    int adept;
    int maximum;
    int help_id;
    DLString name;

    inline const char * getNameColor( ) const
    {
        return "{x";
    }
    inline const char * getPercentColor( ) const
    {
        if (percent == 1)
            return "{R";
        else if (percent >= maximum)
            return "{C";
        else if (percent >= adept)
            return "{c";
        else
            return "{x";
    }
};

typedef std::vector<PracInfo> PracInfoList;
typedef std::map<bitnumber_t, PracInfoList> PracCategoryMap;


void CPractice::pracShow( PCharacter *ch )
{
    ostringstream buf;
    PracCategoryMap categoryMap;
    bool fRussian = ch->getConfig( ).ruskills;
    
    for (int sn = 0; sn < SkillManager::getThis( )->size( ); sn++) {
        Skill *skill = SkillManager::getThis( )->find( sn );

        if (!skill->available( ch ))
            continue;
        
        if (skill->getSpell( ) 
                && skill->getSpell( )->isCasted( ) 
                && ch->getClan( ) == clan_battlerager)
            continue;

        PCSkillData &data = ch->getSkillData( sn );

        PracInfo info;
        info.percent = data.learned;
        info.name = skill->getNameFor( ch ).ruscase('1');
        info.adept = skill->getAdept( ch );
        info.maximum = skill->getMaximum( ch );

        if (skill->getSkillHelp() && skill->getSkillHelp()->getID() > 0)
            info.help_id = skill->getSkillHelp()->getID();
        else
            info.help_id = 0;

        categoryMap[skill->getCategory()].push_back(info);
    }

    const char *patternNoHelp = (fRussian ? "%s%-27s %s%3d%%{x" : "%s%-16s%s%3d%%{x");
    const char *patternHelp = (fRussian ? "%s{hh%d%-27s{hx %s%3d%%{x" : "%s{hh%d%-16s{hx%s%3d%%{x");
    const int columns = (fRussian ? 2 : 3);
    int count = 0;

    for (auto &c: categoryMap) {
        bitnumber_t category = c.first;
        DLString categoryName = skill_category_flags.messages(category).upperFirstCharacter();

        buf << "{W" << categoryName << ":{x" << endl;
        
        for (auto &info: c.second) {
            
            if (info.help_id > 0)
                buf << fmt(0, patternHelp,
                                info.getNameColor( ), 
                                info.help_id,
                                info.name.c_str( ),
                                info.getPercentColor( ),
                                info.percent );
            else
                buf << fmt(0, patternNoHelp,
                                info.getNameColor( ), 
                                info.name.c_str( ),
                                info.getPercentColor( ),
                                info.percent );

            buf << "     ";

            count++;

            if (count % columns == 0)
                buf << endl;
        }

        if (count % columns)
            buf << endl;
            
        buf << endl;
    }
    
    buf << fmt(0, "У тебя %d сесси%s практики.\n\r",
                 ch->practice.getValue( ), GET_COUNT(ch->practice, "я","и","й") );

    page_to_char( buf.str( ).c_str( ), ch );
}

void CPractice::pracHere( PCharacter *ch )
{
    std::basic_ostringstream<char> buf;
    Character *teacher;
    const char *patternNoHelp = "%-27s ";
    const char *patternHelp = "{hh%d%-27s{hx ";
    const char *patternLearned = "изучено на %s%d%%{x";
    PracInfo info;
    bool found = false;

    if (!( teacher = findTeacher( ch ) ))
        if (!( teacher = findPracticer( ch ) )) {
            ch->pecho("Тебе не с кем практиковаться здесь.");
            return;
        }

    buf << fmt( ch, "%1$^C1 может помочь тебе практиковаться в таких умениях:", teacher ) << endl;

    for (int sn = 0; sn < SkillManager::getThis( )->size( ); sn++) {
        ostringstream errbuf;
        Skill *skill = SkillManager::getThis( )->find( sn );

        if (!skill->available( ch ) || !skill->canPractice( ch, errbuf ))
            continue;

        if (ch->getSkillData( sn ).learned >= skill->getAdept( ch ))
            continue;

        if (teacher->is_npc( ) && !skill->canTeach( teacher->getNPC( ), ch, false ))
            continue;

        if (!teacher->is_npc( ) && skill->getEffective( teacher ) < 100)
            continue;

        if (skill->getSkillHelp() && skill->getSkillHelp()->getID() > 0)
            info.help_id = skill->getSkillHelp()->getID();
        else
            info.help_id = 0;

        info.percent = ch->getSkillData( sn ).learned;
        info.name = skill->getNameFor( ch ).ruscase('1');
        info.adept = skill->getAdept( ch );
        info.maximum = skill->getMaximum( ch );

        buf << "     ";

        if (info.help_id > 0)
            buf << fmt(0, patternHelp,
                             info.help_id,
                             info.name.c_str( ) );
        else
            buf << fmt(0, patternNoHelp,
                             info.name.c_str( ) );

        if (info.percent > 1)
            buf << fmt(0, patternLearned,
                             info.getPercentColor( ),
                             info.percent ) << endl;
        else
            buf << "{rне изучено{x" << endl;

        found = true;
    }
    
    if (found) 
        page_to_char( buf.str( ).c_str( ), ch );
    else
        ch->pecho("Тебе нечему научиться у %C2.", teacher );

    if (teacher->is_npc())
        mprog_teach(ch, teacher->getNPC(), "here");
}


void CPractice::pracLearn( PCharacter *ch, DLString &arg )
{
    int sn, adept;
    Skill *skill;
    Character *teacher;
    ostringstream buf;

    if (!IS_AWAKE( ch )) {
        ch->pecho("Во сне или как?");
        return;
    }

    sn = skill_lookup(arg, ch);
    skill = skillManager->find( sn );

    if (!skill) {
        ch->pecho("Умение {W%s{x не существует или еще тебе не доступно.", arg.c_str());
        return;
    }

    if (!skill->available(ch)) {
        ch->pecho("Умение {W%N1{x тебе не доступно.", skill->getNameFor(ch).c_str());
        return;
    }

    if (!skill->canPractice( ch, buf )) {
        if (buf.str().empty())
            ch->pecho("Ты не можешь выучить умение {W%K1{x.", skill);
        else
            ch->send_to(buf);
        return;
    }
    
    if (!( teacher = findTeacher( ch, skill ) ))
        if (!( teacher = findPracticer( ch, skill ) ))
            return;
    
    if (teacher->is_npc() && mprog_cant_teach(ch, teacher->getNPC(), skill->getName().c_str()))
        return;

    if (ch->practice <= 0) {
        ch->pecho("У тебя сейчас нет сессий практики.");
        return;
    }

    int &learned = ch->getSkillData( skill->getIndex( ) ).learned;

    adept = skill->getAdept( ch );

    if (learned >= adept) {
        ch->pecho("Ты уже слишком хорошо владеешь умением {W%K1{x, практиковаться бессмысленно.", skill);
        ch->pecho("Чтобы овладеть умением еще лучше, просто применяй его почаще.");  
        return;
    }

    ch->practice--;
    
    skill->practice( ch );

    ch->pecho("%^C1 обучает тебя умению {W%K1{x.", teacher, skill);
    teacher->pecho("Ты обучаешь %C4 умению {W%K1{x.", ch, skill);
    ch->recho(teacher, "%^C1 обучает %C4 умению {W%K1{x.", teacher, ch, skill);
    
    if (learned < adept)
        ch->pecho( "Ты теперь знаешь умение {W%K1{x на %d процентов.", skill, learned );
    else {
        ch->pecho("Теперь ты хорошо владеешь умением {W%K1{x.", skill);
        ch->pecho( "Дальше практиковать не получится, просто начни применять его почаще." );        
        ch->recho("%^C1 теперь хорошо владеет умением {W%K1{x.", ch, skill);
    }

    if (teacher->is_npc()) {
        mprog_teach(ch, teacher->getNPC(), skill->getName().c_str());
    }
}

PCharacter * CPractice::findTeacher( PCharacter *ch, Skill *skill )
{
    Character *rch;
    PCharacter *teacher;
    
    for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
        if (rch->is_npc( ) || rch == ch)
            continue;
        
        teacher = rch->getPC( );
        
        if (teacher->getRealLevel( ) < HERO - 1)
            continue;

        if (!teacher->getAttributes( ).isAvailable( "teacher" ))
            continue;
        
        if (skill && skill->getEffective( teacher ) < 100)
            continue;

        return teacher;
    }
    
    return NULL;
}

NPCharacter * CPractice::findPracticer( PCharacter *ch, Skill *skill )
{
    NPCharacter *mob;

    mob = find_attracted_mob( ch, OCC_PRACTICER );

    if (!skill || skill->canTeach( mob, ch )) 
        return mob;
    else
        return NULL;
}

