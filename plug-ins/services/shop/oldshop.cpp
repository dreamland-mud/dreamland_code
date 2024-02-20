/* $Id: oldshop.cpp,v 1.1.2.9 2010-09-01 21:20:46 rufina Exp $
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
#include <math.h>
#include <string.h>

#include "commandtemplate.h"
#include "shoptrader.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "morphology.h"

#include "class.h"

#include "attract.h"
#include "occupations.h"
#include "webmanip.h"
#include "wearloc_utils.h"
#include "skillreference.h"
#include "skill.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "core/object.h"
#include "room.h"
#include "bonus.h"

#include "dreamland.h"
#include "arg_utils.h"
#include "skill_utils.h"
#include "merc.h"

#include "act.h"
#include "../../anatolia/handler.h"
#include "interp.h"
#include "def.h"
#include "skill_utils.h"
    
GSN(haggle);
BONUS(black_friday);
RELIG(fili);
PROF(druid);

using std::min;
using std::max;

DLString get_obj_name_hint(Object *obj);

/*
 * Local functions
 */
int get_cost( NPCharacter *keeper, Object *obj, bool fBuy, ShopTrader::Pointer trader );
ShopTrader::Pointer find_keeper( Character *ch );
void obj_to_keeper( Object *obj, NPCharacter *ch );
Object *get_obj_keeper( Character *ch, ShopTrader::Pointer, const DLString &constArguments );
void deduct_cost(Character *ch, int cost);

DLString format_coins(int gold, int silver)
{
    DLString format;

    if (gold != 0)
        format += "%1$d золота";
    if (silver != 0 && gold != 0)
        format += " и ";
    if (silver != 0)
        format += "%2$d серебра";

    return fmt(0, format.c_str(), gold, silver);
}

bool can_afford(Character *ch, int gold, int silver, int number)
{
    int cost = gold * 100 + silver;

    return (ch->silver + ch->gold * 100) >= (cost * number);
}

static bool  
mprog_sell( Character *ch, Character *buyer, Object *obj, int cost, int number )
{
    FENIA_CALL( ch, "Sell", "COii", buyer, obj, cost, number );
    FENIA_NDX_CALL( ch->getNPC( ), "Sell", "CCOii", ch, buyer, obj, cost, number );
    return false;
}

struct StockInfo {
    StockInfo( Object *obj, int cost, int count ) {
        this->obj = obj;
        this->cost = cost;
        this->count = count;
    }
    Object *obj;
    int cost;
    int count;
};

typedef vector<StockInfo> ShopStock;

ShopStock get_stock_keeper( ShopTrader::Pointer trader, Character *client, const DLString &arg )
{
    Object *obj;
    ShopStock stock;
    NPCharacter *keeper = trader->getChar( );

    for( obj = keeper->carrying; obj; obj = obj->next_content) {
        if (obj->wear_loc != wear_none)
            continue;
        
        if (client && !client->can_see( obj ))
            continue;

        int cost = get_cost( keeper, obj, true, trader );
        if (cost <= 0)
            continue;

        if (!arg.empty( ) && !obj_has_name( obj, arg, client ))
            continue;
        
        int count = 1;

        if (!IS_OBJ_STAT( obj, ITEM_INVENTORY )) {
            while (obj->next_content
                    &&  obj->pIndexData == obj->next_content->pIndexData
                    &&  !str_cmp( obj->getShortDescr( ), obj->next_content->getShortDescr( ) ))
            {
                obj = obj->next_content;
                count++;
            }
        }
        
        stock.push_back( StockInfo( obj, cost, count ) );
    }

    return stock;
}

/*----------------------------------------------------------------------------
 * 'buy' command
 *---------------------------------------------------------------------------*/
