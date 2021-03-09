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
 *  This file is a combination of:                                         *
 *                (H)unt.c, (E)nter.c, (R)epair.c and (A)uction.c            *
 *  Thus it is called ACT_HERA.C                                           *
 **************************************************************************/
 
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


#include "skill.h"
#include "spell.h"

#include "char.h"
#include "commandtemplate.h"
#include "affect.h"
#include "room.h"

#include "dreamland.h"
#include "merc.h"
#include "descriptor.h"
#include "ban.h"
#include "act.h"
#include "gsn_plugin.h"
#include "mercdb.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "race.h"
#include "object.h"

#include "wiznet.h"
#include "handler.h"
#include "weapons.h"
#include "act_move.h"
#include "vnum.h"
#include "def.h"

/***************************************************************************
 ************************      auction.c      ******************************
 ***************************************************************************/

#define PULSE_AUCTION             (45 * dreamland->getPulsePerSecond( )) /* 60 seconds */
#define AUC_TIMER_CUTOFF          24

static bool buyer_can_trade()
{
    if (auction->buyer->in_room
        && IS_SET(auction->buyer->in_room->areaIndex()->area_flag, AREA_DUNGEON))
        return false;

    return true;
}

void talk_auction(const char *argument)
{
    DLString msg_en = DLString("{YAUCTION: ") + argument + "{x";
    DLString msg_ru = DLString("{YАУКЦИОН: ") + argument + "{x";
    Descriptor *d;

    for (d = descriptor_list; d != 0; d = d->next) {
        if (d->connected != CON_PLAYING)
            continue;

        Character *ch = d->character;

        if (!ch)
            continue;

        if (IS_SET(ch->getPC( )->comm, COMM_NOAUCTION))
            continue;
    
        bool fRussian = ch->getConfig().rucommands;
        ch->pecho(POS_SLEEPING, fRussian ? msg_ru.c_str() : msg_en.c_str());
    }
}


/*
  This function allows the following kinds of bets to be made:

  Absolute bet
  ============

  bet 14

  Relative bet
  ============

  These bets are calculated relative to the current bet. The '+' symbol adds
  a certain number of percent to the current bet. The default is 25, so
  with a current bet of 1000, bet + gives 1250, bet +50 gives 1500 etc.
  Please note that the number must follow exactly after the +, without any
  spaces!

  The '*' or 'x' bet multiplies the current bet by the number specified,
  defaulting to 2. If the current bet is 1000, bet x  gives 2000, bet x10
  gives 10,000 etc.

*/

int parsebet (const int currentbet, const char *argument)
{
        int newbet = 0;                /* a variable to temporarily hold the new bet */
        char string[MAX_INPUT_LENGTH]; /* a buffer to modify the bet string */
        char *stringptr = string;      /* a pointer we can move around */
        char buf2[MAX_STRING_LENGTH];

        strcpy (string,argument);      /* make a work copy of argument */

        if (*stringptr)               /* check for an empty string */
        {
                if (isdigit (*stringptr)) /* first char is a digit assume e.g. 433k */
                        newbet = atoi(stringptr); /* parse and set newbet to that value */
                else if (*stringptr == '+') /* add ?? percent */
                {
                        if (strlen (stringptr) == 1) /* only + specified, assume default */
                                newbet = (currentbet * 125) / 100; /* default: add 25% */
                        else
                                newbet = (currentbet * (100 + atoi (++stringptr))) / 100; /* cut off the first char */
                }
                else
                {
                        sprintf (buf2,"considering: * x \n\r");
                        if ((*stringptr == '*') || (*stringptr == 'x')) /* multiply */
                        {
                                if (strlen (stringptr) == 1) /* only x specified, assume default */
                                        newbet = currentbet * 2 ; /* default: twice */
                                else /* user specified a number */
                                        newbet = currentbet * atoi (++stringptr); /* cut off the first char */
                        }
                }
        }

        return newbet;        /* return the calculated bet */
}

