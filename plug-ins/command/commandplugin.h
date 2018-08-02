/* $Id: commandplugin.h,v 1.1.2.2 2008/02/24 05:12:26 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef	COMMANDPLUGIN_H
#define	COMMANDPLUGIN_H

#include "plugin.h"
#include "command.h"

class CommandPlugin : public virtual XMLCommand, public virtual Plugin {
public:
	typedef ::Pointer<CommandPlugin> Pointer;

	virtual void initialization( );
	virtual void destruction( );
};

#define COMMAND(C, cmdname)              \
const DLString C::COMMAND_NAME = cmdname; \
C::C( )                                  \
{                                        \
    this->name = COMMAND_NAME;           \
}                                        \
void C::run( Character* ch, const DLString& constArguments ) 

#endif
