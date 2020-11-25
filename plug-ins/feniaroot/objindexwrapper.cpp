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
#include "affectwrapper.h"
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
ObjIndexWrapper::checkTarget( ) const 
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

NMI_GET( ObjIndexWrapper, material, "название материала, из которого сделан предмет") 
{ 
    checkTarget( ); 
    return Register( target->material );
}

NMI_GET( ObjIndexWrapper, description, "описание, видимое на земле") 
{ 
    checkTarget( ); 
    return Register( target->description );
}

NMI_GET( ObjIndexWrapper, name, "имена предмета, на которые он откликается") 
{ 
    checkTarget( ); 
    return Register( target->name);
}

NMI_GET( ObjIndexWrapper, short_descr, "описание, видимое в инвентаре и при манипуляциях") 
{ 
    checkTarget( ); 
    return Register( target->short_descr);
}

NMI_GET( ObjIndexWrapper, limit , "максимальное кол-во экземпляров существующих одновременно или -1") 
{ 
    checkTarget( ); 
    return target->limit;
}

NMI_GET( ObjIndexWrapper, vnum , "внум, уникальный номер прототипа") 
{ 
    checkTarget( ); 
    return target->vnum;
}

NMI_GET( ObjIndexWrapper, reset_num, "сколько раз этот прототип встречается в ресетах") 
{ 
    checkTarget( ); 
    return target->reset_num;
}

NMI_GET( ObjIndexWrapper, count, "кол-во экземпляров предметов этого прототипа") 
{ 
    checkTarget( ); 
    return target->count;
}

NMI_SET( ObjIndexWrapper, count, "кол-во экземпляров предметов этого прототипа") 
{ 
    checkTarget( ); 
    target->count = arg.toNumber();
}

NMI_GET( ObjIndexWrapper, cost , "цена в серебре") 
{ 
    checkTarget( ); 
    return target->cost;
}

#define GETVALUE(x) \
    NMI_GET( ObjIndexWrapper, value##x, "поле value"#x", смысл зависит от типа предмета") { \
        checkTarget( ); \
        return Register( target->value[x]); \
    } 
        
GETVALUE(0)
GETVALUE(1)
GETVALUE(2)
GETVALUE(3)
GETVALUE(4)

NMI_INVOKE(ObjIndexWrapper, create, "(): создать экземпляр предмета")
{
    Object *obj;

    checkTarget( );
    obj = ::create_object( target , target->level );
    obj_to_room( obj, get_room_instance( ROOM_VNUM_FENIA_STORAGE ) );
    return WrapperManager::getThis( )->getWrapper( obj );
}

NMI_GET( ObjIndexWrapper, item_type, "тип предмета (таблица .tables.item_table)")
{
    checkTarget( );
    return Register( target->item_type);
}

NMI_GET( ObjIndexWrapper, instances, "список (List) всех предметов с этим прототипом" )
{
    checkTarget();
    RegList::Pointer rc(NEW);

    for (auto *o: target->instances)
        rc->push_back( WrapperManager::getThis( )->getWrapper( o ) );

    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}

NMI_INVOKE( ObjIndexWrapper, property, "(name, defaultValue): свойство прототипа с именем name или значение по умолчанию" )
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

NMI_GET( ObjIndexWrapper, affected, "список (List) всех аффектов на прототипе (структура .Affect)" )
{
    checkTarget();
    RegList::Pointer rc(NEW);

    for (auto &paf: target->affected) 
        rc->push_back( AffectWrapper::wrap( *paf ) );
        
    Scripting::Object *sobj = &Scripting::Object::manager->allocate();
    sobj->setHandler(rc);

    return Register( sobj );
}

NMI_INVOKE( ObjIndexWrapper, api, "(): печатает этот API" )
{
    ostringstream buf;
    Scripting::traitsAPI<ObjIndexWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( ObjIndexWrapper, rtapi, "(): печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( ObjIndexWrapper, clear, "(): очистка всех runtime полей" )
{
    guts.clear( );
    self->changed();
    return Register( );
}
