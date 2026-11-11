#include "doors.h"
#include "loadsave.h"
#include "directions.h"
#include "character.h"
#include "core/object.h"
#include "room.h"
#include "act.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

void open_door_extra ( Character *ch, int door, void *pexit )
{
    EXIT_DATA *pexit_rev = 0;
    int exit_info;
    bool eexit = door == DIR_SOMEWHERE;
    Room *room = ch->in_room;

    if ( !pexit )
            return;

    exit_info = eexit?
                    ((EXTRA_EXIT_DATA *) pexit)->exit_info
            : ((EXIT_DATA *) pexit)->exit_info;

    if ( !IS_SET(exit_info, EX_CLOSED) )
    {
            ch->pecho( _("Здесь уже открыто.") );
            return;
    }

    if ( IS_SET(exit_info, EX_LOCKED) )
    {
            ch->pecho( _("Здесь заперто.") );
            return;
    }

    REMOVE_BIT( eexit?
                    ((EXTRA_EXIT_DATA *) pexit)->exit_info
            : ((EXIT_DATA *) pexit)->exit_info, EX_CLOSED);

    if ( eexit ) {
        DoorName dn = direction_doorname_langtext(((EXTRA_EXIT_DATA *) pexit)->short_desc_from, '4');
        LangText doorname = dn.lt();
        oldact(_("$c1 открывает $w."), ch, &doorname, 0, TO_ROOM );
        oldact(_("Ты открываешь $w."), ch, &doorname, 0, TO_CHAR );
    }
    else {
        DoorName dn = direction_doorname_langtext((EXIT_DATA *) pexit, '4');
        LangText doorname = dn.lt();
        oldact(_("$c1 открывает $W."), ch, 0, &doorname, TO_ROOM );
        oldact(_("Ты открываешь $W."), ch, 0, &doorname, TO_CHAR );
    }


    /* open the other side */
    if (!eexit && (pexit_rev = direction_reverse(room, door)))
    {
            REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
            DoorName dnr = direction_doorname_langtext(pexit_rev, '1');
            LangText dnrlt = dnr.lt();
            direction_target(room, door)->echo(POS_RESTING, _("%^w открывается."), &dnrlt);
    }
}

void open_door ( Character *ch, int door )
{
    if ( door < 0 || door > 5 )
            return;

    open_door_extra( ch, door, ch->in_room->exit[door] );
}

void unlock_door( Character *ch, int door )
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

bool open_portal( Character *ch, Object *obj )
{
    if ( !IS_SET(obj->value1(), EX_ISDOOR) )
    {
            ch->pecho( _("Ты не можешь сделать этого.") );
            return false;
    }

    if ( !IS_SET(obj->value1(), EX_CLOSED) )
    {
            ch->pecho( _("Это уже открыто.") );
            return false;
    }

    if ( IS_SET(obj->value1(), EX_LOCKED) )
    {
            ch->pecho( _("Здесь заперто.") );
            return false;
    }

    obj->value1(obj->value1() & ~EX_CLOSED);
    oldact(_("Ты открываешь $o4."),ch,obj,0,TO_CHAR);
    oldact(_("$c1 открывает $o4."),ch,obj,0,TO_ROOM);

    return true;
}


bool obj_is_closable( Object *obj )
{
    return obj->item_type == ITEM_PORTAL
           || obj->item_type == ITEM_CONTAINER
           || obj->item_type == ITEM_DRINK_CON;
}

/*
 * Objects in reach and the room's extra exits share one N.keyword count,
 * objects first: with one book carried, "2.book" is the first visible
 * extra exit called "book". N at or below the object count names an
 * object, so no exit is returned. A plain "book" (or "1.book") returns
 * the first matching exit regardless of objects; the caller decides
 * whether an object wins.
 */
EXTRA_EXIT_DATA * find_extra_exit_here( Character *ch, const char *argument )
{
    char buf[MAX_INPUT_LENGTH], keyword[MAX_INPUT_LENGTH];

    strncpy( buf, argument, sizeof(buf) - 1 );
    buf[sizeof(buf) - 1] = '\0';
    int number = number_argument( buf, keyword );

    if (number < 1 || keyword[0] == '\0')
        return 0;

    if (number > 1) {
        int objects = 0;

        for (int k = 1; k <= number; k++) {
            snprintf( buf, sizeof(buf), "%d.%s", k, keyword );
            if (!get_obj_here( ch, buf ))
                break;
            objects = k;
        }

        if (objects >= number)
            return 0;

        number -= objects;
    }

    // ExtraExitList::find returns the first match only: walk a copy,
    // dropping each hit, to reach the Nth visible one.
    ExtraExitList candidates = ch->in_room->extra_exits;
    DLString kw( keyword );
    EXTRA_EXIT_DATA *peexit;

    while ((peexit = candidates.find( kw ))) {
        if (ch->can_see( peexit ) && --number == 0)
            return peexit;

        candidates.remove( peexit );
    }

    return 0;
}
