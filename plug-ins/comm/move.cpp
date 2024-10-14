#include "character.h"
#include "room.h"
#include "core/object.h"
#include "commandtemplate.h"
#include "move_utils.h"
#include "directions.h"
#include "loadsave.h"
#include "portalmovement.h"
#include "merc.h"
#include "def.h"

void do_visible(Character *);

/*-----------------------------------------------------------------------------
 * direction commands
 *----------------------------------------------------------------------------*/
CMDRUNP(north)
{
    move_char(ch, DIR_NORTH, argument);
}

CMDRUNP(east)
{
    move_char(ch, DIR_EAST, argument);
}

CMDRUNP(south)
{
    move_char(ch, DIR_SOUTH, argument);
}

CMDRUNP(west)
{
    move_char(ch, DIR_WEST, argument);
}

CMDRUNP(up)
{
    move_char(ch, DIR_UP, argument);
}

CMDRUNP(down)
{
    move_char(ch, DIR_DOWN, argument);
}

/*
 * Contributed by Alander
 */
CMDRUNP(visible)
{
    do_visible(ch);
}

/*
 * Экстра-выходы - разработка Тирна.
 */
CMDRUNP(enter)
{
    Object *portal = 0;
    EXTRA_EXIT_DATA *peexit = 0;

    // Syntax: enter, enter <portal>, enter <eexit>.
    if (!argument[0])
        portal = get_obj_room_type(ch, ITEM_PORTAL);
    else
    {
        portal = get_obj_list(ch, argument, ch->in_room->contents);
        peexit = ch->in_room->extra_exits.find(argument);
    }

    if (portal == 0 && peexit == 0)
    {
        if (!argument[0])
            ch->pecho("Куда ты хочешь войти?");
        else
            ch->pecho("Ты не видишь здесь такого портала или дополнительного выхода.");
        return;
    }

    if (peexit)
    {
        move_char(ch, peexit);
        return;
    }

    if (portal->item_type != ITEM_PORTAL)
    {
        ch->pecho("Ты не находишь пути внутрь %O2, это не портал.", portal);
        return;
    }

    PortalMovement(ch, portal).move();
}
