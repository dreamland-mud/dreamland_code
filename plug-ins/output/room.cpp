/* $Id$
 *
 * ruffina, 2004
 */
#include "character.h"
#include "room.h"
#include "merc.h"
#include "def.h"

void Room::echo( int pos, const char *f, ... ) const
{
    va_list av;

    va_start( av, f );
    vecho( pos, f, av );
    va_end( av );
}

void Room::vecho( int pos, const char *f, va_list av ) const
{
    for (Character *rch = people; rch; rch = rch->next_in_room)
        if (rch->position >= pos)
            rch->vpecho( f, av );
}

/* Trilinguality (Trello 2594, Phase 4): resolve the format in each occupant's
 * display language. Room echoes are plain %-format (no $-codes), so no
 * act_to_fmt. RU/untranslated -> source literal, byte-identical to the above. */
void Room::echo( int pos, const MultiMessage &f, ... ) const
{
    va_list av;

    va_start( av, f );
    vecho( pos, f, av );
    va_end( av );
}

void Room::vecho( int pos, const MultiMessage &mm, va_list av ) const
{
    const char *slot[LANG_MAX] = { 0, 0, 0 };

    for (Character *rch = people; rch; rch = rch->next_in_room) {
        if (rch->position < pos)
            continue;

        lang_t lg = viewerLang(rch);
        if (slot[lg] == 0)
            slot[lg] = mm.getMessage(lg).c_str();

        rch->vpecho( slot[lg], av );
    }
}

