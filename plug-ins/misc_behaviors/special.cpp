/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************h
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
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT                           *        
 *     ANATOLIA has been brought to you by ANATOLIA consortium                   *
 *         Serdar BULUT {Chronos}                bulut@rorqual.cc.metu.edu.tr       *
 *         Ibrahim Canpunar  {Mandrake}        canpunar@rorqual.cc.metu.edu.tr    *        
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

#include <algorithm>

#include <sys/types.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "logstream.h"

#include "skill.h"

#include "room.h"
#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "dreamland.h"
#include "loadsave.h"
#include "merc.h"

#include "act.h"
#include "interp.h"
#include "magic.h"
#include "clanreference.h"

#include "fight.h"
#include "save.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "def.h"
#include "vnum.h"
  
CLAN(battlerager);
CLAN(ruler);
PROF(thief);
PROF(ninja);
GSN(armor);
GSN(bless);
GSN(blindness);
GSN(cure_blindness);
GSN(cure_disease);
GSN(cure_poison);
GSN(curse);
GSN(heal);
GSN(jail);
GSN(plague);
GSN(poison);
GSN(refresh);
GSN(remove_curse);
GSN(warcry);

#define OBJ_VNUM_WHISTLE           2116
#define MOB_VNUM_PATROLMAN           2106
#define GROUP_VNUM_TROLLS           2100
#define GROUP_VNUM_OGRES           2101
#define ADEPT_MAX_LEVEL        (PK_MIN_LEVEL + 14)

/*
 * The following special functions are available for mobiles.
 */
bool spec_troll_member( NPCharacter *ch)
{
    Character *vch, *victim = 0;
    int count = 0;

    if (!IS_AWAKE(ch) || IS_AFFECTED(ch,AFF_CALM) || ch->in_room == 0
    ||  IS_CHARMED(ch) || ch->fighting != 0)
        return false;

    /* find an ogre to beat up */
    for (vch = ch->in_room->people;  vch != 0;  vch = vch->next_in_room)
    {
        if (!vch->is_npc() || ch == vch)
            continue;

        if (vch->getNPC()->pIndexData->vnum == MOB_VNUM_PATROLMAN)
            return false;

        if (vch->getNPC()->pIndexData->group == GROUP_VNUM_OGRES
        &&  ch->getModifyLevel() > vch->getModifyLevel() - 2 && !is_safe(ch,vch))
        {
            if (number_range(0,count) == 0)
                victim = vch;

            count++;
        }
    }

    if (victim == 0)
        return false;

    /* say something, then raise hell */
    switch (number_range(0,6))
    {
        default: break;
        case 0: do_yell( ch, "Я найду тебя, шпана!");
                break;
        case 1: oldact("Яростно вскрикнув, $c1 бросается на $C4.",ch,0,victim,TO_ALL);
                break;
        case 2: do_say( ch, "Что ты тут забыло, огровское отродье?");
                break;
        case 3: do_say( ch, "Чувствуешь удачу, шпана?");
                break;
        case 4: do_say( ch, "В этот раз стражники тебе не помогут!");
                break;        
        case 5: do_say( ch, "Время отправиться к предкам, уродина.");
                break;
        case 6: do_say( ch, "Ну, понеслась Варда по кочкам.");
                break;
    }

    multi_hit( ch, victim );
    return true;
}

