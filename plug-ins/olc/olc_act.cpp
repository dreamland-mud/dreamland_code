/* $Id$
 *
 * ruffina, 2004
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include <config.h>

#include <skillmanager.h>
#include <skill.h>
#include <spell.h>
#include "skillgroup.h"
#include "affecthandler.h"

#include <character.h>
#include <pcharacter.h>
#include <commandmanager.h>
#include <object.h>
#include <affect.h>
#include "liquid.h"
#include "desire.h"

#include "comm.h"
#include "merc.h"
#include "act.h"
#include "interp.h"

#include "mercdb.h"

#include "olc.h"
#include "loadsave.h"
#include "material-table.h"
#include "damageflags.h"
#include "recipeflags.h"
#include "religionflags.h"
#include "commandflags.h"
#include "skillsflags.h"
#include "def.h"

GROUP(defensive);
GROUP(protective);
GROUP(benediction);

struct olc_help_type {
    const char *command;
    const void *structure;
    const char *desc;
    const char *alias;
};


// stub 
int skill_table;
int wearloc_table;
int liq_table;
int group_table;
int race_table;
int class_table;
int clan_table;
int religion_table;
int mat_table;

// This table contains help commands and a brief description of each.
const struct olc_help_type help_table[] =
{
    {"{YАрии{x", NULL, NULL, NULL },
    {"area_flags", &area_flags, "Флаги арий (поле area_flag)"},

    {"{YКомнаты{x", NULL, NULL, NULL},
    {"room_flags", &room_flags, "Флаги комнат (поле room_flags)."},
    {"sector_table", &sector_table, "Тип местности в комнате (поле sector_type)."},
    {"exit_flags", &exit_flags, "Флаги выходов и экстравыходов (поле exit_info)."},
    {"reset_flags", &reset_flags, "Флаги ресетов."},
    {"rand_table", &rand_table, "Рандомные предметы (поле rand у ресетов)."},

    {"{YПредметы{x", NULL, NULL, NULL},
    {"item_table", &item_table, "Типы предметов (поле item_type)."},
    {"wear_flags", &wear_flags, "Куда одевать предмет (поле wear_flags)."},
    {"extra_flags", &extra_flags, "Флаги предметов (поле extra_flags)."},
    {"weapon_class", &weapon_class, "Вид оружия (поле value0 у оружия)."},
    {"weapon_flags", &weapon_flags, "Типы ударов (поле value3 у оружия, поле dam_type у моба)."},
    {"weapon_type2", &weapon_type2, "Флаги оружия (поле value4 у оружия)."},
    {"container_flags", &container_flags, "Флаги контейнеров (поле value1 у контейнера)."},
    {"portal_flags", &portal_flags, "Флаги порталов (поле value2 у порталов)."},
    {"furniture_flags", &furniture_flags, "Флаги мебели (поле value2 у мебели)."},
    {"drink_flags", &drink_flags, "Флаги емкости для жидкостей (поле value3)."},
    {"recipe_flags", &recipe_flags, "Флаги рецептов (поле value0 у рецепта)"},
    {"liquid", &liq_table, "Жидкости (поле value2 у емкостей и фонтанов)."},
    {"material", &mat_table, "Материалы предметов и мобов."},


    {"{YМобы и персонажи{x", NULL, NULL, NULL},
    {"races", &race_table, "Список всех рас мобов."},
    {"classes", &class_table, "Список всех классов персонажей."},
    {"clans", &clan_table, "Список всех кланов."},
    {"religion", &religion_table, "Список всех религий.", "gods"},
    {"religion_flags", &religion_flags, "Флаги религий."},
    {"align_table", &align_table, "Натура персонажа."},
    {"ethos_table", &ethos_table, "Этос персонажа."},
    {"stat_table", &stat_table, "Параметры персонажа."},
    {"sex_table", &sex_table, "Пол моба (поле sex)."},
    {"position_table", &position_table, "Позиции мобов (поля start_pos, default_pos, position)."},
    {"size_table", &size_table, "Размеры мобов (поле size)."},
    {"ac_type", &ac_type, "Класс брони на мобах и аффектах."},
    {"act_flags", &act_flags, "Аттрибуты мобов (поле act)."},
    {"affect_flags", &affect_flags, "Влияния на мобах (поле affected_by)."},
    {"detect_flags", &detect_flags, "Детекты моба (поле detection)."},
    {"imm_flags", &imm_flags, "Иммунитеты мобов (поле imm_flags)."},
    {"res_flags", &res_flags, "Сопротивляемость мобов (поле res_flags)."},
    {"vuln_flags", &vuln_flags, "Уязвимость мобов (поле vuln_flags)."},
    {"off_flags", &off_flags, "Типы поведения мобов (поле off_flags)."},
    {"form_flags", &form_flags, "Формы тела мобов (поле form)."},
    {"part_flags", &part_flags, "Части тела мобов (поле parts)."},
    {"spec", &spec_table, "Доступные спец. программы моба."},

    {"{YАффекты{x", NULL, NULL, NULL},
    {"apply_flags", &apply_flags, "Поле location у аффекта (на что влияет его modifier)"},
    {"affwhere_flags", &affwhere_flags, "Поле where у аффекта (на что влияет его bitvector)"},
    {"wearloc", &wearloc_table, "Куда мобы одевают предметы."},

    {"{YУмения и заклинания{x", NULL, NULL, NULL}, 
    {"spells", &skill_table, "Имена всех заклинаний."},
    {"practicer", &group_table, "Все группы умений (для поля practicer)."},
    {"target_table", &target_table, "Цели для заклинаний (поле target)."},
    {"spell_types", &spell_types, "Типы заклинаний (поле type)."},
    {"spell_flags", &spell_flags, "Флаги заклинаний (поле flags)."},
    {"order_flags", &order_flags, "Флаги приказов (поле order)."},
    {"damage_table", &damage_table, "Виды повреждений (поле damtype)."},
    {"damage_flags", &damage_flags, "Флаги урона (поле damflags)."},
    {"command_flags", &command_flags, "Флаги для команды."},
    {"argtype_table", &argtype_table, "Тип аргумента для команды умения (поле argtype)."},

    {NULL, NULL, NULL, NULL}
};

bool help_table_matches(const olc_help_type *help_entry, const char *arg)
{
    if (arg[0] == help_entry->command[0] && !str_prefix(arg, help_entry->command))
        return true;

    if (help_entry->alias && !str_prefix(arg, help_entry->alias))
        return true;

    return false;
}

void show_flag_cmds(Character * ch, const FlagTable *table)
{
    ostringstream buf;
    const DLString &tableName = FlagTableRegistry::getName(table);
    
    buf << "Таблица " << (table->enumerated ? "значений" : "флагов")
        << " {Y" << tableName << "{x" << endl
        << "Значение        : Пояснение" << endl;
   
    const FlagTable::Field * f = table->fields; 
    for (int i = 0; i < table->size; i++)
        buf << dlprintf("{g%-15s{x: %s", 
                        f[i].name, 
                       (f[i].message ? russian_case(f[i].message, '1').c_str() : ""))
            << endl;

    ch->send_to(buf);
}

void show_skills(Character *ch, int target)
{
    ostringstream buf;
    int sn;

    buf << fmt(0, "{Y%-20s  %-20s %-10s %s\n\r", "Заклинание", "Группа", "Тип", "Цель");
    for (sn = 0; sn < SkillManager::getThis( )->size( ); sn++) {
        Skill *skill = SkillManager::getThis( )->find( sn );
        Spell::Pointer spell = skill->getSpell( );
        AffectHandler::Pointer aff = skill->getAffect( );

        if (!spell || !aff)
            continue;
        if (target != -1 && !spell)
            continue;
        if (target != -1 && !IS_SET(spell->getTarget(), target))
            continue;

        buf << fmt( 0, "{g%-20s{x: %-20s %-10s %s\n\r",
                       skill->getName( ).c_str( ),
                       skill->getGroups( ).toString().c_str( ),
                       spell_types.name( spell->getSpellType( ) ).c_str( ),
                       target_table.names( spell->getTarget( ) ).c_str( ) );
    }

    page_to_char( buf.str( ).c_str( ), ch );
}



void show_spec_cmds(Character * ch)
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int spec;
    int col;

    buf1[0] = '\0';
    col = 0;
    stc("Все специальные функции начинаются с 'spec_'\n\r\n\r", ch);
    for (spec = 0; spec_table[spec].function != NULL; spec++) {
        sprintf(buf, "%-19s", &spec_table[spec].name[5]);
        strcat(buf1, buf);
        if (++col % 4 == 0)
            strcat(buf1, "\n\r");
    }

    if (col % 4 != 0)
        strcat(buf1, "\n\r");

    stc(buf1, ch);
}

void show_liq_cmds(Character * ch)
{
    std::basic_ostringstream<char> buf;
    Liquid *liq;

    buf << "Name                ShowName            Color       Full/Thirst/Food/Drunk/Blood/Sips" << endl;

    for (int l = 0; l < liquidManager->size( ); l++) {
        liq = liquidManager->find( l );
        
        buf << dlprintf( "%-18s %-18s %-14s ",
                  liq->getName( ).c_str( ),
                  liq->getShortDescr( ).ruscase( '1' ).c_str( ),
                  liq->getColor( ).ruscase( '1' ).c_str( ) );

        for (int i = 0; i < desireManager->size( ); i++)
            buf << dlprintf("%3d ", liq->getDesires( )[i] );
                            
        buf << dlprintf( "%3d\r\n", liq->getSipSize( ) );
    }

    page_to_char(buf.str( ).c_str( ), ch);
}

bool show_help(Character * ch, const char *cargument)
{
    char argumentBuf[MAX_STRING_LENGTH];
    char *argument = argumentBuf;
    char arg[MAX_INPUT_LENGTH];
    char spell[MAX_INPUT_LENGTH];
    int cnt;
    
    strcpy(argument, cargument);
    argument = one_argument(argument, arg);
    one_argument(argument, spell);

    /* Display syntax. */
    if (arg[0] == '\0') {
        ostringstream buf;
        buf << "Таблица             : Пояснение" << endl;
        for (cnt = 0; help_table[cnt].command != NULL; cnt++) {
            if (help_table[cnt].desc)
                buf << dlprintf("{g%-19s{x: %s", help_table[cnt].command, help_table[cnt].desc) << endl;
            else
                buf << help_table[cnt].command << endl;
        }
        buf << endl << "Используй '{Wolchelp таблица{x' или '{W? таблица{x' изнутри редактора." << endl;
        ch->send_to(buf);
        return false;
    }

    // Find the command, show changeable data.
    for (cnt = 0; help_table[cnt].command != NULL; cnt++) {
        if (help_table_matches(&help_table[cnt], arg)) {
            if (help_table[cnt].structure == &spec_table) {
                show_spec_cmds(ch);
                return false;
            }
            else if (help_table[cnt].structure == &liq_table) {
                show_liq_cmds(ch);
                return false;
            }
            else if (help_table[cnt].structure == &wearloc_table) {
                for (int i = 0; i < wearlocationManager->size( ); i++) {
                    Wearlocation *w = wearlocationManager->find( i );
                    ch->printf( "{g%-14s{x: %s\r\n", 
                                w->getName().c_str(), w->getPurpose().c_str() );
                }
                return false;
            }
            else if (help_table[cnt].structure == &group_table) {
                ostringstream buf;
                for (int gn = 0; gn < skillGroupManager->size( ); gn++) {
                    SkillGroup *group = skillGroupManager->find( gn );
                    buf << fmt( 0, "{g%-17s{x: %-25s",
                                group->getName( ).c_str( ),
                                group->getRussianName( ).c_str( ) );
                    if (gn % 2)
                        buf << endl;
                }
                buf << endl;
                ch->send_to(buf);
                return false;
            }
            else if (help_table[cnt].structure == &race_table) {
                ostringstream buf;
                for (int i = 0; i < raceManager->size( ); i++) {
                    Race *race = raceManager->find( i );
                    buf << fmt( 0, "{g%-17s{x: %-25s",
                                race->getName().c_str(),
                                race->getMaleName().ruscase('1').c_str() );
                    if (i % 2)
                        buf << endl;
                }
                buf << endl;
                ch->send_to(buf);
                return false;
            }
            else if (help_table[cnt].structure == &class_table) {
                ostringstream buf;
                for (int i = 0; i < professionManager->size( ); i++) {
                    Profession *prof = professionManager->find(i);
                    buf << fmt( 0, "{g%-17s{x: %-25s",
                                prof->getName().c_str(),
                                prof->getRusName().ruscase('1').c_str() );
                    if (i % 2)
                        buf << endl;
                }
                buf << endl;
                ch->send_to(buf);
                return false;
            }
            else if (help_table[cnt].structure == &clan_table) {
                ostringstream buf;
                for (int i = 0; i < clanManager->size( ); i++) {
                    Clan *clan = clanManager->find(i);
                    buf << fmt( 0, "{g%-17s{x: %-25s",
                                clan->getName().c_str(),
                                clan->getShortName().c_str());
                    if (i % 2)
                        buf << endl;
                }
                buf << endl;
                ch->send_to(buf);
                return false;
            }
            else if (help_table[cnt].structure == &religion_table) {
                ostringstream buf;
                for (int i = 0; i < religionManager->size( ); i++) {
                    Religion *rel = religionManager->find(i);
                    buf << fmt( 0, "{g%-15s{x: %-20s (%-15s)",
                                rel->getShortDescr().c_str(),
                                rel->getRussianName().ruscase('1').c_str(),
				rel->getName().c_str());
		    buf << endl;
                }
                buf << endl;
                ch->send_to(buf);
                return false;
            }
            else if (help_table[cnt].structure == &mat_table) {
                ostringstream buf;
                buf << fmt(0, "{G%-14s {W%15s %10s {m%13s {G%s\r\n",
                             "NAME", "TYPES", "FLAGS", "VULN", "RUSSIAN");

                for (auto &m: material_table)
                    buf << fmt(0, "{g%-14s{x %15s %10s {m%13s {g%s{x\r\n",
                                m.name.c_str(),
                                m.type.names().c_str(),
                                m.flags.names().c_str(),
                                m.vuln.names().c_str(),
                                m.rname.c_str());
                buf << "See {y{hcfedit material{x for more details." << endl;
                page_to_char(buf.str( ).c_str( ), ch);
                return false;
            }
            else if (help_table[cnt].structure == &skill_table) {
                if (spell[0] == '\0') {
                    stc("Синтаксис:  ? spells "
                        "[ignore/room/char/self/object/all]\n\r", ch);
                    return false;
                }

                if (!str_prefix(spell, "all"))
                    show_skills(ch, -1);
                else if (!str_prefix(spell, "ignore"))
                    show_skills(ch, TAR_IGNORE|TAR_CREATE_OBJ|TAR_CREATE_MOB);
                else if (!str_prefix(spell, "room"))
                    show_skills(ch, TAR_ROOM|TAR_PEOPLE);
                else if (!str_prefix(spell, "char"))
                    show_skills(ch, TAR_CHAR_ROOM|TAR_CHAR_WORLD);
                else if (!str_prefix(spell, "self"))
                    show_skills(ch, TAR_CHAR_SELF);
                else if (!str_prefix(spell, "object"))
                    show_skills(ch, TAR_OBJ_INV|TAR_OBJ_ROOM|TAR_OBJ_EQUIP|TAR_OBJ_WORLD);
                else
                    stc("Синтаксис:  ? spells "
                        "[ignore/affect/self/object/all]\n\r", ch);
                return false;
            } 
            else {
                show_flag_cmds(ch, (const FlagTable*)help_table[cnt].structure);
                return false;
            }
        }
    }
    show_help(ch, "");
    return false;
}

CMD(olchelp, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
        "Online editor help command.")
{
    show_help(ch, argument);
}

DLString show_enum_array(const EnumerationArray &array)
{
    ostringstream buf;

    for (int i = 0; i < array.getTable()->size; i++)
        if (array[i] > 0)
            buf << array.getTable()->fields[i].name << " " << array[i] << " ";

    DLString result = buf.str();
    return result.empty() ? "-" : result;
}
