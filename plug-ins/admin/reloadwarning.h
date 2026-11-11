#ifndef RELOADWARNING_H
#define RELOADWARNING_H

#include "descriptor.h"
#include "character.h"

/**
 * Warns everyone in the game that a plugin reload is about to freeze the world.
 * Called when the request is queued: the command/servlet runs inside the
 * scheduler tick before SCDP_IOWRITE, while the reload itself runs after the
 * tick, so the message is flushed before the lag starts.
 * Inline because cplugin.cpp (libplugin_command) and forceservlet.cpp
 * (libadmin) are separate plugins and can't link each other's symbols.
 */
inline void plugin_reload_warning( )
{
    for (Descriptor *d = descriptor_list; d; d = d->next)
        if (d->connected == CON_PLAYING && d->character)
            d->character->pecho(
                "{lE{YThe world freezes: the gods are reshaping reality. Expect some lag.{lR"
                "{YМир замирает: боги перекраивают реальность. Возможна задержка.{lU"
                "{YСвіт завмирає: боги перекроюють реальність. Можлива затримка.{lx{x");
}

#endif