bool spec_ogre_member( NPCharacter *ch)
{
    Character *vch, *victim = 0;
    int count = 0;

    if (!IS_AWAKE(ch) || IS_AFFECTED(ch,AFF_CALM) || ch->in_room == 0
    ||  IS_CHARMED(ch) || ch->fighting != 0)
        return false;

    /* find an troll to beat up */
    for (vch = ch->in_room->people;  vch != 0;  vch = vch->next_in_room)
    {
        if (!vch->is_npc() || ch == vch)
            continue;

        if (vch->getNPC()->pIndexData->vnum == MOB_VNUM_PATROLMAN)
            return false;

        if (vch->getNPC()->pIndexData->group == GROUP_VNUM_TROLLS
        &&  ch->getModifyLevel() > vch->getModifyLevel() - 2 && !is_safe(ch,vch))
        {
            if (number_range(0,count) == 0)
                victim = vch;

            count++;
        }
    }

    if (victim == 0)
        return false;

    /* say something, then raise hell */
    switch (number_range(0,6))
    {
        default: break;
        case 0: do_yell( ch, "Я найду тебя, шпана!");
                break;
        case 1: oldact("Яростно вскрикнув, $c1 бросается на $C4.",ch,0,victim,TO_ALL);
                break;
        case 2: do_say( ch, "Что ты тут забыло, тролльское отродье?");
                break;
        case 3: do_say( ch, "Чувствуешь удачу, шпана?");
                break;
        case 4: do_say( ch, "В этот раз стражники тебе не помогут!");
                break;        
        case 5: do_say( ch, "Время отправиться к предкам, уродина.");
                break;
        case 6: do_say( ch, "Ну, понеслась Варда по кочкам.");
                break;
    }

    multi_hit( ch, victim , "murder" );
    return true;
}

bool spec_patrolman(NPCharacter *ch)
{
    Character *vch,*victim = 0;
    Object *obj;
    int count = 0;

    if (!IS_AWAKE(ch) || IS_AFFECTED(ch,AFF_CALM) || ch->in_room == 0
    ||  IS_CHARMED(ch) || ch->fighting != 0)
        return false;

    /* look for a fight in the room */
    for (vch = ch->in_room->people; vch != 0; vch = vch->next_in_room)
    {
        if (vch == ch)
            continue;

        if (vch->fighting != 0)  /* break it up! */
        {
            if (number_range(0,count) == 0)
                victim = ( vch->getModifyLevel() > vch->fighting->getModifyLevel() )
                    ? vch : vch->fighting;
            count++;
        }
    }

    if (victim == 0 || (victim->is_npc() && *victim->getNPC()->spec_fun == *ch->spec_fun))
        return false;

    if (((obj = get_eq_char(ch,wear_neck_1)) != 0
    &&   obj->pIndexData->vnum == OBJ_VNUM_WHISTLE)
    ||  ((obj = get_eq_char(ch,wear_neck_2)) != 0
    &&   obj->pIndexData->vnum == OBJ_VNUM_WHISTLE))
    {
        oldact("Ты со всей силы свистишь в $o4.",ch,obj,0,TO_CHAR);
        oldact_p("$c1 свистит в $o1, {W***ФРРРРРРРРРРРРРРРР***{x",
               ch,obj,0,TO_ROOM,POS_RESTING);

            for ( vch = char_list; vch != 0; vch = vch->next )
            {
            if ( vch->in_room == 0 )
                    continue;

            if (vch->in_room != ch->in_room
            &&  vch->in_room->area == ch->in_room->area)
                    vch->pecho("До тебя доносится пронзительный свист.");
            }
    }

    switch (number_range(0,6))
    {
        default: break;
        case 0: do_yell( ch, "А ну, прекр-р-р-ратить скопление!");
                break;
        case 1: do_say( ch, "Виновато, конечно, общество, но что уж тут поделаешь?");
                break;
        case 2: do_say( ch, "Чертова шпана нас всех в гроб загонит.");
                break;
        case 3: do_yell( ch, "Сержант Петренко. Предъявите ваши документы!");
                break;
        case 4: oldact("$c1 вытаскивает дубинку и принимается за работу.",ch,0,victim,TO_ALL);
                break;
        case 5: oldact("$c1 обреченно вздыхает и продолжает останавливать драку.",ch,0,victim,TO_ALL);
                break;
        case 6: do_say( ch, "А ну угомонились, хулиганье!");
                break;
    }

    multi_hit( ch , victim , "murder" );
    return true;
}
        
/*
 * Core procedure for dragons.
 */
