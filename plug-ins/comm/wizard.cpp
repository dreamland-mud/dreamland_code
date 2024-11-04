/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/
/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *        
 *     ANATOLIA has been brought to you by ANATOLIA consortium                   *
 *         Serdar BULUT {Chronos}                bulut@rorqual.cc.metu.edu.tr       *
 *         Ibrahim Canpunar  {Asena}        canpunar@rorqual.cc.metu.edu.tr    *        
 *         Murat BICER  {KIO}                mbicer@rorqual.cc.metu.edu.tr           *        
 *         D.Baris ACAR {Powerman}        dbacar@rorqual.cc.metu.edu.tr           *        
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *        
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*        ROM 2.4 is copyright 1993-1995 Russ Taylor                           *
*        ROM has been brought to you by the ROM consortium                   *
*            Russ Taylor (rtaylor@pacinfo.com)                                   *
*            Gabrielle Taylor (gtaylor@pacinfo.com)                           *
*            Brian Moore (rom@rom.efn.org)                                   *
*        By using this code, you have agreed to follow the terms of the           *
*        ROM license, in the file Rom24/doc/rom.license                           *
***************************************************************************/
#include <string.h>

#include "so.h"
#include "plugin.h"


#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "skill.h"
#include "skillmanager.h"
#include "skillgroup.h"
#include "affecthandler.h"

#include "plugininitializer.h"
#include "logstream.h"
#include "dlfileop.h"
#include "dreamland.h"
#include "affect.h"
#include "room.h"
#include "pcharactermanager.h"
#include "race.h"
#include "pcharactermemory.h"
#include "mobilebehavior.h"
#include "objectbehavior.h"
#include "pcharacter.h"
#include "save.h"
#include "merc.h"
#include "descriptor.h"
#include "desire.h"
#include "helpmanager.h"
#include "comm.h"
#include "weapons.h"
#include "badnames.h"
#include "wiznet.h"
#include "json_utils_ext.h"
#include "fight_extract.h"
#include "fight.h"
#include "interp.h"
#include "clan.h"
#include "liquid.h"
#include "morphology.h"
#include "playerattributes.h"
#include "player_exp.h"

#include "npcharacter.h"
#include "core/object.h"
#include "loadsave.h"
#include "fread_utils.h"
#include "stats_apply.h"
#include "wizard.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "act.h"
#include "def.h"

using std::min;

/* dedicated loader for wizard commands */
CMDLOADER_DECL(wizard)

#define IS_TRUSTED(ch,level)        (( ch->get_trust() ) >= (level))

/* command procedures needed */
void do_rstat                ( Character *, char * );
void do_mstat                ( Character *, Character * );
void do_ostat                ( Character *, char * );
void do_mfind                ( Character *, char * );
void do_ofind                ( Character *, char * );
void do_tfind                ( Character *, char * );
void do_mload                ( Character *, const DLString &);
void do_oload                ( Character *, const DLString & );

RELIG(none);
GSN(none);
GSN(plague);
GSN(sleep);
GSN(curse);
GSN(blindness);
GSN(poison);

LIQ(none);
CLAN(none);

/*
 * Local functions.
 */
static Room * find_location( Character *ch, char *arg )
{
    Character *victim;
    Object *obj;

    if ( is_number(arg) )
        return get_room_instance( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != 0 )
        return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != 0 )
        return obj->in_room;

    return 0;
}




struct limit_info {
    DLString description;
    int level;
    int vnum;
    int limit;
    int count;
    int inGame;
};
static bool limit_compare(const limit_info &a, const limit_info &b)
{
    return a.level < b.level;
}

CMDWIZP( limited )
{
        int nMatch;

        if ( argument[0] != '\0' )
        {
                int ingameCount;
                OBJ_INDEX_DATA *obj_index = get_obj_index( atoi(argument) );
                if ( obj_index == 0 )
                {
                        ch->pecho("Формат: limited <vnum>.");
                        return;
                }
                if ( obj_index->limit == -1 )
                {
                        ch->pecho("Это не лимит.");
                        return;
                }
                nMatch = 0;
                ch->pecho( "%-35s [%5d]  Лимит: %3d  Собрано: %3d",
                        obj_index->short_descr,
                        obj_index->vnum,
                        obj_index->limit,
                        obj_index->count );
                ingameCount = obj_index->instances.size();
                for (Object* obj=object_list; obj != 0; obj=obj->next )
                        if ( obj->pIndexData->vnum == obj_index->vnum )
                        {
                                if ( obj->carried_by != 0 && ch->can_see( obj->carried_by ) )
                                        ch->pecho( "У %-30s",
                                                obj->carried_by->getNameC());
                                if ( obj->in_room != 0 )
                                        ch->pecho("В комнате %-20s [%d]",
                                                obj->in_room->getName(), obj->in_room->vnum);
                                if ( obj->in_obj != 0 )
                                        ch->pecho( "Внутри %-20s [%d]",
                                                obj->in_obj->getShortDescr( '1', LANG_DEFAULT ).c_str( ),
                                                obj->in_obj->pIndexData->vnum);
                        }
                ch->pecho("  %d сейчас в игре, а еще %d в профилях игроков.",
                        ingameCount, obj_index->count-ingameCount);
                return;
        }

        list<limit_info> limits;
        nMatch = 0;
        for (int i = 0; i < MAX_KEY_HASH; i++)
        for (OBJ_INDEX_DATA *obj_index = obj_index_hash[i]; obj_index; obj_index = obj_index->next) {
                nMatch++;
                if (obj_index->limit > 0 && obj_index->limit < 100)
                {
                    struct limit_info info;
                    info.description = obj_index->short_descr.get(LANG_DEFAULT);
                    info.level = obj_index->level;
                    info.vnum = obj_index->vnum;
                    info.limit = obj_index->limit;
                    info.count = obj_index->count;
                    info.inGame = obj_index->instances.size();
                    limits.push_back(info);
                }
        }

        limits.sort(limit_compare);
        for (list<limit_info>::const_iterator l = limits.begin(); l != limits.end(); l++)
            ch->pecho( "[%3d] %-33^N1 [%5d]  Лимит: %3d  Собрано: %3d В игре : %3d",
                    l->level,
                    l->description.c_str(),
                    l->vnum,
                    l->limit,
                    l->count,
                    l->inGame );
        
        ch->pecho( "\n\rВсего лимитов: %d из %d.", limits.size(), nMatch );
}

CMDWIZP( wiznet )
{
        int flag;
        char buf[MAX_STRING_LENGTH];

        if (!ch->getPC( ))
            return;

        if ( argument[0] == '\0' )
        {
                if (IS_SET(ch->getPC( )->wiznet,WIZ_ON))
                {
                        ch->pecho("Визнет отключен. Уф.");
                        REMOVE_BIT(ch->getPC( )->wiznet,WIZ_ON);
                }
                else
                {
                        ch->pecho("Визнет включен, добро пожаловать в мир спама!");
                        SET_BIT(ch->getPC( )->wiznet,WIZ_ON);
                }

                return;
        }

        if (!str_prefix(argument,"on"))
        {
                ch->pecho("Визнет включен, добро пожаловать в мир спама!");
                SET_BIT(ch->getPC( )->wiznet,WIZ_ON);
                return;
        }

        if (!str_prefix(argument,"off"))
        {
                ch->pecho("Визнет отключен. Уф.");
                REMOVE_BIT(ch->getPC( )->wiznet,WIZ_ON);
                return;
        }

        /* show wiznet status */
        if (!str_prefix(argument,"status"))
        {
                buf[0] = '\0';

                if (!IS_SET(ch->getPC( )->wiznet,WIZ_ON))
                        strcat(buf,"off ");

                for (flag = 0; wiznet_table[flag].name != 0; flag++)
                        if (IS_SET(ch->getPC( )->wiznet,wiznet_table[flag].flag))
                        {
                                strcat(buf,wiznet_table[flag].name);
                                strcat(buf," ");
                        }

                strcat(buf,"\n\r");

                ch->pecho("Статус Визнета:");
                ch->send_to(buf);
                return;
        }

        if ( !str_prefix(argument,"show") )
        {
                /* list of all wiznet options */
                buf[0] = '\0';

                for (flag = 0; wiznet_table[flag].name != 0; flag++)
                {
                        if (wiznet_table[flag].level <= ch->get_trust())
                        {
                                strcat(buf,wiznet_table[flag].name);
                                strcat(buf," ");
                        }
                }

                strcat(buf,"\n\r");

                ch->pecho("Доступные тебе опции Визнета:");
                ch->send_to(buf);

                return;
        }

        flag = wiznet_lookup(argument);

        if ( flag == -1 || ch->get_trust() < wiznet_table[flag].level )
        {
                ch->pecho("Такой опции Визнета нет, или тебе она пока недоступна.");
                return;
        }

        if ( IS_SET(ch->getPC( )->wiznet,wiznet_table[flag].flag) )
        {
                ch->pecho("Ты больше не следишь за %s через Визнет.",
                           wiznet_table[flag].name);
                REMOVE_BIT(ch->getPC( )->wiznet,wiznet_table[flag].flag);
                return;
        }
        else
        {
                ch->pecho("Ты теперь отслеживаешь %s через Визнет.",
                           wiznet_table[flag].name);
                SET_BIT(ch->getPC( )->wiznet,wiznet_table[flag].flag);
                return;
        }
}