void auction_update (void)
{
    char buf[MAX_STRING_LENGTH];
    if (auction->item != 0)
        if (--auction->pulse <= 0) /* decrease pulse */
        {
            auction->pulse = PULSE_AUCTION;
            switch (++auction->going) /* increase the going state */
            {
            case 1 : /* going once */
            case 2 : /* going twice */
            if (auction->bet > 0)
            {
                talk_auction(fmt(0, "%1$O1{Y: буд%1$nет|ут прода%1$Gно|н|на|ны за %2$d золот%3$s - %4$s{x.", 
                        auction->item,
                        auction->bet,
                        GET_COUNT(auction->bet,"ую монету","ые монеты","ых монет"),
                        ((auction->going == 1) ? "раз" : "два")).c_str( ));
                break;
            }
            else
            {
                sprintf (buf, "%s{Y: ставок не получено - %s{x.", auction->item->getShortDescr( '1' ).c_str( ),
                     ((auction->going == 1) ? "раз" : "два"));
                talk_auction (buf);
                if (auction->startbet != 0)
                {
                  sprintf(buf, "Начальная цена: %d золот%s{x.", auction->startbet,
                                GET_COUNT(auction->startbet,"ая монета","ые монеты","ых монет"));
                  talk_auction(buf);
                }
                break;
            }
            case 3 : /* SOLD! */

            if (auction->bet > 0 && buyer_can_trade())
            {
                sprintf (buf, "%s получает %s{Y за %d золот%s{x.",
                    auction->buyer->getNameP( '1' ).c_str( ),
                    auction->item->getShortDescr( '4' ).c_str( ), auction->bet,
                    GET_COUNT(auction->bet,"ую монету","ые монеты","ых монет"));
                talk_auction(buf);
                obj_to_char (auction->item,auction->buyer);
                oldact_p("Из дымки появляется аукционер и передает тебе $o4.",                     auction->buyer,auction->item,0,TO_CHAR,POS_DEAD);
                oldact("$c1 получает от прибывшего аукционера $o4.",                     auction->buyer,auction->item,0,TO_ROOM);

                auction->seller->gold += auction->bet; /* give him the money */
                oldact_p("Из дымки появляется аукционер и передает тебе вырученные деньги.",                     auction->seller,auction->item,0,TO_CHAR,POS_DEAD);
                act("%^C1 получает вырученные деньги от прибывшего аукционера.",                      auction->seller, 0, auction->item,TO_ROOM);

                auction->item = 0; /* reset item */
                auction->seller = 0;

            }
            else /* not sold */
            {
                DLString msg = fmt(0, "Ставок не получено - %1$#O1{Y снят%1$Gо||а|ы с аукциона{x.", auction->item);
                talk_auction(msg.c_str());

                oldact_p("Из дымки перед тобой появляется аукционер и возвращает тебе {W$o4{w.",                      auction->seller,auction->item,0,TO_CHAR,POS_DEAD);
                oldact("Аукционер появляется перед $c5 и возвращает $m {W$o4{w.",                      auction->seller,auction->item,0,TO_ROOM);
                obj_to_char (auction->item,auction->seller);
                auction->item = 0; /* clear auction */
                auction->seller = 0;
            } /* else */

            } /* switch */
        } /* if */
} /* func */