bool dragon( Character *ch, const char *spell_name )
{
    Character *victim;
    Character *v_next;

    if ( ch->position != POS_FIGHTING )
        return false;

   for ( victim = ch->in_room->people; victim != 0; victim = v_next)
    {
        v_next = victim->next_in_room;
        if (victim->fighting == ch && number_bits( 3 ) == 0)
            break;
    }

    if (victim == 0 )
        return false;
    
    return spell( SkillManager::getThis( )->lookup( spell_name ), 
                  ch->getModifyLevel( ), ch, victim, FSPELL_NOTRIGGER );
}



/*
 * Special procedures for mobiles.
 */


bool spec_breath_acid( NPCharacter *ch )
{
    return dragon( ch, "acid breath" );
}



bool spec_breath_fire( NPCharacter *ch )
{
    return dragon( ch, "fire breath" );
}



bool spec_breath_frost( NPCharacter *ch )
{
    return dragon( ch, "frost breath" );
}



bool spec_breath_gas( NPCharacter *ch )
{
    if (ch->position != POS_FIGHTING)
        return false;

    return spell( SkillManager::getThis( )->lookup( "gas breath" ), 
                 ch->getModifyLevel( ), ch, ch->in_room );
}



bool spec_breath_lightning( NPCharacter *ch )
{
    return dragon( ch, "lightning breath" );
}



bool spec_breath_any( NPCharacter *ch )
{
    if ( ch->position != POS_FIGHTING )
        return false;

    if ( number_percent() < 50 )
        return false;

    switch (number_range(0,6))
    {
        case 0: return spec_breath_fire( ch );
        case 1: return spec_breath_frost( ch );
        case 2: return spec_breath_lightning( ch );
        case 3: return spec_breath_gas( ch );
        case 4: return spec_breath_acid( ch );
    }

    return false;
}

bool spec_cast_adept( NPCharacter *ch )
{
        Character *victim;
        Character *v_next;
        int sn;

        if ( !IS_AWAKE(ch) )
                return false;

        for ( victim = ch->in_room->people; victim != 0; victim = v_next )
        {
                v_next = victim->next_in_room;
                if ( victim != ch
                        && ch->can_see( victim )
                        && number_bits( 1 ) == 0
                        && !victim->is_npc()
                        && ( victim->getModifyLevel() <= ADEPT_MAX_LEVEL || IS_GHOST( victim ) )
                        && victim->getClan() != clan_battlerager)
                        break;
        }

        if ( victim == 0 )
                return false;
        
        sn = -1;
        
        if (victim->isAffected(gsn_plague))    
                sn = gsn_cure_disease;
        else if (victim->isAffected(gsn_blindness))
                sn = gsn_cure_blindness;
        else if (victim->isAffected(gsn_poison))
                sn = gsn_cure_poison;
        else if (victim->isAffected(gsn_curse))
                sn = gsn_remove_curse;
        else if (!victim->isAffected(gsn_armor))
                sn = gsn_armor;
        else if (!victim->isAffected(gsn_bless) && !victim->isAffected(gsn_warcry))
                sn = gsn_bless;
        else if ( (victim->hit < victim->max_hit) && (victim->move < victim->max_move))
        {
                if ( number_percent() < 50 )
                    sn = gsn_heal;
                else
                    sn = gsn_refresh;
        } 
        else if (victim->hit < victim->max_hit) 
            sn = gsn_heal;
        else if ( victim->move < victim->max_move )
            sn = gsn_refresh;
        
        return spell( sn, ch->getModifyLevel( ), ch, victim, FSPELL_VERBOSE );
}


bool spec_cast_judge( NPCharacter *ch )
{
    Character *victim;
    Character *v_next;

    if ( ch->position != POS_FIGHTING )
        return false;

    for ( victim = ch->in_room->people; victim != 0; victim = v_next )
    {
        v_next = victim->next_in_room;
        if ( victim->fighting == ch && number_bits( 2 ) == 0 )
            break;
    }

    if (victim == 0)
        return false;

    return spell( SkillManager::getThis( )->lookup( "high explosive" ), 
                           ch->getModifyLevel( ), ch, victim, FSPELL_BANE );
}