CMDRUN( buy )
{
    if (constArguments.empty( ))
    {
        ch->pecho("Купить что?");
        return;
    }

    char buf[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH];
    char argument[MAX_INPUT_LENGTH];
    int cost, oldcost, roll;
    NPCharacter*  keeper;
    ShopTrader::Pointer trader;
    Object* obj,*t_obj;
    int number, count = 1;

    if ( !( trader = find_keeper( ch ) ) )
        return;
    
    keeper = trader->getChar( );
    strcpy( argument, constArguments.c_str( ) );
    number = mult_argument( argument, arg );
    obj  = get_obj_keeper( ch, trader, arg );
    cost = get_cost( keeper, obj, true, trader );
    
    if ( cost <= 0 || !ch->can_see( obj ) )
    {
        oldact("$c1 говорит тебе '{gЯ не продаю этого -- используй команду {lelist{lrсписок{x'.", keeper, 0, ch, TO_VICT);
        ch->reply = keeper;
        return;
    }


    if ( !IS_OBJ_STAT( obj, ITEM_INVENTORY ) ) {
        for ( t_obj = obj->next_content; count < number && t_obj; t_obj = t_obj->next_content )
        {
            if ( t_obj->pIndexData == obj->pIndexData
                && !str_cmp( t_obj->getShortDescr( ), obj->getShortDescr( ) ) )
            {
                count++;
            }
            else
                break;
        }

        if ( count < number ) {
            oldact_p("$c1 говорит тебе '{gУ меня нет столько.{x'",
            keeper, 0, ch, TO_VICT, POS_RESTING );
            ch->reply = keeper;
            return;
        }
    }

    if (obj->pIndexData->limit > 0 && obj->pIndexData->limit < obj->pIndexData->count - 1 + number) {
        oldact_p("$c1 говорит тебе '{gТакого количества ништяков у меня нету.{x'",
        keeper, 0, ch, TO_VICT, POS_RESTING );
        ch->reply = keeper;
        return;
    }

    if (!can_afford(ch, 0, cost, number))
    {
        if ( number > 1 )
            oldact_p("$c1 говорит тебе '{gТы не можешь заплатить за столько.{x'",
                    keeper, obj, ch, TO_VICT, POS_RESTING );
        else
            oldact_p("$c1 говорит тебе '{gУ тебя нет нужной суммы, чтоб купить $o4.{x'",
                    keeper, obj, ch, TO_VICT,POS_RESTING );
        
        ch->reply = keeper;
        return;
    }

    if ( ch->carry_number + number * obj->getNumber( ) > ch->canCarryNumber( ) )
    {
        ch->pecho("Ты не можешь нести так много вещей.");
        return;
    }

    if (ch->getCarryWeight() + number * obj->getWeight() > ch->canCarryWeight())
    {
        ch->pecho("Ты не можешь нести такую тяжесть.");
        return;
    }

    /* haggle */
    bool bonus = ch->getReligion() == god_fili && get_eq_char(ch, wear_tattoo) != 0;
    roll = bonus ? 100 : number_percent( );
    if ( !IS_OBJ_STAT( obj, ITEM_SELL_EXTRACT ) && (bonus || (roll < gsn_haggle->getEffective(ch) + skill_level_bonus(*gsn_haggle, ch))) )
    {
        oldcost = cost;
        cost -= cost / 2 * roll / 100;
        // can't reduce buying price to less than 25% of the original cost
        cost = URANGE(obj->cost / 4, cost, obj->cost);
        if (cost < oldcost) {
            ch->pecho("Ты торгуешься с %C5 и выбиваешь скидку!", keeper);
            ch->recho("%^C1 торгуется с %C5 и выбивает скидку!", ch, keeper);
            gsn_haggle->improve( ch, true );                      
        }
        else {
            ch->pecho("Ты торгуешься с %C5, но безуспешно.", keeper);
            ch->recho("%^C1 торгуется с %C5, но безуспешно.", ch, keeper);   
            gsn_haggle->improve( ch, false );            
        }
    }

    // everyone reaps off druids
    if (ch->getProfession( ) == prof_druid)
        cost *= 2;

    if ( number > 1 )
    {
        sprintf( buf, "$c1 покупает $o4[%d].", number );
        oldact( buf, ch, obj, 0, TO_ROOM);
        sprintf( buf, "Ты покупаешь $o4[%d] за %d серебрян%s.",
                        number, cost * number,
                        GET_COUNT( cost * number, "ую монету", "ые монеты", "ых монет" ) );
        oldact( buf, ch, obj, 0, TO_CHAR);
    }
    else
    {
        oldact("$c1 покупает $o4.", ch, obj, 0, TO_ROOM);
        sprintf( buf, "Ты покупаешь $o4 за %d серебрян%s.",
                        cost, GET_COUNT( cost, "ую монету", "ые монеты", "ых монет" ) );
        oldact( buf, ch, obj, 0, TO_CHAR);
    }

    int wlevel = get_wear_level( ch, obj );
    if (ch->getRealLevel() < wlevel) {
        say_fmt("Ты сможешь использовать %3$O4 только на %4$d уровне.", keeper, ch, obj, wlevel);
    }

    deduct_cost( ch, cost * number );
    mprog_sell( keeper, ch, obj, cost, number );

    cost += keeper->silver;
    /* 'number' процентов от цены и кассы - в банк */
    dreamland->putToMerchantBank( cost * number / 100 );
    dreamland->save(false);
    /* положить доход в кассу и вычесть то, что ушло в банк */
    keeper->silver = cost * number - ( cost * number / 100 ) * 100;

    for ( count = 0; count < number; count++ )
    {
        if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) 
                && (obj->pIndexData->limit < 0 
            || obj->pIndexData->limit > obj->pIndexData->count))
        {
            t_obj = create_object(obj->pIndexData, 0);
            clone_object(obj, t_obj);
            REMOVE_BIT(t_obj->extra_flags, ITEM_INVENTORY);
        }
        else
        {
            t_obj = obj;
            obj = obj->next_content;
            obj_from_char( t_obj );
        }

        if ( t_obj->timer > 0 && !IS_OBJ_STAT( t_obj, ITEM_HAD_TIMER ) )
            t_obj->timer = 0;
        
        REMOVE_BIT( t_obj->extra_flags, ITEM_HAD_TIMER );
        // prevent predatory resale
        SET_BIT(t_obj->extra_flags, ITEM_SELL_EXTRACT);

        obj_to_char( t_obj, ch );

        if ( cost < t_obj->cost )
            t_obj->cost = cost;
    }

    if (ch->getPC())
        ch->getPC()->save();
}

