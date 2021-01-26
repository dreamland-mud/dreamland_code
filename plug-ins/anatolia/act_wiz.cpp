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
#include "so.h"
#include "plugin.h"
#include "char.h"

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
#include "mercdb.h"
#include "interp.h"
#include "clan.h"
#include "liquid.h"

#include "gsn_plugin.h"
#include "npcharacter.h"
#include "core/object.h"
#include "../anatolia/handler.h"
#include "fread_utils.h"
#include "stats_apply.h"
#include "act_wiz.h"
#include "act_move.h"
#include "act.h"
#include "def.h"

using std::min;

/* dedicated loader for wizard commands */
CMDLOADER_DECL(wizard)


/* command procedures needed */
void do_rstat                ( Character *, char * );
void do_mstat                ( Character *, Character * );
void do_ostat                ( Character *, char * );
void do_mfind                ( Character *, char * );
void do_ofind                ( Character *, char * );
void do_tfind                ( Character *, char * );
void do_mload                ( Character *, char * );
void do_oload                ( Character *, char * );

RELIG(none);
GSN(none);

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
                char buf[1000];
                int ingameCount;
                OBJ_INDEX_DATA *obj_index = get_obj_index( atoi(argument) );
                if ( obj_index == 0 )
                {
                        ch->send_to("Формат: limited <vnum>.\n\r");
                        return;
                }
                if ( obj_index->limit == -1 )
                {
                        ch->send_to("Это не лимит.\n\r");
                        return;
                }
                nMatch = 0;
                sprintf( buf, "%-35s [%5d]  Лимит: %3d  Собрано: %3d\n\r",
                        obj_index->short_descr,
                        obj_index->vnum,
                        obj_index->limit,
                        obj_index->count );
                buf[0] = Char::upper( buf[0] );
                ch->send_to(buf);
                ingameCount = obj_index->instances.size();
                for (Object* obj=object_list; obj != 0; obj=obj->next )
                        if ( obj->pIndexData->vnum == obj_index->vnum )
                        {
                                if ( obj->carried_by != 0 && ch->can_see( obj->carried_by ) )
                                        sprintf(buf, "У %-30s\n\r",
                                                obj->carried_by->getNameP( ));
                                if ( obj->in_room != 0 )
                                        sprintf(buf, "В комнате %-20s [%d]\n\r",
                                                obj->in_room->getName(), obj->in_room->vnum);
                                if ( obj->in_obj != 0 )
                                        sprintf(buf, "Внутри %-20s [%d] \n\r",
                                                obj->in_obj->getShortDescr( '1' ).c_str( ),
                                                obj->in_obj->pIndexData->vnum);
                                ch->send_to(buf);
                        }
                sprintf(buf, "  %d сейчас в игре, а еще %d в профилях игроков.\n\r",
                        ingameCount, obj_index->count-ingameCount);
                ch->send_to(buf);
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
                    info.description = obj_index->short_descr;
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
        
        ch->printf( "\n\rВсего лимитов: %d из %d.\n\r", limits.size(), nMatch );
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
                        ch->send_to("Визнет отключен. Уф.\n\r");
                        REMOVE_BIT(ch->getPC( )->wiznet,WIZ_ON);
                }
                else
                {
                        ch->send_to("Визнет включен, добро пожаловать в мир спама!\n\r");
                        SET_BIT(ch->getPC( )->wiznet,WIZ_ON);
                }

                return;
        }

        if (!str_prefix(argument,"on"))
        {
                ch->send_to("Визнет включен, добро пожаловать в мир спама!\n\r");
                SET_BIT(ch->getPC( )->wiznet,WIZ_ON);
                return;
        }

        if (!str_prefix(argument,"off"))
        {
                ch->send_to("Визнет отключен. Уф.\n\r");
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

                ch->send_to("Статус Визнета:\n\r");
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

                ch->send_to("Доступные тебе опции Визнета:\n\r");
                ch->send_to(buf);

                return;
        }

        flag = wiznet_lookup(argument);

        if ( flag == -1 || ch->get_trust() < wiznet_table[flag].level )
        {
                ch->send_to("Такой опции Визнета нет, или тебе она пока недоступна.\n\r");
                return;
        }

        if ( IS_SET(ch->getPC( )->wiznet,wiznet_table[flag].flag) )
        {
                ch->printf("Ты больше не следишь за %s через Визнет.\n\r",
                           wiznet_table[flag].name);
                REMOVE_BIT(ch->getPC( )->wiznet,wiznet_table[flag].flag);
                return;
        }
        else
        {
                ch->printf("Ты теперь отслеживаешь %s через Визнет.\n\r",
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
            ch->printf("Твое сообщение poofin: %s\n\r",ch->getPC( )->bamfin.c_str( ));
            return;
    }

    if ( strstr(argument,ch->getNameP( )) == 0 
            && strstr(argument,ch->getNameP( '1' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '2' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '3' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '4' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '5' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '6' ).c_str()) == 0 )
    {
            ch->send_to("Неплохо бы включить в poofin свое имя.\n\r");
            return;
    }

    ch->getPC( )->bamfin = argument;
    ch->printf("Твое сообщение poofin теперь: %s\n\r",ch->getPC( )->bamfin.c_str( ));
}



CMDWIZP( poofout )
{
    if (ch->is_npc())
        return;

    if (argument[0] == '\0')
    {
            ch->printf("Твое сообщение poofout: %s\n\r",ch->getPC( )->bamfout.c_str( ));
            return;
    }

    if ( strstr(argument,ch->getNameP( )) == 0 
            && strstr(argument,ch->getNameP( '1' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '2' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '3' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '4' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '5' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '6' ).c_str()) == 0 )
    {
            ch->send_to("Неплохо бы включить в poofout свое имя.\n\r");
            return;
    }

    ch->getPC( )->bamfout = argument;

    ch->printf("Твое сообщение poofout теперь %s\n\r",ch->getPC( )->bamfout.c_str( ));
}

CMDWIZP( disconnect )
{
    char arg[MAX_INPUT_LENGTH];
    Descriptor *d;
    Character *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        ch->send_to("Отключить кого?\n\r");
        return;
    }

    if (is_number(arg))
    {
        int desc;

        desc = atoi(arg);
            for ( d = descriptor_list; d != 0; d = d->next )
            {
            if ( d->descriptor == desc )
            {
                    d->close( );
                    ch->send_to("Игрок отключен. Бай-бай!\n\r");
                    return;
            }
        }
    }

    if ( ( victim = get_char_world( ch, arg ) ) == 0 )
    {
        ch->send_to("Таких сейчас в мире нет.\n\r");
        return;
    }

    if ( victim->desc == 0 )
    {
        act_p( "У $C2 нет дескриптора.", ch, 0, victim, TO_CHAR,POS_DEAD );
        return;
    }

    for ( d = descriptor_list; d != 0; d = d->next )
    {
        if ( d == victim->desc )
        {
            d->close( );
            ch->send_to("Готово.\n\r");
            return;
        }
    }

    bug( "Do_disconnect: desc not found.", 0 );
    ch->send_to("Дескриптор не найден!\n\r");
    return;
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
        ch->send_to("Перенести кого и куда?\n\r");
        return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
        for ( d = descriptor_list; d != 0; d = d->next )
        {
            if ( d->connected == CON_PLAYING
            &&   d->character
            &&   d->character != ch
            &&   ch->can_see( d->character ) )
            {
                char buf[MAX_STRING_LENGTH];
                sprintf( buf, "%s %s", d->character->getNameP( ), arg2 );
                run( ch, buf );
            }
        }
        return;
    }


    if ( ( victim = get_char_world( ch, arg1 ) ) == 0 )
    {
        ch->send_to("Таких сейчас в мире нет.\n\r");
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
            ch->send_to("Цель не найдена.\n\r");
            return;
        }
        

