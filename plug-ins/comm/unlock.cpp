
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
#include "loadsave.h"
#include "act.h"
#include "merc.h"
#include "def.h"

/*--------------------------------------------------------------------
 *    unlock 
 *-------------------------------------------------------------------*/
static void unlock_door( Character *ch, int door )
{
    // 'unlock door'
    Object *key;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev = 0;
    Room *room = ch->in_room;

    pexit = ch->in_room->exit[door];

    if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
    {
            ch->pecho( "Здесь уже не заперто." );
            return;
    }

    if ( pexit->key <= 0 )
    {
            ch->pecho( "К этой двери не существует ключа." );
            return;
    }

    if (!(key = get_key_carry( ch, pexit->key)))
    {
            ch->pecho( "У тебя нет ключа." );
            return;
    }

    const char *doorname = direction_doorname(pexit);

    ch->pecho("Ты отпираешь %O5 и открываешь %N4.", key, doorname);
    ch->recho("%^C1 отпирает %O5 и открывает %N4.", ch, key, doorname);

    REMOVE_BIT(pexit->exit_info, EX_LOCKED | EX_CLOSED);

    // unlock the other side
    if ((pexit_rev = direction_reverse(room, door)))
    {
            REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED | EX_CLOSED );
            direction_target(room, door)->echo(POS_RESTING, "%^N1 щелкает.", direction_doorname(pexit_rev));
    }
}

CMDRUNP( unlock )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    Object *key;
    int door;
    EXTRA_EXIT_DATA *peexit;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
            ch->pecho( "Отпереть что?" );
            return;
    }

    if (( door = find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY ) ) >= 0)
    {
        unlock_door( ch, door );
        return;
    }

    bool canBeDoor = direction_lookup(arg) >= 0;

    if (!canBeDoor && ( obj = get_obj_here( ch, arg ) ) != 0 )
    {
        // portal stuff
        if ( obj->item_type == ITEM_PORTAL )
        {
            if (!IS_SET(obj->value1(),EX_LOCKED))
            {
                    ch->pecho( "Здесь уже не заперто." );
                    return;
            }

            if (!IS_SET(obj->value1(),EX_ISDOOR))
            {
                    ch->pecho("%^O4 невозможно отпереть.", obj);
                    return;
            }

            if (obj->value4() <= 0)
            {
                ch->pecho( "К %O3 не существует ключа.", obj);
                return;
            }

            if (!(key = get_key_carry(ch,obj->value4())))
            {
                    ch->pecho( "У тебя нет ключа." );
                    return;
            }

            ch->pecho("Ты отпираешь %1$O4 %2$O5 и открываешь %1$P2.", obj, key);
            ch->recho("%1$^C1 отпирает %2$O4 %3$O5 и открывает %2$P2.", ch, obj, key);

            obj->value1(obj->value1() & ~EX_LOCKED);
            obj->value1(obj->value1() & ~EX_CLOSED);
        }
        else if ( obj->item_type == ITEM_CONTAINER )
        {
            // 'unlock object'
            if ( !IS_SET(obj->value1(), CONT_LOCKED) )
            {
                    ch->pecho( "Здесь уже не заперто." );
                    return;
            }

            if ( obj->value2() < 0 )
            {
                    ch->pecho( "К %O3 не существует ключа.", obj);
                    return;
            }

            bool canLock = obj->behavior && obj->behavior->canLock( ch );
            key = get_key_carry( ch, obj->value2());

            if (canLock || key) 
            {
                if (key) {
                    ch->pecho("Ты отпираешь %1$O4 %2$O5 и открываешь %1$P2.", obj, key);
                    ch->recho("%1$^C1 отпирает %2$O4 %3$O5 и открывает %2$P2.", ch, obj, key);
                } else {
                    ch->pecho("Ты отпираешь ключом %1$O4 и открываешь %1$P2.", obj);
                    ch->recho("%1$^C1 отпирает ключом %2$O4 и открывает %2$P2.", ch, obj);
                }

                obj->value1(obj->value1() & ~CONT_LOCKED);
                obj->value1(obj->value1() & ~CONT_CLOSED);

            } else if (!canLock && obj->value2() <= 0) {
                ch->pecho("%^O1 -- чья-то личная собственность, ключ есть только у хозяина или хозяйки.", obj);
                return;
            } else {
                ch->pecho( "У тебя нет ключа." );
                return;
            }
        }
        else if ( obj->item_type == ITEM_DRINK_CON ) {
            // uncork a bottle
            if (!IS_SET(obj->value3(), DRINK_LOCKED)) {
                ch->pecho( "Тут не заперто и не закупорено." );
                return;
            }

            key = get_key_carry( ch, obj->value4() );

            if (!key) {
                if (IS_SET(obj->value3(), DRINK_CLOSE_CORK)) 
                    ch->pecho( "У тебя нечем вытащить пробку." );
                else if (IS_SET(obj->value3(), DRINK_CLOSE_NAIL))
                    ch->pecho( "У тебя нечем оторвать крышку." );
                else
                    ch->pecho( "У тебя нечем открыть эту емкость." );
                
                return;
            }

            if (IS_SET(obj->value3(), DRINK_CLOSE_CORK)) {
                oldact("Ты расшатываешь пробку в $O6 с помощью $o4.", ch, key, obj, TO_CHAR );
                oldact("$c1 расшатывает пробку в $O6 с помощью $o4.", ch, key, obj, TO_ROOM );
            }
            else if (IS_SET(obj->value3(), DRINK_CLOSE_NAIL)) {
                oldact("Ты выдергиваешь гвозди из крышки $O2 с помощью $o4.", ch, key, obj, TO_CHAR );
                oldact("$c1 выдергивает гвозди из крышки $O2 с помощью $o4.", ch, key, obj, TO_ROOM );
            }
            else {
                oldact("Ты открываешь $o5 $O2.", ch, key, obj, TO_CHAR );
                oldact("$c1 открывает $o5 $O2.", ch, key, obj, TO_ROOM );
            }

            obj->value3(obj->value3() & ~DRINK_LOCKED);
            obj->value3(obj->value3() & ~DRINK_CLOSED);
                
        }
        else
        {
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

        if ( !IS_SET(peexit->exit_info, EX_LOCKED) )
        {
                ch->pecho( "Здесь уже не заперто." );
                return;
        }

        if ( peexit->key <= 0 )
        {
                ch->pecho( "К этой двери не существует ключа." );
                return;
        }

        if (!(key = get_key_carry( ch, peexit->key)))
        {
                ch->pecho( "У тебя нет ключа." );
                return;
        }

        ch->pecho("Ты отпираешь %O5 и открываешь %N4.", key, peexit->short_desc_from);
        ch->recho("%^C1 отпирает %O5 и открывает %N4.", ch, key, peexit->short_desc_from);

        REMOVE_BIT(peexit->exit_info, EX_LOCKED | EX_CLOSED);

        return;
    }

    find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY|FEX_VERBOSE );
}