/*----------------------------------------------------------------------------
 * 'sell' command
 *---------------------------------------------------------------------------*/
CMDRUN( sell )
{
    NPCharacter *keeper;
    ShopTrader::Pointer trader;
    Object *obj;
    int cost, oldcost;
    int roll;
    int gold, silver;
    DLString arg = constArguments;

    if (arg.empty( ))
    {
        ch->pecho("Продать что?");
        return;
    }

    if ( !( trader = find_keeper( ch ) ) )
        return;
    
    keeper = trader->getChar( );

    ShopStock stock = get_stock_keeper( trader, NULL, "" );
    if (stock.size( ) > 25) {
        tell_dim( ch, keeper, "Я ничего не покупаю! Мне некуда ставить товар!");
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg.c_str( ) ) ) == 0 )
    {
        oldact_p("$c1 говорит тебе '{gУ тебя нет этого.{x'",
        keeper, 0, ch, TO_VICT,POS_RESTING );
        ch->reply = keeper;
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
        ch->pecho("Ты не можешь избавиться от этого.");
        return;
    }

    if (!keeper->can_see(obj))
    {
        oldact("$c1 не видит этого.",keeper,0,ch,TO_VICT);
        return;
    }

    cost = get_cost( keeper, obj, false, trader );
    // everyone reaps off druids
    if (ch->getProfession( ) == prof_druid)
        cost /= 2;
    
    if ( cost <= 0 )
    {
        oldact("$c1 не интересуется $o5.", keeper, obj, ch, TO_VICT);
        return;
    }

    if ( (cost / 100 + 1) > dreamland->getBalanceMerchantBank() )
    {
        oldact_p("$c1 говорит тебе '{gУ меня нет денег, чтоб заплатить тебе за $o4.{x'",
              keeper,obj,ch,TO_VICT,POS_RESTING);
        return;
    }

    /* haggle */
    roll = number_percent();

    if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT) && roll < gsn_haggle->getEffective( ch ) )
    {
        silver = cost - (cost/100) * 100;
        gold   = cost/100;
        oldcost = cost;

        ch->pecho("%^C1 предлагает тебе %s за %O4.", keeper, format_coins(gold, silver).c_str(), obj);
        
        // make sure this is a positive factor
        roll = ::max(1, gsn_haggle->getEffective( ch ) + number_range(1, 20) - 10 + skill_level_bonus(*gsn_haggle, ch));

        cost += obj->cost / 2 * roll / 100;
        // can't increase selling price to more than 125% of the original cost
        cost = ::min(cost, obj->cost * 125 / 100);
        
        if (cost > oldcost) {
            ch->pecho("Ты торгуешься с %C5 и выбиваешь наценок!", keeper);
            ch->recho("%^C1 торгуется с %C5 и выбивает наценок!", ch, keeper);
            gsn_haggle->improve( ch, true );                      
        }
        else {
            ch->pecho("Ты торгуешься с %C5, но безуспешно.", keeper);
            ch->recho("%^C1 торгуется с %C5, но безуспешно.", ch, keeper);   
            gsn_haggle->improve( ch, false );            
        }
        
        if ( (cost / 100 + 1) > dreamland->getBalanceMerchantBank() )
        {
            oldact_p("$c1 говорит тебе '{gУ меня нет денег, чтоб заплатить тебе за $o4.{x'",
            keeper,obj,ch,TO_VICT,POS_RESTING);
            return;
        }
    }

    oldact("$c1 продает $o4.", ch, obj, 0, TO_ROOM);
    silver = cost - (cost/100) * 100;
    gold   = cost/100;

    ch->pecho("Ты продаешь %O4 за %s.", obj, format_coins(gold, silver).c_str());

    if ( cost <= keeper->silver )
        keeper->silver -= cost;
    else
    {
        cost -= keeper->silver;
        
        if ( !dreamland->getFromMerchantBank( cost / 100 + 1 ) )
        {
            oldact_p("$c1 говорит тебе '{GУ меня нет денег.{x'",
            keeper,0,ch,TO_VICT,POS_RESTING);
            return;
        }

        keeper->silver = ( cost / 100 + 1 ) * 100 - cost;
    }

    int gold_old = ch->gold;
    int silver_old = ch->silver;
    ch->gold     += gold;
    ch->silver   += silver;

    if ( ch->getCarryWeight( ) > ch->canCarryWeight( ) )
    {
        ch->gold = gold_old;
        ch->silver = silver_old;
        oldact_p("$c1 пытается дать тебе деньги, но ты не можешь их удержать.",
        keeper, 0, ch, TO_VICT,POS_RESTING );
        oldact("$c1 роняет на пол кучку монет.", ch,0,0,TO_ROOM);
        obj_to_room( create_money( gold, silver ), ch->in_room );
    }

    if ( obj->item_type == ITEM_TRASH || IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT) )
    {
        extract_obj( obj );
    }
    else
    {
        obj_from_char( obj );
        
        if (obj->timer)
            SET_BIT(obj->extra_flags,ITEM_HAD_TIMER);
        else
            obj->timer = number_range(50,100);
        
        obj_to_keeper( obj, keeper );
    }

    if (ch->getPC())
        ch->getPC()->save();
}