/*        if ( !location->isOwner(ch) && location ->isPrivate( ) */
        if ( location ->isPrivate( )
        &&  ch->get_trust() < MAX_LEVEL)
        {
            ch->send_to("Комната приватная и сейчас занята.\n\r");
            return;
        }
    }
    
    if (victim->desc && victim->desc->connected != CON_PLAYING) {
        ch->println("Плохая идея.");
        return;
    }

    transfer_char( victim, ch, location,
                  "%1$^C1 внезапно исчезает в столбе {Cбожественной энергии!{x",
                  (ch != victim ? "%2$^C1 переносит тебя в столбе {Cбожественной энергии!{x" : NULL),
                  "%1$^C1 внезапно прибывает в столбе {Cбожественной энергии!{x" );

    ch->send_to("Ok.\n\r");
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
        ch->send_to("Рядом с кем? Сделать что?\n\r");
        return;
    }

    if ( ( location = find_location( ch, arg ) ) == 0 )
    {
        ch->send_to("Цель не найдена.\n\r");
        return;
    }

    if ( location ->isPrivate( ) &&  ch->get_trust() < MAX_LEVEL)
    {
        ch->send_to("Комната приватная и сейчас занята.\n\r");
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
        ch->send_to("Переместиться куда?\n\r");
        return;
    }

    if ( ( location = find_location( ch, argument ) ) == 0 )
    {
        ch->printf("Цель не найдена: %s.\n\r", argument );
        return;
    }

    pch = ch->getPC( );

    if (!ch->is_npc( )) { // switched imms are silent
        for (rch = ch->in_room->people; rch != 0; rch = rch->next_in_room)
            if (rch->get_trust() >= ch->invis_level) {
                if (!pch->bamfout.empty( ))
                    act( "$t", ch, pch->bamfout.c_str( ), rch, TO_VICT );
                else
                    act( "$c1 исчезает в столбе {Cбожественной энергии.{x", ch, 0, rch, TO_VICT );
            }
    }
    
    transfer_char( ch, ch, location );
    
    if (!ch->is_npc( )) {
        for (rch = ch->in_room->people; rch != 0; rch = rch->next_in_room)
            if (rch->get_trust() >= ch->invis_level) {
                if (!pch->bamfin.empty( ))
                    act( "$t", ch, pch->bamfin.c_str( ), rch, TO_VICT );
                else
                    act( "$c1 внезапно появляется в столбе {Cбожественной энергии.{x", ch, 0, rch, TO_VICT );
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
        ch->send_to("Формат:\n\r");
        ch->send_to("  stat <name>\n\r");
        ch->send_to("  stat obj <name>\n\r");
        ch->send_to("  stat mob <name>\n\r");
        ch->send_to("  stat room <number>\n\r");
        return;
   }

   if (!str_cmp(arg,"room"))
   {
        do_rstat(ch,string);
        return;
   }

   if (!str_cmp(arg,"obj"))
   {
        do_ostat(ch,string);
        return;
   }
    
    fChar = !str_cmp(arg,"char");
    fMob = !str_cmp(arg,"mob");

    if (fChar || fMob) {
        if (!string[0]) {
           ch->println("Stat на кого?");
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

  ch->send_to("С таким именем ничего не найдено.\n\r");
}


/* NOTCOMMAND */ void do_rstat( Character *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    Room *location;
    Object *obj;
    Character *rch;
    int door;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == 0 )
    {
        ch->send_to("Цель не найдена.\n\r");
        return;
    }

/*    if (!location->isOwner(ch) && ch->in_room != location  */
    if ( ch->in_room != location
    &&  location ->isPrivate( ) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        ch->send_to("Комната приватная и сейчас занята.\n\r");
        return;
    }

    if (ch->in_room->affected_by)
    {
        sprintf(buf, "Под воздействием: %s\n\r",
            raffect_flags.messages(ch->in_room->affected_by).c_str( ));
        ch->send_to(buf);
    }

    sprintf( buf, "Имя: '%s'\n\rЗона: '%s'\n\rВладелец: '%s' Клан: '%s'\n\r",
        location->getName(),
        location->areaName() ,
        location->owner,
        location->pIndexData->clan->getShortName( ).c_str( ) );
    ch->send_to(buf);

    sprintf( buf,
        "Vnum: %d  Сектор: %d  Свет: %d  Лечение: %d  Мана: %d\n\r",
        location->vnum,
        location->getSectorType(),
        location->light,
        location->getHealRate(),
        location->getManaRate() );
    ch->send_to(buf);

    sprintf( buf,
        "Флаги: %s.\n\rОписание:\n\r%s",
        room_flags.names(location->room_flags).c_str( ),
        location->getDescription() );
    ch->send_to(buf);

    if ( location->getExtraDescr() != 0 )
    {
        EXTRA_DESCR_DATA *ed;

        ch->send_to("Ключевые слова экстра-описания: '");
        for ( ed = location->getExtraDescr(); ed; ed = ed->next )
        {
            ch->send_to(ed->keyword);
            if ( ed->next != 0 )
                ch->send_to(" ");
        }
        ch->send_to("'.\n\r");
    }

    ch->send_to("Персонажи:");
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
        if (ch->can_see(rch))
        {
            ch->printf( " %s", rch->getNameP( '1' ).c_str() );
        }
    }

    ch->send_to(".\n\rОбъекты:   ");
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
        ch->printf( " %s", obj->getFirstName( ).c_str( ) );
    }
    ch->send_to(".\n\r");

    for ( door = 0; door <= 5; door++ )
    {
        EXIT_DATA *pexit;

        if ( ( pexit = location->exit[door] ) != 0 )
        {
            sprintf( buf,
                "Дверь: %d.  В: %d.  Ключ: %d. Уровень: %d.  Флаги выхода: %s.\n\rКейворд: '%s'.  Шорт: '%s'. Описание: %s",

                door,
                ( pexit->u1.to_room == 0 ? -1 : pexit->u1.to_room->vnum),
                    pexit->key,
                pexit->level,
                    exit_flags.names(pexit->exit_info).c_str(),
                    pexit->keyword,
                direction_doorname(pexit),
                    pexit->description[0] != '\0'
                    ? pexit->description : "(none).\n\r" );
            ch->send_to(buf);
        }
    }
    ch->send_to("Следы:\n\r");

    for (RoomHistory::iterator h = location->history.begin( );
         h != location->history.end( );
         h++)
    {
        ch->printf( "%s проходит через дверь %d.\r\n", h->name.c_str( ), h->went );
    }

    if (location->behavior) {
        ostringstream ostr;
        
        sprintf(buf, "Поведение: [%s]\r\n", location->behavior->getType( ).c_str( ));
        ch->send_to(buf);

        location->behavior.toStream( ostr );
        ch->send_to( ostr );
    }

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
            buf << "добавляет запах " << paf->global.toRussianString('2', ',').colourStrip();
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
        else if (table == &raffect_flags)
            buf << "флаги комнаты ";
        else if (table == &weapon_type2)
            buf << "флаги оружия ";
        else if (table == &extra_flags)
            buf << "флаги предмета ";
        else if (table == &plr_flags)
            buf << "флаги персонажа ";
        else if (table == &form_flags)
            buf << "форму тела ";
        else
            buf << "не пойми что";

        buf << table->messages(b);

        format_affect_duration(paf, buf);
        format_affect_level(paf, buf);
        buf << "." << endl;
        empty = false;
    }

    if (paf->modifier != 0 && paf->location != APPLY_NONE) {
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

/* NOTCOMMAND */ void do_ostat( Character *ch, char *argument )
{
        char buf[MAX_STRING_LENGTH];
        char arg[MAX_INPUT_LENGTH];
        Object *obj;
        Liquid *liquid;

        one_argument( argument, arg );

        if ( arg[0] == '\0' )
        {
                ch->send_to("Stat чему?\n\r");
                return;
        }

        if ( ( obj = get_obj_world( ch, argument ) ) == 0 )
        {
                ch->send_to("Ни на земле, ни в небесах не найдено. Увы и ах.\n\r");
                return;
        }

        sprintf( buf, "Name(s): %s\n\r", obj->getName( ) );
        ch->send_to(buf);

        sprintf( buf, "Vnum: %d  Лимит: %d  Тип: %s  Ресеты: %d\n\r",
                obj->pIndexData->vnum, obj->pIndexData->limit,
                item_table.message(obj->item_type).c_str( ),
                obj->pIndexData->reset_num );
        ch->send_to(buf);

        if (obj->timestamp > 0) {
            DLString d = Date( obj->timestamp ).getTimeAsString( );
            ch->printf("Лимит исчезнет в %s.\r\n", d.c_str( ) );
        }

        sprintf( buf, "Шорт: %s\n\rДлинное описание: %s\n\r",
                obj->getShortDescr( ), obj->getDescription( ) );
        ch->send_to(buf);

        sprintf(buf,"Владелец: %s\n\r", obj->getOwner( ) == 0 ? "nobody" : obj->getOwner( ));
        ch->send_to(buf);

        sprintf( buf, "Материал: %s\n\r", obj->getMaterial( ));
        ch->send_to(buf);

        sprintf( buf, "Берется в: %s\n\rЭкстра флаги: %s\n\r",
                wear_flags.messages(obj->wear_flags, true).c_str( ), 
                extra_flags.messages(obj->extra_flags, true).c_str( ) );
        ch->send_to(buf);

        sprintf( buf, "Число: %d/%d  Вес: %d/%d/%d (десятые доли фунта)\n\r",1,
                obj->getNumber( ), obj->weight, obj->getWeight( ), obj->getTrueWeight( ) );
        ch->send_to(buf);

        sprintf( buf, "Уровень: %d  Цена: %d  Состояние: %d  Таймер: %d По счету: %d\n\r",
                obj->level, obj->cost, obj->condition, obj->timer, obj->pIndexData->count );
        ch->send_to(buf);

        sprintf( buf,        "В комнате: %d  Внутри: %s  В руках у: %s  Надето на: %s\n\r",
                obj->in_room == 0 ? 0 : obj->in_room->vnum,
                obj->in_obj  == 0 ? "(none)" : obj->in_obj->getShortDescr( '1' ).c_str( ),
                obj->carried_by == 0 ? "(none)" :
                        ch->can_see(obj->carried_by) ? obj->carried_by->getNameP( ) : "someone",
                obj->wear_loc->getName( ).c_str( ) );
        ch->send_to(buf);

        sprintf( buf, "Значения: %d %d %d %d %d\n\r",
                obj->value0(), obj->value1(), obj->value2(), obj->value3(),        obj->value4() );
        ch->send_to(buf);

        // now give out vital statistics as per identify

        switch ( obj->item_type )
        {
        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
                sprintf( buf, "Заклинания %d уровня:", obj->value0() );
                ch->send_to(buf);

                if ( obj->value1() >= 0 && obj->value1() < SkillManager::getThis( )->size() )
                {
                        ch->send_to(" '");
                        ch->send_to(SkillManager::getThis( )->find(obj->value1())->getName().c_str());
                        ch->send_to("'");
                }

                if ( obj->value2() >= 0 && obj->value2() < SkillManager::getThis( )->size() )
                {
                        ch->send_to(" '");
                        ch->send_to(SkillManager::getThis( )->find(obj->value2())->getName().c_str());
                        ch->send_to("'");
                }

                if ( obj->value3() >= 0 && obj->value3() < SkillManager::getThis( )->size() )
                {
                        ch->send_to(" '");
                        ch->send_to(SkillManager::getThis( )->find(obj->value3())->getName().c_str());
                        ch->send_to("'");
                }

                if (obj->value4() >= 0 && obj->value4() < SkillManager::getThis( )->size())
                {
                        ch->send_to(" '");
                        ch->send_to(SkillManager::getThis( )->find(obj->value4())->getName().c_str());
                        ch->send_to("'");
                }

                ch->send_to(".\n\r");
                break;

        case ITEM_WAND:
        case ITEM_STAFF:
                sprintf( buf, "Имеет %d(%d) зарядов уровня %d",
                        obj->value1(), obj->value2(), obj->value0() );
                ch->send_to(buf);

                if ( obj->value3() >= 0 && obj->value3() < SkillManager::getThis( )->size() )
                {
                        ch->send_to(" '");
                        ch->send_to(SkillManager::getThis( )->find(obj->value3())->getName().c_str());
                        ch->send_to("'");
                }

                ch->send_to(".\n\r");
                break;

        case ITEM_DRINK_CON:
                liquid = liquidManager->find( obj->value2() );
                sprintf(buf,"Содержит жидкость %s цвета, %s.\n",
                    liquid->getColor( ).ruscase( '2' ).c_str( ),
                    liquid->getShortDescr( ).ruscase( '4' ).c_str( ) );
                ch->send_to(buf);
                break;
                
        case ITEM_WEAPON:
                ch->send_to("Тип оружия: ");
                ch->send_to(weapon_class.name(obj->value0()).c_str( ));
                ch->send_to("\n");
                
                sprintf(buf,"Урон %dd%d (среднее %d)\n\r",
                        obj->value1(),obj->value2(),weapon_ave(obj));
                ch->send_to(buf);

                sprintf(buf,"Тип удара: %s.\n\r", weapon_flags.name(obj->value3()).c_str( ));
                ch->send_to(buf);
        
                if (obj->value4())  /* weapon flags */
                {
                        sprintf(buf,"Флаги оружия: %s\n\r",weapon_type2.messages(obj->value4()).c_str( ));
                        ch->send_to(buf);
                }
                break;

        case ITEM_ARMOR:
                sprintf( buf,"Класс брони: %d колющее, %d удар, %d режущее, %d экзотика\n\r",
                        obj->value0(), obj->value1(), obj->value2(), obj->value3() );
                ch->send_to(buf);
                break;

        case ITEM_CONTAINER:
                sprintf(buf,"Вместимость: %d#  Максимальный вес: %d#  Флаги: %s\n\r",
                        obj->value0(), obj->value3(), container_flags.messages(obj->value1()).c_str( ));
                ch->send_to(buf);
                if (obj->value4() != 100)
                {
                        sprintf(buf,"Уменьшение веса: %d%%\n\r",obj->value4());
                        ch->send_to(buf);
                }
                break;
        
        case ITEM_CORPSE_PC:
        case ITEM_CORPSE_NPC:
                ch->printf( "Стейков: %d, Уровень: %d, Части тела: '%s', Vnum: %d\n\r",
                            obj->value0(), obj->value1(), 
                            part_flags.messages( obj->value2() ).c_str( ), obj->value3() );
                break;
        }

        if ( obj->extra_descr != 0 )
        {
                EXTRA_DESCR_DATA *ed;

                ch->send_to("Ключевые слова экстра-описаний: '");

                for ( ed = obj->extra_descr; ed != 0; ed = ed->next )
                {
                        ch->send_to(ed->keyword);
                        if ( ed->next != 0 )
                                ch->send_to(" ");
                }

                ch->send_to ("'\n\r");
        }

        if ( obj->pIndexData->extra_descr != 0 )
        {
                EXTRA_DESCR_DATA *ed;

                ch->send_to("Оригинал экстра-описания: '");

                for ( ed = obj->pIndexData->extra_descr; ed != 0; ed = ed->next )
                {
                        ch->send_to(ed->keyword);
                        if ( ed->next != 0 )
                                ch->send_to(" ");
                }

                ch->send_to("'\n\r");
        }

    ostringstream ostr;
    for (auto &paf: obj->affected)
        format_affect(paf, ostr);

    if (!obj->enchanted)
        for (auto &paf: obj->pIndexData->affected)
            format_affect(paf, ostr);

    ch->send_to(ostr);

    sprintf(buf,"Состояние : %d (%s) ", obj->condition, obj->get_cond_alias() );        
    ch->send_to(buf);
    
    if (obj->behavior) {
        ostringstream ostr;
        
        sprintf(buf, "Поведение: [%s]\r\n", obj->behavior->getType( ).c_str( ));
        ch->send_to(buf);

        obj->behavior.toStream( ostr );
        ch->send_to( ostr );
    } else {
        ch->send_to("\n\r");
    }

    if (!obj->properties.empty()) {
        ostringstream ostr;

        ostr << "Свойства: ";
        for (auto &prop: obj->properties)
            ostr << "{g" << prop.first << "{x: \"" << prop.second << "\"  ";
        ostr << endl;

        ch->send_to(ostr);
    }

    
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
    
    buf << "Имя: [" << victim->getNameP( ) << "] ";
    if (pc)
        buf << "Шорт: [" << pc->getRussianName( ).normal( ) << "] ";
    if (npc)
        buf << "Родная зона: " << (npc->zone ? npc->zone->name : "?");
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
    if (victim->heal_gain != 0 || victim->mana_gain != 0)
        buf << "Бонус восстановления жизни: " << victim->heal_gain << "%   Маны: " << victim->mana_gain << "%" << endl;
    
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
    
    buf << "Сражается с: " << (victim->fighting ? victim->fighting->getNameP( ) : "(none)") << "  ";
    if (pc)
        buf << "Смертей: " << pc->death << "  ";
    buf << "Несет вещей: " << victim->carry_number << "  "
        << "Общим весом: " << victim->getCarryWeight( ) / 10
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
    
    if (victim->affected_by)
        buf << "Под воздействием " << affect_flags.names(victim->affected_by) << endl;
    
    buf << "Хозяин " <<  (victim->master ? victim->master->getNameP( ) : "(none)") << "  "
        << "Лидер " <<  (victim->leader ? victim->leader->getNameP( ) : "(none)") << "  ";
    if (pc)
        buf << "Пет: " << (pc->pet ? pc->pet->getNameP( ) : "(none)");
    buf << endl;
    
    if (npc) {
        buf << "Шорт: " << npc->getShortDescr( ) << endl
            << "Длинное описание: "  << npc->getLongDescr( );

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

    buf << "Прошлый бой: " << (victim->last_fought ? victim->last_fought->getNameP( ) :"none") << "  "
        << "Когда: " << Date::getTimeAsString( victim->getLastFightTime( ) ) 
        << endl;
    
    if (victim->ambushing[0])
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
        ch->send_to("Формат:\n\r");
        ch->send_to("  vnum obj <name>\n\r");
        ch->send_to("  vnum mob <name>\n\r");
        ch->send_to("  vnum type <item_type>\n\r");
        return;
    }
    
    if (!str_cmp(arg, "type")) {
        do_tfind(ch, string);
        return;
    }

    if (!str_cmp(arg,"obj"))
    {
        do_ofind(ch,string);
         return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
        do_mfind(ch,string);
        return;
    }

    /* do both */
    do_mfind(ch,argument);
    do_ofind(ch,argument);
}


/* NOTCOMMAND */ void do_mfind( Character *ch, char *argument )
{
    extern int top_mob_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        ch->send_to("Найти кого?\n\r");
        return;
    }

    fAll        = false; /* !str_cmp( arg, "all" ); */
    found        = false;
    nMatch        = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
        if ( ( pMobIndex = get_mob_index( vnum ) ) != 0 )
        {
            nMatch++;
            if ( fAll || is_name( argument, pMobIndex->player_name ) )
            {
                found = true;
                sprintf( buf, "[%5d] %s\n\r",
                    pMobIndex->vnum, 
                    russian_case( pMobIndex->short_descr, '1' ).c_str( ) );
                ch->send_to(buf);
            }
        }
    }

    if ( !found )
        ch->send_to("Мобы с таким именем не найдены.\n\r");

    return;
}



