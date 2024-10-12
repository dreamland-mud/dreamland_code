/* $Id: cskills.cpp,v 1.1.2.3.6.2 2008/02/23 13:41:24 rufina Exp $
 *
 * ruffina, 2004
 */

#include "commandtemplate.h"
#include "stringlist.h"
#include "pcharacter.h"
#include "skill.h"
#include "skillgroup.h"
#include "spell.h"
#include "skillmanager.h"
#include "interp.h"
#include "comm.h"
#include "act.h"
#include "loadsave.h"
#include "merc.h"

#include "def.h"

struct SkillInfo 
{
    DLString name;
    int learned;
    int real;
    int level; 
    int mana;
    int adept;
    int maximum;
    int help_id;
    bool spell;
    bool active;
    bool passive;
    bool available;

    static bool cmp_by_name( SkillInfo a, SkillInfo b )
    {
        return a.name.compareRussian(b.name) < 0;
    }
    static bool cmp_by_level( SkillInfo a, SkillInfo b )
    {
        return a.level < b.level;
    }
    static bool cmp_by_learned( SkillInfo a, SkillInfo b )
    {
        return a.learned < b.learned;
    }

    const char * colorLearned( );
    const char * colorReal( );
    const char * color( int );
};

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

struct AllSkillsList : public std::list<SkillInfo> {
    bool parse( DLString &, std::ostream &, Character * );
    void make( Character *ch );
    void display( std::ostream & );
    
    DLString mycmd;
    int levHigh, levLow;
    bool fActive, fPassive, fSpell;
    SkillGroup *group;
    bool (*criteria) ( SkillInfo, SkillInfo );
};


bool AllSkillsList::parse( DLString &argument, std::ostream &buf, Character *ch )
{
    StringList tokens(argument);

    group = NULL;
    criteria = 0;
    fSpell = fActive = fPassive = false;
    int num1 = 0, num2 = 0;
    bool fAll = false;

    // Analyse all arguments one by one for keywords.
    for (unsigned int i = 0; i < tokens.size(); ) {
        DLString t = tokens[i];

        // May be <level> or one of <level1> <level2>?
        if (t.isNumber()) {
            if (fAll) {
                buf << "Диапазон уровней нельзя задать одновременно со словом все." << endl;
                return false;
            }

            if (!num1)
                num1 = t.toInt();
            else if (!num2)
                num2 = t.toInt();
            else {
                buf << "Укажи уровень (например, 15) или диапазон уровней (например, 10 42)" << endl;
                return false;
            }

            i++;
            continue;
        }

        // 'all' argument cannot be mixed with levels. 
        if (arg_is_all(t)) {
            if (num1 || num2) {
                buf << "Диапазон уровней нельзя задать одновременно со словом {yвсе{x." << endl;
                return false;                        
            }
            fAll = true;
            i++;
            continue;
        }

        // Found "sortby" argument, check if it's "sortby <criteria>".
        if (arg_is(t, "sortby")) {
            if (criteria) {
                buf << "Критерий сортировки не может быть задан дважды." << endl;
                return false;
            }

            if (i != tokens.size() - 1) {
                DLString nextToken = tokens[i+1];
                if (arg_is(nextToken, "name"))
                    criteria = SkillInfo::cmp_by_name;
                else if (arg_is(nextToken, "level"))
                    criteria = SkillInfo::cmp_by_level;
                else if (arg_is(nextToken, "learned"))
                    criteria = SkillInfo::cmp_by_learned;
                else {
                    buf << "Неправильный критерий сортировки '" << nextToken << "', допустимые значения: 'имя', 'уровень' или 'изучено'" << endl;
                    return false;
                }

                i += 2;
                continue;
            }

            buf << "Укажите критерий сортировки ('имя', 'уровень' или 'изучено')." << endl;
            return false;
        }

        // Skill type specifiers:
        if (arg_is(t, "skills")) {
            fPassive = fActive = true;
            i++;
            continue;
        }
        if (arg_is(t, "spells")) {
            fSpell = true;
            i++;
            continue;
        }
        if (arg_is(t, "passive")) {
            fPassive = true;
            i++;
            continue;
        }
        if (arg_is(t, "active")) {
            fActive = true;
            i++;
            continue;
        }

        // May be a group name?
        if ((group = skillGroupManager->findUnstrict(t))) {
            i++;
            continue;
        }

        // Unknown garbage.
        buf << fmt(0, 
            "Неверный параметр '%1$s', подходящие фильтры: "
            "заклинания, навыки, пассивные, "
            "активные, название группы умений, диапазон уровней.\r\n", t.c_str());
        return false;
    }

    // Fill in reasonable defaults.
    if (fAll) {
        levLow = 1;
        levHigh = MAX_LEVEL;
    } else if (num1 && !num2) {
        levLow = levHigh = num1; // Strict level is specified.
    } else { // Either no levels or a level range is specified.
        levLow = num1 ? num1 : 1;
        levHigh = num2 ? num2 : ch->getRealLevel(); 
    }

    if (!criteria)
        criteria = &SkillInfo::cmp_by_level;

    if (!fActive && !fPassive && !fSpell)
        fActive = fPassive = fSpell = true; // Display all skill types unless specified explicitly

    return true;
}