/*----------------------------------------------------------------------------
 * 'list' command
 *---------------------------------------------------------------------------*/

CMDRUN( list )
{
    NPCharacter* keeper;
    ShopTrader::Pointer trader;
    DLString arg = constArguments;
    ostringstream buf;

    if ( !( trader = find_keeper( ch ) ) )
        return;
    
    keeper = trader->getChar( );
    ShopStock stock = get_stock_keeper( trader, ch, arg );

    if (stock.empty( )) {
        if (arg.empty( ))
            tell_dim( ch, keeper, "Мне сегодня нечего тебе предложить.");
        else
            tell_dim( ch, keeper, "Я не продаю '$t'.", arg.c_str( ) );
        return;
    }

    buf << "[ Ном.| Ур.  Цена Кол-во] Товар" << endl;

    for (unsigned int i = 0; i < stock.size( ); i++) {
        const StockInfo &si = stock.at( i );
        
        if (IS_OBJ_STAT( si.obj, ITEM_INVENTORY ))
            buf << fmt(0, "[ {Y%3d{x |%3d %5d   --   ] ",
                    i+1, si.obj->level, si.cost );

        else 
            buf << fmt(0, "[ {Y%3d{x |%3d %5d %6d ] ",
                    i+1, si.obj->level, si.cost, si.count );

        webManipManager->decorateShopItem( buf, si.obj->getShortDescr( '1' ), si.obj, ch );

        if (!ch->is_npc() && IS_SET(ch->getPC()->config, CONFIG_OBJNAME_HINT))
            buf << get_obj_name_hint(si.obj);

        buf << endl;
    }

    ch->send_to( buf );

    if (bonus_black_friday->isActive(NULL, time_info))
        tell_dim(ch, keeper, "Сегодня в моем магазине невероятно низкие цены.");
    tell_dim( ch, keeper, "Скажи мне название товара, и я расскажу всё, что о нем знаю, за 1%% от стоимости." );

    int counter = 0;
    NPCharacter *mob = NULL;
    NPCharacter *lastShopper = NULL;
    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room){
        if (( mob = rch->getNPC( ) )
            && mob_has_occupation(mob, OCC_SHOPPER))
        {
            counter++;
            lastShopper = mob;
        }
    }

    if(counter>1){
        DLString mobName = Syntax::noun(lastShopper->getShortDescr());
        hint_fmt(ch, "Внимание остальных продавцов в этом месте можно привлечь. Например, {y{hc{lRпривлечь %1$N4{x{y{hc{lEattract %1$N4{x", mobName.c_str());
    }
}

