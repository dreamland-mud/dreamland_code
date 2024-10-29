/* $Id: objindexwrapper.cpp,v 1.1.2.17.6.7 2009/11/04 03:24:33 rufina Exp $
 *
 * ruffina, 2004
 */

#include "logstream.h"
#include "grammar_entities_impl.h"
#include "object.h"
#include "merc.h"
#include "json_utils_ext.h"
#include "loadsave.h"
#include "behavior.h"
#include "string_utils.h"

#include "objindexwrapper.h"
#include "affectwrapper.h"
#include "structwrappers.h"
#include "wrappermanager.h"
#include "reglist.h"
#include "register-impl.h"
#include "regcontainer.h"
#include "nativeext.h"
#include "wrap_utils.h"
#include "idcontainer.h"
#include "lex.h"

#include "def.h"

using Scripting::NativeTraits;
using namespace Scripting;

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

NMI_GET( ObjIndexWrapper, area, "зона, в которой прописан предмет") 
{
    checkTarget( );
    return WrapperManager::getThis( )->getWrapper( target->area );
}

NMI_GET( ObjIndexWrapper, material, "название материала, из которого сделан предмет") 
{ 
    checkTarget( ); 
    return Register( target->material );
}

NMI_GET( ObjIndexWrapper, description, "описание, видимое на земле") 
{ 
    checkTarget( ); 
    return Register( target->description.get(LANG_DEFAULT) );
}

NMI_GET( ObjIndexWrapper, name, "имена предмета, на которые он откликается") 
{ 
    checkTarget( ); 
    return Register( String::toString(target->keyword));
}

NMI_GET( ObjIndexWrapper, short_descr, "описание, видимое в инвентаре и при манипуляциях") 
{ 
    checkTarget( ); 
    return Register( target->short_descr.get(LANG_DEFAULT));
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

NMI_GET( ObjIndexWrapper, weight , "вес предмета") 
{ 
    checkTarget( ); 
    return target->weight;
}

NMI_GET( ObjIndexWrapper, level, "уровень предмета") 
{ 
    checkTarget( ); 
    return target->level;
}

NMI_GET( ObjIndexWrapper, cost , "цена в серебре") 
{ 
    checkTarget( ); 
    return target->cost;
}

NMI_SET( ObjIndexWrapper, cost , "цена в серебре") 
{ 
    checkTarget( ); 
    target->cost = arg.toNumber();
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
    ::Object *obj;

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

NMI_GET( ObjIndexWrapper, wear_flags, "куда надевается предмет (таблица .tables.wear_flags)")
{
    checkTarget( );
    return Register( target->wear_flags);
}

NMI_GET( ObjIndexWrapper, gender, "грамматический род и число (n, m, f, p или null)")
{
    checkTarget( );
    if (target->gram_gender == MultiGender::UNDEF)
        return Register();
    return Register( target->gram_gender.toString() );
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

    DLString jsonValue;
    if (JsonUtils::findValue(target->props, name, jsonValue))
        return jsonValue;

    return defaultValue;
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

NMI_INVOKE(ObjIndexWrapper, setProperty, "(key,subkey,value): установить значение props[key][subkey] в value")
{
    checkTarget();
    DLString key = argnum2string(args, 1);
    DLString subkey = argnum2string(args, 2);
    DLString value = argnum2string(args, 3);

    if (value.isNumber()) {
        target->props[key][subkey] = value.toInt();
    } else {
        target->props[key][subkey] = value;
    }

    target->area->changed = true;

    return Register();
}

NMI_GET(ObjIndexWrapper, behaviors, "список имен всех поведений")
{
    checkTarget();

    RegList::Pointer rc(NEW);

    for (auto &b: target->behaviors.toSet()) 
        rc->push_back(
            Register(
                behaviorManager->find(b)->getName()));

    return ::wrap(rc);
}

NMI_GET(ObjIndexWrapper, props, "Map (структура) из свойств поведения, ключ - имя поведения")
{
    checkTarget();
    return JsonUtils::toRegister(target->props);
}

NMI_SET(ObjIndexWrapper, behaviors, "список имен всех поведений")
{
    checkTarget();
    arg2globalBitvector<Behavior>(arg, target->behaviors);
    target->area->changed = true;
}

NMI_GET(ObjIndexWrapper, triggers, "список (.List) названий всех тригеров")
{
    checkTarget();

    StringSet triggers, misc;

    // Collect all onXXX, postXXX triggers defined on index data.
    WrapperBase *ndxWrapper = get_wrapper(target->wrapper);
    if (ndxWrapper)
        ndxWrapper->collectTriggers(triggers, misc);

    // Collect all triggers defined on each of the behaviors.
    for (auto &b: target->behaviors.toSet()) {
        WrapperBase *bhvWrapper = behaviorManager->find(b)->getWrapper();
        if (bhvWrapper)
            bhvWrapper->collectTriggers(triggers, misc);
    }

    // Transform triggers set into a Fenia list.
    RegList::Pointer rc(NEW);
    for (auto &trig: triggers)
        rc->push_back(Register(trig));
    return ::wrap(rc);
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