CMDWIZP( poofin )
{
    if (ch->is_npc())
        return;

    if (argument[0] == '\0')
    {
            ch->pecho("Твое сообщение poofin: %s",ch->getPC( )->bamfin.c_str( ));
            return;
    }

    ch->getPC( )->bamfin = argument;
    ch->pecho("Твое сообщение poofin теперь: %s",ch->getPC( )->bamfin.c_str( ));
}



CMDWIZP( poofout )
{
    if (ch->is_npc())
        return;

    if (argument[0] == '\0')
    {
            ch->pecho("Твое сообщение poofout: %s",ch->getPC( )->bamfout.c_str( ));
            return;
    }

    ch->getPC( )->bamfout = argument;

    ch->pecho("Твое сообщение poofout теперь %s",ch->getPC( )->bamfout.c_str( ));
}


CMDWIZP( transfer )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    Room *location;
    Descriptor *d;
    Character *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        ch->pecho("Перенести кого и куда?");
        return;
    }

    if (arg_is_all(arg1))
    {
        for ( d = descriptor_list; d != 0; d = d->next )
        {
            if ( d->connected == CON_PLAYING
            &&   d->character
            &&   d->character != ch
            &&   ch->can_see( d->character ) )
            {
                DLString cmd;
                cmd << d->character->getNameC() << " " << arg2;
                run( ch, cmd.c_str() );
            }
        }
        return;
    }


    if ( ( victim = get_char_world( ch, arg1 ) ) == 0 )
    {
        ch->pecho("Таких сейчас в мире нет.");
        return;
    }
    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
        location = ch->in_room;
    }
    else
    {
        if ( ( location = find_location( ch, arg2 ) ) == 0 )
        {
            ch->pecho("Цель не найдена.");
            return;
        }
        

        if ( location ->isPrivate( )
        &&  ch->get_trust() < MAX_LEVEL)
        {
            ch->pecho("Комната приватная и сейчас занята.");
            return;
        }
    }
    
    if (victim->desc && victim->desc->connected != CON_PLAYING) {
        ch->pecho("Плохая идея.");
        return;
    }

    transfer_char( victim, ch, location,
                  "%1$^C1 внезапно исчезает в столбе {Cбожественной энергии!{x",
                  (ch != victim ? "%2$^C1 переносит тебя в столбе {Cбожественной энергии!{x" : NULL),
                  "%1$^C1 внезапно прибывает в столбе {Cбожественной энергии!{x" );

    ch->pecho("%1$^C1 прибывает в столбе {Cбожественной энергии!{x", victim);
}



CMDWIZP( at )
{
    char arg[MAX_INPUT_LENGTH];
    Room *location;
    Room *original;
    Object *on;
    Character *wch;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        ch->pecho("Рядом с кем? Сделать что?");
        return;
    }

    if ( ( location = find_location( ch, arg ) ) == 0 )
    {
        ch->pecho("Цель не найдена.");
        return;
    }

    if ( location ->isPrivate( ) &&  ch->get_trust() < MAX_LEVEL)
    {
        ch->pecho("Комната приватная и сейчас занята.");
        return;
    }

    original = ch->in_room;
    on = ch->on;
    ch->dismount( );
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != 0; wch = wch->next )
    {
        if ( wch == ch )
        {
            undig( ch ); // handle 'at XXXX dig' case
            ch->dismount( );
            char_from_room( ch );
            char_to_room( ch, original );
            ch->on = on;
            break;
        }
    }

    return;
}


CMDWIZP( goto )
{
    Room *location;
    Character *rch;
    PCharacter *pch;

    if ( argument[0] == '\0' )
    {
        ch->pecho("Переместиться куда?");
        return;
    }

    if ( ( location = find_location( ch, argument ) ) == 0 )
    {
        ch->pecho("Цель не найдена: %s.", argument );
        return;
    }

    pch = ch->getPC( );

    if (!ch->is_npc( )) { // switched imms are silent
        for (rch = ch->in_room->people; rch != 0; rch = rch->next_in_room)
            if (rch->get_trust() >= ch->invis_level) {
                if (!pch->bamfout.empty( ))
                    oldact("$t", ch, pch->bamfout.c_str( ), rch, TO_VICT );
                else
                    oldact("$c1 исчезает в столбе {Cбожественной энергии.{x", ch, 0, rch, TO_VICT );
            }
    }
    
    transfer_char( ch, ch, location );
    
    if (!ch->is_npc( )) {
        for (rch = ch->in_room->people; rch != 0; rch = rch->next_in_room)
            if (rch->get_trust() >= ch->invis_level) {
                if (!pch->bamfin.empty( ))
                    oldact("$t", ch, pch->bamfin.c_str( ), rch, TO_VICT );
                else
                    oldact("$c1 внезапно появляется в столбе {Cбожественной энергии.{x", ch, 0, rch, TO_VICT );
            }
    }
}

/* RT to replace the 3 stat commands */

CMDWIZP( stat )
{
   char arg[MAX_INPUT_LENGTH];
   char *string;
   Object *obj;
   Room *location;
   Character *victim;
   bool fChar, fMob;

   string = one_argument(argument, arg);
   if ( arg[0] == '\0')
   {
        ch->pecho("Формат:");
        ch->pecho("  stat <name>");
        ch->pecho("  stat obj <name>");
        ch->pecho("  stat mob <name>");
        ch->pecho("  stat room <number>");
        return;
   }

   if (arg_is_strict(arg, "room"))
   {
        do_rstat(ch,string);
        return;
   }

   if (arg_is_strict(arg, "obj"))
   {
        do_ostat(ch,string);
        return;
   }
    
    fChar = arg_is_strict(arg, "char");
    fMob = arg_is_strict(arg, "mob");

    if (fChar || fMob) {
        if (!string[0]) {
           ch->pecho("Stat на кого?");
           return;
        }
        
        victim =  fChar ? get_player_world( ch, string ) : get_char_world( ch, string );
        if (!victim) {
            ch->pecho("%s с именем %s не найден.", fMob ? "Персонаж" : "Игрок", string);
            return;
        }
        
        do_mstat(ch, victim);
        return;
   }

   /* do it the old way */

   obj = get_obj_world(ch,argument);
   if (obj != 0)
   {
     do_ostat(ch,argument);
     return;
   }

  victim = get_char_world(ch,argument);
  if (victim != 0)
  {
    do_mstat(ch, victim);
    return;
  }

  location = find_location(ch,argument);
  if (location != 0)
  {
    do_rstat(ch,argument);
    return;
  }

  ch->pecho("С таким именем ничего не найдено.");
}

static void format_affect_duration(Affect *paf, ostringstream &buf)
{
    if (paf->duration >= 0)
        buf << fmt(0, " в течение %1$d час%1$Iа|ов|ов", paf->duration);
    else
        buf << " постоянно";    
}

static void format_affect_level(Affect *paf, ostringstream &buf)
{
    if (paf->level >= 0)
        buf << ", уровень " << paf->level;
}

static void format_affect(Affect *paf, ostringstream &buf)
{
    int b = paf->bitvector;
    int mod = paf->modifier;
    const FlagTable *table = paf->bitvector.getTable();
    const GlobalRegistryBase *registry = paf->global.getRegistry();
    bool empty = true;

    buf << "Аффект";
    if (paf->type != gsn_none) 
        buf << " " << paf->type->getRussianName().quote();
    buf << ": ";

    if (registry) {
        if (registry == skillManager) {
            buf << (mod >= 0 ? "повышает" : "понижает") << " "
                << (paf->location == APPLY_LEARNED ? "владение умением" : "уровень умения")
                << " " << paf->global.toRussianString().quote() 
                << " на " << (int)abs(mod);
        } else if (registry == skillGroupManager) {
            buf << (mod >= 0 ? "повышает" : "понижает") << " "
                << (paf->location == APPLY_LEARNED ? "владение группой умений" : "уровень всех умений группы")
                << " " << paf->global.toRussianString().quote() 
                << " на " << (int)abs(mod);
        } else if (registry == liquidManager) {
            buf << "добавляет запах " << paf->global.toRussianString('2', ",").colourStrip();
        } else if (registry == wearlocationManager) {
            buf << "отнимает конечность " << paf->global.toString(',');
        } else {
            buf << "неизвестный вектор";
        }

        format_affect_duration(paf, buf);
        format_affect_level(paf, buf);
        buf << "." << endl;
        return;
    }

    if (table && b != 0) {
        if (paf->location == APPLY_BITVECTOR && paf->modifier < 0)
            buf << "отнимает ";
        else
            buf << "добавляет ";

        if (table == &affect_flags)
            buf << "аффект ";
        else if (table == &imm_flags)
            buf << "иммунитет к ";
        else if (table == &res_flags)
            buf << "сопротивляемость к ";
        else if (table == &vuln_flags)
            buf << "уязвимость к ";
        else if (table == &detect_flags)
            buf << "обнаружение ";
        else if (table == &room_flags)
            buf << "флаги комнаты ";
        else if (table == &raffect_flags)
            buf << "аффект комнаты ";
        else if (table == &weapon_type2)
            buf << "флаги оружия ";
        else if (table == &extra_flags)
            buf << "флаги предмета ";
        else if (table == &plr_flags)
            buf << "флаги персонажа ";
        else if (table == &form_flags)
            buf << "форму тела ";
        else if (table == &part_flags)
            buf << "часть тела ";
        else
            buf << "не пойми что ";

        buf << table->messages(b);

        format_affect_duration(paf, buf);
        format_affect_level(paf, buf);
        buf << "." << endl;
        empty = false;
    }

    if (paf->modifier != 0 && paf->location != APPLY_NONE && paf->location != APPLY_BITVECTOR) {
        if (!empty)
            buf << "        ";
        buf << "изменяет " << paf->location.message()
            << " на " << paf->modifier;

        format_affect_duration(paf, buf);
        format_affect_level(paf, buf);
        buf << "." << endl;
        empty = false;
    }

    if (empty)
        buf << " (ничего)." << endl;
}


