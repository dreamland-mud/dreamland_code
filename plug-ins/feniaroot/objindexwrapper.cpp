/* $Id: objindexwrapper.cpp,v 1.1.2.17.6.7 2009/11/04 03:24:33 rufina Exp $
 *
 * ruffina, 2004
 */

#include "logstream.h"
#include "object.h"
#include "merc.h"
#include "mercdb.h"
#include "loadsave.h"

#include "objindexwrapper.h"
#include "wrappermanager.h"
#include "reglist.h"
#include "register-impl.h"
#include "nativeext.h"
#include "wrap_utils.h"

#include "def.h"

using Scripting::NativeTraits;

NMI_INIT(ObjIndexWrapper, "прототип для предметов (obj index data)")

ObjIndexWrapper::ObjIndexWrapper( ) : target( NULL )
{
}

void 
ObjIndexWrapper::extract( bool count )
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        LogStream::sendError() << "ObjIndex wrapper: extract without target" << endl;
    }

    GutsContainer::extract( count );
}

void ObjIndexWrapper::setSelf( Scripting::Object *s )
{
    WrapperBase::setSelf( s );

    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void
ObjIndexWrapper::setTarget( OBJ_INDEX_DATA * pIndex )
{
    target = pIndex;
    id = OBJ_VNUM2ID(pIndex->vnum);
}

void 
ObjIndexWrapper::checkTarget( ) const throw( Scripting::Exception )
{
    if (zombie.getValue())
        throw Scripting::Exception( "OBJ_INDEX_DATA is dead" );

    if (!target)
        throw Scripting::Exception( "OBJ_INDEX_DATA is offline?!");
}

OBJ_INDEX_DATA *
ObjIndexWrapper::getTarget( ) const
{
    checkTarget();
    return target;
}

NMI_GET( ObjIndexWrapper, material, "") 
{ 
    checkTarget( ); 
    return Register( target->material );
}

NMI_GET( ObjIndexWrapper, description, "") 
{ 
    checkTarget( ); 
    return Register( target->description );
}

NMI_GET( ObjIndexWrapper, name, "") 
{ 
    checkTarget( ); 
    return Register( target->name);
}

NMI_GET( ObjIndexWrapper, short_descr, "") 
{ 
    checkTarget( ); 
    return Register( target->short_descr);
}

NMI_GET( ObjIndexWrapper, limit , "") 
{ 
    checkTarget( ); 
    return target->limit;
}

NMI_GET( ObjIndexWrapper, vnum , "") 
{ 
    checkTarget( ); 
    return target->vnum;
}

NMI_GET( ObjIndexWrapper, reset_num, "") 
{ 
    checkTarget( ); 
    return target->reset_num;
}

NMI_GET( ObjIndexWrapper, count, "") 
{ 
    checkTarget( ); 
    return target->count;
}

NMI_GET( ObjIndexWrapper, cost , "") 
{ 
    checkTarget( ); 
    return target->cost;
}

#define GETVALUE(x) \
    NMI_GET( ObjIndexWrapper, value##x, "") { \
        checkTarget( ); \
        return Register( target->value[x]); \
    } 
        
GETVALUE(0)
GETVALUE(1)
GETVALUE(2)
GETVALUE(3)
GETVALUE(4)

NMI_INVOKE(ObjIndexWrapper, create, "")
{
    Object *obj;

    checkTarget( );
    obj = ::create_object( target , target->level );
    obj_to_room( obj, get_room_index( ROOM_VNUM_FENIA_STORAGE ) );
    return WrapperManager::getThis( )->getWrapper( obj );
}

NMI_GET( ObjIndexWrapper, item_type, "")
{
    checkTarget( );
    return Register( target->item_type);
}

NMI_GET( ObjIndexWrapper, instances, "список всех предметов с этим pIndexData" )
{
    checkTarget();
    RegList::Pointer rc(NEW);

    for (Object *o = object_list; o; o = o->next)
        if (o->pIndexData == target)
            rc->push_back( WrapperManager::getThis( )->getWrapper( o ) );

    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}

NMI_INVOKE( ObjIndexWrapper, property, "(имя,значение) свойство прототипа с данным именем или значение по умолчанию" )
{
    checkTarget();
    DLString name = args2string(args);
    Register defaultValue = args.size() > 1 ? args.back() : Register();

    Properties::const_iterator p = target->properties.find(name);
    if (p == target->properties.end())
        return defaultValue;
    else
        return Register(p->second);
}

NMI_INVOKE( ObjIndexWrapper, api, "печатает этот API" )
{
    ostringstream buf;
    Scripting::traitsAPI<ObjIndexWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( ObjIndexWrapper, rtapi, "печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( ObjIndexWrapper, clear, "очистка всех runtime полей" )
{
    guts.clear( );
    self->changed();
    return Register( );
}
