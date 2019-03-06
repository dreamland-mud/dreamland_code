/* $Id$
 *
 * ruffina, 2004
 */
#include "commandtemplate.h"
#include "repairman.h"
#include "attract.h"
#include "occupations.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "act.h"
#include "interp.h"
#include "handler.h"
#include "merc.h"
#include "mercdb.h"
#include "vnum.h"
#include "def.h"

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
        interpret_raw(ch, "say", "Настало время для ремонта ваших доспехов.");
        return true;
    }

    return false;
}

void Repairman::doRepair( Character *client, const DLString &cArgs )
{
    Object *obj;
    int cost;
    DLString args = cArgs, arg;
    
    arg = args.getOneArgument( );
    
    if (arg.empty( )) {
        say_act( client, ch, "Я отремонтирую тебе что-нибудь за деньги.");
        client->println("Используй repair <item>, чтоб восстановить поврежденный предмет.");
        client->println("Используй estimate <item>, чтобы узнать, сколько будет стоить починка.");
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
        say_act( client, ch, "У тебя не хватает денег, чтоб оплатить мои услуги.");
        return;
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
}

void Repairman::doEstimate( Character *client, const DLString &cArgs )
{
    Object *obj;
    DLString args = cArgs, arg;
    
    arg = args.getOneArgument( );
    
    if (arg.empty( )) {
        say_act( client, ch, "Попробуй использовать estimate <item>.");
           return;
    }

    if ((obj = get_obj_carry(client, arg.c_str( ))) == 0) {
        say_act( client, ch, "У тебя нет этого");
        return;
    }
    
    if (!canRepair( obj, client ))
        return;
    
    tell_fmt( "Восстановление будет стоить %3$d.", 
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
        say_fmt( "%2$^O1 не подлеж%2$nит|ат ремонту.", ch, obj );
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