/* NOTCOMMAND */ void do_rstat( Character *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ostringstream b;
    Room *r;
    Object *obj;
    Character *rch;
    int door;

    one_argument( argument, arg );
    r = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( r == 0 )
    {
        ch->pecho("Цель не найдена.");
        return;
    }

    b << "Имя: [" << r->getName() << "]  Зона: [" << r->areaName() << "]" << endl;

    {
        ostringstream tbuf;

        if (r->pIndexData->clan != clan_none)
            tbuf << "Клан: " << r->pIndexData->clan->getShortName() << "  ";
        if (!r->pIndexData->guilds.empty())
            tbuf << "Гильдии: " << r->pIndexData->guilds.toRussianString() << "  ";

        if (!tbuf.str().empty())
            b << tbuf.str() << endl;
    }

    b << "Vnum: " << r->vnum << "  "
      << "Сектор: " << sector_table.message(r->getSectorType());

    if (r->getLiquid() != liq_none)
        b << " (" << r->getLiquid()->getShortDescr().ruscase('1') << "{x)";

    b << "  " << "Свет: " << r->light << "  "
      << "Лечение: " << r->getHealRate() << "  "
      << "Мана: " << r->getManaRate() << endl;

    if (r->affected_by)
        b << "Под воздействием: " << raffect_flags.messages(r->affected_by, true) << endl;

    if (r->room_flags)
        b << "Флаги: " << room_flags.names(r->room_flags) << endl;

    b << "Описание:" << endl << r->getDescription();

    if (!r->getExtraDescr().empty())
    {
        b << "Ключевые слова экстра-описания: ";
        for (auto &ed: r->getExtraDescr())
        {
            b << "[" << ed->keyword << "] ";
        }
        b << endl;
    }

    b << "Персонажи: ";
    for ( rch = r->people; rch; rch = rch->next_in_room )
    {
        if (ch->can_see(rch))
            b << "[" << rch->getNameP('1') << "] ";
    }

    b << endl << "Объекты: ";
    for (obj = r->contents; obj; obj = obj->next_content)
        b << "[" << Syntax::label_ru(obj->getKeyword()) << "] ";
    b << endl;

    b << "Выходы:" << endl;
    for (door = 0; door < DIR_SOMEWHERE; door++) {
        EXIT_DATA *pexit = r->exit[door];
        if (!pexit)
            continue;

        Room *to_room = pexit->u1.to_room;

        b << fmt(0, "{G%-6s{x [{W%5u{x] [{W%-10s{x]",
                      dirs[door].rname,
                      to_room ? to_room->vnum : 0,
                      to_room ? to_room->getName() : "");

        b << endl;

        if (pexit->key > 0)
            b << fmt(0, "Ключ: [{W%7u{x]  ", pexit->key) << endl;
            
        if (pexit->exit_info)
            b << fmt(0, "Флаги: [{W%s{x]", exit_flags.names(pexit->exit_info).c_str()) << endl;

        {
            ostringstream tbuf;

            if (!pexit->keyword.empty())
                tbuf << fmt(0, "Имена: [{W%s{x]  ", Syntax::label_ru(pexit->keyword).c_str());

            if (!pexit->short_descr.empty())
                tbuf << fmt(0, "Краткое: [{W%s{x]  ", pexit->short_descr.get(LANG_DEFAULT).ruscase('1').c_str());

            if (!pexit->description.empty())
                tbuf << "Описание: " << pexit->description.get(LANG_DEFAULT);

            if (!tbuf.str().empty())
                b << tbuf.str() << endl;
        }
    }

    for (auto &paf: r->affected)
        format_affect(paf, b);

    if (!r->history.empty()) {
        b << "Следы:" << endl;

        for (RoomHistory::iterator h = r->history.begin( );
            h != r->history.end( );
            h++)
        {
            b << fmt(0, "%s проходит через дверь %d.\r\n", h->name.c_str( ), h->went );
        }
    }

    if (r->behavior) {
        b << "Поведение: [" << r->behavior->getType() << "]" << endl;
        r->behavior.toStream(b);
    }

    page_to_char(b.str().c_str(), ch);
}


/* NOTCOMMAND */ void do_ostat( Character *ch, char *argument )
{
        char arg[MAX_INPUT_LENGTH];
        Object *obj;
        Liquid *liquid;
        ostringstream buf;

        one_argument( argument, arg );

        if ( arg[0] == '\0' )
        {
                ch->pecho("Stat чему?");
                return;
        }

        if ( ( obj = get_obj_world( ch, argument ) ) == 0 )
        {
                ch->pecho("Ни на земле, ни в небесах не найдено. Увы и ах.");
                return;
        }

        buf << fmt(0, "Name(s): %s\n\r", String::toString(obj->getKeyword()).c_str() );

        buf << fmt(0, "Vnum: %d  Лимит: %d  Тип: %s  Ресеты: %d\n\r",
                obj->pIndexData->vnum, obj->pIndexData->limit,
                item_table.message(obj->item_type).c_str( ),
                obj->pIndexData->reset_num );

        if (obj->timestamp > 0) {
            DLString d = Date( obj->timestamp ).getTimeAsString( );
            buf << fmt(0, "Лимит исчезнет в %s.\r\n", d.c_str( ) );
        }

        buf << fmt(0, "Шорт: %s\n\rДлинное описание: %s\n\r",
                obj->getShortDescr(LANG_DEFAULT).c_str(), obj->getDescription(LANG_DEFAULT).c_str() );

        buf << fmt(0, "Владелец: %s\n\r", obj->getOwner().empty() ? "nobody" : obj->getOwner().c_str());

        buf << fmt(0, "Материал: %s\n\r", obj->getMaterial( ).c_str());

        buf << fmt(0, "Берется в: %s\n\rЭкстра флаги: %s\n\r",
                wear_flags.messages(obj->wear_flags, true).c_str( ), 
                extra_flags.messages(obj->extra_flags, true).c_str( ) );

        buf << fmt(0, "Число: %d/%d  Вес: %d/%d/%d (десятые доли фунта)\n\r",1,
                obj->getNumber( ), obj->weight, obj->getWeight( ), obj->getTrueWeight( ) );

        buf << fmt(0, "Уровень: %d  Цена: %d  Состояние: %d  Таймер: %d По счету: %d\n\r",
                obj->level, obj->cost, obj->condition, obj->timer, obj->pIndexData->count );

        buf << fmt(0, "В комнате: %d  Внутри: %s  В руках у: %s  Надето на: %s\n\r",
                obj->in_room == 0 ? 0 : obj->in_room->vnum,
                obj->in_obj  == 0 ? "(none)" : obj->in_obj->getShortDescr( '1', LANG_DEFAULT ).c_str( ),
                obj->carried_by == 0 ? "(none)" :
                        ch->can_see(obj->carried_by) ? obj->carried_by->getNameC() : "someone",
                obj->wear_loc->getName( ).c_str( ) );

        buf << fmt(0, "Значения: %d %d %d %d %d\n\r",
                obj->value0(), obj->value1(), obj->value2(), obj->value3(),        obj->value4() );

        // now give out vital statistics as per identify

        switch ( obj->item_type )
        {
        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
                buf << fmt(0,  "Заклинания %d уровня:", obj->value0() );
                ch->send_to(buf);

                if ( obj->value1() >= 0 && obj->value1() < SkillManager::getThis( )->size() )
                {
                    buf << " '"
                        << SkillManager::getThis( )->find(obj->value1())->getName()
                        << "'";
                }

                if ( obj->value2() >= 0 && obj->value2() < SkillManager::getThis( )->size() )
                {
                    buf << " '"
                        << SkillManager::getThis( )->find(obj->value2())->getName()
                        << "'";
                }

                if ( obj->value3() >= 0 && obj->value3() < SkillManager::getThis( )->size() )
                {
                    buf << " '"
                        << SkillManager::getThis( )->find(obj->value3())->getName()
                        << "'";
                }

                if (obj->value4() >= 0 && obj->value4() < SkillManager::getThis( )->size())
                {
                    buf << " '"
                        << SkillManager::getThis( )->find(obj->value4())->getName()
                        << "'";
                }

                buf << "." << endl;
                break;

        case ITEM_WAND:
        case ITEM_STAFF:
                buf << fmt(0, "Имеет %d(%d) зарядов уровня %d",
                        obj->value1(), obj->value2(), obj->value0() );

                if ( obj->value3() >= 0 && obj->value3() < SkillManager::getThis( )->size() )
                {
                    buf << " '"
                        << SkillManager::getThis( )->find(obj->value3())->getName()
                        << "'";
                }

                buf << "." << endl;
                break;

        case ITEM_DRINK_CON:
                liquid = liquidManager->find( obj->value2() );
                buf << fmt(0, "Содержит жидкость %s цвета, %s.\n",
                    liquid->getColor( ).ruscase( '2' ).c_str( ),
                    liquid->getShortDescr( ).ruscase( '4' ).c_str( ) );
                break;
                
        case ITEM_WEAPON:
                buf << "Тип оружия: " << weapon_class.name(obj->value0()) << endl;
                
                buf << fmt(0, "Урон %dd%d (среднее %d)\n\r",
                        obj->value1(),obj->value2(),weapon_ave(obj));

                buf << fmt(0, "Тип удара: %s.\n\r", weapon_flags.name(obj->value3()).c_str( ));
        
                if (obj->value4())  /* weapon flags */
                {
                        buf << fmt(0, "Флаги оружия: %s\n\r",weapon_type2.messages(obj->value4()).c_str( ));
                }
                break;

        case ITEM_ARMOR:
                buf << fmt(0, "Класс брони: %d колющее, %d удар, %d режущее, %d экзотика\n\r",
                        obj->value0(), obj->value1(), obj->value2(), obj->value3() );
                break;

        case ITEM_CONTAINER:
                buf << fmt(0, "Вместимость: %d#  Максимальный вес: %d#  Флаги: %s\n\r",
                        obj->value0(), obj->value3(), container_flags.messages(obj->value1()).c_str( ));
                if (obj->value4() != 100)
                {
                        buf << fmt(0, "Уменьшение веса: %d%%\n\r",obj->value4());
                }
                break;
        
        case ITEM_CORPSE_PC:
        case ITEM_CORPSE_NPC:
                buf << fmt(0, "Стейков: %d, Уровень: %d, Части тела: '%s', Vnum: %d\r\n",
                            obj->value0(), obj->value1(), 
                            part_flags.messages( obj->value2() ).c_str( ), obj->value3() );
                break;
        }

        if (!obj->extraDescriptions.empty())
        {
                buf << "Ключевые слова экстра-описаний: ";

                for (auto &ed: obj->extraDescriptions)
                {
                       buf << "[" << ed->keyword << "] ";
                }

                buf << endl;
        }

        if (!obj->pIndexData->extraDescriptions.empty())
        {
                buf << "Оригинал экстра-описания: ";

                for (auto &ed: obj->pIndexData->extraDescriptions)
                {
                       buf << "[" << ed->keyword << "] ";
                }

                buf << endl;
        }

    for (auto &paf: obj->affected)
        format_affect(paf, buf);

    for (auto &paf: obj->pIndexData->affected)
        format_affect(paf, buf);


    buf << fmt(0, "Состояние : %d (%s) ", obj->condition, obj->get_cond_alias() );        
    
    if (obj->behavior) {        
        buf << fmt(0, "Поведение: [%s]\r\n", obj->behavior->getType( ).c_str( ));
        obj->behavior.toStream( buf );

    } else {
       buf << endl;
    }

    buf << "Свойства: ";
    
    ch->send_to(buf);

    ch->desc->send(JsonUtils::toString(obj->props).c_str());
}