bool spec_executioner( NPCharacter *ch )
{
        Character *victim;
        Character *v_next;
        const char *crime;

        if ( !IS_AWAKE(ch) || ch->fighting != 0 )
                return false;

        crime = "";

        for ( victim = ch->in_room->people; victim != 0; victim = v_next )
        {
                v_next = victim->next_in_room;

                if ( !victim->is_npc()
                        && ( IS_SET(victim->act, PLR_WANTED)
                                || victim->isAffected(gsn_jail ) )
                        && ch->can_see(victim))
                {
                        crime = "НАРУШИТЕЛЬ ЗАКОНА";
                        break;
                }
        }

        if ( victim == 0 )
                return false;

        DLString msg = fmt(0, "%s -- %s!  ЗАЩИЩАЙ НЕВИННЫХ! БОЛЬШЕ КРОВИ!!!",
                victim->getNameC(), crime );

        do_yell( ch, msg.c_str() );

        multi_hit( ch, victim , "murder" );

        return true;
}

                        

bool spec_fido( NPCharacter *ch )
{
    Object *corpse;
    Object *c_next;

    if ( !IS_AWAKE(ch) )
        return false;

    for ( corpse = ch->in_room->contents; corpse != 0; corpse = c_next )
    {
        c_next = corpse->next_content;
        if ( corpse->item_type != ITEM_CORPSE_NPC ||
             corpse->item_type == ITEM_CORPSE_PC )
            continue;

        oldact_p("$c1 с жадностью раздирает труп на куски.",
                ch, 0, 0, TO_ROOM,POS_RESTING);

        obj_dump_content(corpse);

        extract_obj( corpse );

        return true;
    }

    return false;
}


bool spec_janitor( NPCharacter *ch )
{
    Object *trash;
    Object *trash_next;

    if (!IS_AWAKE(ch))
        return false;

    for ( trash = ch->in_room->contents; trash != 0; trash = trash_next )
    {
        trash_next = trash->next_content;

        if (!IS_SET( trash->wear_flags, ITEM_TAKE ) || !trash->getOwner().empty())
            continue;

        if (count_obj_list( trash->pIndexData, ch->carrying ) >= 10)
            continue;
        
        if (trash->cost >= 1500)
            continue;

        if (trash->item_type != ITEM_DRINK_CON && trash->item_type != ITEM_TRASH)
            continue;

        if (trash->behavior)
            continue;

        if (chance( 33 ))
            continue;
        
        oldact("$c1 поднимает с пола какой-то мусор.", ch, 0, 0, TO_ROOM);
        obj_from_room( trash );
        obj_to_char( trash, ch );
        return true;
    }

    return false;
}



