/* $Id: allskillslist.cpp,v 1.1.2.10.10.9 2008/07/31 04:56:05 rufina Exp $
 *
 * ruffina, 2004
 */
#include "allskillslist.h"

#include "skill.h"
#include "skillgroup.h"
#include "spell.h"
#include "skillmanager.h"
#include "pcharacter.h"

#include "act.h"
#include "loadsave.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

const char * SkillInfo::colorLearned( )
{
    return color( learned );
}
const char * SkillInfo::colorReal( )
{
    return color( real );
}
const char * SkillInfo::color( int x )
{
    if (x <= 1)
        return "{R";
    else if (x >= maximum)
        return "{C";
    else if (x >= adept)
        return "{c";
    else 
        return "{x";
}

bool AllSkillsList::parse( DLString &argument, std::ostream &buf, Character *ch )
{
    DLString arg1, arg2;

    arg1 = argument.getOneArgument( );
    arg2 = argument.getOneArgument( );
    
    group = NULL;
    levLow = 1;
    levHigh = MAX_LEVEL;
    criteria = &SkillInfo::cmp_by_level;
    fUsableOnly = false;
    fShowHint = false;
    fCurrentProfAll = false;

    if (arg1.empty( )) {
        arg1 = "now";
        fShowHint = true;
    }

    if (arg_oneof( arg1, "help", "?", "помощь", "справка" )) {
        DLString what = fSpells ? "заклинания" : "умения";
        buf 
        << "        {y{lR" << rcmd << "{lE" << cmd << "{lx{w: все доступные на этом уровне "  << what << endl;

        buf << "        {y{lR" << rcmd << " все{lE" << cmd << " all{lx{w: доступные на всех уровнях "  << what << endl;

        buf << "        {y{lR" << rcmd << "{lE" << cmd << "{lx <уровень>{w: " << what << ", которые появятся на этом уровне" << endl;
        buf << "        {y{lR" << rcmd << "{lE" << cmd << "{lx <уровень1> <уровень2>{w: " << what << ", которые появятся на этом диапазоне уровней" << endl;
        buf << "        {y{lR" << rcmd << " сорт имя|уровень|изучено{lE" << cmd << " sort name|level|learned{lx{w: сортировать " << what << endl;
        buf << "        {y{lR" << rcmd << "{lE" << cmd << "{lx <название группы>{w: все " << what << " из этой группы" << endl
            << "" << endl
            << "См. также {W{lR? умения{lE? skills{lx{w, {W{lR? группаумений{lE? glist{lx{w." << endl;

        return false;
    }

    if (arg_is_all( arg1 )) {
    }
    else if (arg_oneof_strict( arg1, "now", "сейчас" )) {
        fUsableOnly = true;
    }
    else if (arg_oneof( arg1, "current", "текущие" )) {
        fCurrentProfAll = true;
    }
    else if (arg1.isNumber( )) {
        try {
            levLow = arg1.toInt( );
            
            if (!arg2.empty( )) {
                if (!arg2.isNumber( )) {
                    buf << "Неправильный диапазон уровней." << endl;
                    return false;
                }

                levHigh = arg2.toInt( );
            }
            else
                levHigh = levLow;
        }
        catch (const ExceptionBadType &e) {
            buf << "Неправильный диапазон уровней." << endl;
            return false;
        }
    }
    else if (arg_oneof( arg1, "sortby", "сортировать" )) {
        if (arg2.empty( )) {
            buf << "Укажите критерий сортировки ('{lEname{lRимя{lx', '{lElevel{lRуровень{lx' или '{lElearned{lRизучено{lx')." << endl;
            return false;
        }
        
        if (arg_oneof( arg2, "name", "имя" ))
            criteria = SkillInfo::cmp_by_name;
        else if (arg_oneof( arg2, "level", "уровень" ))
            criteria = SkillInfo::cmp_by_level;
        else if (arg_oneof( arg2, "learned", "изучено" ))
            criteria = SkillInfo::cmp_by_learned;
        else {
            buf << "Неправильный критерий сортировки." << endl;
            return false;
        }
    }
    else if (!(group = skillGroupManager->findUnstrict( arg1 ))) {
        buf << "Такой группы не существует." << endl;
        return false;
    }

    fRussian = ch->getConfig( ).ruskills;
    return true;
}