void show_char_pk_flags( PCharacter *ch, ostringstream &buf );

static bool has_nochannel(Character *ch)
{
    static const DLString nochannel( "nochannel" );
    
    return !ch->is_npc() && ch->getPC()->getAttributes( ).isAvailable( nochannel );
}

static bool has_nopost(Character *ch)
{
    static const DLString nopost( "nopost" );
    
    return !ch->is_npc() && ch->getPC()->getAttributes( ).isAvailable( nopost );
}


/* NOTCOMMAND */ void do_mstat( Character *ch, Character *victim )
{
    ostringstream buf;
    PCharacter *pc = victim->is_npc( ) ? 0 : victim->getPC( ); // no switched data
    NPCharacter *npc = victim->getNPC( );
    
    buf << "Имя: [" << victim->getNameC() << "] ";
    if (pc)
        buf << "Шорт: [" << pc->getRussianName( ).normal( ) << "] ";
    if (npc)
        buf << "Родная зона: " << (npc->zone ? npc->zone->getName() : "?");
    buf << endl;
    
    if (npc)
        buf << "Vnum: "   << npc->pIndexData->vnum << "  "
            << "Группа: "  << npc->group << "  "
            << "По счету: "  << npc->pIndexData->count << "  "
            << "Убито: " << npc->pIndexData->killed
            << endl;

    buf << "Раса: " << victim->getRace( )->getName( ) << "  "
        << "Пол: "  << sex_table.name(victim->getSex( )) << "  "
        << "Комната: " << victim->in_room->vnum
        << endl;
    
    for (int s = 0; s < stat_table.size; s++)
        buf << stat_table.name(s).capitalize( ) << ": "
            << victim->perm_stat[s] 
            << "(" << victim->getCurrStat(s) << ")"
            << "  ";
    buf << endl;
    
    buf << "Хиты: "   << victim->hit << "/" << victim->max_hit << " "
        << "Мана: " << victim->mana << "/" << victim->max_mana << " "
        << "Мувы: " << victim->move << "/" << victim->max_move << " ";
    if (pc)
        buf << "Практик: "  << pc->practice << " "
            << "Тренировок: " << pc->train << " ";
    buf << endl;
    if (victim->heal_gain != 0 || victim->mana_gain != 0 || victim->mod_beats != 0)
        buf << "Бонус восстановления жизни: " << victim->heal_gain 
            << "%   Маны: " << victim->mana_gain << "%  Задержки: "  << victim->mod_beats << "%" << endl;
    
    if (victim->getReligion( ) == god_none)
        buf << "Не верит в богов." << endl;
    else
        buf << "Верит в " << victim->getReligion( )->getShortDescr( ) << endl;
    
    buf << "Уровень: " << victim->getRealLevel( ) << "(" << victim->getModifyLevel( ) << ")  "
        << "Класс: " << victim->getProfession( )->getName( ) << "  "
        << "Натура: " << align_table.name(ALIGNMENT(victim)) << " (" << victim->alignment << ")  "
        << "Этос: " << ethos_table.name( victim->ethos ) << "  "
        << endl;

    buf << "Золото: " << victim->gold << "  "
        << "Серебро: " << victim->silver << "  ";
    if (pc)
        buf << "Золота в банке: " << pc->bank_g << "  "
            << "Серебра в банке: " << pc->bank_s << "  "
            << "QP: " << pc->getQuestPoints() 
            << endl
            << "Опыт: " << pc->exp << "  "
            << "До уровня: " << pc->getExpToLevel( ) << "  "
            << "На уровень: " << pc->getExpPerLevel( pc->getLevel( ) + 1 ) - pc->getExpPerLevel( );
    buf << endl;
        
    buf << "Броня: ";
    for (int a = 0; a < ac_type.size; a++)
        buf << ac_type.name(a) << ": " << GET_AC(victim, a) << "  ";
    buf << endl;
    
    buf << "Хитролл: "   << victim->hitroll << "  "
        << "Дамролл: "   << victim->damroll << "  "
        << "Савесы: " << victim->saving_throw << "  "
        << "Размер: "  << size_table.name(victim->size) << " (" << victim->size << ")  "
        << "Позиция: "   << position_table.name(victim->position) << "  ";
    if (pc)
        buf << "Трусость: " << victim->wimpy;
    buf << endl;

    if (npc)
        buf << "Урон: " 
            << npc->damage[DICE_NUMBER] << "d" 
            << npc->damage[DICE_TYPE] << "+"
            << npc->damage[DICE_BONUS] << "  "
            << "Тип урона: " << weapon_flags.name(npc->dam_type)
            << endl;
    
    buf << "Сражается с: " << (victim->fighting ? victim->fighting->getNameC() : "(none)") << "  ";
    if (pc)
        buf << "Смертей: " << pc->death << "  ";
    buf << "Несет вещей: " << victim->carry_number << "  "
        << "Общим весом: " << Char::getCarryWeight(victim) / 10
        << endl;
    
        
    if (pc) {
        for (int i = 0; i < desireManager->size( ); i++)
            buf << desireManager->find( i )->getName( )
                << ": " << pc->desires[i] << "  ";
        
        buf << endl
            << "Возраст: "        << pc->age.getYears( ) << "  "
            << "Отыграно: "     << pc->age.getHours( ) << "(" << pc->age.getTrueHours( ) << ")  "
            << "Последний уровень: " << pc->last_level << "  "
            << "Таймер: "      << pc->timer
            << endl;
    }
    
    if (npc)
        buf << "Поведение: " << act_flags.names(victim->act) << endl;
        
    if (pc) {
        buf << "Поведение: " << plr_flags.names(victim->act) << " ";
        show_char_pk_flags( pc, buf );
        buf << endl;
    }

    if (victim->affected_by)
        buf << "Под воздействием " << affect_flags.names(victim->affected_by) << endl;
    
    if (pc || victim->comm) {
        buf << "Связь: " 
            << comm_flags.names(victim->comm) << " "
            << add_comm_flags.names(victim->add_comm) << " ";
        
        if (has_nochannel(victim))
            buf << "nochannel ";
        if (has_nopost(victim))
            buf << "nopost ";
        buf << endl;
    }
    
    if (npc && npc->off_flags)
        buf << "Атаки: " << off_flags.names(npc->off_flags) << endl;

    if (victim->imm_flags)
        buf << "Иммунитеты: " << imm_flags.names(victim->imm_flags) << endl;

    if (victim->res_flags)
        buf << "Сопротивляемости: " <<  res_flags.names(victim->res_flags) << endl;

    if (victim->vuln_flags)
        buf << "Уязвимости: " << vuln_flags.names(victim->vuln_flags) << endl;

    if (victim->detection)
        buf << "Видит: " <<  detect_flags.names(victim->detection) << endl;
    
    buf << "Форма:  " << form_flags.names(victim->form) << "  " << endl
        << "Части тела: " << part_flags.names(victim->parts) << endl;
    buf << "Слоты экипировки: " << victim->getWearloc().toString() << endl;
    
    buf << "Хозяин " <<  (victim->master ? victim->master->getNameC() : "(none)") << "  "
        << "Лидер " <<  (victim->leader ? victim->leader->getNameC() : "(none)") << "  ";
    if (pc)
        buf << "Пет: " << (pc->pet ? pc->pet->getNameC() : "(none)");
    buf << endl;
    
    if (npc) {
        buf << "Шорт: " << npc->getShortDescr(LANG_DEFAULT) << endl
            << "Длинное описание: "  << npc->getLongDescr(LANG_DEFAULT) << endl;

        buf << "Все имена: " << npc->getNameP('7') << endl;
        const char *spec_fun_name = spec_name(*npc->spec_fun);
        if (spec_fun_name != 0)
            buf << "Спец-процедура " << spec_fun_name << "." << endl;
    }

    for (auto &paf: victim->affected)
        format_affect(paf, buf);
        
    if (pc) {
        if (pc->getLastAccessTime( ).getTime( ) != 0)
            buf << "Последний раз в мире: " << pc->getLastAccessTime( ).getTimeAsString( ) << endl;
        
        if (!pc->getLastAccessHost( ).empty( ))
            buf << "Последний хост: " << pc->getLastAccessHost( ) << endl;
    }

    buf << "Прошлый бой: " << (victim->last_fought ? victim->last_fought->getNameC() :"none") << "  "
        << "Когда: " << Date::getTimeAsString( victim->getLastFightTime( ) ) 
        << endl;
    
    if (!victim->ambushing.empty())
        buf << "В засаде на: [" << victim->ambushing << "]" << endl;
    
    if (npc && !npc->pIndexData->practicer.empty( ))
        buf << "Обучает: " << npc->pIndexData->practicer.toString( ) << endl;

    if (npc && !npc->pIndexData->religion.empty())
        buf << "Верит в: " << npc->pIndexData->religion.toString( ) << endl;

    if (npc && npc->behavior) { 
        buf << "Поведение: [" << npc->behavior->getType( ) << "]" << endl;
        npc->behavior.toStream( buf );
    }
    
    page_to_char( buf.str( ).c_str( ), ch );
}

