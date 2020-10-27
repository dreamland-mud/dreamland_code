/* $Id$
 *
 * ruffina, 2004
 */
#include "commandmanager.h"
#include "character.h"
#include "logstream.h"
#include "handler.h"
#include "merc.h"

#include "commandwrapper.h"
#include "nativeext.h"
#include "regcontainer.h"
#include "reglist.h"
#include "wrappermanager.h"
#include "wrap_utils.h"
#include "subr.h"

#include "def.h"

using namespace std;

NMI_INIT(CommandWrapper, "команда");

void 
CommandWrapper::setSelf(Scripting::Object *obj)
{
    if(obj) {
        commandManager->registrate( Pointer( this ) );
    } else {
        commandManager->unregistrate( Pointer( this ) );
    }

    self = obj;
}

void 
CommandWrapper::backup()
{
    commandManager->unregistrate( Pointer( this ) );
}

void 
CommandWrapper::run( Character * ch, const DLString &arg )
{
    RegisterList args;
    args.push_back(wrap(ch));
    args.push_back(arg);

    try {
        func.toFunction()->invoke(Register(self), args);
    } catch (::Exception e) {
        ch->send_to( e.what( ) );
    }
}

/*
XMLString name;
XMLStringList aliases, russian;
XMLFlagsNoEmpty extra;
XMLShortNoEmpty level;
XMLShortNoEmpty log;
XMLEnumeration position;
XMLFlagsNoEmpty order; 
XMLStringNoEmpty hint;
XMLPointerNoEmpty<CommandHelp> help;
*/

NMI_GET( CommandWrapper, func, "функция команды: function(ch, args)" ) 
{
    return func;
}

NMI_SET( CommandWrapper, func, "функция команды: function(ch, args)" ) 
{
    func = arg;
    self->changed();
}

NMI_GET( CommandWrapper, name, "название команды" ) 
{ 
    return Register( name.getValue( ) ); 
} 

NMI_SET( CommandWrapper, name, "название команды" ) 
{ 
    name.setValue( arg.toString( ) );
    self->changed();
    commandManager->unregistrate( Pointer( this ) );
    commandManager->registrate( Pointer( this ) );
}

NMI_SET(CommandWrapper, rname, "русское название команды")
{
    russian.clear();
    russian.push_back(arg.toString());
    self->changed();
    commandManager->unregistrate( Pointer( this ) );
    commandManager->registrate( Pointer( this ) );
}

NMI_GET(CommandWrapper, rname, "русское название команды")
{
    return Register(russian.front());
}

NMI_INVOKE( CommandWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<CommandWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

