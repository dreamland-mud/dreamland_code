#include "logstream.h"

#include "skillcommandwrapper.h"
#include "structwrappers.h"
#include "wrappermanager.h"
#include "reglist.h"
#include "register-impl.h"
#include "nativeext.h"
#include "wrap_utils.h"

#include "core/object.h"
#include "merc.h"
#include "mercdb.h"
#include "loadsave.h"
#include "def.h"

using Scripting::NativeTraits;

NMI_INIT(SkillCommandWrapper, "прототип для обработчика команд умений (.SkillCommand)")

SkillCommandWrapper::SkillCommandWrapper( ) : target( NULL )
{
}

void 
SkillCommandWrapper::extract( bool count )
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        LogStream::sendError() << "SkillCommand wrapper: extract without target" << endl;
    }

    GutsContainer::extract( count );
}

void SkillCommandWrapper::setSelf( Scripting::Object *s )
{
    WrapperBase::setSelf( s );

    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void
SkillCommandWrapper::setTarget( SkillCommand * skillcommand )
{
    target = skillcommand;
    id = skillcommand->getID();
}

void 
SkillCommandWrapper::checkTarget( ) const 
{
    if (zombie.getValue())
        throw Scripting::Exception( "SkillCommand is dead" );

    if (!target)
        throw Scripting::Exception( "SkillCommand is offline");
}

SkillCommand *
SkillCommandWrapper::getTarget( ) const
{
    checkTarget();
    return target;
}

NMI_GET( SkillCommandWrapper, name, "название команды") 
{ 
    checkTarget( ); 
    return Register(target->getName());
}

NMI_GET( SkillCommandWrapper, rname, "русское название команды") 
{ 
    checkTarget( ); 
    return Register(target->getRussianName());
}

NMI_GET( SkillCommandWrapper, skill, "навык, внутри которого объявлена команда (.Skill)") 
{ 
    checkTarget( ); 
    return Register::handler<SkillWrapper>(target->getSkill()->getName());
}


NMI_INVOKE( SkillCommandWrapper, api, "(): печатает этот API" )
{
    ostringstream buf;
    Scripting::traitsAPI<SkillCommandWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( SkillCommandWrapper, rtapi, "(): печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( SkillCommandWrapper, clear, "(): очистка всех runtime полей" )
{
    guts.clear( );
    self->changed();
    return Register( );
}