/*----------------------------------------------------------------------------
 * 'value' command
 *---------------------------------------------------------------------------*/
static bool value_one_item(Character *ch, NPCharacter *keeper, ShopTrader::Pointer trader, Object *obj, bool verbose)
{
    if (!keeper->can_see(obj)) {
        if (verbose)
            ch->pecho("%^C1 не видит %O4.", keeper, obj);
        return false;
    }

    if (!can_drop_obj( ch, obj )) {
        if (verbose)
            ch->pecho("Ты не сможешь избавиться от %O2.", obj);
        return false;
    }

    int cost = get_cost( keeper, obj, false, trader );

    // everyone reaps off druids
    if (ch->getProfession( ) == prof_druid)
        cost /= 2;
    
    if (cost <= 0) {
        if (verbose)
            ch->pecho("%^C1 не интересуется %O5.", keeper, obj);
        return false;
    }

    int gold = cost/100;
    int silver = cost - gold * 100;

    if (dreamland->getBalanceMerchantBank() < (gold + 1))
        tell_fmt("Я дал%2$Gо||а бы тебе %3s за %4$O4, но у меня нет денег.", 
                 ch, keeper, format_coins(gold, silver).c_str(), obj);
    else
        tell_fmt("Я дам тебе %3s за %4$O4.", 
                 ch, keeper, format_coins(gold, silver).c_str(), obj);

    return true;
}

