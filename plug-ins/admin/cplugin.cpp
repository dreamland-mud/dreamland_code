/* $Id: cplugin.cpp,v 1.7.2.9.6.4 2009/09/24 14:09:12 rufina Exp $
 *
 * ruffina, 2004
 * based on CPlugin by NoFate, 2001
 */

#include <sstream>

#include "cplugin.h"
#include "class.h"
#include "so.h"
#include "pluginmanager.h"
#include "schedulertask.h"

#include "character.h"
#include "dlscheduler.h"
#include "merc.h"
#include "comm.h"
#include "arg_utils.h"
#include "descriptor.h"
#include "act.h"
#include "def.h"
#include "l10n.h"
#include "reloadwarning.h"

class PluginMessageTask : public SchedulerTask {
public:
    typedef ::Pointer<PluginMessageTask> Pointer;

    virtual void run( )
    {
        Descriptor *d;

        for (d = descriptor_list; d; d = d->next) 
            if (d->connected == CON_PLAYING && d->character) 
                d->character->pecho(_("Мир неуловимо изменился..."));
    }

    virtual int getPriority( ) const
    {
        return SCDP_AUTO + 1;
    }
};
            
void CPlugin::initialization( )
{
    if (descriptor_list)
        DLScheduler::getThis( )->putTaskNOW( PluginMessageTask::Pointer(NEW) );

    CommandPlugin::initialization( );
}

void CPlugin::destruction( )
{
    CommandPlugin::destruction( );
    DLScheduler::getThis( )->slay( PluginMessageTask::Pointer( NEW ) );
}
        
COMMAND(CPlugin, "plugin")
{
    DLString arguments = constArguments, arg;

    arg = arguments.getOneArgument( );

    if (arg.empty( ))
        usage( ch );
    else if (arg_is_list(arg))
        doList( ch );
    else if (arg_is(arg, "reload" ))
        doReload( ch, arguments );
    else if(arg_is(arg, "load")) {
        PluginManager *manager = PluginManager::getThis( );
        manager->setReloadOneRequest(arguments, 1);
        plugin_reload_warning( );
        ch->pecho( "Requesting load for plugin [%s].", arg.c_str( ) );

    } else if(arg_is(arg, "unload")) {
        PluginManager *manager = PluginManager::getThis( );
        manager->setReloadOneRequest(arguments, 2);
        plugin_reload_warning( );
        ch->pecho( "Requesting unload for plugin [%s].", arg.c_str( ) );

    } else
        usage( ch );
}

void CPlugin::usage( Character *ch )
{
    ch->pecho( 
        _("Использование: \r\n"
        "plugin list - список всех загруженных модулей.\r\n"
        "plugin reload [all|changed|<name> [<name>...]] \r\n"
        "            - перегрузка модулей (всех, измененных или по имени)") );
}

void CPlugin::doList( Character *ch )
{
    int cnt = 0;
    ostringstream buf;
    PluginManager::const_iterator i;
    PluginManager *manager = PluginManager::getThis( );

    buf << "{W  # | name                   | new | load time{x" << endl;

    for (i = manager->begin( ); i != manager->end( ); i++)
        buf << fmt(0, " %2d | %-20s |  %s  | %s \r\n", 
                        ++cnt, 
                        i->second.getName( ).c_str( ),
                        (i->second.isChanged( ) ? "*" : " "),
                        i->second.getLoadTime( ).getTimeAsString(
                                    "%d/%m/%y %H:%M:%S" ).c_str( ) );
    
    page_to_char( buf.str( ).c_str( ), ch );
}

void CPlugin::doReload( Character *ch, DLString arg )
{
    PluginManager *manager = PluginManager::getThis( );
    std::list<DLString> names = plugin_reload_names( arg );

    if (names.size( ) > 1) {
        // Several plugins in one pass: unload them all, load the set back once.
        DLString unknown = plugin_reload_unknown( manager, names );
        if (!unknown.empty( )) {
            ch->pecho( "Plugin [%s] not found.", unknown.c_str( ) );
            return;
        }
        manager->setReloadManyRequest( names );
        plugin_reload_warning( );
        ch->pecho( "Requesting reload for plugins [%s].", arg.c_str( ) );
        return;
    }

    arg = names.empty( ) ? DLString::emptyString : names.front( );

    if (arg.empty( ))
        usage( ch );
    else if (arg_is_all(arg)) {
        manager->setReloadAllRequest( );
        plugin_reload_warning( );
        ch->pecho("Requesting plugins reload.");

    } else if (arg_is_strict(arg, "most")) {
        manager->setReloadNonCriticalRequest( );
        plugin_reload_warning( );
        ch->pecho("Requesting reload of all plugins except for [descriptor].");

    } else if (arg_is_strict(arg, "changed")) {
        manager->setReloadChangedRequest( );
        plugin_reload_warning( );
        ch->pecho("Requesting changed plugins reload.");

    } else if (manager->isAvailable( arg )) {
        manager->setReloadOneRequest( arg );
        plugin_reload_warning( );
        ch->pecho( "Requesting reload for plugin [%s].", arg.c_str( ) );

    } else
        ch->pecho("Plugin not found.");
}


extern "C"
{
        SO::PluginList initialize_plugin_command( )
        {
                SO::PluginList ppl;

                Plugin::registerPlugin<CPlugin>( ppl );
                
                return ppl;
        }
}
