/* $Id: commandplugin.h,v 1.1.2.2 2008/02/24 05:12:26 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef        COMMANDPLUGIN_H
#define        COMMANDPLUGIN_H

#include "plugin.h"
#include "wrappedcommand.h"

/** Represents a command with a profile on disk, defined as a separate plugin. 
*/
class CommandPlugin : public virtual WrappedCommand,                       
                      public virtual Plugin {
public:
        typedef ::Pointer<CommandPlugin> Pointer;

        virtual bool saveCommand() const;

        virtual void initialization( );
        virtual void destruction( );
        virtual CommandLoader * getLoader( ) const;
};

/** Convenience macro to shorten command definition */
#define COMMAND(C, cmdname)              \
const DLString C::COMMAND_NAME = cmdname; \
C::C( )                                  \
{                                        \
    this->name[EN] = COMMAND_NAME;           \
}                                        \
void C::run( Character* ch, const DLString& constArguments ) 


#endif
