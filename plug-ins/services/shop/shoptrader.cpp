/* $Id$
 *
 * ruffina, 2004
 */
#include "shoptrader.h"
#include "occupations.h"

#include "skillreference.h"
#include "spelltarget.h"
#include "spell.h"
#include "room.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "object.h"

#include "attract.h"
#include "act.h"
#include "interp.h"
#include "../../anatolia/handler.h"
#include "merc.h"

#include "def.h"

/*
 * Note: shops were never fully refactored to use the Service classes. Unfinished attempt to do so can be found in
 * Git history for this plugin.
 */

void deduct_cost(Character *ch, int cost);
int get_cost( NPCharacter *keeper, Object *obj, bool fBuy, ShopTrader::Pointer trader );
Object *get_obj_keeper( Character *ch, ShopTrader::Pointer trader, const DLString &constArguments );

GSN(identify);

ShopTrader::ShopTrader( )
            : buys( 0, &item_table )
{
}

int ShopTrader::getOccupation( )
{
    return Repairman::getOccupation( ) | (1 << OCC_SHOPPER);
}

void ShopTrader::load( DLString str )
{
    if (str == "spec_repairman") {
        for (int i = 0; i < item_table.size; i++)        
            repairs.setBitNumber( i );
    }
    else {
        for (int i = 0; i < 5; i++) 
            buys.setBitNumber( str.getOneArgument( ).toInt( ) );
        
        profitBuy  = str.getOneArgument( ).toInt( );
        profitSell = str.getOneArgument( ).toInt( );
        openHour   = str.getOneArgument( ).toInt( );
        closeHour  = str.getOneArgument( ).toInt( );
    }
}


void ShopTrader::give( Character *from, Object *obj )
{
    tell_dim( from, ch, "Извини, но я не беру взяток!" );
    oldact("$c1 роняет $o4.", ch, obj, 0, TO_ROOM );
    obj_from_char( obj );
    obj_to_room( obj, ch->in_room );
}

void ShopTrader::speech( Character *victim, const char *speech )
{
    // React only if we would have reacted to the 'properties' command.
    ShopTrader::Pointer trader = find_attracted_mob_behavior<ShopTrader>(victim, OCC_SHOPPER);
    if (trader->getChar() != ch)
        return;

    describeGoods( victim, speech, false );
}

void ShopTrader::tell( Character *victim, const char *speech )
{
    describeGoods( victim, speech, true );
}

void ShopTrader::show(Character *, std::basic_ostringstream<char> &buf) 
{
    buf << "{y({GПродавец{y){x";
}

void ShopTrader::describeGoods( Character *client, const DLString &args, bool verbose )
{
    ostringstream buf;
    Object *obj  = get_obj_keeper( client, this, args );

    if (!obj) {
        if (verbose)
            tell_dim( client, ch, "Я не продаю этого - используй '{lRсписок{lElist{x'." );
        return;
    }

    client->recho( ch, "%^C1 просит %C4 подробнее рассказать о %O6.", client, ch, obj );

    if (!IS_OBJ_STAT( obj, ITEM_INVENTORY )) {
        client->pecho( "%1$^C1 говорит тебе '{gЯ раньше в глаза не виде%1$Gло|л|ла %2$O4.{x'", ch, obj );
        return;
    }

    int itemCost = get_cost( ch, obj, true, this );
    int serviceCost = itemCost / 100;
    if ((client->silver + client->gold * 100) < serviceCost) {
        tell_dim( client, ch, "Я не справочная контора. Будут деньги, тогда и возвращайся." );
        return;
    }

    tell_dim( client, ch, "Нигде больше не найдешь такого замечательного товара:" );

    {
        bool hadMagic = client->detection.isSet(DETECT_MAGIC);
        client->detection.setBit(DETECT_MAGIC);

        if (gsn_identify->getSpell( ))
            gsn_identify->getSpell( )->run( client, SpellTarget::Pointer(NEW, obj), 0 );

        if (!hadMagic)
            client->detection.removeBit(DETECT_MAGIC);
    }

    if (serviceCost < 1) {
        tell_dim( client, ch, "Я сообщаю тебе это совершенно бесплатно." );
    } else {
        deduct_cost( client, serviceCost );
        client->pecho( "%1$^C1 взя%1$Gло|л|ла с тебя {W%2$d{x моне%2$Iту|ты|т за услуги.", ch, serviceCost );
    }
}