CMDWIZP( vnum )
{
    char arg[MAX_INPUT_LENGTH];
    char *string;

    string = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        ch->pecho("Формат:");
        ch->pecho("  vnum obj <name>");
        ch->pecho("  vnum mob <name>");
        ch->pecho("  vnum type <item_type>");
        return;
    }
    
    if (arg_is_strict(arg, "type")) {
        do_tfind(ch, string);
        return;
    }

    if (arg_is_strict(arg, "obj")) {
        do_ofind(ch,string);
         return;
    }

    if (arg_is_strict(arg, "char") || arg_is_strict(arg, "mob")) {
        do_mfind(ch,string);
        return;
    }

    /* do both */
    do_mfind(ch,argument);
    do_ofind(ch,argument);
}


/* NOTCOMMAND */ void do_mfind( Character *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int nMatch;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        ch->pecho("Найти кого?");
        return;
    }

    found        = false;
    nMatch        = 0;

    for (int i=0; i < MAX_KEY_HASH; i++)
        for(MOB_INDEX_DATA *pMob = mob_index_hash[i]; pMob; pMob = pMob->next) 
        {
            nMatch++;
            if (mob_index_has_name(pMob, arg))
            {
                found = true;
                ch->pecho("[%5d] %N1",
                    pMob->vnum, 
                    pMob->getShortDescr(LANG_DEFAULT));
            }
        }   

    if ( !found )
        ch->pecho("Мобы с таким именем не найдены.");

    return;
}



/* NOTCOMMAND */ void do_ofind( Character *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int nMatch;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        ch->pecho("Найти что?");
        return;
    }

    found        = false;
    nMatch        = 0;

    for (int i=0; i<MAX_KEY_HASH; i++)
        for(OBJ_INDEX_DATA *pObj = obj_index_hash[i]; pObj; pObj = pObj->next) 
        {
            nMatch++;
            if (obj_index_has_name(pObj, arg))
            {
                found = true;
                ch->pecho("[%5d] %N1",
                    pObj->vnum, 
                    pObj->getShortDescr(LANG_DEFAULT));
            }
        }

    if ( !found )
        ch->pecho("Объекты с таким именем не найдены.");

    return;
}

/* NOTCOMMAND */ void do_tfind( Character *ch, char *argument )
{
    int type;
    ostringstream buf;
    
    if ((type = item_table.value( argument )) == NO_FLAG) {
        ch->pecho( "Такого типа предмета не существует." );
        return;
    }
    
    for (int i=0; i<MAX_KEY_HASH; i++)
        for(OBJ_INDEX_DATA *pObj = obj_index_hash[i]; pObj; pObj = pObj->next) 
            if (pObj->item_type == type) {
                buf << fmt(0, "[%5d] %N1\n",
                                 pObj->vnum,
                                pObj->getShortDescr(LANG_DEFAULT));
            }

    if (buf.str( ).empty( ))
        ch->pecho( "Такого типа нет ни у одного объекта." );
    else
        page_to_char( buf.str( ).c_str( ), ch ); 
}

CMDWIZP( rwhere )
{
    ostringstream buf;
    bool found = false;

    if (argument[0] == '\0') {
        ch->pecho("Найти какую комнату?");
        return;
    }

    for (auto &r: roomInstances)
        if (is_name(argument, r->getName())) {
            buf << fmt(0, "[%6d] %-30s %s\r\n", r->vnum, r->getName(), r->areaName().c_str());
            found = true;
        }

    if (!found)
        ch->pecho("Комната с таким именем не найдена.");
    else
        ch->send_to(buf);
}

CMDWIZP( owhere )
{
    ostringstream buffer;
    Object *obj;
    Object *in_obj;
    bool found = false;;
    int number = 0, max_found = 200;
    int vnum = -1;

    if ( argument[0] == '\0' )
    {
            ch->pecho("Найти что?");
            return;
    }

    if (is_number( argument ))
        vnum = atoi( argument );

    for ( obj = object_list; obj != 0; obj = obj->next )
    {
        if (!ch->can_see( obj ))
            continue;
        if (ch->get_trust( ) < obj->level)
            continue;

        if (vnum > 0) {
            if (obj->pIndexData->vnum != vnum)
                continue;
        }
        else {
            if (!obj_has_name(obj, argument, 0))
                continue;
        }


        found = true;
        number++;

        for ( in_obj = obj; in_obj->in_obj != 0; in_obj = in_obj->in_obj )
                ;

        if ( in_obj->carried_by != 0 && ch->can_see(in_obj->carried_by)
                && in_obj->carried_by->in_room != 0 )
                buffer << fmt(0, "%3d) %N1 в руках у %s [Комната %d]\n\r",
                        number,
                        obj->getShortDescr( LANG_DEFAULT ).c_str( ),
                        ch->sees(in_obj->carried_by, '2').c_str(),
                        in_obj->carried_by->in_room->vnum );
        else if ( in_obj->in_room != 0 && ch->can_see(in_obj->in_room) )
                buffer << fmt(0, "%3d) %N1 на полу в %s [Комната %d]\n\r",
                        number,
                        obj->getShortDescr( LANG_DEFAULT ).c_str( ),
                        in_obj->in_room->getName(),
                        in_obj->in_room->vnum );
        else
               buffer << fmt(0, "%3d) %N1 черт его знает где.\n\r",
                        number,
                        obj->getShortDescr(LANG_DEFAULT).c_str( ) );

    
        if ( number >= max_found )
            break;
    }

    if (!found)
        ch->pecho("Ни на земле, ни в небесах не найдено. Увы и ах.");
    else
        page_to_char( buffer.str( ).c_str( ), ch );
}


CMDWIZP( mwhere )
{
    ostringstream buffer;
    Character *victim;
    bool found;
    int count = 0;
    int vnum = -1;

    if ( argument[0] == '\0' )
    {
            Descriptor *d;

            /* show characters logged */

            for (d = descriptor_list; d != 0; d = d->next)
            {
                if(!d->character)
                    continue;

                if(d->connected != CON_PLAYING)
                    continue;
                
                if(!d->character->in_room)
                    continue;
                
                if(!ch->can_see(d->character))
                    continue;
                
                if(!ch->can_see(d->character->in_room))
                    continue;
                
                victim = d->character;
                count++;

                if (victim->is_npc( ))
                    buffer << fmt(0, "%3d) %s (в теле %s) находится в %s [%d]\n\r",
                            count, 
                            victim->getPC( )->getNameC(),
                            victim->getNameP('1').c_str( ),
                            victim->in_room->getName(), victim->in_room->vnum);
                else
                    buffer << fmt(0, "%3d) %s находится в %s [%d]\n\r",
                            count, 
                            victim->getNameC(),
                            victim->in_room->getName(), victim->in_room->vnum);

            }

            page_to_char(buffer.str( ).c_str( ), ch);
            return;
    }

    found = false;

    if (is_number( argument ))
        vnum = atoi( argument );

    for (victim = char_list; victim != 0; victim = victim->next)
    {
        if (vnum > 0) {
            if (!victim->is_npc( ))
                continue;
            if (victim->getNPC( )->pIndexData->vnum != vnum)
                continue;
        } 
        else {
            if (!is_name( argument, victim->getNameP( '7' ).c_str( ) ))
                continue;
        }

        found = true;
        count++;
        buffer << fmt(0, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
                victim->is_npc() ? victim->getNPC()->pIndexData->vnum : 0,
                victim->is_npc() ? victim->getNameP( '1' ).c_str( ) : victim->getNameC(),
                victim->in_room->vnum,
                victim->in_room->getName() );
    }

    if (!found)
        oldact_p("Тебе не удалось найти $T.", ch, 0, argument, TO_CHAR,POS_DEAD );
    else
        page_to_char(buffer.str( ).c_str( ),ch);

}