bool spec_mayor( NPCharacter *ch )
{
    static const char open_path[] =
        "W3a3003b000c000d111Oe333333Oe22c222112212111a1S.";

    static const char close_path[] =
        "W3a3003b000c000d111CE333333CE22c222112212111a1S.";

    static const char *path;
    static int pos;
    static bool move;
    static Room *cabinet = 0;
    Room *room;

    if ( !move )
    {
        if ( time_info.hour ==  6 )
        {
            path = open_path;
            move = true;
            pos  = 0;
        }

        if ( time_info.hour == 20 )
        {
            path = close_path;
            move = true;
            pos  = 0;
        }
    }

    if (!move || (ch->position < POS_STANDING && path[pos] != 'W'))
        return false;

    if (!cabinet)
        cabinet = get_room_instance( 3138 );

    room = cabinet; 
    
    for (int i = 0; i < pos; i++) {
        int door = path[i] - '0';
        
        if (door < 0 || door > 3)
            continue;
        
        if (!room->exit[door]) {
            return false;
        }
        
        if (!room->exit[door]->u1.to_room) {
            return false;
        }
        
        room = room->exit[door]->u1.to_room;
    }

    if (room != ch->in_room) {
        LogStream::sendNotice( ) << "Mayor: has to be in  " << room->getName() 
                                 << " while he is in " << ch->in_room->getName() 
                                 << " pos[ " << pos << "] = " << path[pos] << endl;
        
        transfer_char( ch, ch, room,
                      "%1$^C1 вынимает часы из жилетного кармана и восклицает: '{gАх, боже мой! Я опаздываю.{x'"
                      "%1$^C1 убегает с озабоченным видом." );
        LogStream::sendNotice( ) << "Mayor: now in room " << ch->in_room->getName() << endl;
    }

    switch ( path[pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
        move_char( ch, path[pos] - '0' );
        break;

    case 'W':
        ch->position = POS_STANDING;
        oldact_p("$c1 просыпается и издает громкий стон.",
                ch, 0, 0, TO_ROOM,POS_RESTING );
        break;

    case 'S':
        ch->position = POS_SLEEPING;
        oldact_p("$c1 ложится и засыпает.",
                ch, 0, 0, TO_ROOM,POS_RESTING );
        break;

    case 'a':
        do_say( ch, "Привет, дорогуша!" );
        break;

    case 'b':
        do_say(ch, "Ну и зрелище! Я просто обязан что-то сделать с этой свалкой!");
        break;

    case 'c':
        do_say(ch," Вандалы! Эта молодежь ни к чему не питает уважения!");
        break;

    case 'd':
        do_say(ch,"Добрый день, горожане!");
        break;

    case 'e':
        do_say(ch,"Настоящим обьявляю ворота Мидгаарда открытыми!");
        break;

    case 'E':
        break;

    case 'O':
        interpret( ch, "emote широко распахивает ворота Мидгаарда.");
        break;

    case 'C':
        break;

    case '.' :
        move = false;
        break;
    }

    pos++;
    return false;
}



bool spec_poison( NPCharacter *ch )
{
    Character *victim = 0;

    if ( ch->position != POS_FIGHTING
    || ( victim = ch->fighting ) == 0
    || victim->in_room != ch->in_room
    ||   number_percent( ) > 2 * ch->getModifyLevel() )
        return false;

    oldact("Ты кусаешь $C4!",  ch, 0, victim, TO_CHAR);
    oldact("$c1 кусает $C4!",  ch, 0, victim, TO_NOTVICT);
    oldact("$c1 кусает тебя!", ch, 0, victim, TO_VICT);

    spell( gsn_poison, ch->getModifyLevel( ), ch, victim, FSPELL_NOTRIGGER ); 
    return true;
}

bool spec_thief( NPCharacter *ch )
{
    Character *victim;
    Character *v_next;
    int gold,silver;

    if ( ch->position != POS_STANDING )
        return false;

    for ( victim = ch->in_room->people; victim != 0; victim = v_next )
    {
        v_next = victim->next_in_room;

        if ( victim->is_npc()
        ||   victim->getRealLevel( ) >= LEVEL_IMMORTAL
        ||   number_bits( 5 ) != 0
        ||   !ch->can_see(victim))
            continue;

        if ( IS_AWAKE(victim) && number_range( 0, ch->getModifyLevel() ) == 0 )
        {
            oldact_p("$c1 пытается ограбить тебя!",
                ch, 0, victim, TO_VICT,POS_RESTING );
            oldact_p("$C1 пытается ограбить $c4!",
                victim, 0, ch, TO_NOTVICT,POS_RESTING );
            return true;
        }
        else
        {
            gold = victim->gold * min(number_range(1,20), ch->getModifyLevel() / 2) / 100;
            gold = min(gold, ch->getModifyLevel() * ch->getModifyLevel() * 10 );
            ch->gold     += gold;
            victim->gold -= gold;
            silver = victim->silver * min(number_range(1,20), ch->getModifyLevel() /2)/100;
            silver = min(silver, ch->getModifyLevel() * ch->getModifyLevel() * 25);
            ch->silver        += silver;
            victim->silver -= silver;
            return true;
        }
    }

    return false;
}

bool spec_guard( NPCharacter *ch )
{
    return false;
}

bool spec_nasty( NPCharacter *ch )
{
    Character *victim, *v_next;
    long gold;

    if (!IS_AWAKE(ch)) {
       return false;
    }

    if (ch->position != POS_FIGHTING) {
       for ( victim = ch->in_room->people; victim != 0; victim = v_next)
       {
          v_next = victim->next_in_room;
          if (!victim->is_npc()
             && ( victim->getModifyLevel() > ch->getModifyLevel() )
             && ( victim->getModifyLevel() < ch->getModifyLevel() + 10))
          {
             interpret_raw( ch, "backstab", victim->getNameC() );
             if (ch->position != POS_FIGHTING)
                 interpret_raw( ch, "murder", victim->getNameC() );
             /* should steal some coins right away? :) */
             return true;
          }
       }
       return false;    /*  No one to attack */
    }

    /* okay, we must be fighting.... steal some coins and flee */
    if ( (victim = ch->fighting) == 0)
        return false;   /* let's be paranoid.... */

    switch (number_bits(2)) {
    case 0:
        if (ch->can_see(victim)) {
            /* steal 10% of his gold */                
            if (victim->gold > 10)
                gold = victim->gold / 10;
            else
                gold = victim->gold;

            if (gold > 0) {
                victim->gold -= gold;
                ch->gold += gold;

                oldact_p("$c1 разрезает твой кошелек и тянет оттуда золотые монетки!",
                    ch, 0, victim, TO_VICT, POS_RESTING);
                oldact_p("Ты разрезаешь кошелек $C2 и крадешь золото.",
                    ch, 0, victim, TO_CHAR, POS_RESTING);
                oldact_p("БА! Да у $C2 выпотрошили кошелек!",
                    ch, 0, victim, TO_NOTVICT, POS_RESTING);
            }
            
            return true;
        }

        /* FALLTHROUGH */
    case 1:
        interpret_raw(ch, "flee");
        return true;

    default:
        return false;
    }
}

bool spec_assassinater( NPCharacter *ch )
{
        DLString msg;
        Character *victim;
        int rnd_say;

        if (!IS_AWAKE( ch ))
            return false;

        if ( ch->fighting != 0 )
                return false;

        for (victim = ch->in_room->people; victim != 0; victim = victim->next_in_room)
            if (victim != ch
                && !victim->is_npc( )
                && !victim->is_immortal( )
                && victim->getProfession( ) != prof_thief
                && victim->getProfession( ) != prof_ninja)
                break;

        if (victim == 0)
                return false;

        if (victim->getModifyLevel() > ch->getModifyLevel() + 7)
                return false;
        if ( victim->hit < victim->max_hit )
                return false;

        rnd_say = number_range (1, 40);

        switch (rnd_say)
        {
        case  5:
                msg = "Смерть -- вот верное завершение твоего пути...";
                break;
        case  6:
                msg = "Настало время умирать....";
                break;
        case  7:
                msg = "Подонок...";
                break;
        case  8:
                msg = "Настало время встретить свой рок...";
                break;
        case  9:
                msg = "Жертвоприношение Богам... ";
                break;
        case 10:
                msg = "Случалось ли тебе танцевать с дьяволом?";
                break;
        default:
                return false;
        }

        do_say( ch, msg.c_str() );
        interpret_raw( ch, "assassinate", victim->getNameC( ));
        return true;
}


bool spec_captain( NPCharacter *ch )
{

    static const char open_path[] =
"Wn0onc0oe1f2212211s2tw3xw3xd3322a22b22yO00d00a0011e1fe1fn0o3300300w3xs2ts2tS.";

    static const char close_path[] =
"Wn0on0oe1f2212211s2twc3xw3x3322d22a22EC0a00d0b0011e1fe1fn0o3300300w3xs2ts2tS.";

    static const char *path;
    static int pos;
    static bool move;

    if ( !move )
    {
        if ( time_info.hour ==  6 )
        {
            path = open_path;
            move = true;
            pos  = 0;
        }

        if ( time_info.hour == 20 )
        {
            path = close_path;
            move = true;
            pos  = 0;
        }
    }

    if ( ch->fighting != 0 )
        return false;

    if ( !move || (ch->position < POS_STANDING && path[pos] != 'W'))
        return false;

    switch ( path[pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
        move_char( ch, path[pos] - '0' );
        break;

    case 'W':
        ch->position = POS_STANDING;
        oldact_p("$c1 резко просыпается и зевает.",
                ch, 0, 0, TO_ROOM,POS_RESTING );
        break;

    case 'S':
        ch->position = POS_SLEEPING;
        oldact("$c1 ложится и засыпает.", ch, 0, 0, TO_ROOM);
        break;

    case 'a':
        do_say(ch, "Приветствую! Удачной вам охоты!");
        break;

    case 'b':
        do_say( ch, "Пожалуйста, поддерживайте порядок на улицах. Содержите Утеху в чистоте.");
        break;

    case 'c':
        do_say(ch, "Я должен что-то сделать со всеми этими дверьми.");
        do_say(ch, "Я так никогда отсюда не выберусь."); 
        break;

    case 'd':
        do_say(ch, "Приветствую вас, жители Утехи!");
        break;

    case 'y':
        do_say(ch, "Настоящим объявляю город Утеху открытым!");
        break;

    case 'E':
        do_say(ch, "Настоящим объявляю город Утеху закрытым!");
        break;

    case 'O':
        interpret_raw( ch, "unlock", "gate" );
        interpret_raw( ch, "open", "gate" );
        break;

    case 'C':
        interpret_raw( ch, "close", "gate" );
//        interpret_raw( ch, "lock", "gate" );
        break;

    case 'n':
        interpret_raw( ch, "open", "north" );
        break;

    case 'o':
        interpret_raw( ch, "close", "south" );
        break;

    case 's':
        interpret_raw( ch, "open", "south" );
        break;

    case 't':
        interpret_raw( ch, "close", "north" );
        break;

    case 'e':
        interpret_raw( ch, "open", "east" );
        break;

    case 'f':
        interpret_raw( ch, "close", "west" );
        break;

    case 'w':
        interpret_raw( ch, "open", "west" );
        break;

    case 'x':
        interpret_raw( ch, "close", "east" );
        break;

    case '.' :
        move = false;
        break;
    }

    pos++;
    return false;
}


/* 
 * The function table 
 */
struct  spec_type    local_spec_table[] =
{
    {        "spec_breath_any",                spec_breath_any                },
    {        "spec_breath_acid",                spec_breath_acid        },
    {        "spec_breath_fire",                spec_breath_fire        },
    {        "spec_breath_frost",                spec_breath_frost        },
    {        "spec_breath_gas",                spec_breath_gas                },
    {        "spec_breath_lightning",        spec_breath_lightning        },        
    {        "spec_cast_adept",                spec_cast_adept                },
    {        "spec_cast_judge",                spec_cast_judge                },
    {        "spec_executioner",                spec_executioner        },
    {        "spec_fido",                        spec_fido                },
    {        "spec_guard",                        spec_guard                },
    {        "spec_janitor",                        spec_janitor                },
    {        "spec_mayor",                        spec_mayor                },
    {        "spec_poison",                        spec_poison                },
    {        "spec_thief",                        spec_thief                },
    {        "spec_nasty",                        spec_nasty                },
    {        "spec_troll_member",                spec_troll_member        },
    {        "spec_ogre_member",                spec_ogre_member        },
    {        "spec_patrolman",                spec_patrolman                },
    {   "spec_assassinater",            spec_assassinater        },
    {        "spec_captain",                        spec_captain                },
    {        0,                                0                        }
};

