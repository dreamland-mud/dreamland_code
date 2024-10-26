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
 *    close 
 *-------------------------------------------------------------------*/
static bool oprog_close( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "Close", "C", ch );
    FENIA_NDX_CALL( obj, "Close", "OC", obj, ch );
    return false;
}

static void close_door( Character *ch, int door )
{
    // 'close door'
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev = 0;
    Room *room = ch->in_room;

    pexit        = ch->in_room->exit[door];
    if ( IS_SET(pexit->exit_info, EX_CLOSED) )
    {
            ch->pecho( "Здесь уже закрыто." );
            return;
    }

    SET_BIT(pexit->exit_info, EX_CLOSED);
    
    const char *doorname = direction_doorname(pexit);
    oldact("$c1 закрывает $N4.", ch, 0, doorname, TO_ROOM );
    oldact("Ты закрываешь $N4.", ch, 0, doorname, TO_CHAR );

    // close the other side
    if ((pexit_rev = direction_reverse(room, door)))
    {
            SET_BIT( pexit_rev->exit_info, EX_CLOSED );
            direction_target(room, door)->echo(POS_RESTING, "%^N1 закрывается.", direction_doorname(pexit_rev));
    }
}

CMDRUNP( close )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    EXTRA_EXIT_DATA *peexit;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->pecho( "Закрыть что?" );
        return;
    }

    bool canBeDoor = direction_lookup(arg) >= 0;

    if (( door = find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY ) ) >= 0)
    {
        close_door( ch, door );
        return;
    }

    if (!canBeDoor && ( obj = get_obj_here( ch, arg ) ) != 0 )
    {
        if ( obj->item_type == ITEM_PORTAL )
        {
            // portal stuff
            if ( !IS_SET(obj->value1(),EX_ISDOOR)
                    || IS_SET(obj->value1(),EX_NOCLOSE) )
            {
                ch->pecho( "Ты не можешь сделать этого." );
                return;
            }

            if ( IS_SET(obj->value1(),EX_CLOSED) )
            {
                ch->pecho( "Здесь уже закрыто." );
                return;
            }

            obj->value1(obj->value1() | EX_CLOSED);
            oldact("Ты закрываешь $o4.",ch,obj,0,TO_CHAR);
            oldact("$c1 закрывает $o4.",ch,obj,0,TO_ROOM);
        }
        else if ( obj->item_type == ITEM_CONTAINER )
        {
            // 'close object'
            if ( IS_SET(obj->value1(), CONT_CLOSED) )
            {
                ch->pecho( "Здесь уже закрыто." );
                return;
            }

            if ( !IS_SET(obj->value1(), CONT_CLOSEABLE) )
            {
                ch->pecho( "Ты не можешь сделать этого." );
                return;
            }

            obj->value1(obj->value1() | CONT_CLOSED);
            oldact("Ты закрываешь $o4.",ch,obj,0,TO_CHAR);
            oldact("$c1 закрывает $o4.", ch, obj, 0, TO_ROOM);
            oprog_close( obj, ch );
        }
        else if (obj->item_type == ITEM_DRINK_CON) {
            // cork a bottle 
            
            if (!IS_SET(obj->value3(), DRINK_CLOSE_CORK|DRINK_CLOSE_NAIL|DRINK_CLOSE_KEY)) {
                oldact("$O4 невозможно закрыть или закупорить.", ch, 0, obj, TO_CHAR );
                return;
            }

            if (IS_SET(obj->value3(), DRINK_CLOSED)) {
                oldact("$O4 уже закрыли.", ch, 0, obj, TO_CHAR );
                return;
            }
            
            if (IS_SET(obj->value3(), DRINK_CLOSE_CORK)) {
                Object *cork = get_obj_carry_vnum( ch, OBJ_VNUM_CORK );

                if (!cork) {
                    oldact("У тебя нет пробки от $O2.", ch, 0, obj, TO_CHAR );
                    oldact("$c1 шарит по карманам в поисках пробки.", ch, 0, obj, TO_ROOM );
                    return;
                }

                extract_obj( cork );
                oldact("Ты закупориваешь $O4 пробкой.", ch, 0, obj, TO_CHAR );
                oldact("$c1 закупоривает $O4 пробкой.", ch, 0, obj, TO_ROOM );
            }
            else if (IS_SET(obj->value3(), DRINK_CLOSE_NAIL)) {
                oldact("Ты закрываешь $O4 крышкой.", ch, 0, obj, TO_CHAR );
                oldact("$c1 закрывает $O4 крышкой.", ch, 0, obj, TO_ROOM );
            }
            else {
                oldact("Ты закрываешь $O4.", ch, 0, obj, TO_CHAR );
                oldact("$c1 закрывает $O4.", ch, 0, obj, TO_ROOM );
            }
            
            obj->value3(obj->value3() | DRINK_CLOSED);
        }
        else {
            ch->pecho( "Это не контейнер." );
            return;
        }

        if ( obj->in_room != 0 )
                save_items( obj->in_room );

        return;
    }

    if ((peexit = ch->in_room->extra_exits.find(arg))
            && ch->can_see( peexit ) )
    {
        if ( !IS_SET(peexit->exit_info, EX_ISDOOR) )
        {
                ch->pecho( "Это не дверь!" );
                return;
        }

        if ( IS_SET(peexit->exit_info, EX_CLOSED) )
        {
                ch->pecho( "Здесь уже закрыто." );
                return;
        }

        SET_BIT(peexit->exit_info, EX_CLOSED);
        oldact("$c1 закрывает $N4.", ch, 0, peexit->short_desc_from.get(LANG_DEFAULT).c_str(), TO_ROOM);
        oldact("Ты закрываешь $N4.", ch, 0, peexit->short_desc_from.get(LANG_DEFAULT).c_str(), TO_CHAR);

        return;
    }        

    find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY|FEX_VERBOSE );
}

