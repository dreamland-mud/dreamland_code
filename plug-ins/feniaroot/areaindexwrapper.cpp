#include "logstream.h"
#include "object.h"
#include "merc.h"
#include "mercdb.h"
#include "loadsave.h"

#include "areaindexwrapper.h"
#include "structwrappers.h"
#include "wrappermanager.h"
#include "reglist.h"
#include "register-impl.h"
#include "nativeext.h"
#include "wrap_utils.h"

#include "def.h"

using Scripting::NativeTraits;

NMI_INIT(AreaIndexWrapper, "прототип для зоны (area index data)")

AreaIndexWrapper::AreaIndexWrapper( ) : target( NULL )
{
}

void 
AreaIndexWrapper::extract( bool count )
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        LogStream::sendError() << "AreaIndex wrapper: extract without target" << endl;
    }

    GutsContainer::extract( count );
}

void AreaIndexWrapper::setSelf( Scripting::Object *s )
{
    WrapperBase::setSelf( s );

    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void
AreaIndexWrapper::setTarget( AreaIndexData * pIndex )
{
    target = pIndex;
    id = AREA_VNUM2ID(pIndex->min_vnum);
}

void 
AreaIndexWrapper::checkTarget( ) const 
{
    if (zombie.getValue())
        throw Scripting::Exception( "AreaIndexData is dead" );

    if (!target)
        throw Scripting::Exception( "AreaIndexData is offline?!");
}

AreaIndexData *
AreaIndexWrapper::getTarget( ) const
{
    checkTarget();
    return target;
}

NMI_GET( AreaIndexWrapper, name, "название зоны") 
{ 
    checkTarget( ); 
    return Register( target->name);
}
    
NMI_GET( AreaIndexWrapper, low_range , "нижний диапазон уровней зоны") 
{ 
    checkTarget( ); 
    return target->low_range;
}

NMI_GET( AreaIndexWrapper, high_range , "верхний диапазон уровней зоны") 
{ 
    checkTarget( ); 
    return target->high_range;
}

NMI_GET( AreaIndexWrapper, min_vnum , "нижняя граница диапазона внумов зоны") 
{ 
    checkTarget( ); 
    return target->min_vnum;
}

NMI_GET( AreaIndexWrapper, max_vnum , "верхняя граница диапазона внумов зоны") 
{ 
    checkTarget( ); 
    return target->max_vnum;
}


NMI_INVOKE( AreaIndexWrapper, api, "(): печатает этот API" )
{
    ostringstream buf;
    Scripting::traitsAPI<AreaIndexWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( AreaIndexWrapper, rtapi, "(): печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( AreaIndexWrapper, clear, "(): очистка всех runtime полей" )
{
    guts.clear( );
    self->changed();
    return Register( );
}