/* NOTCOMMAND */ void do_ofind( Character *ch, char *argument )
{
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        ch->send_to("Найти что?\n\r");
        return;
    }

    fAll        = false; /* !str_cmp( arg, "all" ); */
    found        = false;
    nMatch        = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
        if ( ( pObjIndex = get_obj_index( vnum ) ) != 0 )
        {
            nMatch++;
            if ( fAll || is_name( argument, pObjIndex->name ) )
            {
                found = true;
                sprintf( buf, "[%5d] %s\n\r",
                    pObjIndex->vnum, 
                    russian_case( pObjIndex->short_descr, '1' ).c_str( ) );
                ch->send_to(buf);
            }
        }
    }

    if ( !found )
        ch->send_to("Объекты с таким именем не найдены.\n\r");

    return;
}

/* NOTCOMMAND */ void do_tfind( Character *ch, char *argument )
{
    int type;
    ostringstream buf;
    
    if ((type = item_table.value( argument )) == NO_FLAG) {
        ch->println( "Такого типа предмета не существует." );
        return;
    }
    
    for (int i=0; i<MAX_KEY_HASH; i++)
        for(OBJ_INDEX_DATA *pObj = obj_index_hash[i]; pObj; pObj = pObj->next) 
            if (pObj->item_type == type) {
                buf << dlprintf( "[%5d] %s\n",
                                 pObj->vnum,
                                 russian_case( pObj->short_descr, '1' ).c_str( ) );
            }

    if (buf.str( ).empty( ))
        ch->println( "Такого типа нет ни у одного объекта." );
    else
        page_to_char( buf.str( ).c_str( ), ch ); 
}

