/* $Id$
 *
 * ruffina, 2004
 */

#include "plugininitializer.h"
#include "commandmanager.h"
#include "admincommand.h"

//COMMAND_LOADER(AdminCommandLoader, "commands/admin")

CMDLOADER_DECL(admin)

CommandLoader * AdminCommand::getLoader( ) const
{
//    return AdminCommandLoader::getThis( );
    return CMDLOADER(admin)::getThis( );
}

