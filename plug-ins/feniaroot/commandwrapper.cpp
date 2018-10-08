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
        LogStream::sendNotice() << "commandManager->registrate" << endl;
        commandManager->registrate( Pointer( this ) );
    } else {
        LogStream::sendNotice() << "commandManager->unregistrate" << endl;
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
    args.push_back(arg);
    try {
        func.toFunction()->invoke(wrap(ch), args);
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

NMI_GET( CommandWrapper, func, "command function" ) 
{
    return func;
}

NMI_SET( CommandWrapper, func, "command function" ) 
{
    func = arg;
    self->changed();
}

NMI_GET( CommandWrapper, name, "command name" ) 
{ 
    return Register( name.getValue( ) ); 
} 

NMI_SET( CommandWrapper, name, "command name" ) 
{ 
    name.setValue( arg.toString( ) );
    self->changed();
}


NMI_INVOKE( CommandWrapper, api, "" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<CommandWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

