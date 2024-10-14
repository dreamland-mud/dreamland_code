#include "pcharacter.h"
#include "npcharacter.h"
#include "core/object.h"
#include "room.h"
#include "grammar_entities_impl.h"
#include "commandtemplate.h"
#include "areaquestutils.h"
#include "../loadsave/behavior_utils.h"
#include "core/behavior/behavior_utils.h"
#include "wrapperbase.h"
#include "wrappertarget.h"
#include "wearloc_utils.h"
#include "interp.h"
#include "act.h"
#include "loadsave.h"
#include "merc.h"
#include "def.h"

DLString get_pocket_argument( char *arg );


/*
 *   PUT OBJECT [IN|ON] CONTAINER[:POCKET]
 */

#define PUT_OBJ_STOP        -1
#define PUT_OBJ_ERR        0
#define PUT_OBJ_OK        1

static bool oprog_cant_put( Character *ch, Object *obj, Object *container,
                            const char *pocket, bool verbose )
{
    if (behavior_trigger(container, "CantPut", "OCOsi", container, ch, obj, pocket, verbose))
        return true;

    FENIA_CALL( container, "CantPut", "COsi", ch, obj, pocket, verbose );
    FENIA_NDX_CALL( container, "CantPut", "OCOsi", container, ch, obj, pocket, verbose );
    return false;
}

static bool can_put_into( Character *ch, Object *container, const DLString &pocket )
{
    switch (container->item_type) {
    case ITEM_CONTAINER:
        if (IS_SET(container->value1(), CONT_LOCKED)) {
            ch->pecho("%1$^O1 заперт%1$Gо||а|ы на ключ, попробуй отпереть.", container);
            return false;
        }
        if (IS_SET(container->value1(), CONT_CLOSED)) {
            ch->pecho("%1$^O1 закрыт%1$Gо||а|ы, попробуй открыть.", container);
            return false;
        }

        if (!pocket.empty( ) && !IS_SET(container->value1(), CONT_WITH_POCKETS)) {
            ch->pecho( "Тебе не удалось нашарить ни одного кармана на %O6.", container );
            return false;
        }

        return true;

    case ITEM_KEYRING:
        return true;

    default:
        ch->pecho("%^O1 не контейнер, туда ничего нельзя положить.", container);
        return false;
    }
}

static bool can_put_money_into( Character *ch, Object *container )
{
    if (container->item_type != ITEM_CONTAINER) {
        ch->pecho("Ты пытаешься положить деньги в %O4, но это не контейнер.", container);
        return false;
    }

    if (IS_SET(container->wear_flags, ITEM_TAKE) || !container->in_room) {
        ch->pecho("Ты можешь положить деньги только в стационарные контейнеры: ямы для пожертвований, алтари.");
        return false;
    }

    if (IS_SET(container->value1(), CONT_CLOSED)) {
        ch->pecho("%1$^O1 закрыт%1$Gо||а|ы.", container);
        return false;
    }

    return true;
}

static int can_put_obj_into( Character *ch, Object *obj, Object *container, const DLString &pocket, bool verbose )
{
    int pcount;

    if (obj == container) {
        if (verbose)
            ch->pecho("Ты не можешь положить что-то в себя же.");
        return PUT_OBJ_ERR;
    }
    
    if (oprog_cant_put( ch, obj, container, pocket.c_str( ), verbose )) 
        return PUT_OBJ_ERR;

    if (!Item::canDrop( ch, obj )) {
        oldact("Ты не можешь избавиться от $o2.", ch, obj, 0, TO_CHAR );
        return PUT_OBJ_ERR;
    }
    
    pcount = count_obj_in_obj( container );

    if (container->item_type == ITEM_KEYRING) {
        if (pcount >= container->value0()) {
            oldact("На $o6 не осталось свободного места.", ch, container, 0, TO_CHAR );
            return PUT_OBJ_STOP;
        }

        if (obj->item_type != ITEM_KEY && obj->item_type != ITEM_LOCKPICK) {
            if (verbose)
                oldact("На $o4 ты можешь нанизать только ключи или отмычки.", ch, container, 0, TO_CHAR );
            return PUT_OBJ_ERR;
        }

        return PUT_OBJ_OK;
    }

    if (pcount > container->value0()) {
        oldact("Опасно запихивать столько вещей в $o4!", ch,container,0, TO_CHAR);
        return PUT_OBJ_STOP;
    }

    if (obj->getWeightMultiplier() != 100 && !IS_SET(container->value1(), CONT_NESTED)) {
        if (verbose)
            ch->pecho("Ты не можешь положить %O4 в другой контейнер.", obj);
        return PUT_OBJ_ERR;
    }

    if (obj->pIndexData->limit != -1) {
        oldact("$o4 нельзя хранить в такой дребедени.", ch,obj,0,TO_CHAR );
        return PUT_OBJ_ERR;
    }

    if (IS_SET(container->value1(),CONT_FOR_ARROW)
            && (obj->item_type != ITEM_WEAPON
            || obj->value0()  != WEAPON_ARROW ))
    {
        if (verbose)
            oldact("Ты можешь положить только стрелы в $o4.",ch,container,0,TO_CHAR);
        return PUT_OBJ_ERR;
    }

    if (obj->getWeight( ) + container->getTrueWeight( ) > (container->value0() * 10)
        ||  obj->getWeight( ) > (container->value3() * 10))
    {
        if (verbose)
            ch->pecho("%1$^O1 не сможет вместить в себя %2$O4.", container, obj);
        return PUT_OBJ_ERR;
    }

    if (obj->item_type == ITEM_POTION && IS_SET(container->wear_flags, ITEM_TAKE)) {
        pcount = count_obj_in_obj( container, ITEM_POTION );
                
       if (pcount > 15) {
            oldact("Небезопасно далее складывать снадобья в $o4.",ch,container,0, TO_CHAR);
            return PUT_OBJ_ERR;
       }
    }

    return PUT_OBJ_OK;
}