void AllSkillsList::make( Character *ch )
{
    SkillInfo info;

    // Collate all displayable skills into a list.
    for (int sn = 0; sn < SkillManager::getThis( )->size( ); sn++) {
        Skill *skill = SkillManager::getThis( )->find( sn );
        Spell::Pointer spell = skill->getSpell( );                

        // Hide skills that will never become available for this player on any level.
        if (!skill->visible( ch ))
            continue;

        // Start collecting skill details.
        info.name = skill->getNameFor( ch );
        info.real = skill->getEffective( ch );
        info.spell = spell && spell->isCasted();
        info.passive = skill->isPassive();
        info.active = !info.spell && !info.passive;

        // Apply skill type filters.
        if (!fSpell && info.spell)
            continue;

        if (!fActive && info.active)
            continue;

        if (!fPassive && info.passive)
            continue;

        // Apply group filter.
        if (group && !skill->hasGroup(group->getIndex()))
            continue;

        // Apply level filters.
        info.level = skill->getLevel( ch );

        if (info.level > levHigh || info.level < levLow)
            continue;

        // Collect all other skill details.
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
        
        info.mana = skill->getMana(ch);

        if (skill->getSkillHelp() && skill->getSkillHelp()->getID() > 0)
            info.help_id = skill->getSkillHelp()->getID();
        else
            info.help_id = 0;

        push_back( info );
    }

    // Apply sorting criteria.
    sort( criteria );
}


void AllSkillsList::display( std::ostream & buf )
{
    int prevLevel = 0, firstColumn = true;

    if (empty( )) {
        buf << "Не найдено ни одного умения." << endl;
        return;
    }

    // Deside if we should display two columns or just one, for wider skill names.
    int tmp_max = 0;
    int tmp_len;
    for (iterator i = begin( ); i != end( ); i++) {
        SkillInfo info = *i;
        tmp_len = info.name.size();
        if ( tmp_len > tmp_max ) tmp_max = tmp_len;
    }    
    int bool_long_name = 0;
    if ( tmp_max > 19 ) bool_long_name = 1;

    // Draw the header. 
    buf << "{W=========================================================="
        << (bool_long_name ? "{x" : "====================={x")
        << endl
        << fmt(0, (bool_long_name ? 
                       "%7s| %-30s| %-7s |%4s{W|{x " :
                       "%7s| %-18s| %-7s |%4s{W|{x "),
                      "Уровень", "Умение", "Изучено", "Мана" )
        << fmt(0, (bool_long_name ? 
                       "" : "%-18s| %-7s |%4s"),
                     "Умение", "Изучено", "Мана")
        << endl
        << (bool_long_name ?
            "{W-------+----------------------------------+---------+----+{x" : 
            "{W-------+--------------------+---------+----+--------------------+---------+----{x")
        << endl;

    // Draw the skill table.
    for (iterator i = begin( ); i != end( ); i++) {
        SkillInfo info = *i;

        if (info.level != prevLevel) {
            if (!firstColumn) {
                firstColumn = true;
                buf << "                    |         |" << endl;
            }

            buf << fmt(0, "  %3d  |", info.level );
        }
        else if (firstColumn)
            buf << "       |";

        if (info.help_id > 0)
            buf << "{hh" << info.help_id << " ";
        else
            buf << " ";
            
        if (bool_long_name)
            buf << fmt(0, "{c%-30s{x|", info.name.c_str( ) );
        else
            buf << fmt(0, "{c%-18s{x|", info.name.c_str( ) );

        if (info.available)
            buf << fmt(0, " %s%3d{x(%s%3d{x)|", 
                              info.colorLearned( ), info.learned, 
                              info.colorReal( ), info.real );
        else
            buf << "   n/a   |";

        if (info.mana > 0 && info.available)
            buf << fmt(0, bool_long_name ? " %3d" : " %-3d", info.mana );
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

    buf << fmt(0, 
        "Также используй фильтры "
        "{y{hc%1$s заклинания{x, {y{hc%1$s навыки{x, {y{hc%1$s пассивные{x, "
        "{y{hc%1$s активные{x, группа умений и диапазон уровней (например, 10 или 10 42)\r\n", mycmd.c_str());

    buf << endl << "См. также {y{hc" << mycmd << " ?{x." << endl;
}

/*
 * Skill command arguments, a combination of ANY of the following:
 * 
 * skills|навыки
 * spells|заклинания
 * active|активные
 * passive|пассивные
 * sortby name|level|learned
 * <skill group>
 * 
 * A level filter can be specified as well, one of:
 * 
 * all|все -- show skills from 1 to 100
 * <level1> <level2> -- show skills from given level range
 * <level> -- show skills only for the given level
 * If ommitted, show all skills from 1 to current player level.
 */
CMDRUN( skills )
{
    DLString argument = constArguments;

    if (arg_is_help(argument)) {
        interpret_raw(ch, "help", DLString(getHelp()->getID()).c_str());
        return;
    }

    AllSkillsList slist;
    std::basic_ostringstream<char> buf;
    
    slist.mycmd = "" + getRussianName() + "";

    if (!slist.parse( argument, buf, ch )) {
        ch->send_to( buf );
        return;
    }
    
    slist.make( ch );
    slist.display( buf );

    page_to_char( buf.str( ).c_str( ), ch );
}


