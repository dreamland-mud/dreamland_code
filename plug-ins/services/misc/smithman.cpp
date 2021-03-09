/* $Id$
 *
 * ruffina, 2004
 */
#include <iomanip>

#include "smithman.h"
#include "behavior_utils.h"
#include "attract.h"
#include "occupations.h"
#include "commandtemplate.h"

#include "affect.h"
#include "object.h"
#include "pcharacter.h"
#include "npcharacter.h"

#include "merc.h"
#include "mercdb.h"
#include "interp.h"
#include "handler.h"
#include "arg_utils.h"
#include "act.h"
#include "def.h"

#define OBJ_VNUM_HORSESHOE 107

WEARLOC(hooves);

/*-------------------------------------------------------------------------
 *  Smithman
 *------------------------------------------------------------------------*/
int Smithman::getOccupation( )
{
    return BasicMobileDestiny::getOccupation( ) | (1 << OCC_SMITHMAN);
}

bool Smithman::specIdle( )
{
    if (chance(99))
        return false;

    interpret_raw(ch, "say", "Эх, скучно мне! Никому {yкузнец{g не нужен?");
    return true;
}

bool Smithman::canServeClient( Character *client )
{
    if (client->is_npc( ))
        return false;

    if (getKeeper( )->fighting) {
        say_act( client, getKeeper( ), "Обожди, $c1, мне сейчас не до тебя." );
        return false;
    }
    
    if (IS_GHOST( client )) {
        say_act( client, getKeeper( ), "Э, $c1, так не пойдет.. Нет тела - нет клиента." );
        return false;
    }

    if (IS_CHARMED(client)) {
        say_act( client, getKeeper( ), "Не буду я тебя обслуживать, $c1, слишком уж странно ты себя ведешь." );
        return false;
    }

    return true;
}

void Smithman::msgListRequest( Character *client ) 
{
    act("Ты просишь у %2$C4 список услуг.", client, 0, getKeeper( ), TO_CHAR );
    oldact("$c1 просит $C4 рассказать, что $E умеет делать.", client, 0, getKeeper( ), TO_ROOM );
}

void Smithman::msgListBefore( Character *client ) 
{
    tell_dim( client, getKeeper( ), "Вот список того, что я умею: " );
}

void Smithman::msgListAfter( Character *client )
{
    tell_dim( client, getKeeper( ), "Пока, пожалуй, все." );
}

void Smithman::msgListEmpty( Character *client )
{
    say_act( client, getKeeper( ), "Извини, $c1, но для тебя у меня сегодня выходной." );
}

void Smithman::msgArticleNotFound( Character *client ) 
{
    interpret_raw( getKeeper( ), "eyebrow", client->getNameP( ) );
    say_act(client, getKeeper(), "Я не понимаю, чего ты хочешь. Используй {y{lEsmith list{lRкузница список{lx{g для списка услуг.");
}

void Smithman::msgArticleTooFew( Character *client, Article::Pointer )
{
    say_act( client, getKeeper( ), "Не жадничай." );
}

void Smithman::msgBuyRequest( Character *client ) 
{
    act("Ты просишь %2$C4 обслужить тебя.", client, 0, getKeeper( ), TO_CHAR );
    oldact("$c1 просит $C4 обслужить $s.", client, 0, getKeeper( ), TO_ROOM );
}


/*-------------------------------------------------------------------------
 * SmithService 
 *------------------------------------------------------------------------*/
void SmithService::printLine( Character *client, 
                              Price::Pointer price,
                              const DLString &name,
                              const DLString &descr,
                              ostringstream &buf )
{
    ostringstream mbuf;
    
    price->toStream( client, mbuf );

    buf << dlprintf( "%9s.....{c%-9s{x - %s\r\n",
                     mbuf.str( ).c_str( ),
                     name.c_str( ),
                     descr.c_str( ) );
}

void SmithService::toStream( Character *client, ostringstream &buf ) const
{
    DLString myname = client->getConfig().rucommands && !rname.empty() ? rname : name;
    printLine( client, price, myname, descr, buf );
}

bool SmithService::matches( const DLString &argument ) const
{
    if (argument.empty())
        return false;
    
    return arg_oneof(argument, name.c_str(), rname.c_str());
}

int SmithService::getQuantity( ) const
{
    return 1;
}

/*-------------------------------------------------------------------------
 * HorseshoeSmithService 
 *------------------------------------------------------------------------*/
bool HorseshoeSmithService::visible( Character *client ) const
{
    return client->getWearloc( ).isSet( wear_hooves );
}

bool HorseshoeSmithService::available( Character *client, NPCharacter *smithman ) const
{
    if (visible( client )) 
        return true;

    say_act( client, smithman, "Ну, $c1, и куда же тебе подковки приделать?" );
    return false;
}