CMDWIZP( shutdown )
{
    Descriptor *d,*d_next;
    
    DLFileAppend( dreamland->getBasePath( ), dreamland->getShutdownFile( ) )
         .printf( "Shutdown %s by %s\n",
                    Date::getCurrentTimeAsString( ).c_str( ),
                    ch->getNameC()
                );

    /* TODO save all */
    dreamland->shutdown( );

    for ( d = descriptor_list; d != 0; d = d_next)
    {
        d_next = d->next;
        d->close( );
    }
}


CMDWIZP( snoop )
{
    char arg[MAX_INPUT_LENGTH];
    Descriptor *d;
    Character *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->pecho("Следить за кем?");
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == 0 )
    {
        ch->pecho("Таких сейчас в мире нет");
        return;
    }

    if ( victim->desc == 0 )
    {
        ch->pecho("Дескриптор не найден!");
        return;
    }

    if ( victim == ch )
    {
        ch->pecho("Отменяем всю слежку.");
        wiznet( WIZ_SNOOPS, WIZ_SECURE, ch->get_trust(), "%C1 прекращает отслеживание.", ch );
        for ( d = descriptor_list; d != 0; d = d->next )
        {
            if ( d->snoop_by == ch->desc )
                d->snoop_by = 0;
        }
        return;
    }

    if ( victim->desc->snoop_by != 0 )
    {
        ch->pecho("Кто-то уже следит за этим игроком.");
        return;
    }

    if (ch->in_room != victim->in_room
    &&  victim->in_room->isPrivate( ) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        ch->pecho("Этот игрок сейчас в приватной комнате.");
        return;
    }

    if ( victim->get_trust() >= ch->get_trust()
    ||   IS_SET(victim->comm,COMM_SNOOP_PROOF))
    {
        ch->pecho("У тебя пока не хватает прав на слежку за игроками.");
        return;
    }

    if ( ch->desc != 0 ) {
        for ( d = ch->desc->snoop_by; d != 0; d = d->snoop_by ) {
            if ( d->character == victim || d->character->getPC( ) == victim ) {
                ch->pecho("Упс, круговая слежка!");
                return;
            }
        }
    }

    victim->desc->snoop_by = ch->desc;
    wiznet( WIZ_SNOOPS, WIZ_SECURE, ch->get_trust(), "%C1 начинает следить за %C1.", ch, victim );
    ch->pecho("Начинаем слежку.");
}



CMDWIZP( switch )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    if(ch->is_npc( )) {
        ch->pecho("Мобы этого не умеют.");
        return;
    }
    
    one_argument( argument, arg );

    PCharacter *pch = ch->getPC( );

    if ( arg[0] == '\0' )
    {
        ch->pecho("Вселиться в чье тело?");
        return;
    }

    if ( ch->desc == 0 )
        return;

    if ( pch->switchedTo )
    {
        ch->pecho("Ты уже в чужом теле.");
        return;
    }
    
    victim = get_char_world( ch, arg );
    if ( !victim )
    {
        ch->pecho("Таких сейчас в мире нет.");
        return;
    }

    if ( victim == ch ) {
        ch->pecho("Ты в своем теле.");
        return;
    }

    if (!victim->is_npc()) {
        ch->pecho("Вселиться можно только в тело мобов.");
        return;
    }

    NPCharacter *mob = victim->getNPC( );

    if (ch->in_room != victim->in_room &&  victim->in_room->isPrivate( ) &&
            !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        ch->pecho("Этот персонаж сейчас в приватной комнате.");
        return;
    }

    if ( victim->desc != 0 ) {
        ch->pecho("В это тело уже кто-то вселился!");
        return;
    }

    wiznet( WIZ_SWITCHES, WIZ_SECURE, ch->get_trust(), "%C1 вселяется в тело %C4.", pch, mob );

    mob->switchedFrom = pch;
    pch->switchedTo = mob;
    
    ch->desc->associate( victim );
    ch->desc = 0;
    
    /* change communications to match */
    victim->prompt = ch->prompt;
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    victim->pecho("Ты вселяешься в чужое тело.");
    return;
}


static bool mprog_return( Character *ch )
{
    FENIA_CALL( ch, "Return", "" );
    FENIA_NDX_CALL( ch->getNPC( ), "Return", "C", ch );
    return false;
}

CMDWIZP( return )
{
    NPCharacter *mob = ch->getNPC();

    if(!mob) {
        ch->pecho("Эта команда нужна для смены тел -- а ты и так в своем теле.");
        ch->pecho("А чтобы вернуться в Храм, напиши {y{hcвозврат{x.");
        return;
    }
    
    if ( mob->desc == 0 )
        return;
    
    if ( !mob->switchedFrom ) {
        ch->pecho("Ты и так в своем теле.");
        return;
    }

    if (mprog_return( mob ))
        return;

    mob->pecho("Ты возвращаешься в своё привычное тело.");    

    wiznet( WIZ_SWITCHES, WIZ_SECURE, ch->get_trust( ), 
            "%C1 returns from %C2.", mob->switchedFrom, mob );
    
    PCharacter *pch = mob->switchedFrom;
    mob->desc->associate( mob->switchedFrom );
    mob->switchedFrom->switchedTo = 0;
    mob->switchedFrom = 0;
    mob->desc = 0;

    pch->getAttributes().handleEvent(AfkArguments(pch, false));
}


/* RT to replace the two load commands */

CMDWIZP( load )
{
   DLString args = argument;
   DLString arg = args.getOneArgument();

    if (arg.empty()) {
        ch->pecho("Формат:");
        ch->pecho("  load mob <vnum>");
        ch->pecho("  load obj <vnum> <level>");
        return;
    }

    if (arg_is_strict(arg, "mob") || arg_is_strict(arg, "char"))
    {
        do_mload(ch, args);
        return;
    }

    if (arg_is_strict(arg, "obj"))
    {
        do_oload(ch, args);
        return;
    }

    /* echo syntax */
    run(ch, DLString::emptyString);
}


/* NOTCOMMAND */ void do_mload( Character *ch, const DLString &cArgs )
{
        DLString args = cArgs;
        DLString arg = args.getOneArgument();
        MOB_INDEX_DATA *pMobIndex;
        Character *victim;


        if (arg.empty() || !arg.isNumber())
        {
                ch->pecho("Формат: load mob <vnum>.");
                return;
        }

        if ( ( pMobIndex = get_mob_index(arg.toInt()) ) == 0 )
        {
                ch->pecho("Моба с таким внумом не найдено.");
                return;
        }

        victim = create_mobile( pMobIndex );

        if (victim->in_room == 0)
            char_to_room( victim, ch->in_room );

        oldact("$c1 создает $C4!", ch, 0, victim, TO_ROOM);
        oldact("Ты создаешь $C4!", ch, 0, victim, TO_CHAR);


        wiznet( WIZ_LOAD, WIZ_SECURE, ch->get_trust(), 
                "%C1 создает %C4.", ch, victim );
        ch->pecho("Готово.");
        return;
}



/* NOTCOMMAND */ void do_oload( Character *ch, const DLString &cArgs )
{
    DLString args = cArgs;
    DLString arg1 = args.getOneArgument();
    DLString arg2 = args.getOneArgument();
    OBJ_INDEX_DATA *pObjIndex;
    Object *obj;
    short level;

    if (arg1.empty() || !arg1.isNumber())
    {
        ch->pecho("Формат: load obj <vnum> <level>.");
        return;
    }

    level = ch->get_trust(); /* default */

    if (!arg2.empty())  /* load with a level */
    {
        if (!arg2.isNumber())
        {
          ch->pecho("Формат: oload <vnum> <level>.");
          return;
        }
        level = arg2.toInt();
        if (level < 0 || level > ch->get_trust())
        {
          ch->pecho("Уровень не может быть ниже нуля или выше твоего.");
            return;
        }
    }

    if ( ( pObjIndex = get_obj_index( arg1.toInt() ) ) == 0 )
    {
        ch->pecho("Объект с таким внумом не найден.");
        return;
    }

    obj = create_object( pObjIndex, level );
    if ( obj->can_wear( ITEM_TAKE) )
        obj_to_char( obj, ch );
    else
        obj_to_room( obj, ch->in_room );
        
    oldact("$c1 создает $o4!", ch, obj, 0, TO_ROOM);
    oldact("Ты создаешь $o4!", ch, obj, 0, TO_CHAR);
    wiznet( WIZ_LOAD, WIZ_SECURE, ch->get_trust( ), "%C1 loads %O4.", ch, obj );
    
    LogStream::sendNotice( ) 
        << ch->getNameC( ) << " loads obj vnum " << obj->pIndexData->vnum
        << " id " << obj->getID( ) << endl;

    return;
}



