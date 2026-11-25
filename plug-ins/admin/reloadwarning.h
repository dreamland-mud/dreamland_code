#ifndef RELOADWARNING_H
#define RELOADWARNING_H

#include <list>
#include "descriptor.h"
#include "character.h"
#include "pluginmanager.h"

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

/**
 * Split a reload target into plugin names: 'comm loadsave' or 'comm,loadsave'.
 * One name (or a keyword like most) comes back as a one-element list.
 */
inline std::list<DLString> plugin_reload_names( const DLString &arg )
{
    DLString rest = arg;

    rest.substitute( ',', ' ' );
    return rest.split( " " );
}

/** The first of several names that is no loaded plugin, or empty when all are. */
inline DLString plugin_reload_unknown( PluginManager *manager, const std::list<DLString> &names )
{
    for (std::list<DLString>::const_iterator n = names.begin( ); n != names.end( ); n++)
        if (!manager->isAvailable( *n ))
            return *n;
    return DLString::emptyString;
}

#endif