CMDWIZP( rwhere )
{
    ostringstream buf;
    bool found = false;

    if (argument[0] == '\0') {
        ch->send_to("Найти какую комнату?\n\r");
        return;
    }

    for (auto &r: roomInstances)
        if (is_name(argument, r->getName())) {
            buf << dlprintf("[%6d] %-30s %s\r\n", r->vnum, r->getName(), r->areaName());
            found = true;
        }

    if (!found)
        ch->println("Комната с таким именем не найдена.");
    else
        ch->send_to(buf);
}

CMDWIZP( owhere )
{
    char buf[MAX_INPUT_LENGTH];
    ostringstream buffer;
    Object *obj;
    Object *in_obj;
    bool found = false;;
    int number = 0, max_found = 200;
    int vnum = -1;

    if ( argument[0] == '\0' )
    {
            ch->send_to("Найти что?\n\r");
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
            if (!is_name( argument, obj->getName( ) ))
                continue;
        }


        found = true;
        number++;

        for ( in_obj = obj; in_obj->in_obj != 0; in_obj = in_obj->in_obj )
                ;

        if ( in_obj->carried_by != 0 && ch->can_see(in_obj->carried_by)
                && in_obj->carried_by->in_room != 0 )
                sprintf( buf, "%3d) %s в руках у %s [Комната %d]\n\r",
                        number,
                        obj->getShortDescr( '1' ).c_str( ),
                        ch->sees(in_obj->carried_by, '2').c_str(),
                        in_obj->carried_by->in_room->vnum );
        else if ( in_obj->in_room != 0 && ch->can_see(in_obj->in_room) )
                sprintf( buf, "%3d) %s на полу в %s [Комната %d]\n\r",
                        number,
                        obj->getShortDescr( '1' ).c_str( ),
                        in_obj->in_room->getName(),
                        in_obj->in_room->vnum );
        else
                sprintf( buf, "%3d) %s черт его знает где.\n\r",
                        number,
                        obj->getShortDescr( '1' ).c_str( ) );

        buf[0] = Char::upper(buf[0]);
        buffer << buf;

        if ( number >= max_found )
            break;
    }

    if (!found)
        ch->send_to("Ни на земле, ни в небесах не найдено. Увы и ах.\n\r");
    else
        page_to_char( buffer.str( ).c_str( ), ch );
}


CMDWIZP( mwhere )
{
    char buf[MAX_STRING_LENGTH];
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
                    sprintf(buf,"%3d) %s (в теле %s) находится в %s [%d]\n\r",
                            count, 
                            victim->getPC( )->getNameP( ),
                            victim->getNameP('1').c_str( ),
                            victim->in_room->getName(), victim->in_room->vnum);
                else
                    sprintf(buf,"%3d) %s находится в %s [%d]\n\r",
                            count, 
                            victim->getNameP( ),
                            victim->in_room->getName(), victim->in_room->vnum);

                buffer << buf;
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
        sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
                victim->is_npc() ? victim->getNPC()->pIndexData->vnum : 0,
                victim->is_npc() ? victim->getNameP( '1' ).c_str( ) : victim->getNameP( ),
                victim->in_room->vnum,
                victim->in_room->getName() );
        buffer << buf;
    }

    if (!found)
        act_p( "Тебе не удалось найти $T.", ch, 0, argument, TO_CHAR,POS_DEAD );
    else
        page_to_char(buffer.str( ).c_str( ),ch);

}






CMDWIZP( shutdown )
{
    Descriptor *d,*d_next;
    
    DLFileAppend( dreamland->getBasePath( ), dreamland->getShutdownFile( ) )
         .printf( "Shutdown %s by %s\n",
                    Date::getCurrentTimeAsString( ).c_str( ),
                    ch->getNameP( )
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
        ch->send_to("Следить за кем?\n\r");
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == 0 )
    {
        ch->send_to("Таких сейчас в мире нет\n\r");
        return;
    }

    if ( victim->desc == 0 )
    {
        ch->send_to("Дескриптор не найден!\n\r");
        return;
    }

    if ( victim == ch )
    {
        ch->send_to("Отменяем всю слежку.\n\r");
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
        ch->send_to("Кто-то уже следит за этим игроком.\n\r");
        return;
    }

    if (!victim->in_room->isOwner(ch) && ch->in_room != victim->in_room
    &&  victim->in_room->isPrivate( ) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        ch->send_to("Этот игрок сейчас в приватной комнате.\n\r");
        return;
    }

    if ( victim->get_trust() >= ch->get_trust()
    ||   IS_SET(victim->comm,COMM_SNOOP_PROOF))
    {
        ch->send_to("У тебя пока не хватает прав на слежку за игроками.\n\r");
        return;
    }

    if ( ch->desc != 0 ) {
        for ( d = ch->desc->snoop_by; d != 0; d = d->snoop_by ) {
            if ( d->character == victim || d->character->getPC( ) == victim ) {
                ch->send_to("Упс, круговая слежка!\n\r");
                return;
            }
        }
    }

    victim->desc->snoop_by = ch->desc;
    wiznet( WIZ_SNOOPS, WIZ_SECURE, ch->get_trust(), "%C1 начинает следить за %C1.", ch, victim );
    ch->send_to("Начинаем слежку.\n\r");
}



CMDWIZP( switch )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    if(ch->is_npc( )) {
        ch->send_to("Мобы этого не умеют.\n\r");
        return;
    }
    
    one_argument( argument, arg );

    PCharacter *pch = ch->getPC( );

    if ( arg[0] == '\0' )
    {
        ch->send_to("Вселиться в чье тело?\n\r");
        return;
    }

    if ( ch->desc == 0 )
        return;

    if ( pch->switchedTo )
    {
        ch->send_to("Ты уже в чужом теле.\n\r");
        return;
    }
    
    victim = get_char_world( ch, arg );
    if ( !victim )
    {
        ch->send_to("Таких сейчас в мире нет.\n\r");
        return;
    }

    if ( victim == ch ) {
        ch->send_to("Ты в своем теле.\n\r");
        return;
    }

    if (!victim->is_npc()) {
        ch->send_to("Вселиться можно только в тело мобов.\n\r");
        return;
    }

    NPCharacter *mob = victim->getNPC( );

    if (ch->in_room != victim->in_room &&  victim->in_room->isPrivate( ) &&
            !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        ch->send_to("Этот персонаж сейчас в приватной комнате.\n\r");
        return;
    }

    if ( victim->desc != 0 ) {
        ch->send_to("В это тело уже кто-то вселился!\n\r");
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
    victim->send_to("Ты вселяешься в чужое тело.\n\r");
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
        ch->println("Ты и так в своем теле.");
        return;
    }
    
    if ( mob->desc == 0 )
        return;
    
    if ( !mob->switchedFrom ) {
        ch->send_to("Ты и так в своем теле.\n\r");
        return;
    }

    if (mprog_return( mob ))
        return;

    mob->send_to("Ты возвращаешься в своё привычное тело. Используй команду {y{hc{lRпрослушать{lEreplay{x для просмотра пропущенных сообщений.\n\r");
    ch->prompt.clear( );

    wiznet( WIZ_SWITCHES, WIZ_SECURE, ch->get_trust( ), 
            "%C1 returns from %C2.", mob->switchedFrom, mob );
    
    mob->desc->associate( mob->switchedFrom );
    mob->switchedFrom->switchedTo = 0;
    mob->switchedFrom = 0;
    mob->desc = 0;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(Character *ch, Object *obj, Object *clone)
{
        Object *c_obj, *t_obj;

        for (c_obj = obj->contains; c_obj != 0; c_obj = c_obj->next_content)
        {
            t_obj = create_object(c_obj->pIndexData,0);
            clone_object(c_obj,t_obj);
            obj_to_obj(t_obj,clone);
            recursive_clone(ch,c_obj,t_obj);
        }
}

