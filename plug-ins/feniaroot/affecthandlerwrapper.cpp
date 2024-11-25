#include "logstream.h"
#include "object.h"
#include "merc.h"

#include "loadsave.h"

#include "affecthandlerwrapper.h"
#include "structwrappers.h"
#include "skillwrapper.h"
#include "wrappermanager.h"
#include "reglist.h"
#include "register-impl.h"
#include "nativeext.h"
#include "wrap_utils.h"

#include "def.h"

using Scripting::NativeTraits;

NMI_INIT(AffectHandlerWrapper, "прототип для обработчика аффектов (.AffectHandler)")

AffectHandlerWrapper::AffectHandlerWrapper( ) : target( NULL )
{
}

void 
AffectHandlerWrapper::extract( bool count )
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        LogStream::sendError() << "AffectHandler wrapper: extract without target" << endl;
    }

    GutsContainer::extract( count );
}

void AffectHandlerWrapper::setSelf( Scripting::Object *s )
{
    WrapperBase::setSelf( s );

    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void
AffectHandlerWrapper::setTarget( AffectHandler * affecthandler )
{
    target = affecthandler;
    id = affecthandler->getID();
}

void 
AffectHandlerWrapper::checkTarget( ) const 
{
    if (zombie.getValue())
        throw Scripting::Exception( "AffectHandler is dead" );

    if (!target)
        throw Scripting::Exception( "AffectHandler is offline");
}

AffectHandler *
AffectHandlerWrapper::getTarget( ) const
{
    checkTarget();
    return target;
}

NMI_GET( AffectHandlerWrapper, name, "название аффекта") 
{ 
    checkTarget( ); 
    return Register( target->getSkill()->getName() );
}

NMI_GET( AffectHandlerWrapper, rname, "русское название аффекта") 
{ 
    checkTarget( ); 
    return Register( target->getSkill()->getRussianName() );
}

NMI_GET( AffectHandlerWrapper, dispelled, "поддается ли заклинанию снятия воздействий") 
{ 
    checkTarget(); 
    return Register(target->isDispelled());
}

NMI_GET( AffectHandlerWrapper, cancelled, "поддается ли заклинанию отмены") 
{ 
    checkTarget(); 
    return Register(target->isCancelled());
}

NMI_GET( AffectHandlerWrapper, skill, "навык, внутри которого объявлен аффект (.Skill)") 
{ 
    checkTarget( ); 
    return Register::handler<SkillWrapper>(target->getSkill()->getName());
}


NMI_INVOKE( AffectHandlerWrapper, api, "(): печатает этот API" )
{
    ostringstream buf;
    Scripting::traitsAPI<AffectHandlerWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( AffectHandlerWrapper, rtapi, "(): печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( AffectHandlerWrapper, clear, "(): очистка всех runtime полей" )
{
    guts.clear( );
    self->changed();
    return Register( );
}
