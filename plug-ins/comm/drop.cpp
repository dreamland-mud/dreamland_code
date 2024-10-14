#include "core/object.h"
#include "character.h"
#include "room.h"
#include "roomutils.h"
#include "material.h"
#include "act.h"
#include "loadsave.h"
#include "commandtemplate.h"
#include "damageflags.h"
#include "save.h"
#include "wearloc_utils.h"
#include "dreamland.h"
#include "merc.h"
#include "def.h"

bool oprog_drop( Object *obj, Character *ch );

#define DROP_OBJ_NORMAL  0
#define DROP_OBJ_EXTRACT 1

static int drop_obj( Character *ch, Object *obj )
{
    obj_from_char( obj );
    obj_to_room( obj, ch->in_room );

    ch->pecho( "Ты бросаешь %1$O4.", obj );
    oldact("$c1 бросает $o4.", ch, obj, 0, TO_ROOM );

    if (oprog_drop( obj, ch ))
        return DROP_OBJ_EXTRACT;
    
    if (RoomUtils::isWater( ch->in_room ) 
        && !obj->may_float( ) 
        && !IS_SET(obj->extra_flags, ITEM_NOPURGE)
        && obj->pIndexData->limit <= 0
        && material_swims( obj ) == SWIM_NEVER)
    {
        ch->recho( "%1$^O1 тон%1$nет|ут в %2$N6.", obj, ch->in_room->getLiquid()->getShortDescr( ).c_str( ) );
        ch->pecho( "%1$^O1 тон%1$nет|ут в %2$N6.", obj, ch->in_room->getLiquid()->getShortDescr( ).c_str( ) );
    }
    else if (IS_OBJ_STAT(obj, ITEM_MELT_DROP))
    {
        ch->recho( "%1$^O1 превраща%1$nется|ются в ничто и исчеза%1$nет|ют.", obj );
        ch->pecho( "%1$^O1 превраща%1$nется|ются в ничто и исчеза%1$nет|ют.", obj );
    }
    else if (!RoomUtils::isWater( ch->in_room ) 
             && ch->in_room->getSectorType() != SECT_AIR
             && ch->in_room->getSectorType() != SECT_FOREST
             && ch->in_room->getSectorType() != SECT_DESERT
             && material_is_flagged( obj, MAT_FRAGILE )
             && chance( 40 ))
    {
        ch->recho( "%1$^O1 пада%1$nет|ют и разбива%1$nется|ются на мелкие осколки.", obj );
        ch->pecho( "%1$^O1 пада%1$nет|ют и разбива%1$nется|ются на мелкие осколки.", obj );
    }
    else
        return DROP_OBJ_NORMAL;

    obj_dump_content(obj);
    extract_obj( obj );
    return DROP_OBJ_EXTRACT;
}

CMDRUNP( drop )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    DLString arguments = argument;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->pecho("Бросить что?");
        return;
    }

    if ( is_number( arg ) && get_arg_id( arg ) == 0)
    {
        /* 'drop NNNN coins' */
        int amount, gold = 0, silver = 0;

        amount   = atoi(arg);
        argument = one_argument( argument, arg );

        if (!Money::parse( ch, arg, amount, gold, silver ))
            return;

        ch->silver -= silver;
        ch->gold -= gold;
        Money::dematerialize( ch->in_room->contents, gold, silver );
        obj = Money::create( gold, silver );

        if ( RoomUtils::isWater( ch->in_room ) )
        {
            extract_obj( obj );
            oldact("Монеты падают и тонут в $n6.", ch, ch->in_room->getLiquid()->getShortDescr( ).c_str( ), 0, TO_ROOM);
            oldact("Монеты падают и тонут в $n6.", ch, ch->in_room->getLiquid()->getShortDescr( ).c_str( ), 0, TO_CHAR);
        }
        else
        {
            obj_to_room( obj, ch->in_room );

            if (obj->value0() == 1 || obj->value1() == 1)
            {
                oldact("$c1 бросает монетку.", ch, 0, 0, TO_ROOM);
                ch->pecho( "Ты бросаешь монетку." );
            }
            else
            {
                oldact("$c1 бросает несколько монет.", ch, 0, 0, TO_ROOM);
                ch->pecho( "Ты бросаешь несколько монет." );
            }
         
        }

        return;
    }

    if (!arg_is_alldot( arg ))
    {
        /* 'drop obj' */
        if ( ( obj = get_obj_carry( ch, arg ) ) == 0 )
        {
            ch->pecho("У тебя нет этого.");
            return;
        }

        if (Item::canDrop( ch, obj, true )) 
            drop_obj( ch, obj );
    }
    else { /* 'drop all' or 'drop all.obj', drop all.'obj names' */
        bool found = false;
        Object *obj_next;

        dreamland->removeOption( DL_SAVE_OBJS );

        bool fAll = arg[3] == '\0';
        DLString objnames; 
        if (!fAll)
            objnames = DLString(arguments.substr(4)).getOneArgument( );
        
        for (obj = ch->carrying; obj != 0; obj = obj_next) {
            obj_next = obj->next_content;

            if (!fAll && !obj_has_name( obj, objnames, ch ))
                continue;
            if (!ch->can_see( obj ))
                continue;
            if (obj->wear_loc != wear_none)
                continue;
            if (!Item::canDrop( ch, obj, true ))
                continue;

            found = true;
            drop_obj( ch, obj );
        }

        dreamland->resetOption( DL_SAVE_OBJS );

        if (!found) {
            if (arg[3] == '\0')
                oldact("У тебя ничего нет.", ch, 0, arg, TO_CHAR );
            else
                oldact("У тебя нет $T.", ch, 0, is_number(&arg[4]) ? "этого":&arg[4], TO_CHAR );
        }
        else {
            save_items( ch->in_room );
        }
    }
}


