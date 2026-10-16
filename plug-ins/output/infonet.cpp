/* $Id$
 *
 * ruffina, 2004
 */
#include "pcharacter.h"

#include "infonet.h"
#include "act.h"
#include "multimessage.h"
#include "descriptor.h"

/*
 * Info channel: world news (arrivals, departures, levels, deaths, clan raids)
 * heard by every player in the world. It used to need the crystal orb; the orb
 * was retired so newcomers see that the world is alive.
 */
bool infonet_listener( Descriptor *d, Character *ch, int min_level )
{
    if (!d->character || d->connected != CON_PLAYING)
        return false;

    if (ch && ch->is_immortal( ))
        return false;

    if (d->character->get_trust() < min_level)
        return false;

    if (d->character == ch)
        return false;

    return true;
}

/**
 * Send a infonet message with a bit more flexible formatting.
 * 'prefix' normally contains the '{CNews: ' frame but can be anything.
 */
void infonet( Character *ch, int min_level, const DLString &prefix, const char *fmt, ...)
{
    va_list args;
    DLString msg;

    va_start( args, fmt );
    msg = prefix + vfmt( NULL, fmt, args );
    va_end( args );

    infonet(msg.c_str(), ch, min_level);
}

void infonet( const char *string, Character *ch, int min_level )
{
    for (Descriptor *d = descriptor_list; d != 0; d = d->next)
        if (infonet_listener( d, ch, min_level ))
            oldact_p( string, d->character, 0, ch, TO_CHAR, POS_DEAD );
}

// Per-recipient info channel: resolve `format` in each listener's language and
// render its %N$ args per viewer, then oldact_p the $-codes. Same shape as the
// const char* overload above, just rendered inside the loop instead of once.
void infonet( Character *ch, int min_level, const MultiMessage &format, ... )
{
    va_list args;

    for (Descriptor *d = descriptor_list; d != 0; d = d->next) {
        if (!infonet_listener( d, ch, min_level ))
            continue;

        va_start( args, format );
        DLString msg = vfmt( d->character, format.getMessage( d->character ).c_str( ), args );
        va_end( args );
        oldact_p( msg.c_str( ), d->character, 0, ch, TO_CHAR, POS_DEAD );
    }
}
