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

bool CommandWrapper::saveCommand() const
{
    self->changed();
    return true;
}

void 
CommandWrapper::run( Character * ch, const DLString &arg )
{
    RegisterList args;
    args.push_back(wrap(ch));
    args.push_back(arg);

    try {
        func.toFunction()->invoke(Register(self), args);
    } catch (const ::Exception &e) {
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
    // Replace first alias in the list with the rname.
    if (!russian.empty())
        russian.pop_front();

    russian.push_front(arg.toString());
    self->changed();

    commandManager->unregistrate( Pointer( this ) );
    commandManager->registrate( Pointer( this ) );
}

NMI_GET(CommandWrapper, rname, "русское название команды")
{
    return Register(russian.front());
}

NMI_SET(CommandWrapper, raliases, "список русских синонимов команды через пробел")
{
    // Retain first russian alias as rname and replace all remaining ones. 
    StringList words(arg.toString());
    DLString rname = russian.front();
    russian.clear();

    if (!rname.empty())
        russian.push_front(rname);

    for (auto &w: words)
        russian.push_back(w);

    self->changed();
    commandManager->unregistrate( Pointer( this ) );
    commandManager->registrate( Pointer( this ) );
}

NMI_GET(CommandWrapper, aliases, "список английских синонимов команды через пробел")
{
    StringList words;
    for (auto &a: aliases)
        words.push_back(a);
    return words.join(" ");
}

NMI_GET(CommandWrapper, raliases, "список русских синонимов команды через пробел")
{
    StringList words;
    for (auto &r: russian)
        words.push_back(r);
    return words.join(" ");
}

NMI_SET(CommandWrapper, aliases, "список английских синонимов команды через пробел")
{
    // Replace a list of all EN aliases.
    StringList words(arg.toString());
    aliases.clear();
    for (auto &w: words)
        aliases.push_back(w);

    self->changed();
    commandManager->unregistrate( Pointer( this ) );
    commandManager->registrate( Pointer( this ) );
}

NMI_INVOKE( CommandWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<CommandWrapper>( buf );
    return Scripting::Register( buf.str( ) );
}

//------------------------------------------------------------------------


using Scripting::NativeTraits;

NMI_INIT(FeniaCommandWrapper, "прототип для команды (.FeniaCommand)")

FeniaCommandWrapper::FeniaCommandWrapper() : target(NULL)
{
}

void FeniaCommandWrapper::extract(bool count)
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        LogStream::sendError() << "Command wrapper: extract without target" << endl;
    }

    GutsContainer::extract( count );
}

void FeniaCommandWrapper::setSelf(Scripting::Object *s)
{
    WrapperBase::setSelf( s );

    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void FeniaCommandWrapper::setTarget(WrappedCommand * cmd)
{
    target = cmd;
    id = cmd->getID();
}

void FeniaCommandWrapper::checkTarget() const 
{
    if (zombie.getValue())
        throw Scripting::Exception( "Command is dead" );

    if (!target)
        throw Scripting::Exception( "Command is offline");
}

WrappedCommand * FeniaCommandWrapper::getTarget() const
{
    checkTarget();
    return target;
}

NMI_GET(FeniaCommandWrapper, name, "название команды") 
{ 
    checkTarget(); 
    return Register(target->getName());
}

NMI_GET(FeniaCommandWrapper, rname, "русское название команды") 
{ 
    checkTarget(); 
    return Register(target->getRussianName());
}

NMI_INVOKE( FeniaCommandWrapper, api, "(): печатает этот API" )
{
    ostringstream buf;
    Scripting::traitsAPI<FeniaCommandWrapper>(buf);
    return Register(buf.str());
}

NMI_INVOKE( FeniaCommandWrapper, rtapi, "(): печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI(buf);
    return Register(buf.str());
}

NMI_INVOKE( FeniaCommandWrapper, clear, "(): очистка всех runtime полей" )
{
    guts.clear();
    self->changed();
    return Register();
}