CMDRUN( value )
{
    NPCharacter *keeper;
    ShopTrader::Pointer trader;
    Object *obj;
    DLString arg = constArguments;

    if (arg.empty( ))
    {
        ch->pecho("Узнать цену чего?");
        return;
    }

    if ( !( trader = find_keeper( ch ) ) )
        return;
    
    keeper = trader->getChar( );

    if (arg_is_all(arg)) {
        bool saidSomething = false;

        if (!ch->carrying) {
            tell_fmt("Но у тебя же ничего нет!", ch, keeper);
            return;
        }

        for (obj = ch->carrying; obj; obj = obj->next_content) {
            if (value_one_item(ch, keeper, trader, obj, false))
                saidSomething = true;
        }

        if (!saidSomething)
            tell_fmt("Я не вижу у тебя ничего, что могло бы меня заинтересовать.", ch, keeper);

        return;
    }

    if ( ( obj = get_obj_carry( ch, arg.c_str( ) ) ) == 0 ) {
        tell_fmt("У тебя нет этого.", ch, keeper);
        return;
    }

    value_one_item(ch, keeper, trader, obj, true);
}

/*----------------------------------------------------------------------------
 * 'properties' command
 *---------------------------------------------------------------------------*/
CMDRUN( properties )
{
    ShopTrader::Pointer trader;
    DLString arg = constArguments;

    if (arg.empty( ))
    {
        ch->pecho("Узнать характеристики чего?");
        return;
    }

    if ( !( trader = find_keeper( ch ) ) )
        return;
    
    trader->describeGoods( ch, arg, true );
}

/*
 * Local functions
 */
// TO-DO: (RUFFINA) Add *ch to parameters to make druid and CHA-related calculations here
int get_cost( NPCharacter *keeper, Object *obj, bool fBuy, ShopTrader::Pointer trader ) 
{
    int cost;

    if(!obj)
        return 0;

    if( IS_OBJ_STAT( obj, ITEM_NOSELL ) ) 
        return 0;

    if( fBuy ) {
        if (bonus_black_friday->isActive(NULL, time_info))
            cost = obj->cost / 2;
        else
            cost = obj->cost * trader->profitBuy  / 100;
    } else {
        Object *obj2;

        cost = 0;
    
        if (trader->buys.isSetBitNumber( obj->item_type ))
            cost = obj->cost * trader->profitSell / 100;

        if( !IS_OBJ_STAT( obj, ITEM_SELL_EXTRACT ) )
            for( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content ) {
                if( obj->pIndexData == obj2->pIndexData &&
                    !str_cmp( obj->getShortDescr( ), obj2->getShortDescr( ) ) )
                {
                    cost /= 2;
                }
            }
    }

    if( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND ) {
        if( !obj->value1() ) 
            cost /= 4;
        else 
            cost = cost * obj->value2() / obj->value1();
    }

    return cost;
}

ShopTrader::Pointer find_keeper( Character *ch )
{
    NPCharacter *keeper = 0;
    ShopTrader::Pointer trader, null;
    
    trader = find_attracted_mob_behavior<ShopTrader>( ch, OCC_SHOPPER );
    if (!trader) {
        ch->pecho("Здесь нет продавцов.");
        return null;
    }

    keeper = trader->getChar( );

    if (!IS_AWAKE(keeper)) {
        interpret_raw( keeper, "snore" );
        return null;
    }
    
    if ( IS_SET(keeper->in_room->area->area_flag,AREA_HOMETOWN)
         && !ch->is_npc() && IS_SET(ch->act,PLR_WANTED) )
    {
        do_say( keeper, "Преступникам не место здесь!" );
        DLString msg = fmt( 0, "%1$C1 -- преступни%1$Gк|к|ца|ки! Хватайте %1$P2!", ch );
        do_yell( keeper, msg.c_str( ) );
        return null;
    }

    /*
    * Shop hours.
    */
    if (time_info.hour > trader->closeHour) 
    {
        do_say( keeper, "Извини, магазин уже закрыт. Приходи завтра." );
        return null;
    }

    /*
    * Invisible or hidden people.
    */
    if ( !keeper->can_see( ch ) && !ch->is_immortal() )
    {
        do_say( keeper, "Я не торгую с тем, кого не вижу." );
        return null;
    }

    if (keeper->fighting) {
        do_say( keeper, "Подожди немного, мне сейчас не до тебя." );
        return null;
    }

    if (!ch->can_see(keeper)) {
        do_say(keeper, "Ты же меня не видишь, торговать у нас не получится.");
        return null;
    }

/*    
    if (ch->getCurrStat( STAT_CHA ) < 18) {
        switch (number_range( 1, 10 )) {
        case 1:
            do_say( keeper, "Физиономия мне твоя несимпатична, не буду я тебя обслуживать." );
            return 0;
        case 2:
            do_say();
            return 0;
        }
    }
*/  
    return trader;
}

