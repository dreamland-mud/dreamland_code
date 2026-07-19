
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
#include "l10n.h"

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
            ch->pecho( _("Здесь уже не заперто.") );
            return;
    }

    if ( pexit->key <= 0 )
    {
            ch->pecho( _("К этой двери не существует ключа.") );
            return;
    }

    if (!(key = get_key_carry( ch, pexit->key)))
    {
            ch->pecho( _("У тебя нет ключа.") );
            return;
    }

    DoorName dn = direction_doorname_langtext(pexit, '4');
    LangText doorname = dn.lt();

    ch->pecho(_("Ты отпираешь %O5 и открываешь %w."), key, &doorname);
    ch->recho(_("%^C1 отпирает %O5 и открывает %w."), ch, key, &doorname);

    REMOVE_BIT(pexit->exit_info, EX_LOCKED | EX_CLOSED);

    // unlock the other side
    if ((pexit_rev = direction_reverse(room, door)))
    {
            REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED | EX_CLOSED );
            DoorName dnr = direction_doorname_langtext(pexit_rev, '1');
            LangText dnrlt = dnr.lt();
            direction_target(room, door)->echo(POS_RESTING, _("%^w щелкает."), &dnrlt);
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
            ch->pecho( _("Отпереть что?") );
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
                    ch->pecho( _("Здесь уже не заперто.") );
                    return;
            }

            if (!IS_SET(obj->value1(),EX_ISDOOR))
            {
                    ch->pecho(_("%^O4 невозможно отпереть."), obj);
                    return;
            }

            if (obj->value4() <= 0)
            {
                ch->pecho( _("К %O3 не существует ключа."), obj);
                return;
            }

            if (!(key = get_key_carry(ch,obj->value4())))
            {
                    ch->pecho( _("У тебя нет ключа.") );
                    return;
            }

            ch->pecho(_("Ты отпираешь %1$O4 %2$O5 и открываешь %1$P2."), obj, key);
            ch->recho(_("%1$^C1 отпирает %2$O4 %3$O5 и открывает %2$P2."), ch, obj, key);

            obj->value1(obj->value1() & ~EX_LOCKED);
            obj->value1(obj->value1() & ~EX_CLOSED);
        }
        else if ( obj->item_type == ITEM_CONTAINER )
        {
            // 'unlock object'
            if ( !IS_SET(obj->value1(), CONT_LOCKED) )
            {
                    ch->pecho( _("Здесь уже не заперто.") );
                    return;
            }

            if ( obj->value2() < 0 )
            {
                    ch->pecho( _("К %O3 не существует ключа."), obj);
                    return;
            }

            bool canLock = obj->behavior && obj->behavior->canLock( ch );
            key = get_key_carry( ch, obj->value2());

            if (canLock || key) 
            {
                if (key) {
                    ch->pecho(_("Ты отпираешь %1$O4 %2$O5 и открываешь %1$P2."), obj, key);
                    ch->recho(_("%1$^C1 отпирает %2$O4 %3$O5 и открывает %2$P2."), ch, obj, key);
                } else {
                    ch->pecho(_("Ты отпираешь ключом %1$O4 и открываешь %1$P2."), obj);
                    ch->recho(_("%1$^C1 отпирает ключом %2$O4 и открывает %2$P2."), ch, obj);
                }

                obj->value1(obj->value1() & ~CONT_LOCKED);
                obj->value1(obj->value1() & ~CONT_CLOSED);

            } else if (!canLock && obj->value2() <= 0) {
                ch->pecho(_("%^O1 -- чья-то личная собственность, ключ есть только у хозяина или хозяйки."), obj);
                return;
            } else {
                ch->pecho( _("У тебя нет ключа.") );
                return;
            }
        }
        else if ( obj->item_type == ITEM_DRINK_CON ) {
            // uncork a bottle
            if (!IS_SET(obj->value3(), DRINK_LOCKED)) {
                ch->pecho( _("Тут не заперто и не закупорено.") );
                return;
            }

            key = get_key_carry( ch, obj->value4() );

            if (!key) {
                if (IS_SET(obj->value3(), DRINK_CLOSE_CORK)) 
                    ch->pecho( _("У тебя нечем вытащить пробку.") );
                else if (IS_SET(obj->value3(), DRINK_CLOSE_NAIL))
                    ch->pecho( _("У тебя нечем оторвать крышку.") );
                else
                    ch->pecho( _("У тебя нечем открыть эту емкость.") );
                
                return;
            }

            if (IS_SET(obj->value3(), DRINK_CLOSE_CORK)) {
                oldact(_("Ты расшатываешь пробку в $O6 с помощью $o4."), ch, key, obj, TO_CHAR );
                oldact(_("$c1 расшатывает пробку в $O6 с помощью $o4."), ch, key, obj, TO_ROOM );
            }
            else if (IS_SET(obj->value3(), DRINK_CLOSE_NAIL)) {
                oldact(_("Ты выдергиваешь гвозди из крышки $O2 с помощью $o4."), ch, key, obj, TO_CHAR );
                oldact(_("$c1 выдергивает гвозди из крышки $O2 с помощью $o4."), ch, key, obj, TO_ROOM );
            }
            else {
                oldact(_("Ты открываешь $o5 $O2."), ch, key, obj, TO_CHAR );
                oldact(_("$c1 открывает $o5 $O2."), ch, key, obj, TO_ROOM );
            }

            obj->value3(obj->value3() & ~DRINK_LOCKED);
            obj->value3(obj->value3() & ~DRINK_CLOSED);
                
        }
        else
        {
            ch->pecho( _("Это не контейнер.") );
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
                ch->pecho( _("Это не дверь!") );
                return;
        }

        if ( !IS_SET(peexit->exit_info, EX_LOCKED) )
        {
                ch->pecho( _("Здесь уже не заперто.") );
                return;
        }

        if ( peexit->key <= 0 )
        {
                ch->pecho( _("К этой двери не существует ключа.") );
                return;
        }

        if (!(key = get_key_carry( ch, peexit->key)))
        {
                ch->pecho( _("У тебя нет ключа.") );
                return;
        }

        ch->pecho(_("Ты отпираешь %O5 и открываешь %N4."), key, peexit->short_desc_from.getForLang(viewerLang(ch)).c_str());
        ch->recho(_("%^C1 отпирает %O5 и открывает %N4."), ch, key, peexit->short_desc_from.getForLang(viewerLang(ch)).c_str());

        REMOVE_BIT(peexit->exit_info, EX_LOCKED | EX_CLOSED);

        return;
    }

    find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY|FEX_VERBOSE );
}



