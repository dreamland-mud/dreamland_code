/* $Id$
 *
 * ruffina, 2004
 */
#ifndef COMMANDTEMPLATE_H
#define COMMANDTEMPLATE_H

#include "commandplugin.h"
#include "plugininitializer.h"
#include "defaultcommand.h"
#include "classselfregistratorplugin.h"

#define INITPRIO_COMMANDS 51

template <const char *&tn>
class CommandTemplate : public DefaultCommand, public CommandPlugin, public ClassSelfRegistratorPlugin<tn> {
public:
    typedef ::Pointer<CommandTemplate> Pointer;
    
    CommandTemplate( ) {
        name = cmdName;
    }

    virtual void run( Character * ch, const DLString & constArguments ) {
        DefaultCommand::run( ch, constArguments );
    }
    virtual void run( Character * ch, char *argument ) { 
        DefaultCommand::run( ch, argument );
    }

    virtual const DLString &getType( ) const {
        return ClassSelfRegistratorPlugin<tn>::getType( );
    }
    virtual CommandLoader * getLoader( ) const {
        return CommandPlugin::getLoader( );
    }
protected:
    virtual void initialization( ) 
    {
        ClassSelfRegistratorPlugin<tn>::initialization( );
        CommandPlugin::initialization( );
    }
    virtual void destruction( ) 
    {
        CommandPlugin::destruction( );
        ClassSelfRegistratorPlugin<tn>::destruction( );
    }
    
private:
    static const char *cmdName;
};

#define CMD_DUMMY(x)         dummyCmd_ ##x## _TypeName
#define CMD(x) CommandTemplate<CMD_DUMMY(x)>

#define CMD_DECL(x) \
const char *CMD_DUMMY(x) = "CMD(" #x ")"; \
template<> const char *CMD(x)::cmdName = #x; \
PluginInitializer<CMD(x)> dummyCmd_ ##x## _init(INITPRIO_COMMANDS);

#define CMDRUN(x) \
CMD_DECL(x) \
template <> void CMD(x)::run( Character* ch, const DLString& constArguments ) 

#define CMDRUNP(x) \
CMD_DECL(x) \
template <> void CMD(x)::run( Character* ch, char *argument ) 

#endif