bool HorseshoeSmithService::purchase( Character *client, NPCharacter *smithman, const DLString &, int )
{
    Affect af;
    Object *old_shoe, *shoe;
    int level, hr, dr;

    if (!price->canAfford( client )) {
        say_act( client, smithman, "У тебя не хватает $n2, чтобы оплатить мою работу.", price->toCurrency( ).c_str( ) );
        return false;
    }

    level = client->getModifyLevel( );
    shoe = create_object( get_obj_index( OBJ_VNUM_HORSESHOE ), level );
    shoe->level = level;

    for (int j = 0; j < 4; j++)
        if (level < 25)                shoe->valueByIndex(j, min( level , 15 ));
        else if (level < 60)        shoe->valueByIndex(j,  max( 20, number_fuzzy( 20 ) ));
        else if (level < 80)        shoe->valueByIndex(j, max( 23, number_fuzzy( 23 ) ));
        else if (level < 90)        shoe->valueByIndex(j, max( 26, number_fuzzy( 26 ) ));
        else                        shoe->valueByIndex(j, max( 30, number_fuzzy( 30 ) ));

    
    if (level < 10)                { hr = 1; dr = 1; }
    else if (level < 20)        { hr = 3; dr = 1; }
    else if (level < 30)        { hr = 3; dr = 2; }
    else if (level < 50)        { hr = 4; dr = 4; }
    else if (level < 75)        { hr = 5; dr = 6; }
    else if (level < 90)        { hr = 6; dr = 6; }
    else                        { hr = 10; dr = 10; }
    
    af.duration = -1;
    af.type = 0;
    af.level = level;
    
    af.location = APPLY_HITROLL;
    af.modifier = hr;
    affect_to_obj( shoe, &af );

    af.location = APPLY_DAMROLL;
    af.modifier = dr;
    affect_to_obj( shoe, &af );
    
    price->deduct( client );
    oldact("$C1 забирает у тебя $n4.", client, price->toString( client ).c_str( ), smithman, TO_CHAR );
    
    if ((old_shoe = get_eq_char( client, wear_hooves ))) {
        unequip_char( client, old_shoe );
        act("%^C1 снимает с тебя старые подковы.", smithman, 0, client, TO_VICT );
        oldact("$c1 снимает с $C2 старые подковы.", smithman, 0, client, TO_NOTVICT );
    }

    obj_to_char( shoe, client );
    equip_char( client, shoe, wear_hooves);
    act("%^C1 прилаживает новые подковы на твои копыта.", smithman, 0, client, TO_VICT );
    oldact("$c1 прилаживает новые подковы на копыта $C2.", smithman, 0, client, TO_NOTVICT );

    if (client->getSex( ) == SEX_FEMALE && chance( 50 )) {
        act("%^C1 хлопает тебя по крупу, приговаривая '{gХороша, голубушка!{x'", smithman, 0, client, TO_VICT );
        oldact("$c1 хлопает $C4 по крупу, приговаривая '{gХороша, голубушка!{x'", smithman, 0, client, TO_NOTVICT );
    }

    return true;
}


/*-------------------------------------------------------------------------
 * ItemSmithService 
 *------------------------------------------------------------------------*/
bool ItemSmithService::visible( Character * ) const
{
    return true;
}

bool ItemSmithService::available( Character *, NPCharacter * ) const
{
    return true;
}

bool ItemSmithService::purchase( Character *client, NPCharacter *smithman, const DLString &constArguments, int )
{
    Object *obj;
    DLString argument( constArguments );
    DLString arg;
    
    arg = argument.getOneArgument( );

    if (arg.empty( )) {
        say_act( client, smithman, "Не томи, скажи что $t будем?", verb.getValue( ).c_str( ) );
        return false;
    }

    obj = get_obj_carry( client, arg.c_str( ) );

    if (!obj) {
        say_act( client, smithman, "Издеваешься? У тебя нет этого." );
        return false;
    }

    oldact("Ты протягиваешь $C3 $o4.", client, obj, smithman, TO_CHAR );
    oldact("$c1 протягивает $C3 $o4.", client, obj, smithman, TO_NOTVICT );

    if (obj->pIndexData->limit != -1) {
        say_act( client, smithman, "Эта вещь - уникальна. Я ничего не могу тут поделать." );
        return false;
    }
    
    if (!checkPrice( client, smithman, price ))
        return false;

    smith( client, smithman, obj );
    return true;
}   

