#include "doors.h"
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

