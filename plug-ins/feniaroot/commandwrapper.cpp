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