static bool oprog_put(Object *obj, Character *ch, Object *container)
{
    aquest_trigger(container, ch, "Put", "OCO", container, ch, obj);

    if (behavior_trigger(container, "Put", "OCO", container, ch, obj))
        return true;

    FENIA_CALL( ch, "Put", "COO", ch, obj, container );
    FENIA_CALL( obj, "Put", "COO", ch, obj, container );
    FENIA_CALL( container, "Put", "COO", ch, obj, container );

    FENIA_NDX_CALL( ch->getNPC( ), "Put", "CCOO", ch, ch, obj, container );
    FENIA_NDX_CALL( obj, "Put", "OCOO", obj, ch, obj, container );
    FENIA_NDX_CALL( container, "Put", "OCOO", container, ch, obj, container );
    
    return false;
}

static bool oprog_put_msg(Object *obj, Character *ch, Object *container)
{
    FENIA_CALL( container, "PutMsg", "COO", ch, obj, container );
    FENIA_NDX_CALL( container, "PutMsg", "OCOO", container, ch, obj, container );

    FENIA_CALL( obj, "PutMsg", "COO", ch, obj, container );
    FENIA_NDX_CALL( obj, "PutMsg", "OCOO", obj, ch, obj, container );

    return false;
}


static bool oprog_put_money(Object *container, Character *ch, int gold, int silver)
{
    FENIA_CALL( container, "PutMoney", "Cii", ch, gold, silver );
    FENIA_NDX_CALL( container, "PutMoney", "OCii", container, ch, gold, silver );
    return false;
}

static bool oprog_put_money_msg(Object *container, Character *ch, int gold, int silver)
{
    FENIA_CALL( container, "PutMoneyMsg", "Cii", ch, gold, silver );
    FENIA_NDX_CALL( container, "PutMoneyMsg", "OCii", container, ch, gold, silver );
    return false;
}

static bool put_obj_container( Character *ch, Object *obj, Object *container, 
                               DLString &pocket )
{
    ostringstream toChar, toRoom;

    obj_from_char( obj );
    obj_to_obj( obj, container );
    
    if (container->item_type == ITEM_KEYRING) {
        toRoom << "$c1 нанизывает $o4 на $O4.";
        toChar << "Ты нанизываешь $o4 на $O4.";
    }
    else {

        if (!pocket.empty( )) 
            obj->pocket = pocket;

        toRoom << "$c1 кладет $o4 "
               << (IS_SET( container->value1(), CONT_PUT_ON|CONT_PUT_ON2 ) ?
                             "на" : "в")
               << " $O4.";
        
        if (pocket.empty( ))
            toChar << "Ты кладешь $o4 "
                   << (IS_SET( container->value1(), CONT_PUT_ON|CONT_PUT_ON2 ) ?
                             "на" : "в")
                   << " $O4.";
        else {
            toChar << "Ты кладешь $o4 ";

            if (IS_SET(container->value1(),CONT_PUT_ON|CONT_PUT_ON2)) {
                toChar << "на $O4 в отделение '" << pocket << "'.";
            }
            else if (!container->can_wear(ITEM_TAKE)) {
                toChar << "на полку $O2 с надписью '" << pocket << "'.";
            }
            else
                toChar << "в карман $O2 с надписью '" << pocket << "'.";
        }
    }
    
    if (!oprog_put_msg( obj, ch, container )) {
        oldact( toRoom.str( ).c_str( ), ch, obj, container, TO_ROOM );
        oldact( toChar.str( ).c_str( ), ch, obj, container, TO_CHAR );
    }

    return oprog_put( obj, ch, container );
}

