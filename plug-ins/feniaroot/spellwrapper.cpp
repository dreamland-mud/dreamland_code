#include "logstream.h"
#include "object.h"
#include "merc.h"
#include "mercdb.h"
#include "loadsave.h"

#include "spellwrapper.h"
#include "structwrappers.h"
#include "wrappermanager.h"
#include "reglist.h"
#include "register-impl.h"
#include "nativeext.h"
#include "wrap_utils.h"

#include "def.h"

using Scripting::NativeTraits;

NMI_INIT(SpellWrapper, "прототип для заклинания (spell)")

SpellWrapper::SpellWrapper( ) : target( NULL )
{
}

void 
SpellWrapper::extract( bool count )
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        LogStream::sendError() << "Spell wrapper: extract without target" << endl;
    }

    GutsContainer::extract( count );
}

void SpellWrapper::setSelf( Scripting::Object *s )
{
    WrapperBase::setSelf( s );

    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void
SpellWrapper::setTarget( Spell * spell )
{
    target = spell;
    id = spell->getID();
}

void 
SpellWrapper::checkTarget( ) const 
{
    if (zombie.getValue())
        throw Scripting::Exception( "Spell is dead" );

    if (!target)
        throw Scripting::Exception( "Spell is offline");
}

Spell *
SpellWrapper::getTarget( ) const
{
    checkTarget();
    return target;
}

NMI_GET( SpellWrapper, name, "название заклинания") 
{ 
    checkTarget( ); 
    return Register( target->getSkill()->getName() );
}

NMI_GET( SpellWrapper, rname, "русское название заклинания") 
{ 
    checkTarget( ); 
    return Register( target->getSkill()->getRussianName() );
}

NMI_GET( SpellWrapper, skill, "навык, внутри которого объявлено заклинание (.Skill)") 
{ 
    checkTarget( ); 
    return Register::handler<SkillWrapper>(target->getSkill()->getName());
}


NMI_INVOKE( SpellWrapper, api, "(): печатает этот API" )
{
    ostringstream buf;
    Scripting::traitsAPI<SpellWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( SpellWrapper, rtapi, "(): печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( SpellWrapper, clear, "(): очистка всех runtime полей" )
{
    guts.clear( );
    self->changed();
    return Register( );
}