/* command that is similar to load */
CMDWIZP( clone )
{
        char arg[MAX_INPUT_LENGTH];
        char *rest;
        Character *mob;
        Object  *obj;

        rest = one_argument(argument,arg);

        if (arg[0] == '\0')
        {
                ch->send_to("Клонировать кого или что?\n\r");
                return;
        }

        if (!str_prefix(arg,"object"))
        {
                mob = 0;
                obj = get_obj_here(ch,rest);
                if (obj == 0)
                {
                        ch->send_to("Этого здесь нет.\n\r");
                        return;
                }
        }
        else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
        {
                obj = 0;
                mob = get_char_room(ch,rest);
                if (mob == 0)
                {
                        ch->send_to("Таких здесь нет.\n\r");
                        return;
                }
        }
        else /* find both */
        {
                mob = get_char_room(ch,argument);
                obj = get_obj_here(ch,argument);
                if (mob == 0 && obj == 0)
                {
                        ch->send_to("Этого здесь нет.\n\r");
                        return;
                }
        }

        /* clone an object */
        if (obj != 0)
        {
                Object *clone;

                clone = create_object(obj->pIndexData,0);
                clone_object(obj,clone);

                if (obj->carried_by != 0)
                        obj_to_char(clone,ch);
                else
                        obj_to_room(clone,ch->in_room);

                recursive_clone(ch,obj,clone);

                act_p("$c1 создает $o4.",ch,clone,0,TO_ROOM,POS_RESTING);
                act_p("Ты создаешь дубликат $o2.",ch,clone,0,TO_CHAR,POS_RESTING);
                wiznet( WIZ_LOAD, WIZ_SECURE, ch->get_trust( ), 
                        "%C1 клонирует %O4.", ch, obj );
                return;
        }
        else if (mob != 0)
        {
                NPCharacter *clone;
                Object *new_obj;

                if (!mob->is_npc())
                {
                        ch->send_to("Клонировать игроков запрещено Женевской конвенцией.\n\r");
                        return;
                }

                if ( (mob->getRealLevel( ) > 20 && !IS_TRUSTED(ch,GOD))
                        || (mob->getRealLevel( ) > 10 && !IS_TRUSTED(ch,IMMORTAL))
                        || (mob->getRealLevel( ) >  5 && !IS_TRUSTED(ch,DEMI))
                        || (mob->getRealLevel( ) >  0 && !IS_TRUSTED(ch,ANGEL))
                        || !IS_TRUSTED(ch,AVATAR) )
                {
                        ch->send_to("У тебя пока не хватает прав для клонирования.\n\r");
                        return;
                }

                clone = create_mobile(mob->getNPC()->pIndexData);
                clone_mobile(mob->getNPC(),clone);
        
                for (obj = mob->carrying; obj != 0; obj = obj->next_content)
                {
                    new_obj = create_object(obj->pIndexData,0);
                    clone_object(obj,new_obj);
                    recursive_clone(ch,obj,new_obj);
                    obj_to_char(new_obj,clone);
                    new_obj->wear_loc = obj->wear_loc;
                }

                char_to_room(clone,ch->in_room);
                act_p("$c1 создает $C4.",ch,0,clone,TO_ROOM,POS_RESTING);
                act_p("Ты клонируешь $C4.",ch,0,clone,TO_CHAR,POS_RESTING);
                wiznet( WIZ_LOAD, WIZ_SECURE, ch->get_trust( ), 
                        "%C1 клонирует %C4.", ch, clone );
                
                return;
        }
}

/* RT to replace the two load commands */

CMDWIZP( load )
{
   char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        ch->send_to("Формат:\n\r");
        ch->send_to("  load mob <vnum>\n\r");
        ch->send_to("  load obj <vnum> <level>\n\r");
        return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
        do_mload(ch,argument);
        return;
    }

    if (!str_cmp(arg,"obj"))
    {
        do_oload(ch,argument);
        return;
    }
    /* echo syntax */
    run(ch, str_empty);
}


/* NOTCOMMAND */ void do_mload( Character *ch, char *argument )
{
        char arg[MAX_INPUT_LENGTH];
        MOB_INDEX_DATA *pMobIndex;
        Character *victim;

        one_argument( argument, arg );

        if ( arg[0] == '\0' || !is_number(arg) )
        {
                ch->send_to("Формат: load mob <vnum>.\n\r");
                return;
        }

        if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == 0 )
        {
                ch->send_to("Моба с таким внумом не найдено.\n\r");
                return;
        }

        victim = create_mobile( pMobIndex );

        if (victim->in_room == 0)
            char_to_room( victim, ch->in_room );

        act_p( "$c1 создает $C4!", ch, 0, victim, TO_ROOM,POS_RESTING );
        act_p( "Ты создаешь $C4!", ch, 0, victim, TO_CHAR,POS_RESTING );


        wiznet( WIZ_LOAD, WIZ_SECURE, ch->get_trust(), 
                "%C1 создает %C4.", ch, victim );
        ch->send_to("Готово.\n\r");
        return;
}



/* NOTCOMMAND */ void do_oload( Character *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    Object *obj;
    short level;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1))
    {
        ch->send_to("Формат: load obj <vnum> <level>.\n\r");
        return;
    }

    level = ch->get_trust(); /* default */

    if ( arg2[0] != '\0')  /* load with a level */
    {
        if (!is_number(arg2))
        {
          ch->send_to("Формат: oload <vnum> <level>.\n\r");
          return;
        }
        level = atoi(arg2);
        if (level < 0 || level > ch->get_trust())
        {
          ch->send_to("Уровень не может быть ниже нуля или выше твоего.\n\r");
            return;
        }
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == 0 )
    {
        ch->send_to("Объект с таким внумом не найден.\n\r");
        return;
    }

    obj = create_object( pObjIndex, level );
    if ( obj->can_wear( ITEM_TAKE) )
        obj_to_char( obj, ch );
    else
        obj_to_room( obj, ch->in_room );
        
    act_p( "$c1 создает $o4!", ch, obj, 0, TO_ROOM,POS_RESTING );
    act_p( "Ты создаешь $o4!", ch, obj, 0, TO_CHAR,POS_RESTING );
    wiznet( WIZ_LOAD, WIZ_SECURE, ch->get_trust( ), "%C1 loads %O4.", ch, obj );
    
    LogStream::sendNotice( ) 
        << ch->getName( ) << " loads obj vnum " << obj->pIndexData->vnum
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

        act_p( "$c1 изничтожает все в комнате!", ch, 0, 0, TO_ROOM,POS_RESTING);
        ch->send_to("Готово.\n\r");
        dreamland->resetOption( DL_SAVE_MOBS );
        dreamland->resetOption( DL_SAVE_OBJS );
        save_items( ch->in_room );
        save_mobs( ch->in_room );
        return;
    }

    // Try to purge a mob in the room or an item in the room/inventory/equip.
    if ((victim = get_char_room(ch, arg))) {
        if (!victim->is_npc()) {
            ch->send_to("Сначала научись рисовать и отрасти усики.\n\r");
            return;
        }

        act( "$c1 изничтожает $C4.", ch, 0, victim, TO_NOTVICT );
        act( "Ты изничтожаешь $C4.", ch, 0, victim, TO_CHAR );
        extract_char( victim );
        return;
    }

    if ((obj = get_obj_here(ch, arg))) {
        act( "$c1 изничтожает $o4.", ch, obj, 0, TO_ROOM );
        act( "Ты изничтожаешь $o4.", ch, obj, 0, TO_CHAR );
        extract_obj(obj);
        return;
    }

    ch->println("Ты не видишь здесь моба или предмет с таким именем.");
}





CMDWIZP( restore )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;
    Character *vch;
    Descriptor *d;

    one_argument( argument, arg );
    if (arg[0] == '\0' || !str_cmp(arg,"room"))
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
            act_p("$c1 {Gвосстановил$gо||а {xтвои силы!",ch,0,vch,TO_VICT,POS_DEAD);
        }

        wiznet( WIZ_RESTORE, WIZ_SECURE, ch->get_trust(), 
                "%C1 restored room %d.", ch, ch->in_room->vnum );

        ch->send_to("Комната восстановлена.\n\r");
        return;

    }

    if ( ch->get_trust() >=  MAX_LEVEL - 5 && !str_cmp(arg,"all"))
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
                act_p("$c1 {Gвосстановил$gо||а {xтвои силы!",ch,0,victim,TO_VICT,POS_DEAD);
        }
        ch->send_to("Все активные игроки восстановлены!\n\r");
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == 0 )
    {
        ch->send_to("Таких здесь нет.\n\r");
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

    act_p( "$c1 {Gвосстановил$gо||а {xтвои силы!", ch, 0, victim, TO_VICT,POS_DEAD );
    wiznet( WIZ_RESTORE, WIZ_SECURE, ch->get_trust( ), "%C1 restored %C4.", ch, victim );
    ch->send_to("Готово.\n\r");
}

         
CMDWIZP( freeze )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->send_to("Заморозить кого?\n\r");
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == 0 )
    {
        ch->send_to("Тут таких нет.\n\r");
        return;
    }

    if ( victim->is_npc() )
    {
        ch->send_to("Мобов замораживать бесполезно.\n\r");
        return;
    }

    if ( victim->get_trust() >= ch->get_trust() )
    {
        ch->send_to("У тебя пока не хватает прав для этого.\n\r");
        return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
        victim->act.removeBit( PLR_FREEZE);
        victim->send_to("{CТебя разморозили!{x Ты чувствуешь как способность двигаться возвращается.\n\r");
        ch->send_to("Цель разморожена.\n\r");
        wiznet( WIZ_PENALTIES, WIZ_SECURE, 0, "%C1 thaws %C4.", ch, victim );
    }
    else
    {
        victim->act.setBit( PLR_FREEZE);
        victim->send_to("{CТЕБЯ ЗАМОРОЗИЛИ!{x Ты теряешь способность двигаться и говорить.\n\r");
        ch->send_to("Цель заморожена.\n\r");
        wiznet( WIZ_PENALTIES, WIZ_SECURE, 0, 
                "%C1 puts %C4 in the deep freeze.", ch, victim );
    }

    if( !victim->is_npc( ) ) 
        victim->getPC( )->save();
}



