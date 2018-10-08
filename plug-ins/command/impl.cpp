/* $Id: impl.cpp,v 1.1.2.6.6.3 2008/05/22 11:00:38 rufina Exp $
 *
 * ruffina, 2004
 */
#include "so.h"
#include "xmlvariableregistrator.h"
#include "commandmanager.h"
#include "commandhelp.h"

extern "C"
{
        SO::PluginList initialize_command( )
        {
                SO::PluginList ppl;
                
                Plugin::registerPlugin<XMLVariableRegistrator<CommandHelp> >( ppl );
                Plugin::registerPlugin<CommandManager>( ppl );

                return ppl;
        }
}
