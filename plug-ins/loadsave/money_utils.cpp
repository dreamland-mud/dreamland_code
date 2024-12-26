#include "core/object.h"
#include "character.h"
#include "vnum.h"
#include "loadsave.h"
#include "money_utils.h"
#include "arg_utils.h"
#include "msgformatter.h"
#include "act.h"

void Money::dematerialize( Object *list, int &gold, int &silver )
{
    Object *obj, *obj_next;

    for ( obj = list; obj != 0; obj = obj_next )
    {
        obj_next = obj->next_content;

        switch ( obj->pIndexData->vnum ) {
        case OBJ_VNUM_SILVER_ONE:
            silver += 1;
            extract_obj(obj);
            break;

        case OBJ_VNUM_GOLD_ONE:
            gold += 1;
            extract_obj( obj );
            break;

        case OBJ_VNUM_SILVER_SOME:
            silver += obj->value0();
            extract_obj(obj);
            break;

        case OBJ_VNUM_GOLD_SOME:
            gold += obj->value1();
            extract_obj( obj );
            break;

        case OBJ_VNUM_COINS:
            silver += obj->value0();
            gold += obj->value1();
            extract_obj(obj);
            break;
        }
    }
}

/*
 * Create a 'money' obj.
 */
Object * Money::create( int gold, int silver )
{
    Object *obj;

    if ( gold < 0 || silver < 0 || (gold == 0 && silver == 0) )
    {
        bug( "Money::create: zero or negative money.",min(gold,silver));
        gold = max(1,gold);
        silver = max(1,silver);
    }

    if (gold == 0 && silver == 1)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_SILVER_ONE ), 0 );
    }
    else if (gold == 1 && silver == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_GOLD_ONE), 0 );
    }
    else if (silver == 0)
    {
        // TODO short descrs in all languages
        obj = create_object( get_obj_index( OBJ_VNUM_GOLD_SOME ), 0 );
        DLString moneyArg;
        moneyArg << gold << " золот|" << GET_COUNT( gold, "ая|ой|ой|ую|ой|ой", "ые|ых|ым|ые|ыми|ых", "ых|ых|ым|ых|ыми|ых" )
        << " монет|" << GET_COUNT(gold, "а|ы|е|у|ой|е", "ы||ам|ы|ами|ах", "||ам||ами|ах");
        obj->setShortDescr( moneyArg, LANG_RU );
        obj->value1(gold);
        obj->cost               = 100 * gold;
        obj->weight                = gold/5;
    }
    else if (gold == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_SILVER_SOME ), 0 );
        DLString moneyArg;
        moneyArg << silver << " серебрян|" << GET_COUNT(silver, "ая|ой|ой|ую|ой|ой", "ые|ых|ым|ые|ыми|ых", "ых|ых|ым|ых|ыми|ых")
        << " монет|" << GET_COUNT(silver, "а|ы|е|у|ой|е", "ы||ам|ы|ами|ах", "||ам||ами|ах");
        obj->setShortDescr( moneyArg, LANG_RU );
        obj->value0(silver);
        obj->cost               = silver;
        obj->weight                = silver/20;
    }

    else
    {
        obj = create_object( get_obj_index( OBJ_VNUM_COINS ), 0 );
        DLString moneyArg;
        moneyArg << gold << " золот|" << GET_COUNT( gold, "ая|ой|ой|ую|ой|ой", "ые|ых|ым|ые|ыми|ых", "ых|ых|ым|ых|ыми|ых" )
        << " и " << silver << " серебрян|" << GET_COUNT(silver, "ая|ой|ой|ую|ой|ой", "ые|ых|ым|ые|ыми|ых", "ых|ых|ым|ых|ыми|ых")
        << " монет|" << GET_COUNT(silver, "а|ы|е|у|ой|е", "ы||ам|ы|ами|ах", "||ам||ами|ах");
        obj->setShortDescr( moneyArg, LANG_RU );
        obj->value0(silver);
        obj->value1(gold);
        obj->cost                = 100 * gold + silver;
        obj->weight                = gold / 5 + silver / 20;
    }

    return obj;
}

DLString Money::describe( int gold, int silver, const Grammar::Case &gcase )
{
    static const char *cases_gold [] = {
        "золот%1$Iая|ые|ых",
        "золот%1$Iая|ые|ых",
        "золот%1$Iой|ых|ых",
        "золот%1$Iой|ым|ым",
        "золот%1$Iую|ые|ых",
        "золот%1$Iой|ыми|ыми",
        "золот%1$Iой|ых|ых",
    };
    static const char *cases_silver [] = {
        "серебрян%1$Iая|ые|ых",
        "серебрян%1$Iая|ые|ых",
        "серебрян%1$Iой|ых|ых",
        "серебрян%1$Iой|ым|ым",
        "серебрян%1$Iую|ые|ых",
        "серебрян%1$Iой|ыми|ыми",
        "серебрян%1$Iой|ых|ых",
    };
    static const char *cases_coin [] = {
        "моне%1$Iта|ты|т",
        "моне%1$Iта|ты|т",
        "моне%1$Iты|т|т",
        "моне%1$Iте|там|там",
        "моне%1$Iту|ты|т",
        "моне%1$Iтой|тами|тами",
        "моне%1$Iте|тах|тах",
    };

    DLString msg;
    
    if (gold > 0)
        msg << gold << " " << fmt( 0, cases_gold[gcase], gold );

    if (silver > 0) {
        if (gold > 0)
            msg << " и ";

        msg << silver << " " << fmt( 0, cases_silver[gcase], silver);
    }
    
    msg << " " << fmt( 0, cases_coin[gcase], silver > 0 ? silver : gold );
    return msg;
}

bool Money::parse( Character *ch, const char *arg, int amount, int &gold, int &silver )
{
    if ((!arg_is_silver( arg ) && !arg_is_gold( arg ) )) {
        if (!str_prefix( arg, "серебр" ) || !str_prefix( arg, "silver" )) {
            ch->pecho( "Укажи название монеты полностью: серебро." );
            return false;
        }
        if (!str_prefix( arg, "золот" ) || !str_prefix( arg, "gold" )) {
            ch->pecho( "Укажи название монеты полностью: золото." );
            return false;
        }
        ch->pecho( "Ты можешь указать количество денег в серебре или золоте ." );
        return false;
    }

    if (amount < 0) {
        ch->pecho( "Отрицательное количество денег?" );
        return false;
    }
    
    if (amount == 0) {
        ch->pecho( "Ноль монет -- это как?" );
        return false;
    }


    if (arg_is_silver( arg )) {
            if (ch->silver < amount)
            {
                    ch->pecho("У тебя нет столько серебра.");
                    return false;
            }

            silver = amount;
    }
    else
    {
            if (ch->gold < amount)
            {
                    ch->pecho("У тебя нет столько золота.");
                    return false;
            }

            gold = amount;
    }

    return true;
}

