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
#include "descriptor.h"
#include "act.h"
#include "def.h"

class PluginMessageTask : public SchedulerTask {
public:
    typedef ::Pointer<PluginMessageTask> Pointer;

    virtual void run( )
    {
        Descriptor *d;

        for (d = descriptor_list; d; d = d->next) 
            if (d->connected == CON_PLAYING && d->character) 
                d->character->pecho("Мир неуловимо изменился...");
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
    else if (arg.strPrefix( "list" ))
        doList( ch );
    else if (arg.strPrefix( "reload" ))
        doReload( ch, arguments.getOneArgument( ) );
    else if(arg.strPrefix( "load" )) {
        PluginManager *manager = PluginManager::getThis( );
        manager->setReloadOneRequest(arguments, 1);
        ch->pecho( "Requesting load for plugin [%s].", arg.c_str( ) );
    } else if(arg.strPrefix( "unload" )) {
        PluginManager *manager = PluginManager::getThis( );
        manager->setReloadOneRequest(arguments, 2);
        ch->pecho( "Requesting unload for plugin [%s].", arg.c_str( ) );
    } else
        usage( ch );
}

void CPlugin::usage( Character *ch )
{
    ch->pecho( 
        "Использование: \r\n"
        "plugin list - список всех загруженных модулей.\r\n"
        "plugin reload [all|changed|<name>] \r\n"
        "            - перегрузка модулей (всех, измененных или по имени)" );
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
    
    if (arg.empty( ))
        usage( ch );
    else if (arg == "all") {
        manager->setReloadAllRequest( );
        ch->pecho("Requesting plugins reload.");

    } else if (arg == "most") {
        manager->setReloadNonCriticalRequest( );
        ch->pecho("Requesting reload of all plugins except for [descriptor].");

    } else if (arg == "changed") {
        manager->setReloadChangedRequest( );
        ch->pecho("Requesting changed plugins reload.");

    } else if (manager->isAvailable( arg )) {
        manager->setReloadOneRequest( arg );
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