/* insert an object at the right spot for the keeper */
void obj_to_keeper( Object *obj, NPCharacter *ch )
{
    Object *t_obj, *t_obj_next;

    /* see if any duplicates are found */
    for (t_obj = ch->carrying; t_obj != 0; t_obj = t_obj_next)
    {
        t_obj_next = t_obj->next_content;

        if (obj->pIndexData == t_obj->pIndexData
            &&  !str_cmp(obj->getShortDescr( ), t_obj->getShortDescr( )))
        {
            if (IS_OBJ_STAT(t_obj,ITEM_INVENTORY))
            {
                extract_obj(obj);
                return;
            }

            obj->cost = t_obj->cost; /* keep it standard */
            break;
        }
    }

    if (t_obj == 0)
    {
        obj->next_content = ch->carrying;
        ch->carrying = obj;
    }
    else
    {
        obj->next_content = t_obj->next_content;
        t_obj->next_content = obj;
    }

    obj->carried_by      = ch;
    obj->in_room         = 0;
    obj->in_obj          = 0;
    ch->carry_number    += obj->getNumber( );
    ch->carry_weight    += obj->getWeight( );
}


/* get an object from a shopkeeper's list */
Object *get_obj_keeper( Character *ch, ShopTrader::Pointer trader, const DLString &constArguments ) 
{
    char arg[MAX_INPUT_LENGTH];
    char argument[MAX_INPUT_LENGTH];
    Object *obj;
    int number;
    int count;
    int item, item_number;
    NPCharacter *keeper = trader->getChar( );

    strcpy( argument, constArguments.c_str( ) );
    long long id = get_arg_id( argument );

    if( is_number( argument ) && !id) {
        item_number = atoi( argument );
        
        for( obj = keeper->carrying, item = 1; obj;obj = obj->next_content ) {
            if( obj->wear_loc == wear_none 
                && get_cost( keeper, obj, true, trader ) > 0 
                && ch->can_see( obj ) )  
            {
                /* skip other objects of the same name */
                while( obj->next_content &&
                        obj->pIndexData == obj->next_content->pIndexData &&
                        !str_cmp( obj->getShortDescr( ), obj->next_content->getShortDescr( ) ) )
                    obj = obj->next_content;
                
                if( item == item_number )  
                    return obj;
                
                item++;
            }
        }
        
        return 0;
        
    } else {
        number = number_argument( argument, arg );
        count = 0;
        
        for( obj = keeper->carrying; obj; obj = obj->next_content ) {
            if( obj->wear_loc == wear_none &&
                keeper->can_see( obj ) &&
                ch->can_see( obj )  &&
                get_cost( keeper, obj, true, trader ) > 0 &&
                ((id && obj->getID( ) == id) || (!id && obj_has_name( obj, arg, ch ))))
            {
                
                if (!id)
                    /* skip other objects of the same name */
                    while( obj->next_content &&
                            obj->pIndexData == obj->next_content->pIndexData &&
                            !str_cmp( obj->getShortDescr( ), obj->next_content->getShortDescr( ) ) )
                        obj = obj->next_content;
                
                if(id || ++count == number ) 
                    return obj;
            }
        }
    }

    return 0;
}

/* deduct cost from a character */
void deduct_cost(Character *ch, int cost)
{
        int silver = 0, gold = 0;

        silver = min((int)ch->silver,cost);

        if (silver < cost)
        {
                gold = ((cost - silver + 99) / 100);
                silver = cost - 100 * gold;
        }

        ch->gold -= gold;
        ch->silver -= silver;

        if (ch->gold < 0)
            ch->gold = 0;
        
        if (ch->silver < 0)
            ch->silver = 0;
}

