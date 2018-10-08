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
#include "damageflags.h"
#include "material.h"
#include "def.h"

GROUP(defensive);
GROUP(protective);
GROUP(benediction);

struct olc_help_type {
    const char *command;
    const void *structure;
    const char *desc;
};


// stub 
int skill_table;
int wearloc_table;
int liq_table;
int mat_table;

// This table contains help commands and a brief description of each.
const struct olc_help_type help_table[] =
{
    {"area", &area_flags, "Аттрибуты арий."},
    {"room", &room_flags, "Аттрибуты комнат."},
    {"sector", &sector_table, "Типы земель, секторов."},
    {"exit", &exit_flags, "Флаги выходов и экстравыходов."},
    {"type", &item_table, "Типы предметов."},
    {"extra", &extra_flags, "Аттрибуты предметов."},
    {"trap", &trap_flags, "Флаги ловушек."},
    {"detecion", &detect_flags, "Детекты."},
    {"wear", &wear_flags, "Куда одевать предмет."},
    {"spec", &spec_table, "Доступные спец. программы."},
    {"sex", &sex_table, "Пол."},
    {"act", &act_flags, "Аттрибуты монстров."},
    {"affect", &affect_flags, "Влияния на монстрах."},
    {"wear-loc", &wearloc_table, "Куда монстры одевают предметы."},
    {"spells", &skill_table, "Имена текущих заклинаний."},
    {"weapon", &weapon_flags, "Типы оружия."},
    {"container", &container_flags, "Статус контейнеров."},

    {"armor", &ac_type, "Защита от разных атак."},
    {"apply", &apply_flags, "Apply флаги"},
    {"affwhere", &affwhere_flags, "where флаги (TO_XXX)"},
    {"form", &form_flags, "Формы тела монстров."},
    {"part", &part_flags, "Части тела монстров."},
    {"imm", &imm_flags, "Иммунитеты монстров."},
    {"res", &res_flags, "Сопротивляемость монстров."},
    {"vuln", &vuln_flags, "Уязвимость монстров."},
    {"off", &off_flags, "Типы поведения монстров."},
    {"size", &size_table, "Размеры монстров."},
    {"position", &position_table, "Позиции монстров."},
    {"material", &mat_table, "Материалы." },
    {"wclass", &weapon_class, "Класс оружия."},
    {"wtype", &weapon_type2, "Специальные типы оружия."},
    {"portal", &portal_flags, "Типы порталов."},
    {"furniture", &furniture_flags, "Типы диванов и лежаков."},
    {"liquid", &liq_table, "Жидкости"},
    {"drink", &drink_flags, "Емкости для жидкостей"},
    {NULL, NULL, NULL}
};

/*****************************************************************************
 Name:          show_flag_cmds
 Purpose:       Displays settable flags and stats.
 Called by:     show_help(olc_act.c).
 ****************************************************************************/
void show_flag_cmds(Character * ch, const FlagTable *flag_table)
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int flag;
    int col;

    buf1[0] = '\0';
    col = 0;
    for (flag = 0; flag < flag_table->size; flag++) {
        sprintf(buf, "%-19s", flag_table->fields[flag].name);
        strcat(buf1, buf);
        if (++col % 4 == 0)
            strcat(buf1, "\n\r");
    }
    if (col % 4 != 0)
        strcat(buf1, "\n\r");
    stc(buf1, ch);
}

/*****************************************************************************
 Name:          show_skill_cmds
 Purpose:       Displays all skill functions.
                Does remove those damn immortal commands from the list.
                Could be improved by:
                (1) Adding a check for a particular class.
                (2) Adding a check for a level range.
 Called by:     show_help(olc_act.c).
 ****************************************************************************/
void show_skill_cmds(Character * ch, int tar)
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH * 2];
    int sn;
    int col;

    buf1[0] = '\0';
    col = 0;
    for (sn = 0; sn < SkillManager::getThis( )->size( ); sn++) {
        Skill *skill = SkillManager::getThis( )->find( sn );
        Spell::Pointer spell = skill->getSpell( );

        if (!spell)
            continue;
        
        if (tar == -1 || IS_SET( spell->getTarget( ), tar )) {
            sprintf(buf, "%-19s", skill->getName( ).c_str( ));
            strcat(buf1, buf);
            if (++col % 4 == 0)
                strcat(buf1, "\n\r");
        }
    }

    if (col % 4 != 0)
        strcat(buf1, "\n\r");
    stc(buf1, ch);
}