static void put_money_container(Character *ch, int amount, const char *currencyName, const char *containerArg)
{
    /* 'put -N ...' and 'put 0 ...' not allowed. */
    if (amount <= 0) {
        ch->pecho("Сколько-сколько монет?");
        return;
    }

    DLString containerArgs = containerArg;
    DLString containerName = containerArgs.getOneArgument();
    /* 'put N gold|silver in|on container' becomes 'put N gold|silver container' */
    if (arg_is_in(containerName) || arg_is_on(containerName))
        containerName = containerArgs;

    Object *container = get_obj_here(ch, containerName.c_str());
    if (!container) {
        oldact("Ты не видишь здесь $T.", ch, 0, containerName.c_str(), TO_CHAR);
        return;
    }

    // See if it's an open no-take container on the floor.
    if (!can_put_money_into(ch, container))
        return;

    // Parse out gold and silver amounts.
    int gold = 0, silver = 0;
    if (!Money::parse(ch, currencyName, amount, gold, silver))
        return;

    // Create temporary money object and see if it fits.
    Object *money = Money::create(gold, silver);
    obj_to_char(money, ch);
    if (can_put_obj_into(ch, money, container, "", true) != PUT_OBJ_OK) {
        extract_obj(money);
        return;
    }
  
    oprog_put_money(container, ch, gold, silver); 

    if (!oprog_put_money_msg(container, ch, gold, silver)) {
        DLString moneyArg = Money::describe(gold, silver, 4);
        DLString preposition = IS_SET( container->value1(), CONT_PUT_ON|CONT_PUT_ON2 ) ? "на" : "в";
        ch->pecho("Ты кладешь %s %s %O4.", moneyArg.c_str(), preposition.c_str(), container);
        ch->recho("%^C1 кладет %s %O4 несколько монет.", ch, preposition.c_str(), container);
    }
    
    // Add money to the container or merge with existing coins.
    ch->silver -= silver;
    ch->gold -= gold;
    Money::dematerialize( container->contains, gold, silver );
    extract_obj(money);
    money = Money::create( gold, silver );
    obj_to_obj(money, container);
}

CMDRUNP( put )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    DLString pocket;
    Object *container;
    Object *obj;
    DLString origArguments = argument;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    /* 'put N gold|silver container' */
    if (is_number(arg1) && get_arg_id(arg1) == 0 && arg_is_money(arg2)) {
        put_money_container(ch, atoi(arg1), arg2, argument);
        return;
    }

    /* 'put obj in|on container' becomes 'put obj container */
    if (arg_is_in( arg2 ) || arg_is_on( arg2 ))
        argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        ch->pecho("Положить что и куда?");
        return;
    }

    /* no 'put obj all.container' syntax allowed */
    if (arg_is_alldot( arg2 ))
    {
        ch->pecho("Ты не можешь сделать этого.");
        return;
    }
    
    pocket = get_pocket_argument( arg2 );

    if ( ( container = get_obj_here( ch, arg2 ) ) == 0 ) {
        oldact("Ты не видишь здесь $T.", ch, 0, is_number(arg2) ? "этого" : arg2, TO_CHAR);
        return;
    }
    
    if (!can_put_into( ch, container, pocket ))
        return;

    if (!arg_is_alldot( arg1 ))
    {
        /* 'put obj container' */
        if ( ( obj = get_obj_carry( ch, arg1 ) ) == 0 )
        {
            ch->pecho("У тебя нет этого.");
            return;
        }
        
        if (can_put_obj_into( ch, obj, container, pocket, true ) == PUT_OBJ_OK) 
            put_obj_container( ch, obj, container, pocket );
    }
    else
    {
        Object *obj_next;
        bool found = false;

        /* 'put all container' or 'put all.obj container' */
        for (obj = ch->carrying; obj != 0; obj = obj_next) {
            obj_next = obj->next_content;

            if ((arg1[3] == '\0' || obj_has_name( obj, &arg1[4], ch ))
                &&   ch->can_see( obj )
                &&   obj->wear_loc == wear_none)
            {

                switch (can_put_obj_into( ch, obj, container, pocket, true )) {
                case PUT_OBJ_STOP:
                    return;

                case PUT_OBJ_OK:
                    if (put_obj_container( ch, obj, container, pocket ))
                        return;
                    found = true;
                    break;

                case PUT_OBJ_ERR:
                    break;
                }
            }
        }
        
        if (!found) {
            if (container->item_type == ITEM_KEYRING)
                oldact("Ты не наш$gло|ел|ла ничего, что можно нанизать на $o4.", ch, container, 0, TO_CHAR );
            else
                oldact("Ты не наш$gло|ел|ла ничего, что можно положить в $o4.", ch, container, 0, TO_CHAR );
        }
    }
}