CMDWIZP( log )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->send_to("Log whom?\n\r");
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        if (!dreamland->hasOption( DL_LOG_ALL )) {
            dreamland->setOption( DL_LOG_ALL );
            ch->send_to("Log ALL on.\n\r");
        }
        else {
            dreamland->removeOption( DL_LOG_ALL );
            ch->send_to("Log ALL off.\n\r");
        }

        return;
    }

    if (!str_cmp( arg, "imm" )) {
        if (!dreamland->hasOption( DL_LOG_IMM )) {
            dreamland->setOption( DL_LOG_IMM );
            ch->send_to( "Immortal logging is now ON.\r\n" );
        }
        else {
            dreamland->removeOption( DL_LOG_IMM );
            ch->send_to( "Immortal logging is now OFF.\r\n" );
        }

        return;
    }
    
    if ( ( victim = get_char_world( ch, arg ) ) == 0 )
    {
        ch->send_to("They aren't here.\n\r");
        return;
    }

    if ( victim->is_npc() )
    {
        ch->send_to("Not on NPC's.\n\r");
        return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
        victim->act.removeBit( PLR_LOG);
        ch->send_to("LOG removed.\n\r");
    }
    else
    {
        victim->act.setBit( PLR_LOG);
        ch->send_to("LOG set.\n\r");
    }

    return;
}



CMDWIZP( noemote )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->send_to("Noemote whom?\n\r");
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == 0 )
    {
        ch->send_to("They aren't here.\n\r");
        return;
    }


    if ( victim->get_trust() >= ch->get_trust() )
    {
        ch->send_to("You failed.\n\r");
        return;
    }

    if ( IS_SET(victim->comm, COMM_NOEMOTE) )
    {
        victim->comm.removeBit( COMM_NOEMOTE);
        victim->send_to("{MЧувства возвращаются к тебе!{x Ты снова можешь использовать эмоции.\n\r");
        ch->send_to("NOEMOTE removed.\n\r");
        wiznet( WIZ_PENALTIES, WIZ_SECURE, 0, "%C1 restores emotes to %C1.", ch, victim );
    }
    else
    {
        victim->comm.setBit( COMM_NOEMOTE);
        victim->send_to("{MТЫ ТЕРЯЕШЬ ЧУВСТВА!{x Ты больше не можешь использовать эмоции.\n\r");
        ch->send_to("NOEMOTE set.\n\r");
        wiznet( WIZ_PENALTIES, WIZ_SECURE, 0, "%C1 revokes %C2 emotes.", ch, victim );
    }
}


CMDWIZP( notell )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->send_to("Notell whom?");
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == 0 )
    {
        ch->send_to("They aren't here.\n\r");
        return;
    }

    if ( victim->get_trust() >= ch->get_trust() )
    {
        ch->send_to("You failed.\n\r");
        return;
    }

    if ( IS_SET(victim->comm, COMM_NOTELL) )
    {
        victim->comm.removeBit( COMM_NOTELL);
        victim->send_to("{GПечать на твоих устах пропадает.{x Ты снова можешь говорить.\n\r");
        ch->send_to("NOTELL removed.\n\r");
        wiznet( WIZ_PENALTIES, WIZ_SECURE, 0, "%C1 restores tells to %C2.", ch, victim );
    }
    else
    {
        victim->comm.setBit( COMM_NOTELL);
        victim->send_to("{GБессмертный запечатывает твои уста!{x Ты не можешь говорить!\n\r");
        ch->send_to("NOTELL set.\n\r");
        wiznet( WIZ_PENALTIES, WIZ_SECURE, 0, "%C1 revokes %C1 tells.", ch, victim );
    }
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
        ch->send_to("Game un-wizlocked.\n\r");
    }
    else
    {
        dreamland->setOption( opt );
        wiznet( 0, 0, 0, "%C1 has wizlocked the game.", ch );
        ch->send_to("Game wizlocked.\n\r");
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
        ch->send_to("Newlock removed.\n\r");
    }
    else
    {
        dreamland->setOption( opt );
        wiznet( 0, 0, 0, "%C1 locks out new characters.", ch );
        ch->send_to("New characters have been locked out.\n\r");
    }
}





CMDWIZP( string )
{
        char type [MAX_INPUT_LENGTH];
        char arg1 [MAX_INPUT_LENGTH];
        char arg2 [MAX_INPUT_LENGTH];
        char arg3 [MAX_INPUT_LENGTH];
        Character *victim;
        Object *obj;

        argument = one_argument( argument, type );
        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );
        strcpy( arg3, argument );

        if ( type[0] == '\0'
                || arg1[0] == '\0'
                || arg2[0] == '\0'
                || arg3[0] == '\0' )
        {
                ch->send_to("Формат:\n\r");
                ch->send_to("  string mob <name> <field> <string>\n\r");
                ch->send_to("    поля: name short long desc title spec\n\r");
                ch->send_to("  string obj  <name> <field> <string>\n\r");
                ch->send_to("    поля: name short long\n\r");
                ch->send_to("  string obj  <name> ed <add|remove|clear> <keyword> <string>\n\r");
                return;
        }

        if ( !str_prefix(type,"character")
                || !str_prefix(type,"mobile"))
        {
                if ( ( victim = get_char_world( ch, arg1 ) ) == 0 )
                {
                        ch->send_to("They aren't here.\n\r");
                        return;
                }

                /* clear zone for mobs */
                if (victim->is_npc())
                    victim->getNPC()->zone = 0;

                /* string something */

                if ( !str_prefix( arg2, "name" ) )
                {
                        if ( !victim->is_npc() )
                        {
                                ch->send_to("Not on PC's.\n\r");
                                return;
                        }

                        DLString name( arg3 );
                        victim->setName( name );
                        return;
                }
            
                if ( !str_prefix( arg2, "description" ) )
                {
                        victim->setDescription( arg3 );
                        return;
                }

                if ( !str_prefix( arg2, "short" ) )
                {
                        if ( !victim->is_npc() )
                        {
                                ch->send_to("Not on PC's.\n\r");
                                return;
                        }
                        victim->getNPC()->setShortDescr( arg3 );
                        return;
                }

                if ( !str_prefix( arg2, "long" ) )
                {
                        if ( !victim->is_npc() )
                        {
                                ch->send_to("Not on PC's.\n\r");
                                return;
                        }
                        strcat(arg3,"\n\r");
                        victim->getNPC()->setLongDescr( arg3 );
                        return;
                }

                if ( !str_prefix( arg2, "title" ) )
                {
                        if ( victim->is_npc() )
                        {
                                ch->send_to("Not on NPC's.\n\r");
                                return;
                        }
                        
                        victim->getPC( )->setTitle( arg3 );
                        return;
                }

                if ( !str_prefix( arg2, "spec" ) )
                {
                        if ( !victim->is_npc() )
                        {
                                ch->send_to("Not on PC's.\n\r");
                                return;
                        }

                        if ( ( victim->getNPC()->spec_fun = spec_lookup( arg3 ) ) == 0 )
                        {
                                ch->send_to("No such spec fun.\n\r");
                                return;
                        }

                        return;
                }
        }

        if (!str_prefix(type,"object"))
        {
                // string an obj
            
                if ( ( obj = get_obj_world( ch, arg1 ) ) == 0 )
                {
                        ch->send_to("Nothing like that in heaven or earth.\n\r");
                        return;
                }

                if ( obj->pIndexData->limit != -1 )
                {
                        ch->send_to("Хмм... Тайфоэн будет возмущена, не надо.\n\r");
                        return;
                }
            
                if ( !str_prefix( arg2, "name" ) )
                {
                        obj->setName( arg3 );
                }
                else
                if ( !str_prefix( arg2, "short" ) )
                {
                        obj->setShortDescr( arg3 );
                }
                else
                if ( !str_prefix( arg2, "long" ) )
                {
                        obj->setDescription( arg3 );
                }
                else
                if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
                {
                        EXTRA_DESCR_DATA *ed;

                        argument = one_argument( argument, arg3 );
                        if ( argument == 0 )
                        {
                                ch->send_to("Syntax: oset <object> ed <add|remove|clear> <keyword> <string>\n\r");
                                return;
                        }

                        if ( !str_prefix (arg3, "clear") )
                        {
                                EXTRA_DESCR_DATA *ed_next;

                                for (ed = obj->extra_descr; ed != 0; ed = ed_next)
                                {
                                        ed_next = ed->next;
                                        free_extra_descr (ed);
                                }

                                obj->extra_descr = 0;

                        }
                        else if ( !str_prefix (arg3, "add") )
                        {

                                argument = one_argument( argument, arg3 );

                                strcat(argument,"\n\r");

                                ed = new_extra_descr();

                                ed->keyword                = str_dup( arg3     );
                                ed->description        = str_dup( argument );
                                ed->next                = obj->extra_descr;
                                obj->extra_descr        = ed;
                        }
                        else if ( !str_prefix (arg3, "remove") )
                        {
                                EXTRA_DESCR_DATA *ed_next;

                                argument = one_argument( argument, arg3 );

                                for (ed = obj->extra_descr; ed != 0; ed = ed_next)
                                {
                                        ed_next = ed->next;

                                        if ( is_name (arg3, ed->keyword) )
                                        {
                                                if (obj->extra_descr == ed)
                                                {
                                                        obj->extra_descr = ed_next;
                                                }
                                                free_extra_descr (ed);
                                        }
                                }
                        }
                        else
                        {
                                ch->send_to ("А может все таки синтакс проверим, а?\n\r");
                                return;
                        }
                }
                else
                {
                        /* echo bad use message */
                        run(ch, str_empty);
                        return;
                }

                save_items_at_holder( obj );
        }
  else
        {
                /* echo bad use message */
                run(ch, str_empty);
        }
}