void AllSkillsList::make( Character *ch )
{
    SkillInfo info;
    int savedLevel = ch->getLevel();

    if (fCurrentProfAll)
        ch->setLevel(MAX_LEVEL);

    for (int sn = 0; sn < SkillManager::getThis( )->size( ); sn++) {
        Skill *skill = SkillManager::getThis( )->find( sn );
        Spell::Pointer spell = skill->getSpell( );

        if (!skill->visible( ch ))
            continue;

        if (fUsableOnly && !skill->usable( ch, false ))
            continue;

        if (fCurrentProfAll && !skill->usable( ch, false ))
            continue;

        if (fSpells != (spell && spell->isCasted( )))
            continue;

        if (group && skill->getGroup( ) != group)
            continue;

        info.level = skill->getLevel( ch );

        if (info.level > levHigh || info.level < levLow)
            continue;

        info.name = skill->getNameFor( ch );
        info.real = skill->getEffective( ch );
        info.spell = fSpells;
        info.available = skill->available( ch );
        info.maximum = skill->getMaximum( ch );

        if (ch->is_npc( )) {
            info.learned = skill->getLearned( ch );
            info.adept = info.learned;
        }
        else {
            PCSkillData &data = ch->getPC( )->getSkillData( sn );
            info.learned = data.learned;
            info.adept = skill->getAdept( ch->getPC( ) );
        }
        
        if (spell && spell->isCasted( ))
            info.mana = spell->getManaCost( ch );
        else
            info.mana = skill->getMana( );

        if (skill->getSkillHelp() && skill->getSkillHelp()->getID() > 0)
            info.help_id = skill->getSkillHelp()->getID();
        else
            info.help_id = 0;

        push_back( info );
    }

    if (fCurrentProfAll)
        ch->setLevel(savedLevel);

    sort( criteria );
}


void AllSkillsList::display( std::ostream & buf )
{
    int prevLevel = 0, firstColumn = true;

    if (empty( )) {
        buf << "Не найдено ни одного " << (fSpells ? "заклинания" : "умения") << "." << endl;
        return;
    }

    int tmp_max = 0;
    int tmp_len;
    for (iterator i = begin( ); i != end( ); i++) {
        SkillInfo info = *i;
        tmp_len = info.name.size();
        if ( tmp_len > tmp_max ) tmp_max = tmp_len;
    }
    int bool_long_name = 0;
    if ( tmp_max > 19 ) bool_long_name = 1;

    buf << "{W=========================================================="
        << (bool_long_name ? "{x" : "====================={x")
        << endl
        << dlprintf( (bool_long_name ? 
                       "%7s| %-30s| %-7s |%4s{W|{x " :
                       "%7s| %-18s| %-7s |%4s{W|{x "),
                      "Уровень", (fSpells ? "Заклинание" : "Умение"), "Изучено", "Мана" )
        << dlprintf( (bool_long_name ? 
                       "" : "%-18s| %-7s |%4s"),
                     (fSpells ? "Заклинание" : "Умение"), "Изучено", "Мана")
        << endl
        << (bool_long_name ?
            "{W-------+----------------------------------+---------+----+{x" : 
            "{W-------+--------------------+---------+----+--------------------+---------+----{x")
        << endl;

    for (iterator i = begin( ); i != end( ); i++) {
        SkillInfo info = *i;

        if (info.level != prevLevel) {
            if (!firstColumn) {
                firstColumn = true;
                buf << "                    |         |" << endl;
            }

            buf << dlprintf( "  %3d  |", info.level );
        }
        else if (firstColumn)
            buf << "       |";

        if (info.help_id > 0)
            buf << "{hh" << info.help_id << " ";
        else
            buf << " ";
            
        if (bool_long_name)
            buf << dlprintf( "{c%-30s{x|", info.name.c_str( ) );
        else
            buf << dlprintf( "{c%-18s{x|", info.name.c_str( ) );

        if (info.available)
            buf << dlprintf( " %s%3d{x(%s%3d{x)|", 
                              info.colorLearned( ), info.learned, 
                              info.colorReal( ), info.real );
        else
            buf << "   n/a   |";

        if (info.mana > 0 && info.available)
            buf << dlprintf( bool_long_name ? " %3d" : " %-3d", info.mana );
        else
            buf << "    ";

        if (firstColumn)
            buf << "{W|{x";
        else 
            buf << endl;

        prevLevel = info.level;

        if (!bool_long_name)
            firstColumn = !firstColumn;
        else
            buf << endl;
    }

    if (!firstColumn) 
        buf << "                    |         |" << endl;

    if (fShowHint)
        buf << endl << "См. также {W{lR" << rcmd << "{lE" << cmd << "{lx ?{w." << endl;
}
