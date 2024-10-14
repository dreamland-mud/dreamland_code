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
 *   lock 
 *-------------------------------------------------------------------*/
static void lock_door( Character *ch, int door )
{
    // 'lock door'
    Object *key;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev = 0;
    Room *room = ch->in_room;

    pexit        = ch->in_room->exit[door];

    if ( IS_SET(pexit->exit_info, EX_NOLOCK) ) 
    {
        ch->pecho( "Это невозможно запереть." );
        return;
    }

    if ( pexit->key <= 0 )
    {
            ch->pecho( "Здесь нет замочной скважины -- просто закрой дверь." );
            return;
    }

    if (!(key = get_key_carry( ch, pexit->key)))
    {
            ch->pecho( "У тебя нет ключа." );
            return;
    }

    if ( IS_SET(pexit->exit_info, EX_LOCKED) )
    {
            ch->pecho( "Здесь уже заперто." );
            return;
    }

    const char *doorname = direction_doorname(pexit);

    if (IS_SET(pexit->exit_info, EX_CLOSED)) {
        ch->pecho("Ты запираешь %N4 %O5.", doorname, key);
        ch->recho("%^C1 запирает %N4 %O5.", ch, doorname, key);
    } else {
        ch->pecho("Ты закрываешь %N4 и запираешь %O5.", doorname, key);
        ch->recho("%^C1 закрывает %N4 и запирает %O5.", ch, doorname, key);
    }

    SET_BIT(pexit->exit_info, EX_LOCKED | EX_CLOSED);

    /* lock the other side */
    if ((pexit_rev = direction_reverse(room, door)))
    {
            SET_BIT( pexit_rev->exit_info, EX_LOCKED | EX_CLOSED);
            direction_target(room, door)->echo(POS_RESTING, "%^N1 защелкивается.", direction_doorname(pexit_rev));
    }
}

CMDRUNP( lock )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    Object *key;
    EXTRA_EXIT_DATA *peexit;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
            ch->pecho( "Запереть что?" );
            return;
    }

    if (( door = find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY ) ) >= 0)
    {
        lock_door( ch, door );
        return;
    }

    bool canBeDoor = direction_lookup(arg) >= 0;

    if (!canBeDoor && ( obj = get_obj_here( ch, arg ) ) != 0 )
    {
        // portal stuff
        if (obj->item_type == ITEM_PORTAL)
        {
                if ( !IS_SET(obj->value1(),EX_ISDOOR)
                        || IS_SET(obj->value1(),EX_NOCLOSE|EX_NOLOCK) )
                {
                        ch->pecho("%^O4 невозможно запереть.", obj);
                        return;
                }

                if (obj->value4() <= 0) 
                {
                    ch->pecho("В %O6 нет замочной скважины -- просто закрой.", obj);
                    return;
                }

                if (!(key = get_key_carry(ch, obj->value4())))
                {
                        ch->pecho("У тебя нет ключа.");
                        return;
                }

                if (IS_SET(obj->value1(), EX_LOCKED))
                {
                        ch->pecho( "%1$^O1 уже заперт%1$Gо||а.", obj );
                        return;
                }

                if (IS_SET(obj->value1(), EX_CLOSED)) {
                    ch->pecho("Ты запираешь %O4 %O5.", obj, key);
                    ch->recho("%^C1 запирает %O4 %O5.", ch, obj, key);
                } else {
                    ch->pecho("Ты закрываешь %O4 и запираешь %O5.", obj, key);
                    ch->recho("%^C1 закрывает %O4 и запирает %O5.", ch, obj, key);
                }
                
                obj->value1(obj->value1() | EX_LOCKED | EX_CLOSED);
        }
        else if ( obj->item_type == ITEM_CONTAINER )
        {
            // 'lock object'
            if ( obj->value2() < 0 )
            {
                    ch->pecho("В %O6 нет замочной скважины -- просто закрой.", obj);
                    return;
            }
            
            if ( IS_SET(obj->value1(), CONT_LOCKED) )
            {
                    ch->pecho( "%1$^O1 уже заперт%1$Gо||а.", obj );
                    return;
            }

            key = get_key_carry(ch, obj->value2());
            if (!key && (!obj->behavior || !obj->behavior->canLock(ch))) {
                    ch->pecho("У тебя нет ключа.");
                    return;
            }

            if (key) {
                if (IS_SET(obj->value1(), CONT_CLOSED)) {
                    ch->pecho("Ты запираешь %O4 %O5.", obj, key);
                    ch->recho("%^C1 запирает %O4 %O5.", ch, obj, key);
                } else {
                    ch->pecho("Ты закрываешь %O4 и запираешь %O5.", obj, key);
                    ch->recho("%^C1 закрывает %O4 и запирает %O5.", ch, obj, key);
                }
            } else {
                if (IS_SET(obj->value1(), CONT_CLOSED)) {
                    ch->pecho("Ты запираешь %O4 на ключ.", obj);
                    ch->recho("%^C1 запирает %O4 на ключ.", ch, obj);
                } else {
                    ch->pecho("Ты закрываешь %O4 и запираешь на ключ.", obj);
                    ch->recho("%^C1 закрывает %O4 и запирает на ключ.", ch, obj);
                }
            }

            obj->value1(obj->value1() | CONT_LOCKED | CONT_CLOSED);

        }
        else if (obj->item_type == ITEM_DRINK_CON) {
            // lock drink containers

            if (IS_SET(obj->value3(), DRINK_LOCKED)) {
                if (IS_SET(obj->value3(), DRINK_CLOSE_CORK))
                    ch->pecho( "%1$^O1 и так плотно закупоре%1$Gно|н|но|ны пробкой.", obj );
                else if (IS_SET(obj->value3(), DRINK_CLOSE_NAIL))
                    ch->pecho( "%1$^O1 и так закры%1$Gто|т|то|ты крышкой и заколоче%1$Gно|н|но|ны.", obj );
                else
                    ch->pecho( "%1$^O1 и так крепко запер%1$Gто|т|то|ты.", obj );
            }
            else {
                ch->pecho("%1$^O1 уже невозможно закупорить или заколотить намертво.", obj );
            }
            
            return;
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

        if ( IS_SET(peexit->exit_info, EX_NOLOCK) ) 
        {
            ch->pecho( "Это невозможно запереть." );
            return;
        }

        if ( peexit->key <= 0 )
        {
                ch->pecho( "Здесь нет замочной скважины -- просто закрой." );
                return;
        }

        if (!(key = get_key_carry( ch, peexit->key)))
        {
                ch->pecho( "У тебя нет ключа." );
                return;
        }

        if ( IS_SET(peexit->exit_info, EX_LOCKED) )
        {
                ch->pecho( "Здесь уже заперто." );
                return;
        }

        if (IS_SET(peexit->exit_info, EX_CLOSED)) {
            ch->pecho("Ты запираешь %N4 %O5.", peexit->short_desc_from, key);
            ch->recho("%^C1 запирает %N4 %O5.", ch, peexit->short_desc_from, key);
        } else {
            ch->pecho("Ты закрываешь %N4 и запираешь %O5.", peexit->short_desc_from, key);
            ch->recho("%^C1 закрывает %N4 и запирает %O5.", ch, peexit->short_desc_from, key);
        }

        SET_BIT(peexit->exit_info, EX_LOCKED | EX_CLOSED);
        return;
    }


    find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY|FEX_VERBOSE );
}



