#ifndef SKILL_SOUND_H
#define SKILL_SOUND_H

#include "msp.h"
#include "character.h"
#include "room.h"
#include "merc.h"

/**
 * A skill's MSP cue with the listeners of Fenia's .tmp.sound.room
 * (dreamland_fenia utils/sound): the actor at 70, a player victim at 100 if
 * awake, and for a roomwide cue every other awake player in the room at 60.
 * victim may be 0. Header-only, so a caller's plugin needs no new link to
 * descriptor and descriptor itself needs no reload.
 */
inline void skill_sound( Character *ch, Character *victim, const DLString &name, bool roomwide = false )
{
    msp_sound( ch, name, 70, 1, 40 );

    if (roomwide && ch->in_room)
        for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room)
            if (rch != ch && rch != victim && IS_AWAKE(rch))
                msp_sound( rch, name, 60, 1, 40 );

    if (victim && victim != ch && IS_AWAKE(victim))
        msp_sound( victim, name, 100, 1, 50 );
}

#endif
