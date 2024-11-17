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

#include "skill.h"
#include "spell.h"


#include "commandtemplate.h"
#include "affect.h"
#include "room.h"

#include "dreamland.h"
#include "merc.h"
#include "descriptor.h"
#include "ban.h"
#include "act.h"


#include "npcharacter.h"
#include "pcharacter.h"
#include "race.h"
#include "object.h"
#include "dlscheduler.h"
#include "wiznet.h"
#include "loadsave.h"
#include "weapons.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "vnum.h"
#include "def.h"
#include "messengers.h"

GSN(identify);

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

static int parsebet (const int currentbet, const DLString &constArgument)
{
    int newbet = 0;          
    DLString args(constArgument);
    args.stripWhiteSpace();

    try {
        if (args.size() > 0) {
            if (isdigit(args.at(0))) {
                newbet = args.toInt();       

            } else if (args.at(0) == '+') {
                int percent = 25;

                if (args.size() > 1)
                    percent = DLString(args.substr(1)).toInt();

                newbet = (currentbet * (100 + percent)) / 100; 

            } else if (args.at(0) == '*' || args.at(0) == 'x') {
                int coef = 2;

                if (args.size() > 1)
                    coef = DLString(args.substr(1)).toInt();

                newbet = currentbet * coef;
            }
        }
        
    } catch (const ExceptionBadType &e) {
        newbet = 0;
    }

    return newbet;    
}

void auction_update (void)
{
    DLString msg;
    
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
                talk_auction(fmt(0, "%1$O1{Y: буд%1$nет|ут прода%1$Gно|н|на|ны за %2$d золот%2$Iую|ых|ых монет%2$Iу|ы| -- %3$s{x.", 
                        auction->item,
                        auction->bet,
                        ((auction->going == 1) ? "раз" : "два")).c_str( ));
                break;                
            }
            else
            {
                talk_auction(fmt(0, "%s{Y: ставок не получено -- %s{x.", 
                        auction->item->getShortDescr( '1', LANG_DEFAULT ).c_str( ),
                        ((auction->going == 1) ? "раз" : "два")).c_str());

                if (auction->startbet != 0)
                {
                  talk_auction(fmt(0, "Начальная цена: %d золот%s{x.", 
                                auction->startbet,
                                GET_COUNT(auction->startbet,"ая монета","ые монеты","ых монет")).c_str());
                }
                break;
            }
            case 3 : /* SOLD! */

            if (auction->bet > 0 && buyer_can_trade())
            {
                msg = fmt(0, "{1{C%1$^C1 {Yпокупает {2%2$O4{1 за %3$d золот%3$Iую|ых|ых монет%3$Iу|ы|{2.",
                    auction->buyer, auction->item, auction->bet);                           
                talk_auction(msg.c_str());
                send_to_discord_stream(":moneybag: " + msg);
                send_telegram(msg);
                obj_to_char (auction->item,auction->buyer);
                oldact_p("Из дымки появляется аукционер и передает тебе $o4.",
                     auction->buyer,auction->item,0,TO_CHAR,POS_DEAD);
                oldact_p("$c1 получает от прибывшего аукционера $o4.",
                     auction->buyer,auction->item,0,TO_ROOM,POS_RESTING);

                auction->seller->gold += auction->bet; /* give him the money */
                oldact_p("Из дымки появляется аукционер и передает тебе вырученные деньги.",
                     auction->seller,auction->item,0,TO_CHAR,POS_DEAD);
                oldact_p("$c1 получает вырученные деньги от прибывшего аукционера.",
                     auction->seller,auction->item,0,TO_ROOM,POS_RESTING);

                auction->item = 0; /* reset item */
                auction->seller = 0;

            }
            else /* not sold */
            {
                msg = fmt(0, "Ставок не получено -- %1$#O1{Y снят%1$Gо||а|ы с аукциона{x.", auction->item);
                talk_auction(msg.c_str());

                oldact_p("Из дымки перед тобой появляется аукционер и возвращает тебе {W$o4{w.",
                      auction->seller,auction->item,0,TO_CHAR,POS_DEAD);
                oldact_p("Аукционер появляется перед $c5 и возвращает $m {W$o4{w.",
                      auction->seller,auction->item,0,TO_ROOM,POS_RESTING);
                obj_to_char (auction->item,auction->seller);
                auction->item = 0; /* clear auction */
                auction->seller = 0;
            } /* else */

            } /* switch */
        } /* if */
} /* func */

