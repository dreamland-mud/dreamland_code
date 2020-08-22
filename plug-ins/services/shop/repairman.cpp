/* $Id$
 *
 * ruffina, 2004
 */
#include "commandtemplate.h"
#include "repairman.h"
#include "attract.h"
#include "occupations.h"

#include "feniamanager.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "skillreference.h"

#include "act.h"
#include "interp.h"
#include "handler.h"
#include "merc.h"
#include "mercdb.h"
#include "vnum.h"
#include "def.h"

GSN(haggle);
RELIG(fili);

static void mprog_repair( Character *mob, Character *client, Object *obj, int cost )
{
    FENIA_VOID_CALL(mob, "Repair", "COi", client, obj, cost);
    FENIA_NDX_VOID_CALL(mob->getNPC(), "Repair", "CCOi", mob, client, obj, cost);
}

/*
 * Repairman
 */
Repairman::Repairman( )
            : repairs( 0, &item_table )
{
}

int Repairman::getOccupation( )
{
    int occ = BasicMobileDestiny::getOccupation( );
    
    if (repairs.getValue( ) != 0)
        occ |= (1 << OCC_REPAIRMAN);

    return occ;
}

bool Repairman::specIdle( )
{
    if (BasicMobileDestiny::specIdle( ))
        return true;
        
    if (!IS_AWAKE( ch ))
        return false;

    if (repairs.getValue( ) == 0)
        return false;

    if (chance( 1 )) {
        interpret_raw(ch, "say", "Я буду рад отремонтировать тебе что-нибудь!");
        return true;
    }

    return false;
}

void Repairman::doRepair( Character *client, const DLString &cArgs )
{
    Object *obj;
    int cost, roll;
    DLString args = cArgs, arg;
    
    arg = args.getOneArgument( );
    
    if (arg.empty( )) {
        say_act( client, ch, "Я отремонтирую тебе что-нибудь за деньги.");
        client->println("Напиши {y{lrчинить{lerepair{x <название предмета>, чтоб восстановить его.");
        client->println("Напиши {y{lrоценить{leestimate{x <название предмета>, чтобы узнать, сколько будет стоить починка.");
        return;
    }

    if (( obj = get_obj_carry(client, arg.c_str( ))) == 0) {
        say_act( client, ch, "У тебя нет этого");
        return;
    }
    
    if (!canRepair( obj, client ))
        return;
    
    cost = getRepairCost( obj );

    if (cost > client->gold) {
        say_act( client, ch, "У тебя не хватит денег, чтоб оплатить ремонт этой вещи.");
        return;
    }

    /* haggle */
    if(cost > 1){
        bool bonus = client->getReligion() == god_fili && get_eq_char(client, wear_tattoo) != 0;
        roll = bonus ? 100 : number_percent( );
        if ( bonus || (roll < gsn_haggle->getEffective(client)) )
        {
            cost -= cost / 2 * roll / 100;
            act_p( "Ты торгуешься с $C5.", client, 0, this->getChar(), TO_CHAR, POS_RESTING );
            gsn_haggle->improve( client, true );
        }
    }

    client->setWaitViolence( 1 );

    client->gold -= cost;
    ch->gold += cost;

    act("$C1 берет $o4 y $c2, восстанавливает и возвращает $c3.",client,obj,ch,TO_ROOM);
    act("$C1 берет $o4, восстанавливает и возвращает тебе.",client,obj,ch,TO_CHAR);

    if (cost) 
        client->pecho( "Твой кошелек стал легче на %1$d золот%1$Iую|ые|ых моне%1$Iту|ты|т.", cost );
    else
        client->println( "В честь Дня Защиты Животных починка обошлась тебе бесплатно." );

    obj->condition = 100;
    mprog_repair(ch, client, obj, cost);
}

void Repairman::doEstimate( Character *client, const DLString &cArgs )
{
    Object *obj;
    DLString args = cArgs, arg;
    
    arg = args.getOneArgument( );
    
    if (arg.empty( )) {
        say_act( client, ch, "Напиши {y{lrоценить{leestimate{x <название предмета>, чтобы узнать, сколько будет стоить починка.");
           return;
    }

    if ((obj = get_obj_carry(client, arg.c_str( ))) == 0) {
        say_act( client, ch, "У тебя нет этого");
        return;
    }
    
    if (!canRepair( obj, client ))
        return;
    
    tell_fmt( "Восстановление этой вещи будет стоить %3$d.", 
               client, ch, getRepairCost( obj ) );
}

int Repairman::getRepairCost( Object *obj )
{
    int cost;
    
    cost = obj->level * 10 + (obj->cost * (100 - obj->condition)) / 100;
    cost /= 100;
    return cost;
}

bool Repairman::canRepair( Object *obj, Character *client )
{
    if (!repairs.isSetBitNumber( obj->item_type )) {
        say_act( client, ch, 
                 "Я не сумею отремонтировать $t.", 
                 item_table.message( obj->item_type, '4' ).c_str( ) );
        return false;
    }
    
    if (obj->pIndexData->vnum == OBJ_VNUM_HAMMER) {
        say_act( client, ch, "Я не ремонтирую боевые молоты.");
        return false;
    }

    if (obj->condition >= 100) {
        say_fmt( "%2$^O1 не нужда%2$nется|ются в ремонте.", ch, obj );
        return false;
    }

    if (obj->cost == 0) {
        say_fmt( "%2$^O1 ничего не стоит, а значит я не смогу оценить починку.", ch, obj );
        say_act( client, ch, "Ты случайно не из ямы для пожертвований ее вытащил{Sfа{Sx?");        
           return false;
    }

    return true;
}


/*
 * 'repair' command
 */
CMDRUN( repair )
{
    Repairman::Pointer man;
    
    if (!( man = find_attracted_mob_behavior<Repairman>( ch, OCC_REPAIRMAN ) )) {
        ch->println( "Здесь нет ремонтника.");
        return;
    }

    if (!ch->is_npc( )) 
        man->doRepair( ch, constArguments );
}

/*
 * 'estimate' command
 */
CMDRUN( estimate )
{
    Repairman::Pointer man;
    
    if (!( man = find_attracted_mob_behavior<Repairman>( ch, OCC_REPAIRMAN ) )) {
        ch->println( "Здесь нет ремонтника.");
        return;
    }

    if (!ch->is_npc( )) 
        man->doEstimate( ch, constArguments );
}