CMDWIZP( purge )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;
    Object *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        /* 'purge' */
        Character *vnext;
        Object  *obj_next;
        
        dreamland->removeOption( DL_SAVE_MOBS );
        dreamland->removeOption( DL_SAVE_OBJS );

        for ( victim = ch->in_room->people; victim != 0; victim = vnext )
        {
            vnext = victim->next_in_room;
            if ( victim->is_npc() && !IS_SET(victim->act,ACT_NOPURGE)
            &&   victim != ch /* safety precaution */ )
                extract_char( victim );
        }

        for ( obj = ch->in_room->contents; obj != 0; obj = obj_next )
        {
            obj_next = obj->next_content;
            if (!IS_OBJ_STAT(obj,ITEM_NOPURGE))
              extract_obj( obj );
        }

        oldact("$c1 изничтожает все в комнате!", ch, 0, 0, TO_ROOM);
        ch->pecho("Готово.");
        dreamland->resetOption( DL_SAVE_MOBS );
        dreamland->resetOption( DL_SAVE_OBJS );
        save_items( ch->in_room );
        save_mobs( ch->in_room );
        return;
    }

    // Try to purge a mob in the room or an item in the room/inventory/equip.
    if ((victim = get_char_room(ch, arg))) {
        if (!victim->is_npc()) {
            ch->pecho("Сначала научись рисовать и отрасти усики.");
            return;
        }

        oldact("$c1 изничтожает $C4.", ch, 0, victim, TO_NOTVICT );
        oldact("Ты изничтожаешь $C4.", ch, 0, victim, TO_CHAR );
        extract_char( victim );
        return;
    }

    if ((obj = get_obj_here(ch, arg))) {
        oldact("$c1 изничтожает $o4.", ch, obj, 0, TO_ROOM );
        oldact("Ты изничтожаешь $o4.", ch, obj, 0, TO_CHAR );
        extract_obj(obj);
        return;
    }

    ch->pecho("Ты не видишь здесь моба или предмет с таким именем.");
}





CMDWIZP( restore )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;
    Character *vch;
    Descriptor *d;

    one_argument( argument, arg );
    if (arg[0] == '\0' || arg_is_strict(arg, "room"))
    {
    /* cure room */
            
        for (vch = ch->in_room->people; vch != 0; vch = vch->next_in_room)
        {
            affect_strip(vch,gsn_plague);
            affect_strip(vch,gsn_poison);
            affect_strip(vch,gsn_blindness);
            affect_strip(vch,gsn_sleep);
            affect_strip(vch,gsn_curse);

            vch->hit         = vch->max_hit;
            vch->mana        = vch->max_mana;
            vch->move        = vch->max_move;
            update_pos( vch);
            oldact_p("$c1 {Gвосстановил$gо||а {xтвои силы!",ch,0,vch,TO_VICT,POS_DEAD);
        }

        wiznet( WIZ_RESTORE, WIZ_SECURE, ch->get_trust(), 
                "%C1 restored room %d.", ch, ch->in_room->vnum );

        ch->pecho("Комната восстановлена.");
        return;

    }

    if ( ch->get_trust() >=  MAX_LEVEL - 5 && arg_is_all(arg))
    {
    /* cure all */
            
        for (d = descriptor_list; d != 0; d = d->next)
        {
            if (d->connected != CON_PLAYING)
                continue;

            victim = d->character;

            if (victim == 0 || victim->is_npc())
                continue;

            affect_strip(victim,gsn_plague);
            affect_strip(victim,gsn_poison);
            affect_strip(victim,gsn_blindness);
            affect_strip(victim,gsn_sleep);
            affect_strip(victim,gsn_curse);

            victim->hit         = victim->max_hit;
            victim->mana        = victim->max_mana;
            victim->move        = victim->max_move;
            update_pos( victim);
            if (victim->in_room != 0)
                oldact_p("$c1 {Gвосстановил$gо||а {xтвои силы!",ch,0,victim,TO_VICT,POS_DEAD);
        }
        ch->pecho("Все активные игроки восстановлены!");
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == 0 )
    {
        ch->pecho("Таких здесь нет.");
        return;
    }

    affect_strip(victim,gsn_plague);
    affect_strip(victim,gsn_poison);
    affect_strip(victim,gsn_blindness);
    affect_strip(victim,gsn_sleep);
    affect_strip(victim,gsn_curse);
    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos( victim );

    oldact_p("$c1 {Gвосстановил$gо||а {xтвои силы!", ch, 0, victim, TO_VICT,POS_DEAD );
    wiznet( WIZ_RESTORE, WIZ_SECURE, ch->get_trust( ), "%C1 restored %C4.", ch, victim );
    ch->pecho("Готово.");
}

         
CMDWIZP( freeze )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->pecho("Заморозить кого?");
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == 0 )
    {
        ch->pecho("Тут таких нет.");
        return;
    }

    if ( victim->is_npc() )
    {
        ch->pecho("Мобов замораживать бесполезно.");
        return;
    }

    if ( victim->get_trust() >= ch->get_trust() )
    {
        ch->pecho("У тебя пока не хватает прав для этого.");
        return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
        victim->act.removeBit( PLR_FREEZE);
        victim->pecho("{CТебя разморозили!{x Ты чувствуешь как способность двигаться возвращается.");
        ch->pecho("Цель разморожена.");
        wiznet( WIZ_PENALTIES, WIZ_SECURE, 0, "%C1 thaws %C4.", ch, victim );
    }
    else
    {
        victim->act.setBit( PLR_FREEZE);
        victim->pecho("{CТЕБЯ ЗАМОРОЗИЛИ!{x Ты теряешь способность двигаться и говорить.");
        ch->pecho("Цель заморожена.");
        wiznet( WIZ_PENALTIES, WIZ_SECURE, 0, 
                "%C1 puts %C4 in the deep freeze.", ch, victim );
    }

    if( !victim->is_npc( ) ) 
        victim->getPC( )->save();
}


CMDWIZP( peace )
{
    Character *rch;

    for ( rch = ch->in_room->people; rch != 0; rch = rch->next_in_room )
    {
        if ( rch->fighting != 0 )
            stop_fighting( rch, true );
        if (rch->is_npc() && IS_SET(rch->act,ACT_AGGRESSIVE))
            rch->act.removeBit(ACT_AGGRESSIVE);
    }

    ch->pecho("Ты наполняешь комнату миром и спокойствием.");
    ch->recho("%^C1 жестом прекращает войны и агрессию вокруг.", ch);
    return;
}

CMDWIZP( wizlock )
{
    bitstring_t opt = DL_WIZLOCK;

    if (dreamland->hasOption( opt ))
    {
        dreamland->removeOption( opt );
        wiznet( 0, 0, 0, "%C1 removes wizlock.", ch );
        ch->pecho("Game un-wizlocked.");
    }
    else
    {
        dreamland->setOption( opt );
        wiznet( 0, 0, 0, "%C1 has wizlocked the game.", ch );
        ch->pecho("Game wizlocked.");
    }
}

/* RT anti-newbie code */

CMDWIZP( newlock )
{
    bitstring_t opt = DL_NEWLOCK;

    if (dreamland->hasOption( opt ))
    {
        dreamland->removeOption( opt );
        wiznet( 0, 0, 0, "%C1 allows new characters back in.", ch );
        ch->pecho("Newlock removed.");
    }
    else
    {
        dreamland->setOption( opt );
        wiznet( 0, 0, 0, "%C1 locks out new characters.", ch );
        ch->pecho("New characters have been locked out.");
    }
}


/*
 * Syntax:
 * force all|players|gods|<name> <command with args>
 */
CMDWIZP( force )
{
    const char *msg = "%s вежливо принуждает тебя выполнить команду '%s'.";
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        ch->pecho("Принудить кого к чему?");
        return;
    }

    if (arg_is_all(arg))
    {
        Character *vch;
        Character *vch_next;

        if (ch->get_trust() < MAX_LEVEL - 5)
        {
            ch->pecho("У тебя пока не хватает прав на принуждение.");
            return;
        }

        for ( vch = char_list; vch != 0; vch = vch_next )
        {
            vch_next = vch->next;

            if ( !vch->is_npc() && vch->get_trust() < ch->get_trust() )
            {
                vch->pecho(msg, vch->sees(ch, '1').c_str(), argument);
                interpret( vch, argument );
            }
        }
    }
    else
    {
        Character *victim;

        if ( ( victim = get_char_world( ch, arg ) ) == 0 )
        {
            ch->pecho("Тут таких нет.");
            return;
        }

        if ( victim == ch )
        {
            ch->pecho("Сила воли, бессердечная ты сука.");
            return;
        }

            if (ch->in_room != victim->in_room
        &&  victim->in_room->isPrivate( ) && !IS_TRUSTED(ch,IMPLEMENTOR))
            {
            ch->pecho("Этот персонаж сейчас в приватной комнате.");
            return;
        }

        if ( victim->get_trust() >= ch->get_trust() )
        {
            ch->pecho("Эту цель ты пока не можешь принудить!");
            return;
        }

        if ( !victim->is_npc() && ch->get_trust() < MAX_LEVEL - 4)
        {
            ch->pecho("У тебя пока не хватает прав на такое принуждение.");
            return;
        }

        victim->pecho(msg, victim->sees(ch, '1').c_str(), argument);
        interpret( victim, argument );
    }

    ch->pecho("Готово.");
    return;
}



/*
 * New routines by Dionysos.
 */