void show_skill_affects(Character *ch)
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

        buf << fmt( 0, "{g%-20s{x: %-20s %-10s %s\n\r",
                       skill->getName( ).c_str( ),
                       skill->getGroup( )->getName( ).c_str( ),
                       spell_types.name( spell->getSpellType( ) ).c_str( ),
                       target_table.names( spell->getTarget( ) ).c_str( ) );
    }

    page_to_char( buf.str( ).c_str( ), ch );
}



/*****************************************************************************
 Name:          show_spec_cmds
 Purpose:       Displays settable special functions.
 Called by:     show_help(olc_act.c).
 ****************************************************************************/
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

void show_mat_cmds( Character *ch )
{
    ostringstream buf;
    const material_t *mat;
    
    buf << "{gНазвание        Описание               Горючесть{x" << endl;
    for (mat = &material_table[0]; mat->name; mat++)
        buf << fmt( 0, "%-15s %-22N1 %-3d\n\r", 
                       mat->name, 
                       mat->rname ? mat->rname : "",
                       mat->burns );

    page_to_char( buf.str( ).c_str( ), ch );
}

void show_liq_cmds(Character * ch)
{
    std::basic_ostringstream<char> buf;
    Liquid *liq;

    buf << "Name                ShowName            Color       Proof Full Thirst Food Size" << endl;

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

/*****************************************************************************
 Name:          show_help
 Purpose:       Displays help for many tables used in OLC.
 Called by:     olc interpreters.
 ****************************************************************************/
bool show_help(Character * ch, const char *cargument)
{
    char buf[MAX_STRING_LENGTH];
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
        stc("Синтаксис: ? [комманда]\n\r\n\r", ch);
        stc("[комманда]  [описание]\n\r", ch);
        for (cnt = 0; help_table[cnt].command != NULL; cnt++) {
            sprintf(buf, "%-10s -%s\n\r",
                      DLString(help_table[cnt].command).capitalize( ).c_str( ),
                      help_table[cnt].desc);
            stc(buf, ch);
        }
        return false;
    }

    // Find the command, show changeable data.
    for (cnt = 0; help_table[cnt].command != NULL; cnt++) {
        if (arg[0] == help_table[cnt].command[0]
            && !str_prefix(arg, help_table[cnt].command)) {
            if (help_table[cnt].structure == &spec_table) {
                show_spec_cmds(ch);
                return false;
            }
            else if (help_table[cnt].structure == &liq_table) {
                show_liq_cmds(ch);
                return false;
            }
            else if (help_table[cnt].structure == &mat_table) {
                show_mat_cmds(ch);
                return false;
            }
            else if (help_table[cnt].structure == &wearloc_table) {
                int cnt = 0;
                for (int i = 0; i < wearlocationManager->size( ); i++) {
                    ch->printf( "%10s", wearlocationManager->find( i )->getName( ).c_str( ) );
                    if (++cnt%5 == 0)
                        ch->send_to( "\r\n" );
                }
                return false;
            }
            else if (help_table[cnt].structure == &skill_table) {
                if (spell[0] == '\0') {
                    stc("Синтаксис:  ? spells "
                        "[ignore/room/char/self/object/all]\n\r", ch);
                    return false;
                }

                if (!str_prefix(spell, "all"))
                    show_skill_cmds(ch, -1);
                else if (!str_prefix(spell, "ignore"))
                    show_skill_cmds(ch, TAR_IGNORE|TAR_CREATE_OBJ|TAR_CREATE_MOB);
                else if (!str_prefix(spell, "room"))
                    show_skill_cmds(ch, TAR_ROOM|TAR_PEOPLE);
                else if (!str_prefix(spell, "char"))
                    show_skill_cmds(ch, TAR_CHAR_ROOM|TAR_CHAR_WORLD);
                else if (!str_prefix(spell, "self"))
                    show_skill_cmds(ch, TAR_CHAR_SELF);
                else if (!str_prefix(spell, "object"))
                    show_skill_cmds(ch, TAR_OBJ_INV|TAR_OBJ_ROOM|TAR_OBJ_EQUIP|TAR_OBJ_WORLD);
                else if (!str_prefix(spell, "affect"))
                    show_skill_affects(ch);
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

