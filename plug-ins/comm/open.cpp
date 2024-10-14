#include "core/object.h"
#include "character.h"
#include "room.h"
#include "commandtemplate.h"
#include "wrapperbase.h"
#include "wrappertarget.h"
#include "loadsave.h"
#include "directions.h"
#include "doors.h"
#include "vnum.h"
#include "save.h"
#include "act.h"
#include "merc.h"
#include "def.h"

/*--------------------------------------------------------------------
 *    open 
 *-------------------------------------------------------------------*/
static bool oprog_cant_open( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "CantOpen", "C", ch );
    FENIA_NDX_CALL( obj, "CantOpen", "OC", obj, ch );
    return false;
}

static bool oprog_open(Object *obj, Character *ch)
{
    FENIA_CALL( obj, "Open", "C", ch );
    FENIA_NDX_CALL( obj, "Open", "OC", obj, ch );
    return false;
}

static bool oprog_open_msg(Object *obj, Character *ch)
{
    FENIA_CALL( obj, "OpenMsg", "C", ch );
    FENIA_NDX_CALL( obj, "OpenMsg", "OC", obj, ch );
    return false;
}

static bool open_drink_container( Character *ch, Object *obj )
{
    if (!IS_SET(obj->value3(), DRINK_CLOSED)) {
        ch->pecho( "%1$^O1 и так не запер%1$Gто|т|та|ты.", obj );
        return false;
    }
    
    if (IS_SET(obj->value3(), DRINK_LOCKED)) {
        if (IS_SET(obj->value3(), DRINK_CLOSE_CORK))
            ch->pecho( "%1$^O1 плотно закупоре%1$Gно|н|на|ны пробкой, поищи штопор.", obj );
        else if (IS_SET(obj->value3(), DRINK_CLOSE_NAIL))
            ch->pecho( "%1$^O1 закры%1$Gто|т|та|ты крышкой и заколоче%1$Gн|но|на|ны.", obj );
        else if (IS_SET(obj->value3(), DRINK_CLOSE_KEY))
            ch->pecho( "%1$^O1 крепко запер%1$Gто|т|та|ты.", obj );
        else
            ch->pecho( "%1$^O1 запер%1$Gто|т|та|ты.", obj );

        return false;
    }
    
    obj->value3(obj->value3() & ~DRINK_CLOSED);

    if (IS_SET(obj->value3(), DRINK_CLOSE_CORK)) {
        Object *cork;

        cork = create_object( get_obj_index( OBJ_VNUM_CORK ), 0 );
        obj_to_char( cork, ch );

        oldact("Ты вынимаешь пробку из $O2.", ch, 0, obj, TO_CHAR );
        oldact("$c1 вынимает пробку из $O2.", ch, 0, obj, TO_ROOM );
    }
    else if (IS_SET(obj->value3(), DRINK_CLOSE_NAIL)) {
        oldact("Ты открываешь крышку $O2.", ch, 0, obj, TO_CHAR );
        oldact("$c1 открывает крышку $O2.", ch, 0, obj, TO_ROOM );
    }
    else {
        oldact("Ты открываешь $O4.", ch, 0, obj, TO_CHAR );
        oldact("$c1 открывает $O4.", ch, 0, obj, TO_ROOM );
    }

    return true;
}

static bool open_container( Character *ch, Object *obj )
{
    if ( !IS_SET(obj->value1(), CONT_CLOSED) )
    {
            ch->pecho( "Это уже открыто." );
            return false;
    }

    if ( !IS_SET(obj->value1(), CONT_CLOSEABLE) )
    {
            ch->pecho( "Ты не можешь сделать этого." );
            return false;
    }

    if ( IS_SET(obj->value1(), CONT_LOCKED) )
    {
            ch->pecho( "Здесь заперто." );
            return false;
    }
    
    if (oprog_cant_open( obj, ch ))
        return false;

    obj->value1(obj->value1() & ~CONT_CLOSED);

    if (!oprog_open_msg( obj, ch )) {
        oldact("Ты открываешь $o4.",ch,obj,0,TO_CHAR);
        oldact("$c1 открывает $o4.", ch, obj, 0, TO_ROOM);
    }

    oprog_open( obj, ch );
    return true;
}



CMDRUNP( open )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    EXTRA_EXIT_DATA *peexit;
    int door;
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->pecho( "Открыть что?" );
        return;
    }

    bool canBeDoor = direction_lookup(arg) >= 0;

    if (( door = find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY ) ) >= 0)
    {
        open_door( ch, door );
        return;
    }

    if (!canBeDoor && ( obj = get_obj_here( ch, arg ) ) != 0 )
    {
        bool changed = false;
        
        switch (obj->item_type) {
        case ITEM_PORTAL:
            changed = open_portal( ch, obj );
            break;
            
        case ITEM_DRINK_CON:
            changed = open_drink_container( ch, obj );
            break;
            
        case ITEM_CONTAINER:
            changed = open_container( ch, obj );
            break;

        default:
            ch->pecho( "%^O4 невозможно открыть.", obj );
            return;
        }
        
        if ( obj->in_room != 0 && changed )
            save_items( obj->in_room );
            
        return;
    }

    if ((peexit = ch->in_room->extra_exits.find(arg))
            && ch->can_see( peexit ) )
    {
        open_door_extra( ch, DIR_SOMEWHERE, (void *)peexit );
        return;
    }        

    find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY|FEX_VERBOSE );
}