CMDWIZP( wizinvis )
{
    short level;
    char arg[MAX_STRING_LENGTH];

    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    /* take the default path */

      if ( ch->invis_level)
      {
          ch->invis_level = 0;
          oldact("$c1 внезапно проявляется в реальности.", ch, 0, 0, TO_ROOM);
          ch->pecho("Ты снова проявляешься в реальности.");
      }
      else
      {
          ch->invis_level = 102;
          oldact("$c1 подмигивает и растворяется за подкладкой реальности.", ch, 0, 0, TO_ROOM);
          ch->pecho("Ты растворяешься за подкладкой реальности.");
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > ch->get_trust())
      {
        ch->pecho("Invis level must be between 2 and your level.");
        return;
      }
      else
      {
          ch->reply = 0;
          ch->invis_level = level;
          oldact("$c1 подмигивает и растворяется за подкладкой реальности.", ch, 0, 0, TO_ROOM);
          ch->pecho("Ты растворяешься за подкладкой реальности.");
      }
    }

    return;
}


CMDWIZP( incognito )
{
    short level;
    char arg[MAX_STRING_LENGTH];

    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    /* take the default path */

      if ( ch->incog_level)
      {
          ch->incog_level = 0;
          oldact("$c1 больше не маскируется.", ch, 0, 0, TO_ROOM);
          ch->pecho("Ты больше не маскируешься.");
      }
      else
      {
          ch->incog_level = 102;
          oldact("$c1 скрывает $s присутствие.", ch, 0, 0, TO_ROOM);
          ch->pecho("Ты скрываешь свое присутствие.");
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > ch->get_trust())
      {
        ch->pecho("Incog level must be between 2 and your level.");
        return;
      }
      else
      {
          ch->reply = 0;
          ch->incog_level = level;
          oldact("$c1 скрывает $s присутствие.", ch, 0, 0, TO_ROOM);
          ch->pecho("Ты скрываешь свое присутствие.");
      }
    }

    return;
}




CMDWIZP( advance )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    Character *vict;
    PCharacter *victim;
    short level;
    int iLevel;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
        ch->pecho("Формат: advance <char> <level>.");
        return;
    }

    if ( ( vict = get_char_room( ch, arg1 ) ) == 0 )
    {
        ch->pecho("Этого игрока тут нет.");
        return;
    }

    if ( vict->is_npc() )
    {
        ch->pecho("Not on NPC's.");
        return;
    }

    victim = vict->getPC();
    
    if ( ( level = atoi( arg2 ) ) < 1 || level > 110 )
    {
        ch->pecho("Level must be 1 to 110.");
        return;
    }

    if ( level > ch->get_trust() )
    {
        ch->pecho("Limited to your trust level.");
        return;
    }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if ( level <= victim->getRealLevel( ) )
    {
        int temp_prac, temp_train;

        ch->pecho("Lowering a player's level!");
        victim->pecho("**** ОООООО НЕЕЕЕЕЕЕЕЕТ!!! ****");
        temp_prac = victim->practice;
        temp_train = victim->train;
        victim->setLevel( 1 );
        victim->exp      = victim->getExpToLevel( );
        victim->max_hit  = 10;
        victim->max_mana = 100;
        victim->max_move = 100;
        victim->practice = 0;
        victim->train    = 0;
        victim->hit      = victim->max_hit;
        victim->mana     = victim->max_mana;
        victim->move     = victim->max_move;
        Player::advanceLevel(victim->getPC( ));
        victim->practice = temp_prac;
        victim->train    = temp_train;
    }
    else
    {
        ch->pecho("Raising a player's level!");
        victim->pecho("**** ООООО ДАААААААААА ****");
    }

    for ( iLevel = victim->getRealLevel( ) ; iLevel < level; iLevel++ )
    {
        if( victim->get_trust() != 0xFFFF )
            victim->send_to("Ты повышаешь уровень!  ");
        victim->exp += victim->getExpToLevel( );;
        victim->setLevel( victim->getRealLevel( ) + 1 );
        Player::advanceLevel(victim->getPC( ));
    }
    victim->getPC( )->setTrust( 0 );
    if( !victim->is_npc( ) ) victim->getPC( )->save();
    return;
}



CMDWIZP( rename )
{
    DLString arguments = argument;
    DLString oldName = arguments.getOneArgument( );
    DLString newName = arguments.getOneArgument( );
    DLString& russianName = arguments;

    newName.capitalize( );
    russianName.capitalize( );
    
    if (oldName.empty( )) {
        ch->pecho("Rename who?");
        return;
    }
    
    PCharacter* victim = get_player_world( ch, oldName.c_str( ) );
    
    if (!victim) {
        ch->pecho("There is no such a person online.");
        return;
    }
    
    if( (victim != ch ) && ( victim->get_trust( ) >= ch->get_trust( ) ) ) {
        ch->pecho("You failed.");
        return;
    }
    
    if( newName.empty( ) ) {
        ch->pecho("Rename to what new name?");
        return;
    }

    if (oldName ^ newName) {
        if (!russianName.empty( )) {
            victim->setRussianName( russianName );
            ch->pecho( "Russian name set." );
        }
        else {
            ch->pecho( "Both names are equal!" );
        }

        return;
    }
    
    DLString rc;
    if (!(rc = badNames->checkName(newName)).empty()) {
        ch->pecho( "New name failed sanity checks, reason: %s.", rc.c_str() );
        return;
    }
    
    if (PCharacterManager::find( newName )) {
        ch->pecho("A player with that name already exists!");
        return;                
    }

    // Переименовываем объекты
    Object *obj;
    Object *obj_next;
    for ( obj = object_list; obj != 0; obj = obj_next )
    {
            obj_next = obj->next;
            if (obj->hasOwner( victim ))
            {
                    obj->setOwner( newName );
                    save_items_at_holder( obj );
            }
    }

    PCharacterManager::rename( victim->getName( ), newName );

    victim->setName( newName );
    victim->setRussianName( russianName );

    victim->save( );

    DLFile( dreamland->getPlayerDir( ), 
            oldName.toLower( ), 
            PCharacterManager::ext ).remove( );

    ch->pecho("Character renamed.");
    oldact_p("$c1 переименова$gло|л|ла тебя в $C4!",ch,0,victim,TO_VICT,POS_DEAD);
}

CMDWIZP( noaffect )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    argument = one_argument(argument,arg);

    if ( (victim = get_char_world(ch ,arg)) == 0 )
    {
        ch->pecho("He is not currently playing.");
        return;
    }

    AffectList affects = victim->affected.clone();
    for (auto paf_iter = affects.cbegin(); paf_iter != affects.cend(); paf_iter++) {
        Affect *paf = *paf_iter;
        if ( paf->duration >= 0 )
        {
            if (!affects.hasNext(paf_iter) && paf->type->getAffect( ))
                paf->type->getAffect()->onRemove(SpellTarget::Pointer(NEW, victim), paf);

            affect_remove( victim, paf );
        }
    }
}


CMDWIZP( olevel )
{
    char level[MAX_INPUT_LENGTH];
    char name[MAX_INPUT_LENGTH];
    ostringstream buffer;
    Object *obj;
    Object *in_obj;
    bool found;
    int number = 0, max_found;

    found = false;
    number = 0;
    max_found = 200;

    argument = one_argument(argument, level);
    if (level[0] == '\0')
    {
        ch->pecho("Syntax: olevel <level>");
        ch->pecho("        olevel <level> <name>");
        return;
    }

    argument = one_argument(argument, name);
    for ( obj = object_list; obj != 0; obj = obj->next )
    {
        if ( obj->level != atoi(level) )
            continue;

        if ( name[0] != '\0' && !is_name(name, String::toString(obj->getKeyword()).c_str() ) )
            continue;

        found = true;
        number++;

        for ( in_obj = obj; in_obj->in_obj != 0; in_obj = in_obj->in_obj );

        if ( in_obj->carried_by != 0 && ch->can_see(in_obj->carried_by)
        &&   in_obj->carried_by->in_room != 0)
            buffer << fmt(0, "%3d) [%d] %N1 is carried by %s [Room %d]\n\r",
                number, obj->pIndexData->vnum, obj->getShortDescr(LANG_DEFAULT).c_str( ),ch->sees(in_obj->carried_by, '5').c_str(),
                in_obj->carried_by->in_room->vnum );
        else if (in_obj->in_room != 0 && ch->can_see(in_obj->in_room))
            buffer << fmt(0, "%3d) [%d] %N1 is in %s [Room %d]\n\r",
                number, obj->pIndexData->vnum,obj->getShortDescr(LANG_DEFAULT ).c_str( ),in_obj->in_room->getName(),
                in_obj->in_room->vnum);
        else
            buffer << fmt(0, "%3d) [%d] %N1 is somewhere\n\r",number, obj->pIndexData->vnum,obj->getShortDescr(LANG_DEFAULT).c_str( ));

        if (number >= max_found)
            break;
    }

    if ( !found )
        ch->pecho("Nothing like that in heaven or earth.");
    else
        page_to_char(buffer.str( ).c_str( ), ch);
}

CMDWIZP( mlevel )
{
    ostringstream buffer;
    Character *victim;
    bool found;
    int count = 0;

    if ( argument[0] == '\0' )
    {
        ch->pecho("Syntax: mlevel <level>");
        return;
    }

    found = false;

    for ( victim = char_list; victim != 0; victim = victim->next )
    {
        if ( victim->in_room != 0
        &&   atoi(argument) == victim->getRealLevel( ) )
        {
            found = true;
            count++;
            buffer << fmt(0, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
                victim->is_npc() ? victim->getNPC()->pIndexData->vnum : 0,
                victim->getNameP( '1' ).c_str(),
                victim->in_room->vnum,
                victim->in_room->getName() );
        }
    }

    if ( !found )
        oldact_p("You didn't find any mob of level $T.", ch, 0, argument, TO_CHAR,POS_DEAD );
    else
        page_to_char(buffer.str( ).c_str( ), ch);
}