int decode_flags(char * arg, int * value_add, int * value_sub)
{
        bool negative = false, additive = false;

        int value = atoi( arg );

        *value_add = 0;
        *value_sub = 0;

        if ( value == 0 )
                for (int i = 0; arg[i] != '\0'; i++ )
                {
                        switch ( arg[i] )
                        {
                        case '|' :
                                break;
                        case '-' :
                                additive = false;
                                negative = true;
                                break;
                        case '+' :
                                negative = false;
                                additive = true;
                                break;
                        default :
                                if ( ('A' <= arg[i] && arg[i] <= 'Z')
                                        || ('a' <= arg[i] && arg[i] <= 'z') )
                                {
                                        if ( additive )
                                                SET_BIT( *value_add, flag_convert( arg[i] ) );
                                        else if ( negative )
                                                SET_BIT( *value_sub, flag_convert( arg[i] ) );
                                        else
                                                SET_BIT( value, flag_convert( arg[i] ) );
                                }
                        }
                }

        return value;
}


/*
 * Syntax:
 * force all|players|gods|<name> <command with args>
 */
CMDWIZP( force )
{
    const char *msg = "%s вежливо принуждает тебя выполнить команду '%s'.\r\n";
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        ch->send_to("Принудить кого к чему?\n\r");
        return;
    }

    if (arg_is_all(arg))
    {
        Character *vch;
        Character *vch_next;

        if (ch->get_trust() < MAX_LEVEL - 5)
        {
            ch->send_to("У тебя пока не хватает прав на принуждение.\n\r");
            return;
        }

        for ( vch = char_list; vch != 0; vch = vch_next )
        {
            vch_next = vch->next;

            if ( !vch->is_npc() && vch->get_trust() < ch->get_trust() )
            {
                vch->printf(msg, vch->sees(ch, '1').c_str(), argument);
                interpret( vch, argument );
            }
        }
    }
    else if (!str_cmp(arg,"players"))
    {
        Character *vch;
        Character *vch_next;

        if (ch->get_trust() < MAX_LEVEL - 2)
        {
            ch->send_to("У тебя пока не хватает прав на такое принуждение.\n\r");
            return;
        }

        for ( vch = char_list; vch != 0; vch = vch_next )
        {
            vch_next = vch->next;

            if ( !vch->is_npc() && vch->get_trust() < ch->get_trust()
            &&         vch->getRealLevel( ) < LEVEL_HERO)
            {
                vch->printf(msg, vch->sees(ch, '1').c_str(), argument);
                interpret( vch, argument );
            }
        }
    }
    else if (!str_cmp(arg,"gods"))
    {
        Character *vch;
        Character *vch_next;

        if (ch->get_trust() < MAX_LEVEL - 2)
        {
            ch->send_to("У тебя пока не хватает прав на такое принуждение.\n\r");
            return;
        }

        for ( vch = char_list; vch != 0; vch = vch_next )
        {
            vch_next = vch->next;

            if ( !vch->is_npc() && vch->get_trust() < ch->get_trust()
            &&   vch->getRealLevel( ) >= LEVEL_HERO)
            {
                vch->printf(msg, vch->sees(ch, '1').c_str(), argument);
                interpret( vch, argument );
            }
        }
    }
    else
    {
        Character *victim;

        if ( ( victim = get_char_world( ch, arg ) ) == 0 )
        {
            ch->send_to("Тут таких нет.\n\r");
            return;
        }

        if ( victim == ch )
        {
            ch->send_to("Сила воли, бессердечная ты сука.\n\r");
            return;
        }

            if (!victim->in_room->isOwner(ch)
        &&  ch->in_room != victim->in_room
        &&  victim->in_room->isPrivate( ) && !IS_TRUSTED(ch,IMPLEMENTOR))
            {
            ch->send_to("Этот персонаж сейчас в приватной комнате.\n\r");
            return;
        }

        if ( victim->get_trust() >= ch->get_trust() )
        {
            ch->send_to("Эту цель ты пока не можешь принудить!\n\r");
            return;
        }

        if ( !victim->is_npc() && ch->get_trust() < MAX_LEVEL - 4)
        {
            ch->send_to("У тебя пока не хватает прав на такое принуждение.\n\r");
            return;
        }

        victim->printf(msg, victim->sees(ch, '1').c_str(), argument);
        interpret( victim, argument );
    }

    ch->send_to("Готово.\n\r");
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
          act_p( "$c1 внезапно проявляется в реальности.", ch, 0, 0, TO_ROOM,POS_RESTING );
          ch->send_to("Ты снова проявляешься в реальности.\n\r");
      }
      else
      {
          ch->invis_level = 102;
          act_p( "$c1 подмигивает и растворяется за подкладкой реальности.", ch, 0, 0, TO_ROOM,POS_RESTING );
          ch->send_to("Ты растворяешься за подкладкой реальности.\n\r");
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > ch->get_trust())
      {
        ch->send_to("Invis level must be between 2 and your level.\n\r");
        return;
      }
      else
      {
          ch->reply = 0;
          ch->invis_level = level;
          act_p( "$c1 подмигивает и растворяется за подкладкой реальности.", ch, 0, 0, TO_ROOM,POS_RESTING );
          ch->send_to("Ты растворяешься за подкладкой реальности.\n\r");
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
          act_p( "$c1 больше не маскируется.", ch, 0, 0, TO_ROOM,POS_RESTING );
          ch->send_to("Ты больше не маскируешься.\n\r");
      }
      else
      {
          ch->incog_level = 102;
          act_p( "$c1 скрывает $s присутствие.", ch, 0, 0, TO_ROOM,POS_RESTING );
          ch->send_to("Ты скрываешь свое присутствие.\n\r");
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > ch->get_trust())
      {
        ch->send_to("Incog level must be between 2 and your level.\n\r");
        return;
      }
      else
      {
          ch->reply = 0;
          ch->incog_level = level;
          act_p( "$c1 скрывает $s присутствие.", ch, 0, 0, TO_ROOM,POS_RESTING );
          ch->send_to("Ты скрываешь свое присутствие.\n\r");
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
        ch->send_to("Формат: advance <char> <level>.\n\r");
        return;
    }

    if ( ( vict = get_char_room( ch, arg1 ) ) == 0 )
    {
        ch->send_to("Этого игрока тут нет.\n\r");
        return;
    }

    if ( vict->is_npc() )
    {
        ch->send_to("Not on NPC's.\n\r");
        return;
    }

    victim = vict->getPC();
    
    if ( ( level = atoi( arg2 ) ) < 1 || level > 110 )
    {
        ch->send_to("Level must be 1 to 110.\n\r");
        return;
    }

    if ( level > ch->get_trust() )
    {
        ch->send_to("Limited to your trust level.\n\r");
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

        ch->send_to("Lowering a player's level!\n\r");
        victim->send_to("**** ОООООО НЕЕЕЕЕЕЕЕЕТ!!! ****\n\r");
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
        victim->getPC( )->advanceLevel( );
        victim->practice = temp_prac;
        victim->train    = temp_train;
    }
    else
    {
        ch->send_to("Raising a player's level!\n\r");
        victim->send_to("**** ООООО ДАААААААААА ****\n\r");
    }

    for ( iLevel = victim->getRealLevel( ) ; iLevel < level; iLevel++ )
    {
        if( victim->get_trust() != 0xFFFF )
            victim->send_to("Ты повышаешь уровень!  ");
        victim->exp += victim->getExpToLevel( );;
        victim->setLevel( victim->getRealLevel( ) + 1 );
        victim->getPC( )->advanceLevel( );
    }
    victim->getPC( )->setTrust( 0 );
    if( !victim->is_npc( ) ) victim->getPC( )->save();
    return;
}



CMDWIZP( smite )
{
  Character *victim;

  if (argument[0] == '\0')
    {
      ch->send_to("От расстройства ты бьешь молнией себя!  Oххх!\n\r");
      return;
    }

  if ((victim = get_char_world(ch, argument)) == 0)
    {
      ch->send_to("Придеться подождать немного и послать на них молнию в другой раз.\n\r");
      return;
    }

  if (victim->is_npc())
    {
      ch->send_to("Этот бедный моб не сделал тебе ничего плохого.\n\r");
      return;
    }

  if (victim->getRealLevel() > ch->getRealLevel())
    {
      ch->send_to("Как ты смеешь!\n\r");
      return;
    }

  if (victim->position < POS_SLEEPING)
    {
      ch->send_to("Грешно издеваться над больными людьми.\n\r");
      return;
    }

  act_p("{CБоги {Rв гневе{C!{/{cОгромная молния, сорвавшаяся с небес, поражает тебя!{/{RЭто было БОЛЬНО! Это было МУЧИТЕЛЬНО БОЛЬНО!{x\n\r", victim, 0,
        ch, TO_CHAR,POS_DEAD);
  act_p("Твоя молния поражает $c4!\n\r", victim, 0, ch, TO_VICT,POS_DEAD);
  act_p("{cОгромная молния, сорвавшаяся с небес, поражает $c4!{x\n\r", victim, 0, ch, TO_NOTVICT,POS_DEAD);
  victim->hit = victim->hit / 2;
  victim->move = victim->move / 2;
  victim->mana = victim->mana / 2;
  return;
}