class AuctionUpdateTask : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<AuctionUpdateTask> Pointer;

    virtual void run( ) 
    {
        auction_update();
    }

    virtual void after( )
    {
        DLScheduler::getThis( )->putTaskInitiate( Pointer( this ) );  
    }

    virtual int getPriority( ) const
    {
        return SCDP_AUTO + 10;
    }
};

PluginInitializer<AuctionUpdateTask> initAuctionUpdate;

CMDRUNP( auction )
{
        Object *obj;
        char arg1[MAX_INPUT_LENGTH];
        argument = one_argument (argument, arg1);
        DLString msg; 

        if (ch->is_npc())    /* NPC extracted can't auction! */
                return;

        if (IS_SET(ch->comm,COMM_NOAUCTION))
        {
                if (arg_is_switch_on( arg1 ))
                {
                        ch->pecho("Канал Аукциона теперь {Rвключен{x.");
                        REMOVE_BIT(ch->comm,COMM_NOAUCTION);
                        return;
                }
                else
                {
                        ch->pecho("Канал Аукциона теперь {Rвыключен{x.");
                        ch->pecho("Для получения информации по этому каналу включи его.");
                        return;
                }
        }

        if (arg1[0] == '\0')
        {
                if (auction->item != 0)
                {
                        if ( ch->is_immortal() )
                        {
                                ch->pecho("Продавец -- %s, текущая ставка -- %s",
                                        auction->seller->getNameC(),
                                        auction->buyer ? auction->buyer->getNameC() : "Нет");
                        }
                        /* show item data here */
                        if (auction->bet > 0)
                        {
                                ch->pecho("Текущая ставка на выставленный лот -- %d золот%s{x.",
                                        auction->bet,
                                        GET_COUNT(auction->bet,"ая монета","ые монеты","ых монет"));
                        }
                        else
                        {
                                ch->pecho("Ставок на выставленный лот не получено{x.");
                                if (auction->startbet != 0)
                                {
                                        ch->pecho("Начальная цена -- %d золот%s{x.", auction->startbet,
                                                GET_COUNT(auction->startbet,"ая монета","ые монеты","ых монет"));
                                }
                        }

                        obj = auction->item;
                        if ( ch->is_immortal() )
                        {
                            if (gsn_identify->getSpell( ))
                                gsn_identify->getSpell( )->run( ch, auction->item, gsn_identify, 0 );
                            return;
                        }
                        ch->pecho(
                                "Лот: '%s{x'. Тип: %s. Экстра флаги: %s.\n\rВес: %d. Стоимость: %d. Уровень: %d.",
                                obj->getShortDescr( '1', LANG_DEFAULT ).c_str( ),
                                item_table.message(obj->item_type).c_str( ), 
                                extra_flags.messages( obj->extra_flags, true).c_str( ),
                                obj->weight / 10, obj->cost, obj->level );

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
                                ch->pecho("Тип оружия: %s (%s), среднее повреждение %d.",
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
                        ch->pecho("Что ты хочешь выставить на аукцион?");
                        return;
                }
        }

        if (arg_is_switch_off( arg1 ))
        {
                ch->pecho("Канал Аукциона теперь {Rвыключен{x.");
                SET_BIT(ch->comm,COMM_NOAUCTION);
                return;
        }
        
        if (arg_is_strict(arg1, "talk"))
        {
            if ( ch != auction->seller ) {
                ch->pecho("Ты ничего не выставлял%Gо||а на аукцион -- рекламировать тебе нечего.", ch);
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
        
        if (ch->is_immortal() && arg_is_strict(arg1, "stop"))
        {
                if (auction->item == 0)
                {
                        ch->pecho("На аукцион ничего не выставлено. Будь внимательней!");
                        return;
                }
                else /* stop the auction */
                {
                        talk_auction(fmt(0, "Продажа остановлена Богами. Лот '%s{Y' конфискован{x.",
                                auction->item->getShortDescr( '1', LANG_DEFAULT ).c_str( )).c_str());
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

        if (arg_is_strict(arg1, "bet"))
        {
                if (auction->item != 0)
                {
                        int newbet;

                        if ( ch == auction->seller )
                        {
                                ch->pecho("Ты не можешь купить свой же лот.");
                                return;
                        }

                        if (auction->item->pIndexData->limit != -1 && auction->item->isAntiAligned(ch)) {
                            ch->pecho("Твоя натура не позволит тебе владеть этим предметом.");
                            return;
                        }

                        /* make - perhaps - a bet now */
                        if (argument[0] == '\0')
                        {
                                ch->pecho("Какую ставку ты хочешь сделать на этот лот?");
                                return;
                        }

                        int currentbet = max(auction->bet, auction->startbet);
                        newbet = parsebet(currentbet, argument);

                        if ((auction->startbet != 0) && (newbet < (auction->startbet + 1)))
                        {
                                ch->pecho("Тебе необходимо повысить ставку хотя бы на один золотой выше начальной цены.");
                                return;
                        }

                        if (newbet < (auction->bet + 1))
                        {
                                ch->pecho("Тебе необходимо повысить ставку хотя бы на один золотой выше текущей ставки.");
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

                        talk_auction(fmt(0, "На %s{Y получена новая ставка: %d золот%s{x.\n\r",
                                auction->item->getShortDescr( '4', LANG_DEFAULT ).c_str( ),newbet,
                                GET_COUNT(newbet,"ая монета","ые монеты","ых монет")).c_str());
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
                ch->pecho("Этот предмет нельзя выставить на аукцион -- он исчезнет всего через %1$d минут%1$Iу|ы|.", obj->timer);
                return;
        }

        if (IS_OBJ_STAT(obj, ITEM_NOSELL)) {
            ch->pecho("Этот предмет не подлежит продаже.");
            return;
        }
        if (IS_OBJ_STAT(obj, ITEM_NODROP) || IS_OBJ_STAT(obj, ITEM_NOREMOVE)) {
            ch->pecho("С этого предмета нужно снять проклятие перед продажей.");
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
                case ITEM_CRAFT_TATTOO:
                        oldact_p("Ты не можешь выставить на аукцион $T.",
                                ch, 0, item_table.message(obj->item_type).c_str( ),TO_CHAR,POS_SLEEPING);
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

                        msg = fmt(0, "{1{YНа аукцион выставлен новый лот -- {2%O1!", auction->item);                           
                        talk_auction(msg.c_str());
                        send_to_discord_stream(":moneybag: " + msg);
                        send_telegram(msg);

                        if (auction->startbet == 0)
                        {
                                talk_auction("Начальная цена владельцем не установлена{x.");
                        }
                        else
                        {
                                talk_auction(fmt(0, "Начальная цена: %d золот%s{x.", auction->startbet,
                                        GET_COUNT(auction->startbet,"ая монета","ые монеты","ых монет")).c_str());
                        }
                        wiznet( 0, 0, 0, "Продавец -- %C1", ch );
                        return;

                } /* switch */
        }
        else
        {
                oldact_p("Попробуй позже! Кто-то другой уже выставил на аукцион $o4!",
                        ch,auction->item,0,TO_CHAR,POS_RESTING);
                return;
        }
}