bool ItemSmithService::checkPrice( Character *client, NPCharacter *smithman, Price::Pointer price ) const
{
    if (!price->canAfford( client )) {
        say_act( client, smithman, noMoney.getValue( ).c_str( ), price->toCurrency( ).c_str( ) );
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------------
 * BurnproofSmithService 
 *------------------------------------------------------------------------*/
void BurnproofSmithService::smith( Character *client, NPCharacter *smithman, Object *obj )
{
    if (obj->item_type != ITEM_CONTAINER && obj->item_type != ITEM_DRINK_CON) {
        say_act( client, smithman, "Это не контейнер, с этим я не умею." );
        return;
    }
       
    if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
        say_act( client, smithman, "Жаль. Но мою работу сделал кто-то другой." );
        return;
    }
    
    price->deduct( client );
    obj->level += 1;

    SET_BIT(obj->extra_flags, ITEM_BURN_PROOF);

    oldact("$c1 обрабатывает чем-то $o4 и возвращает тебе.", smithman, obj, client, TO_VICT );
    oldact("$c1 обрабатывает чем-то $o4 и возвращает $C3.", smithman, obj, client, TO_NOTVICT ); 
}

/*-------------------------------------------------------------------------
 * AlignSmithService 
 *------------------------------------------------------------------------*/
void AlignSmithService::smith( Character *client, NPCharacter *smithman, Object *obj )
{
    int boff, bon;

    if (!obj->isAntiAligned( client )) {
        say_act( client, smithman, "Оно и так для тебя подходит, $c1." );
        return;
    }

    if (obj_is_special(obj)) {
        say_act(client, smithman, "Магия этой вещи неподвласна мне.");
        return;
    }
    
    if (!obj->getRealShortDescr( )) {
        say_act( client, smithman, "$c1, тебе придется сперва потратить рестринг купон и изменить внешний вид вещи." );
        return;
    }
    
    price->deduct( client );
    obj->level += 1;
    
    if (IS_GOOD( client )) {
        boff = ITEM_ANTI_GOOD    | ITEM_EVIL;
        bon  = ITEM_ANTI_NEUTRAL | ITEM_ANTI_EVIL;
    }
    else if (IS_NEUTRAL( client )) {
        boff = ITEM_ANTI_NEUTRAL | ITEM_EVIL;
        bon  = ITEM_ANTI_GOOD    | ITEM_ANTI_EVIL;
    } 
    else {
        boff = ITEM_ANTI_EVIL;
        bon  = ITEM_ANTI_GOOD    | ITEM_ANTI_NEUTRAL;
    }
    
    REMOVE_BIT( obj->extra_flags, boff );
    SET_BIT( obj->extra_flags, bon );

    oldact("$C1 что-то делает с $o5 и возвращает тебе.", client, obj, smithman, TO_CHAR );
    oldact("$C1 что-то делает с $o5 и возвращает $c3.", client, obj, smithman, TO_ROOM );
}

/*-------------------------------------------------------------------------
 * SharpSmithService 
 *------------------------------------------------------------------------*/
void SharpSmithService::toStream( Character *client, ostringstream &buf ) const
{
    DLString myname = client->getConfig().rucommands && !rname.empty() ? rname : name;
    printLine( client, price, myname, descr, buf );
    printLine( client, extraPrice, myname, extraDescr, buf );
}

void SharpSmithService::smith( Character *client, NPCharacter *smithman, Object *obj )
{
    Price::Pointer myprice;

    if (obj->item_type != ITEM_WEAPON) {
        say_act( client, smithman, "И с какой стороны? $o1 не оружие.", obj );
        return;
    }

    if (IS_WEAPON_STAT(obj, WEAPON_SHARP|WEAPON_VORPAL)) {
        say_act( client, smithman, "Но $o1 и без того острое оружие.", obj );
        return;
    }

    if (IS_WEAPON_STAT( obj, WEAPON_HOLY )) {
        say_act( client, smithman, "Но $o1 наполнено священной силой, острота ему ни к чему.", obj );
        return;
    }

    if (IS_WEAPON_STAT( obj, WEAPON_FLAMING | WEAPON_FROST |
                                WEAPON_VAMPIRIC | WEAPON_SHOCKING | WEAPON_POISON ))
    {
        myprice = extraPrice;
        say_act( client, smithman, "У $o2 и без того очень хорошо, сложно будет, потому и дороже.", obj );
    }
    else
        myprice = price;

    if (!checkPrice( client, smithman, myprice ))
        return;

    myprice->deduct( client );
    obj->level += 1;
    obj->value4(obj->value4() | WEAPON_SHARP);

    oldact("$C1 точит $o4 и возвращает тебе.", client, obj, smithman, TO_CHAR );
    oldact("$C1 точит $o4 и возвращает $c3.", client, obj, smithman, TO_ROOM );
}


/*-------------------------------------------------------------------------
 * 'smith' command
 *------------------------------------------------------------------------*/
CMDRUN( smith )
{
    DLString arguments = constArguments;
    Smithman::Pointer smithman;
    
    smithman = find_attracted_mob_behavior<Smithman>( ch, OCC_SMITHMAN );

    if (!smithman) {
        ch->pecho("Здесь нет кузнеца.");
        return;
    }

    if (ch->is_npc( )) {
        ch->pecho("Тебя обслуживать не будут, извини.");
        return;
    }
    
    if (arguments.empty( ) || arg_is_list(arguments))
        smithman->doList( ch->getPC( ) );
    else
        smithman->doBuy( ch->getPC( ), arguments );
}

