#include "areaquestwrapper.h"
#include "logstream.h"
#include "object.h"
#include "merc.h"
#include "mercdb.h"
#include "loadsave.h"

#include "areaquestwrapper.h"
#include "structwrappers.h"
#include "wrappermanager.h"
#include "reglist.h"
#include "register-impl.h"
#include "nativeext.h"
#include "wrap_utils.h"

#include "def.h"

using Scripting::NativeTraits;

NMI_INIT(AreaQuestWrapper, "квест в арии")

AreaQuestWrapper::AreaQuestWrapper( ) : target( NULL )
{
}

void 
AreaQuestWrapper::extract( bool count )
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        LogStream::sendError() << "AreaQuest wrapper: extract without target" << endl;
    }

    GutsContainer::extract( count );
}

void AreaQuestWrapper::setSelf( Scripting::Object *s )
{
    WrapperBase::setSelf( s );

    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void
AreaQuestWrapper::setTarget( AreaQuest * quest )
{
    target = quest;
    id = quest->getID();
}

void 
AreaQuestWrapper::checkTarget( ) const 
{
    if (zombie.getValue())
        throw Scripting::Exception( "AreaQuest is dead" );

    if (!target)
        throw Scripting::Exception( "AreaQuest is offline");
}

AreaQuest *
AreaQuestWrapper::getTarget( ) const
{
    checkTarget();
    return target;
}

NMI_GET( AreaQuestWrapper, title, "название квеста") 
{ 
    checkTarget( ); 
    return Register( target->title );
}

NMI_GET( AreaQuestWrapper, description, "описание квеста") 
{ 
    checkTarget( ); 
    return Register( target->description );
}

NMI_GET( AreaQuestWrapper, vnum, "номер квеста") 
{ 
    checkTarget( ); 
    return Register( target->vnum );
}

NMI_GET( AreaQuestWrapper, minLevel, "мин. уровень, на котором выдают квест") 
{ 
    checkTarget( ); 
    return Register( target->minLevel );
}

NMI_GET( AreaQuestWrapper, maxLevel, "макс. уровень, на котором выдают квест") 
{ 
    checkTarget( ); 
    return Register( target->maxLevel );
}

NMI_GET( AreaQuestWrapper, limitPerLife, "сколько раз за реморт можно выполнить") 
{ 
    checkTarget( ); 
    return Register( target->limitPerLife );
}

NMI_GET( AreaQuestWrapper, flags, "флаги квеста (таблица .tables.areaquest_flags)" ) 
{
    checkTarget();
    return Register((int)target->flags);
}


NMI_INVOKE( AreaQuestWrapper, api, "(): печатает этот API" )
{
    ostringstream buf;
    Scripting::traitsAPI<AreaQuestWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( AreaQuestWrapper, rtapi, "(): печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( AreaQuestWrapper, clear, "(): очистка всех runtime полей" )
{
    guts.clear( );
    self->changed();
    return Register( );
}
