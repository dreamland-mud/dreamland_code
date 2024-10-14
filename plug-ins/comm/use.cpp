#include "character.h"
#include "core/object.h"
#include "room.h"
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

/*
 * fenia-related commands: use 
 */
static bool oprog_use( Object *obj, Character *ch, const char *argument )
{
    if (aquest_trigger(obj, ch, "Use", "OCs", obj, ch, argument))
        return true;

    if (behavior_trigger(obj, "Use", "OCs", obj, ch, argument))
        return true;

    FENIA_CALL( obj, "Use", "Cs", ch, argument );
    FENIA_NDX_CALL( obj, "Use", "OCs", obj, ch, argument );
    BEHAVIOR_CALL( obj, use, ch, argument );

    switch(obj->item_type) {
        case ITEM_POTION:
            if (obj->carried_by != ch || obj->wear_loc != wear_none) 
                ch->pecho("%1$^O1 долж%1$Gно|ен|на|ны находиться в твоем инвентаре.", obj);
            else {
                DLString idArg = DLString( obj->getID( ) ) + " " + argument;
                interpret_cmd( ch, "quaff", idArg.c_str( ) );
             }
            return true;
        case ITEM_SCROLL:
            if (obj->carried_by != ch || obj->wear_loc != wear_none) 
                ch->pecho("%1$^O1 долж%1$Gно|ен|на|ны находиться в твоем инвентаре.", obj);
            else {
                DLString idArg = DLString( obj->getID( ) ) + " " + argument;
                interpret_cmd( ch, "recite", idArg.c_str( ) );
            }
            return true;
        case ITEM_WAND:
            if (obj->wear_loc != wear_hold) 
                ch->pecho("%1$^O4 сперва необходимо зажать в руках.", obj);
            else
                interpret_cmd( ch, "zap", argument );
            return true;
        case ITEM_STAFF:
            if (obj->wear_loc != wear_hold) 
                ch->pecho("%1$^O4 сперва необходимо зажать в руках.", obj);
            else
                interpret_cmd( ch, "brandish", argument );
            return true;
    }

    return false;
}

CMDRUNP( use )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;

    argument = one_argument( argument, arg );

    if (!arg[0]) {
        ch->pecho("Использовать что?");
        return;
    }

    // First try to use items in your own inventory/eq,
    // then items on the floor, as it often causes confusion.
    obj = get_obj_wear_carry(ch, arg, 0);
    if (!obj)
        obj = get_obj_here(ch, arg);

    if (!obj)
    {
        oldact("Ты не видишь здесь этого.", ch, 0, 0, TO_CHAR);
        return;
    }
    
    if (oprog_use( obj, ch, argument ))
        return;
    
    // Can only "handle" something in the inventory -- otherwise, "touch"
    if (obj->carried_by == ch && obj->wear_loc == wear_none) {
        oldact("Ты вертишь в руках $o4, не находя способа это использовать.", ch, obj, 0, TO_CHAR);
        oldact("$c1 вертит в руках $o4, не находя способа это использовать.", ch, obj, 0, TO_ROOM);
    } else {
        oldact("Ты озадаченно ощупываешь $o4, не находя способа это использовать.", ch, obj, 0, TO_CHAR);
        oldact("$c1 озадаченно ощупывает $o4, не находя способа это использовать.", ch, obj, 0, TO_ROOM);
    }
}        

