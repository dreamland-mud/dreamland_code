/* $Id: commandplugin.h,v 1.1.2.2 2008/02/24 05:12:26 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef        COMMANDPLUGIN_H
#define        COMMANDPLUGIN_H

#include "plugin.h"
#include "command.h"
#include "commandhelp.h"
#include "commandloader.h"
#include "wrappertarget.h"

/** Represents a command with a profile on disk, defined as a separate plugin. 
 * Such commands (and derived ones) are editable via cmdedit and their 'run'
 * method can be overridden from Fenia.
*/
class CommandPlugin : public virtual Command, 
                      public WrapperTarget, 
                      public virtual Plugin {
public:
        typedef ::Pointer<CommandPlugin> Pointer;

        /** Gets unique command ID for Fenia */
        virtual long long getID() const;

        /** Main entry point for command interpreter, with fenia override */
        virtual void entryPoint( Character *, const DLString & );

        virtual void initialization( );
        virtual void destruction( );
        virtual CommandLoader * getLoader( ) const;

protected:
        bool feniaOverride(Character *, const DLString &);
};

/** Default command loader that reads from 'commands' folder. */
class CommandPluginLoader : public CommandLoader {
public:
    typedef ::Pointer<CommandPluginLoader> Pointer;
    
    CommandPluginLoader();
    virtual ~CommandPluginLoader();
    
    virtual DLString getTableName( ) const; 
private:
    static const DLString TABLE_NAME;
};

extern CommandPluginLoader *commandPluginLoader;

/** Convenience macro to shorten command definition */
#define COMMAND(C, cmdname)              \
const DLString C::COMMAND_NAME = cmdname; \
C::C( )                                  \
{                                        \
    this->name = COMMAND_NAME;           \
}                                        \
void C::run( Character* ch, const DLString& constArguments ) 

#endif