CMDWIZP( ititle )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )  {
        ch->send_to("Change whose title to what?\n\r");
        return;
    }

    victim = get_char_world( ch, arg );
    if (victim == 0)  {
        ch->send_to("Nobody is playing with that name.\n\r");
        return;
    }

    if ( victim->is_npc() )
        return;

    if ( argument[0] == '\0' )
    {
        ch->send_to("Change the title to what?\n\r");
        return;
    }
    victim->getPC( )->setTitle( argument );
    ch->send_to("Ok.\n\r");
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
        ch->send_to( "Rename who?\n\r" );
        return;
    }
    
    PCharacter* victim = get_player_world( ch, oldName.c_str( ) );
    
    if (!victim) {
        ch->send_to("There is no such a person online.\n\r");
        return;
    }
    
    if( (victim != ch ) && ( victim->get_trust( ) >= ch->get_trust( ) ) ) {
        ch->send_to( "You failed.\n\r" );
        return;
    }
    
    if( newName.empty( ) ) {
        ch->send_to( "Rename to what new name?\n\r" );
        return;
    }

    if (oldName ^ newName) {
        if (!russianName.empty( )) {
            victim->setRussianName( russianName );
            ch->println( "Russian name set." );
        }
        else {
            ch->println( "Both names are equal!" );
        }

        return;
    }
    
    DLString rc;
    if (!(rc = badNames->checkName(newName)).empty()) {
        ch->printf( "New name failed sanity checks, reason: %s.\n\r", rc.c_str() );
        return;
    }
    
    if (PCharacterManager::find( newName )) {
        ch->send_to( "A player with that name already exists!\n\r" );
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
                    obj->setOwner( newName.c_str( ) );
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

    ch->send_to("Character renamed.\n\r");
    act_p("$c1 переименова$gло|л|ла тебя в $C4!",ch,0,victim,TO_VICT,POS_DEAD);
}

CMDWIZP( notitle )
{
  char arg[MAX_INPUT_LENGTH];
  Character *victim;

    if ( !ch->is_immortal() )
        return;
    argument = one_argument(argument,arg);

    if ( (victim = get_char_world(ch ,arg)) == 0 )
    {
        ch->send_to("He is not currently playing.\n\r");
        return;
    }

   if (IS_SET(victim->act, PLR_NO_TITLE) )
        {
   victim->act.removeBit(PLR_NO_TITLE);
   victim->send_to("Теперь ты снова можешь поменять титул.\n\r");
   ch->send_to("Ok.\n\r");
        }
   else
        {                
   victim->act.setBit(PLR_NO_TITLE);
   victim->send_to("Бессмертный запрещает тебе менять титулы!\n\r");
   ch->send_to("Ok.\n\r");
        }
   return;
}


CMDWIZP( noaffect )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    if ( !ch->is_immortal() )
        return;

    argument = one_argument(argument,arg);

    if ( (victim = get_char_world(ch ,arg)) == 0 )
    {
        ch->send_to("He is not currently playing.\n\r");
        return;
    }

    AffectList affects = victim->affected.clone();
    for (auto paf_iter = affects.cbegin(); paf_iter != affects.cend(); paf_iter++) {
        Affect *paf = *paf_iter;
        if ( paf->duration >= 0 )
        {
            if (!affects.hasNext(paf_iter) && paf->type->getAffect( ))
                paf->type->getAffect( )->remove( victim );

            affect_remove( victim, paf );
        }
    }
}

CMDWIZP( reboot )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  argument = one_argument(argument,arg);

 if (arg[0] == '\0')
    {
      ch->send_to("Usage: reboot now\n\r");
      ch->send_to("Usage: reboot <ticks to reboot>\n\r");
      ch->send_to("Usage: reboot cancel\n\r");
      ch->send_to("Usage: reboot status\n\r");
      return;
    }

    if (is_name(arg,"cancel"))
     {
      dreamland->setRebootCounter( -1 );
      ch->send_to("Reboot canceled.\n\r");
      return;
    }

    if (is_name(arg, "now"))
     {
      reboot_anatolia();
      return;
    }

    if (is_name(arg, "status"))
    {
      if (dreamland->getRebootCounter( ) == -1)
        sprintf(buf, "Automatic rebooting is inactive.\n\r");
      else
        sprintf(buf,"Reboot in %i minutes.\n\r", dreamland->getRebootCounter( ));
      ch->send_to(buf);
      return;
    }

    if (is_number(arg))
    {
     dreamland->setRebootCounter( atoi(arg) );
     sprintf(buf,"Мир Мечты будет ПЕРЕЗАГРУЖЕН через %i тиков!\n\r",dreamland->getRebootCounter( ));
     ch->send_to(buf);
     return;
    }

    run(ch, str_empty);
}


CMDWIZP( olevel )
{
    char buf[MAX_INPUT_LENGTH];
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
        ch->send_to("Syntax: olevel <level>\n\r");
        ch->send_to("        olevel <level> <name>\n\r");
        return;
    }

    argument = one_argument(argument, name);
    for ( obj = object_list; obj != 0; obj = obj->next )
    {
        if ( obj->level != atoi(level) )
            continue;

        if ( name[0] != '\0' && !is_name(name, obj->getName( ) ) )
            continue;

        found = true;
        number++;

        for ( in_obj = obj; in_obj->in_obj != 0; in_obj = in_obj->in_obj );

        if ( in_obj->carried_by != 0 && ch->can_see(in_obj->carried_by)
        &&   in_obj->carried_by->in_room != 0)
            sprintf( buf, "%3d) [%d] %s is carried by %s [Room %d]\n\r",
                number, obj->pIndexData->vnum, obj->getShortDescr('1').c_str( ),ch->sees(in_obj->carried_by, '5').c_str(),
                in_obj->carried_by->in_room->vnum );
        else if (in_obj->in_room != 0 && ch->can_see(in_obj->in_room))
            sprintf( buf, "%3d) [%d] %s is in %s [Room %d]\n\r",
                number, obj->pIndexData->vnum,obj->getShortDescr( '1' ).c_str( ),in_obj->in_room->getName(),
                in_obj->in_room->vnum);
        else
            sprintf( buf, "%3d) [%d]%s is somewhere\n\r",number, obj->pIndexData->vnum,obj->getShortDescr( '1' ).c_str( ));

        buf[0] = Char::upper(buf[0]);
        buffer << buf;

        if (number >= max_found)
            break;
    }

    if ( !found )
        ch->send_to("Nothing like that in heaven or earth.\n\r");
    else
        page_to_char(buffer.str( ).c_str( ), ch);
}

CMDWIZP( mlevel )
{
    char buf[MAX_INPUT_LENGTH];
    ostringstream buffer;
    Character *victim;
    bool found;
    int count = 0;

    if ( argument[0] == '\0' )
    {
        ch->send_to("Syntax: mlevel <level>\n\r");
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
            sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
                victim->is_npc() ? victim->getNPC()->pIndexData->vnum : 0,
                victim->getNameP( '1' ).c_str(),
                victim->in_room->vnum,
                victim->in_room->getName() );
            buffer << buf;
        }
    }

    if ( !found )
        act_p( "You didn't find any mob of level $T.", ch, 0, argument, TO_CHAR,POS_DEAD );
    else
        page_to_char(buffer.str( ).c_str( ), ch);
}

CMDWIZP( nopk )
{
  if( !*argument ) {
    if( dreamland->hasOption( DL_PK ) ) {
      ch->send_to("{RPK{x разрешен.\n\r");
    } else {
      ch->send_to("{RPK{x запрещен.\n\r");
    }
  } else {
    if( !str_cmp( argument, "off" ) ) {
      dreamland->setOption( DL_PK );
      ch->send_to("{RPK{x разрешен.\n\r");
    } else {
      if( !str_cmp( argument, "on" ) ) {
        dreamland->removeOption( DL_PK );
        ch->send_to("{RPK{x запрещен.\n\r");
      } else {
        ch->send_to("Синтаксис: nopk [on/off]");
        return;
      }
    }
  }
}

CMDWIZP( merchant )
{
        char buf[MAX_STRING_LENGTH];
        sprintf(buf,"Состояние всемирного банка : {Y%ld gold{x\n\r",
                dreamland->getBalanceMerchantBank());
        ch->send_to(buf);
}


CMDWIZP( version )
{
    ch->send_to( dreamland->getVersion( ) );
}

/*------------------------------------------------------------------
 *
 *-----------------------------------------------------------------*/

extern "C"
{
    SO::PluginList initialize_cmd_wizard( )
    {
        SO::PluginList ppl;
        return ppl;
    }
}


