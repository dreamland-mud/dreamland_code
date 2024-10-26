/* $Id: mobindexwrapper.cpp,v 1.1.2.15.6.10 2009/11/04 03:24:33 rufina Exp $
 *
 * ruffina, 2004
 */

#include "logstream.h"
#include "grammar_entities_impl.h"
#include "npcharacter.h"
#include "room.h"
#include "merc.h"
#include "json_utils_ext.h"
#include "loadsave.h"
#include "pet.h"
#include "behavior.h"

#include "mobindexwrapper.h"
#include "wrappermanager.h"
#include "structwrappers.h"
#include "reglist.h"
#include "nativeext.h"
#include "regcontainer.h"
#include "register-impl.h"
#include "wrap_utils.h"

#include "def.h"


using Scripting::NativeTraits;

NMI_INIT(MobIndexWrapper, "Прототип для мобов (mob index data)")

MobIndexWrapper::MobIndexWrapper( ) : target( NULL )
{
}

void 
MobIndexWrapper::extract( bool count )
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        LogStream::sendError() << "MobIndex wrapper: extract without target" << endl;
    }

    GutsContainer::extract( count );
}

void MobIndexWrapper::setSelf( Scripting::Object *s )
{
    WrapperBase::setSelf( s );

    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void
MobIndexWrapper::setTarget( mob_index_data * pIndex )
{
    target = pIndex;
    id = MOB_VNUM2ID(pIndex->vnum);
}

void 
MobIndexWrapper::checkTarget( ) const 
{
    if (zombie.getValue())
        throw Scripting::Exception( "MOB_INDEX_DATA is dead" );

    if (!target)
        throw Scripting::Exception( "MOB_INDEX_DATA is offline?!");
}

MOB_INDEX_DATA *
MobIndexWrapper::getTarget( ) const
{
    checkTarget();
    return target;
}

NMI_GET( MobIndexWrapper, name, "имена, на которые откликается моб") 
{ 
    checkTarget( ); 
    return target->keyword.toString();
}
NMI_GET( MobIndexWrapper, short_descr, "имя, которое видно когда моб совершает действия") 
{ 
    checkTarget( ); 
    return target->short_descr.get(LANG_DEFAULT);
}
NMI_GET( MobIndexWrapper, long_descr, "как моба видно в комнате") 
{ 
    checkTarget( ); 
    return target->long_descr.get(LANG_DEFAULT);
}
NMI_GET( MobIndexWrapper, description, "описание, видное если посмотреть на моба") 
{ 
    checkTarget( ); 
    return target->description.get(LANG_DEFAULT);
}
NMI_GET( MobIndexWrapper, count, "кол-во экземпляров мобов этого прототипа") 
{ 
    checkTarget( ); 
    return target->count;
}
NMI_SET( MobIndexWrapper, count, "кол-во экземпляров мобов этого прототипа") 
{ 
    checkTarget( ); 
    target->count = arg.toNumber();
}
NMI_GET( MobIndexWrapper, vnum , "внум, уникальный номер прототипа") 
{ 
    checkTarget( ); 
    return target->vnum;
}
NMI_GET( MobIndexWrapper, size , "численный размер моба или расовый (таблица .tables.size_table)") 
{ 
    checkTarget( ); 
    return target->getSize();
}
NMI_GET( MobIndexWrapper, imm_flags , "флаги иммунитета (таблица .tables.imm_flags)") 
{ 
    checkTarget( ); 
    return (int)target->imm_flags;
}
NMI_GET( MobIndexWrapper, group, "к какой группе принадлежит моб (нужно для assist)") 
{ 
    checkTarget( ); 
    return target->group;
}
NMI_SET( MobIndexWrapper, group, "к какой группе принадлежит моб (нужно для assist)") 
{ 
    checkTarget( ); 
    target->group = arg.toNumber( );
}
NMI_GET( MobIndexWrapper, sex, "пол прототипа (.tables.sex_table)") 
{ 
    checkTarget( ); 
    return target->sex;
}
NMI_GET( MobIndexWrapper, gram_number, "грамматическое число, s или p") 
{ 
    checkTarget( ); 
    return target->gram_number.toString();
}

NMI_GET( MobIndexWrapper, spec_fun, "спец-процедура") 
{
    checkTarget( ); 
    if (target->spec_fun.func)
        return Register( spec_name(target->spec_fun.func) );
    else
        return Register( );
}

NMI_GET( MobIndexWrapper, practicer, "какие группы умений может практиковать (glist или olchelp groups)") 
{
    checkTarget( );
    return target->practicer.toString( );
}

NMI_GET( MobIndexWrapper, affects, "какие аффекты вешаются при создании (olchelp spell)") 
{
    checkTarget( );
    return target->affects.toString( );
}

NMI_GET( MobIndexWrapper, religion, "какие религии исповедует (reledit list)") 
{
    checkTarget( );
    return target->religion.toString( );
}

NMI_GET( MobIndexWrapper, area, "зона, в которой прописан моб (.Area)") 
{
    checkTarget( );
    return AreaWrapper::wrap(target->area->area_file->file_name);
}

NMI_INVOKE( MobIndexWrapper, property, "(name, defaultValue): свойство прототипа с именем name или значение по умолчанию" )
{
    checkTarget();
    DLString name = args2string(args);
    Register defaultValue = args.size() > 1 ? args.back() : Register();

    DLString jsonValue;
    if (JsonUtils::findValue(target->props, name, jsonValue))
        return jsonValue;

    return defaultValue;
}

NMI_INVOKE(MobIndexWrapper, setProperty, "(key,subkey,value): установить значение props[key][subkey] в value")
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

NMI_GET( MobIndexWrapper, repopPlaces, "список внумов комнат, в которых ресетится моб") 
{
    RegList::Pointer rc(NEW);
    
    checkTarget( );
    
    for (auto &r: roomIndexMap)
        for (auto &pReset: r.second->resets)
            if (pReset->command == 'M' && pReset->arg1 == target->vnum)
                rc->push_back( Register( r.first ) );

    Scripting::Object *obj = &Scripting::Object::manager->allocate( );
    obj->setHandler( rc );

    return Register( obj );
}    

NMI_GET( MobIndexWrapper, instances, "список всех экземпляров мобов с этим прототипом" )
{
    checkTarget();
    RegList::Pointer rc(NEW);
    Character *ch;

    for (ch = char_list; ch; ch = ch->next)
        if (ch->is_npc( ) && ch->getNPC( )->pIndexData == target)
            rc->push_back( WrapperManager::getThis( )->getWrapper( ch ) );

    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}

NMI_INVOKE(MobIndexWrapper, create, "(): создать экземпляр моба")
{
    NPCharacter *mob;

    checkTarget( );
    mob = create_mobile( target );
    char_to_room( mob, get_room_instance( ROOM_VNUM_FENIA_STORAGE ) );
    return WrapperManager::getThis( )->getWrapper( mob ); 
}

NMI_INVOKE(MobIndexWrapper, createFor, "(ch): создать экземпляр пета по уровню игрока ch")
{
    NPCharacter *mob;
    checkTarget();

    if (!target->behavior || target->behavior->getFirstNode( )->getAttribute(XMLNode::ATTRIBUTE_TYPE) != "LevelAdaptivePet")
        throw Scripting::Exception("This mob is not a pet");

    PCharacter *client = args2player(args);
    mob = create_mobile( target );            
    char_to_room( mob, get_room_instance( ROOM_VNUM_FENIA_STORAGE ) );        

    LevelAdaptivePet::Pointer pet = mob->behavior.getDynamicPointer<LevelAdaptivePet>();
    if (pet)
        pet->config(client, pet->getChar());
        
    return WrapperManager::getThis( )->getWrapper( mob ); 
}

NMI_GET(MobIndexWrapper, behaviors, "список имен всех поведений")
{
    checkTarget();

    RegList::Pointer rc(NEW);

    for (auto &b: target->behaviors.toSet()) 
        rc->push_back(
            Register(
                behaviorManager->find(b)->getName()));

    return ::wrap(rc);
}

NMI_SET(MobIndexWrapper, behaviors, "список имен всех поведений")
{
    checkTarget();
    arg2globalBitvector<Behavior>(arg, target->behaviors);
    target->area->changed = true;
}

NMI_GET(MobIndexWrapper, props, "Map (структура) из свойств поведения, ключ - имя поведения") 
{
    checkTarget();
    return JsonUtils::toRegister(target->props);
}

NMI_GET(MobIndexWrapper, triggers, "список (.List) названий всех тригеров")
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

NMI_INVOKE( MobIndexWrapper, api, "(): печатает этот API" )
{
    ostringstream buf;
    Scripting::traitsAPI<MobIndexWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( MobIndexWrapper, rtapi, "(): печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( MobIndexWrapper, clear, "(): очистка всех runtime полей" )
{
    guts.clear( );
    self->changed();
    return Register( );
}