CMDRUNP( auction )
{
        Object *obj;
        char arg1[MAX_INPUT_LENGTH];
        char buf[MAX_STRING_LENGTH];
        char betbuf[MAX_STRING_LENGTH];
        argument = one_argument (argument, arg1);

        if (ch->is_npc())    /* NPC extracted can't auction! */
                return;

        if (IS_SET(ch->comm,COMM_NOAUCTION))
        {
                if (arg_is_switch_on( arg1 ))
                {
                        ch->pecho("Канал Аукциона (Auction) теперь {Rвключен{x.");
                        REMOVE_BIT(ch->comm,COMM_NOAUCTION);
                        return;
                }
                else
                {
                        ch->pecho("Канал Аукциона (Auction) теперь {Rвыключен{x.");
                        ch->pecho("Для получения информации по этому каналу включите его.");
                        return;
                }
        }

        if (arg1[0] == '\0')
        {
                if (auction->item != 0)
                {
                        if ( ch->is_immortal() )
                        {
                                sprintf(buf,"Продавец: %s Текущая ставка: %s\n\r",
                                        auction->seller->getNameP(),
                                        auction->buyer ? auction->buyer->getNameP() : "Нет");
                                ch->send_to(buf);
                        }
                        /* show item data here */
                        if (auction->bet > 0)
                        {
                                sprintf (buf, "Текущая ставка на выставленный лот - %d золот%s{x.\n\r",
                                        auction->bet,
                                        GET_COUNT(auction->bet,"ая монета","ые монеты","ых монет"));
                                ch->send_to( buf);
                        }
                        else
                        {
                                sprintf (buf, "Ставок на выставленный лот не получено{x.\n\r");
                                ch->send_to( buf);
                                if (auction->startbet != 0)
                                {
                                        sprintf(buf, "Начальная цена: %d золот%s{x.\n\r", auction->startbet,
                                                GET_COUNT(auction->startbet,"ая монета","ые монеты","ых монет"));
                                        ch->send_to( buf);
                                }
                        }

                        obj = auction->item;
                        if ( ch->is_immortal() )
                        {
                            if (gsn_identify->getSpell( ))
                                gsn_identify->getSpell( )->run( ch, auction->item, gsn_identify, 0 );
                            return;
                        }
                        sprintf( buf,
                                "Лот: '%s{x'. Тип: %s. Экстра флаги: %s.\n\rВес: %d. Стоимость: %d. Уровень: %d.\n\r",
                                obj->getShortDescr( '1' ).c_str( ),
                                item_table.message(obj->item_type).c_str( ), 
                                extra_flags.messages( obj->extra_flags, true).c_str( ),
                                obj->weight / 10, obj->cost, obj->level );
                        ch->send_to( buf);

                        {        
                            map<DLString, bool> purposes;
                            map<DLString, bool>::iterator p;

                            for (int i = 0; i < wearlocationManager->size( ); i++) {
                                Wearlocation *loc = wearlocationManager->find( i );
                                if (loc->matches( obj ) && !loc->getPurpose().empty())
                                    purposes[loc->getPurpose( )] = true;
                            }
                            for (p = purposes.begin( ); p != purposes.end( ); p++)
                                ch->pecho( p->first.c_str( ) );
                        }

                        if  (obj->item_type == ITEM_WEAPON) {
                                ch->printf("Тип оружия: %s (%s), среднее повреждение %d.\r\n",
                                           weapon_class.message(obj->value0() ).c_str( ),
                                           weapon_class.name( obj->value0() ).c_str( ),
                                           weapon_ave(obj));
                        }

                        if (obj->timer != 0) {
                            ch->pecho("{WЭтот предмет исчезнет через %1$d мину%1$Iту|ты|т после продажи.{x", obj->timer);
                        }

                        return;
                }
                else
                {
                        ch->pecho("{RВыставить на Аукцион ЧТО{x?");
                        return;
                }
        }

        if (arg_is_switch_off( arg1 ))
        {
                ch->pecho("Канал Аукциона (Auction) теперь {Rвыключен{x.");
                SET_BIT(ch->comm,COMM_NOAUCTION);
                return;
        }
        
        if (arg_oneof_strict( arg1, "talk", "реклама", "говорить" ))
        {
            if ( ch != auction->seller ) {
                ch->pecho("Ты ничего не выставлял%Gо||а на аукцион - рекламировать тебе нечего.", ch);
                return;
            }
            if (argument[0] == '\0') {
                ch->pecho("Как ты хочешь разрекламировать товар?");
                return;
            }
            
            REMOVE_BIT(ch->comm,COMM_NOAUCTION);
            talk_auction( argument );
            return;
        }
        
        if (ch->is_immortal() && arg_oneof_strict( arg1, "stop", "стоп" ))
        {
                if (auction->item == 0)
                {
                        ch->pecho("На аукцион ничего не выставлено. Будь внимательней!");
                        return;
                }
                else /* stop the auction */
                {
                        sprintf(buf,"Продажа остановлена Богами. Лот '%s{Y' конфискован{x.",
                                auction->item->getShortDescr( '1' ).c_str( ));
                        talk_auction(buf);
                        obj_to_char(auction->item, auction->seller);
                        auction->item = 0;
                        auction->seller = 0;
                        if (auction->buyer != 0) /* return money to the buyer */
                        {
                                auction->buyer->gold += auction->bet;
                                auction->buyer->pecho("Твои деньги возвращены.");
                        }
                        return;
                }
        }

        if (arg_oneof_strict( arg1, "bet", "ставка" ))
        {
                if (auction->item != 0)
                {
                        int newbet;

                        if ( ch == auction->seller )
                        {
                                ch->pecho("Ты не можешь купить свой же лот..:)");
                                return;
                        }

                        if (auction->item->pIndexData->limit != -1 && auction->item->isAntiAligned(ch)) {
                            ch->pecho("Твоя натура не позволит тебе владеть этим предметом.");
                            return;
                        }

                        /* make - perhaps - a bet now */
                        if (argument[0] == '\0')
                        {
                                ch->pecho("Ставка (Bet) сколько?");
                                return;
                        }

                        newbet = parsebet (auction->bet, argument);
                        sprintf (betbuf,"Ставка: %d\n\r",newbet);

                        if ((auction->startbet != 0) && (newbet < (auction->startbet + 1)))
                        {
                                ch->pecho("Тебе необходимо повысить ставку хотя бы на 1 золотой выше начальной цены.");
                                return;
                        }

                        if (newbet < (auction->bet + 1))
                        {
                                ch->pecho("Тебе необходимо повысить ставку хотя бы на 1 золотой выше текущей ставки.");
                                return;
                        }

                        if (newbet > ch->gold)
                        {
                                ch->pecho("У тебя нет необходимой суммы!");
                                return;
                        }

                        /* the actual bet is OK! */

                        /* return the gold to the last buyer, if one exists */
                        if (auction->buyer != 0)
                                auction->buyer->gold += auction->bet;

                        ch->gold -= newbet; /* substract the gold - important :) */
                        auction->buyer = ch;
                        auction->bet   = newbet;
                        auction->going = 0;
                        auction->pulse = PULSE_AUCTION; /* start the auction over again */

                        sprintf( buf, "На %s{Y получена новая ставка: %d золот%s{x.\n\r",
                                auction->item->getShortDescr( '4' ).c_str( ),newbet,
                                GET_COUNT(newbet,"ая монета","ые монеты","ых монет"));
                        talk_auction( buf );
                        return;

                }
                else
                {
                        ch->pecho("В данный момент на аукцион ничего не выставлено.");
                        return;
                }
        }

        /* finally... */

        obj = get_obj_carry (ch, arg1 ); /* does char have the item ? */

        if (obj == 0)
        {
                ch->pecho("У тебя нет этого.");
                return;
        }

        if (obj->timer != 0 && obj->timer < AUC_TIMER_CUTOFF)
        {
                ch->pecho("Этот предмет не может быть выставлен на аукцион, т.к. он исчезнет всего через %1$d минут%1$Iу|ы|.", obj->timer);
                return;
        }

        if (IS_OBJ_STAT(obj, ITEM_NOSELL)) {
            ch->pecho("Этот предмет не подлежит продаже.");
            return;
        }

        if (auction->item == 0) {
                if (ch->desc && banManager->check( ch->desc, BAN_COMMUNICATE )) {
                    ch->pecho( "Ты не можешь ничего выставлять на аукцион." );
                    return;
                }

                switch (obj->item_type)
                {
                case ITEM_MONEY:
                case ITEM_CORPSE_PC:
                case ITEM_CORPSE_NPC:
                case ITEM_TATTOO:
                        oldact_p("Ты не можешь выставить на аукцион $T.", ch, 0, item_table.message(obj->item_type).c_str( ),TO_CHAR,POS_SLEEPING);
                        return;
                default:
                        obj_from_char (obj);
                        auction->item = obj;
                        auction->bet = 0;         /* obj->cost / 100 */
                        auction->startbet = parsebet (auction->startbet, argument);
                        auction->buyer = 0;
                        auction->seller = ch;
                        auction->pulse = PULSE_AUCTION;
                        auction->going = 0;

                        sprintf(buf, "На аукцион выставлен новый лот: %s{x.",
                                obj->getShortDescr( '1' ).c_str( ));
                        talk_auction( buf );
                        if (auction->startbet == 0)
                        {
                                sprintf(buf, "Начальная цена владельцем не установлена{x.");
                                talk_auction( buf );
                        }
                        else
                        {
                                sprintf(buf, "Начальная цена: %d золот%s{x.", auction->startbet,
                                        GET_COUNT(auction->startbet,"ая монета","ые монеты","ых монет"));
                                talk_auction( buf );
                        }
                        wiznet( 0, 0, 0, "Продавец - %C1", ch );
                        return;

                } /* switch */
        }
        else
        {
                act("Попробуй позже! Кто-то другой уже выставил на аукцион %3$O4!",  ch, 0, auction->item,TO_CHAR);
                return;
        }
}

