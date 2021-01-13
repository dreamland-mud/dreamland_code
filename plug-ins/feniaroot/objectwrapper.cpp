/* $Id: objectwrapper.cpp,v 1.1.4.33.6.15 2009/11/04 03:24:33 rufina Exp $
 *
 * ruffina, 2004
 */

#include "logstream.h"
#include "character.h"
#include "object.h"
#include "affect.h"
#include "save.h"
#include "merc.h"
#include "loadsave.h"
#include "wearloc_utils.h"
#include "weapons.h"
#include "occupations.h"
#include "mercdb.h"
#include "grammar_entities_impl.h"
#include "personalquestreward.h"
#include "objectwrapper.h"
#include "roomwrapper.h"
#include "characterwrapper.h"
#include "objindexwrapper.h"
#include "wrappermanager.h"
#include "affectwrapper.h"
#include "reglist.h"
#include "nativeext.h"
#include "wrap_utils.h"
#include "subr.h"

#include "material.h"
#include "damageflags.h"
#include "def.h"

using namespace std;
using namespace Scripting;

NMI_INIT(ObjectWrapper, "предмет")


ObjectWrapper::ObjectWrapper( ) : target( NULL )
{
    
}

void ObjectWrapper::setSelf( Scripting::Object *s )
{
    WrapperBase::setSelf( s );

    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void ObjectWrapper::extract( bool count )
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        LogStream::sendError() << "Object wrapper: extract without target" << endl;
    }

    GutsContainer::extract( count );
}

void ObjectWrapper::setTarget( ::Object *target )
{
    this->target = target;
    id = target->getID( );
}

::Object *
ObjectWrapper::getTarget() const
{
    checkTarget();
    return target;
}

void ObjectWrapper::checkTarget( ) const 
{
    if (zombie.getValue())
        throw Scripting::Exception( "Non existent object referenced" );

    if (target == NULL) 
        throw Scripting::Exception( "Object is offline" );
}

NMI_GET( ObjectWrapper, online, "предмет сейчас в мире (а не в профайлах)" )
{
    return Register( target != NULL );
}

NMI_GET( ObjectWrapper, dead, "true если предмет уничтожен" )
{
    return Register( zombie.getValue() );
}

NMI_GET( ObjectWrapper, id , "уникальный идентификатор предмета")
{
    return DLString(id.getValue( ));
}

NMI_GET( ObjectWrapper, vnum , "номер прототипа предмета из арии") 
{ 
    checkTarget( ); 
    return target->pIndexData->vnum;
}

NMI_GET( ObjectWrapper, short_descr , "описание, видимое в инвентаре и при манипуляциях")
{
    checkTarget( );
    return Register( target->getShortDescr( ) );
}

NMI_SET( ObjectWrapper, short_descr , "описание, видимое в инвентаре и при манипуляциях")
{
    DLString d = arg.toString( );
    
    checkTarget( );
    target->setShortDescr( d.c_str( ) );
}

NMI_GET( ObjectWrapper, real_short_descr , "описание без учета restring-а")
{
    checkTarget( );
    return Register( target->getRealShortDescr( ) );
}

NMI_GET( ObjectWrapper, gender , "грамматический род и число (n, m, f, p или null)")
{
    checkTarget();
    if (target->gram_gender == MultiGender::UNDEF)
        return Register();
    return Register( target->gram_gender.toString() );
}

NMI_SET( ObjectWrapper, gender , "грамматический род и число (n, m, f, p или null)")
{
    checkTarget();
    MultiGender mg(MultiGender::UNDEF);

    if (arg.type != Register::NONE) {
        mg.fromString(arg.toString().c_str());
        if (mg == MultiGender::UNDEF)
            throw Scripting::Exception("Invalid gender, must be one of: n m f p, or null");
    }

    target->gram_gender = mg;
    target->updateCachedNoun();
}

NMI_GET( ObjectWrapper, enchanted, "висят ли на предмете аффекты, меняющие его свойства")
{
    checkTarget( );
    return Register( target->enchanted );
}

NMI_SET( ObjectWrapper, enchanted , "висят ли на предмете аффекты, меняющие его свойства")
{
    checkTarget( );
    target->enchanted = arg.toNumber( );
}

NMI_GET( ObjectWrapper, special, "обладает ли прототип предмета сложным поведением (через феню или код)")
{
    checkTarget( );
    return Register( obj_is_special(target) );
}

NMI_GET( ObjectWrapper, description , "описание, видимое на земле")
{
    checkTarget( );
    return Register( target->getDescription( ) );
}

NMI_SET( ObjectWrapper, description , "описание, видимое на земле")
{
    DLString d = arg.toString( );

    checkTarget( );
    target->setDescription( d.c_str( ) );
}

NMI_GET( ObjectWrapper, material, "название материала, из которого сделан предмет")
{
    checkTarget( );
    return Register( target->getMaterial( ) );
}

NMI_SET( ObjectWrapper, material, "название материала, из которого сделан предмет")
{
    DLString d = arg.toString( );

    checkTarget( );
    target->setMaterial( d.c_str( ) );
}

NMI_GET( ObjectWrapper, name , "имена предмета, на которые он откликается")
{
    checkTarget( );
    return Register( target->getName( ) );
}

NMI_SET( ObjectWrapper, name , "имена предмета, на которые он откликается")
{
    DLString d = arg.toString( );

    checkTarget( );
    target->setName( d.c_str() );
}

NMI_GET( ObjectWrapper, pocket, "название кармана, в котором лежит предмет, или пустая строка")
{
    checkTarget( );
    return Register( target->pocket );
}

NMI_SET( ObjectWrapper, pocket, "название кармана, в котором лежит предмет, или пустая строка")
{
    checkTarget( );
    target->pocket = arg.toString( );
}

NMI_GET( ObjectWrapper, level , "уровень предмета")
{
    checkTarget( );
    return Register( target->level );
}

NMI_SET( ObjectWrapper, level , "уровень предмета")
{
    checkTarget( );
    target->level = arg.toNumber( );
}

NMI_GET( ObjectWrapper, condition , "состояние, от 0 [уж.] до 100")
{
    checkTarget( );
    return Register( target->condition );
}

NMI_SET( ObjectWrapper, condition , "состояние, от 0 [уж.] до 100")
{
    checkTarget( );
    target->condition = arg.toNumber( );
}

NMI_GET( ObjectWrapper, cost , "цена в серебре")
{
    checkTarget( );
    return Register( target->cost );
}

NMI_SET( ObjectWrapper, cost , "цена в серебре")
{
    checkTarget( );
    target->cost = arg.toNumber( );
}

NMI_GET( ObjectWrapper, extra_flags, "дополнительные флаги (таблица .tables.extra_flags)")
{
    checkTarget( );
    return Register( target->extra_flags );
}

NMI_SET( ObjectWrapper, extra_flags, "дополнительные флаги (таблица .tables.extra_flags)")
{
    checkTarget( );
    target->extra_flags = arg.toNumber( );
}

NMI_GET( ObjectWrapper, wear_flags, "куда можно надеть предмет (таблица .tables.wear_flags)")
{
    checkTarget( );
    return Register( target->wear_flags);
}

NMI_SET( ObjectWrapper, wear_flags, "куда можно надеть предмет (таблица .tables.wear_flags)")
{
    checkTarget( );
    target->wear_flags = arg.toNumber( );
}

NMI_GET( ObjectWrapper, timer, "через сколько тиков исчезнет или -1")
{
    checkTarget( );
    return Register( target->timer );
}

NMI_SET( ObjectWrapper, timer, "через сколько тиков исчезнет или -1")
{
    checkTarget( );
    target->timer = arg.toNumber( );
}

NMI_GET( ObjectWrapper, owner , "имя персонажа-владельца (для трупов и личных вещей)")
{
    checkTarget( );
    const char *o = target->getOwner( );

    if (o == NULL)
        return Register( "" );
    else
        return Register( o );
}

NMI_SET( ObjectWrapper, owner , "имя персонажа-владельца (для трупов и личных вещей)")
{
    DLString d = arg.toString( );
    checkTarget( );
    target->setOwner( d.c_str() );
}

NMI_GET( ObjectWrapper, personal, "установить или вернуть англ. имя собственника для личной вещи (или null)")
{
    checkTarget();
    if (target->getOwner() && target->behavior && target->behavior.getDynamicPointer<PersonalQuestReward>())
        return Register(target->getOwner());
    return Register();
}

NMI_SET( ObjectWrapper, personal, "установить или вернуть англ. имя собственника для личной вещи (или null)")
{
    checkTarget();

    if (arg.type == Register::NONE) {
        target->setOwner(str_empty);
        target->behavior.clear();
        return;
    }

    PCMemoryInterface *pci = PCharacterManager::findPlayer(arg.toString());
    if (!pci)
        throw Scripting::Exception("Player with this name not found");

    target->setOwner(pci->getName().c_str());
    SET_BIT(target->extra_flags, ITEM_NOPURGE|ITEM_NOSAC|ITEM_BURN_PROOF|ITEM_NOSELL);
    target->setMaterial( "platinum" );

    try {
        AllocateClass::Pointer p = Class::allocateClass( "PersonalQuestReward" );
        if (p) {
            target->behavior.setPointer( p.getDynamicPointer<ObjectBehavior>( ) );
            target->behavior->setObj(target);
        }
    } catch (const ExceptionClassNotFound &e) {
        LogStream::sendError( ) << e.what( ) << endl;
        throw Scripting::Exception(e.what());
    }
}

NMI_GET( ObjectWrapper, item_type, "тип предмета (таблица .tables.item_table)")
{
    checkTarget( );
    return Register( target->item_type);
}
NMI_GET( ObjectWrapper, wear_loc, "имя локации, куда надет сейчас, или none")
{
    checkTarget( );
    return Register( target->wear_loc->getName( ) );
}

NMI_GET(ObjectWrapper, worn, "true если надето, но не на хвост и не в волосы")
{
    checkTarget();
    return Register(obj_is_worn(target));
}
NMI_GET( ObjectWrapper, weightTotal, "вес предмета с учетом содержимого")
{
    checkTarget( );
    return Register( target->getWeight( ) );
}

NMI_GET( ObjectWrapper, weight, "вес предмета")
{
    checkTarget( );
    return Register( target->weight );
}

NMI_GET( ObjectWrapper, ave, "среднее повреждение оружия или 0")
{
    checkTarget( );
    return weapon_ave(target);
}

#define SETGETVALUE(x) \
    NMI_GET( ObjectWrapper, value##x, "поле value"#x", смысл зависит от типа предмета") { \
        checkTarget( ); \
        return Register( target->value##x() ); \
    } \
    NMI_SET( ObjectWrapper, value##x, "поле value"#x", смысл зависит от типа предмета") { \
        checkTarget( ); \
        target->value##x(arg.toNumber()); \
    }
        
SETGETVALUE(0)
SETGETVALUE(1)
SETGETVALUE(2)
SETGETVALUE(3)
SETGETVALUE(4)

#define GETWRAP(x, h) NMI_GET(ObjectWrapper, x, h) { \
    checkTarget(); \
    return WrapperManager::getThis( )->getWrapper(target->x); \
}

GETWRAP( pIndexData, "прототип предмета" )
GETWRAP( next, "указывает на следующий предмет в глобальном списке .object_list" )
GETWRAP( next_content, "указывает на следующий предмет в том же контейнере или инвентаре" )
GETWRAP( contains, "указывает на первый предмет, содержащийся внутри этого" )
GETWRAP( in_obj, "внутри какого предмета находится этот, или null" )
GETWRAP( carried_by, "персонаж, несущий предмет в инвентаре-экипировке, или null" )
GETWRAP( in_room, "комната, на полу которой лежит предмет, или null" )

/*
 * Methods
 */
NMI_INVOKE( ObjectWrapper, getCarrier, "(): персонаж, который несет предмет с учетом вложенности, или null")
{
    checkTarget();
    Character *ch = target->getCarrier();

    if (ch)
        return WrapperManager::getThis( )->getWrapper(ch); 
    else
        return Register();
}
NMI_INVOKE( ObjectWrapper, getRoom, "(): комната, в которой находится предмет с учетом вложенности, или null")
{
    checkTarget();
    Room *r = target->getRoom();

    if (r) 
        return WrapperManager::getThis( )->getWrapper(r); 
    else
        return Register();
}

void obj_from_anywhere( ::Object *obj )
{
    if (obj->in_room)
        obj_from_room( obj );
    else if (obj->carried_by)
        obj_from_char( obj );
    else if (obj->in_obj)
        obj_from_obj( obj );
}

NMI_INVOKE( ObjectWrapper, obj_from_char , "(): deprecated")
{
    return Register( );
}

NMI_INVOKE( ObjectWrapper, obj_to_char , "(ch): дать предмет в руки персонажу ch")
{
    CharacterWrapper *chWrap;
    
    checkTarget();
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
    
    chWrap = wrapper_cast<CharacterWrapper>(args.front( ));
    
    obj_from_anywhere( target );
    ::obj_to_char( target, chWrap->getTarget( ) );
    return wrap(target);
}

NMI_INVOKE( ObjectWrapper, obj_from_room , "(): deprecated")
{
    return Register( );
}

NMI_INVOKE( ObjectWrapper, obj_to_room , "(room): положить предмет на пол в комнате room")
{
    RoomWrapper *roomWrap;
    
    checkTarget();
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
    
    roomWrap = wrapper_cast<RoomWrapper>(args.front( ));
    
    obj_from_anywhere( target );
    ::obj_to_room( target, roomWrap->getTarget( ) );
    return wrap(target);
}

NMI_INVOKE( ObjectWrapper, obj_from_obj , "(): deprecated")
{
    return Register( );
}

NMI_INVOKE( ObjectWrapper, obj_to_obj , "(obj): положить предмет внутрь другого предмета obj")
{
    ObjectWrapper *objWrap;
    
    checkTarget();
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
    
    objWrap = wrapper_cast<ObjectWrapper>(args.front( ));
    
    obj_from_anywhere( target );
    ::obj_to_obj( target, objWrap->getTarget( ) );
    return wrap(target);
}

NMI_INVOKE( ObjectWrapper, extract , "(): полностью уничтожить предмет")
{
    Register thiz( self );
    bool count = true;
    
    checkTarget();
    
    if (!args.empty())
        count = args.front().toNumber();
    
    ::extract_obj_1( target, count );
    return Register();
}

NMI_INVOKE( ObjectWrapper, get_extra_descr , "(key): найти экстра-описание с ключевым словом key")
{
    char *desc;
    
    checkTarget();
    
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );


    DLString d = args.front().toString( );
    desc = ::get_extra_descr(d.c_str( ), target->pIndexData->extra_descr);
    if (desc != 0)
        return desc;

    return ::get_extra_descr(d.c_str( ), target->extra_descr);
}

NMI_INVOKE( ObjectWrapper, set_extra_descr , "(key, text): установить экстра-описание text по ключевому слову key")
{
    checkTarget();

    RegisterList::const_iterator i = args.begin();

    if(i == args.end())
        throw Scripting::NotEnoughArgumentsException( );

    DLString name = i->toString();

    i++;
    
    if(i == args.end())
        throw Scripting::NotEnoughArgumentsException( );

    DLString text;
    
    if(i->type == Register::STRING)
        text = i->toString();

    EXTRA_DESCR_DATA **ed, *ned;
    
    for(ed = &target->extra_descr; *ed; )
        if(!str_cmp((*ed)->keyword, name.c_str())) {
            EXTRA_DESCR_DATA *n = (*ed)->next;
            
            free_extra_descr(*ed);
            *ed = n;
        } else
            ed = &(*ed)->next;
    
    ned = new_extra_descr();
    ned->next = target->extra_descr;
    ned->keyword = str_dup(name.c_str());
    ned->description = str_dup(text.c_str());
    target->extra_descr = ned;
    
    return Register( );
}


NMI_INVOKE( ObjectWrapper, equip, "(wearloc): надеть в указанную локацию тому, кто несет предмет" )
{
    Wearlocation *loc;
    
    checkTarget( );
    
    if(args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
    if (!target->carried_by)
        throw Scripting::Exception("object not in inventory");
    if (target->wear_loc != wear_none)
        throw Scripting::Exception("object already equipped");
    if (!( loc = wearlocationManager->findExisting( args.front( ).toString( ) ) ))
        throw Scripting::Exception("unknown wearlocation");

    loc->equip( target );
    return Register( );
}

NMI_INVOKE( ObjectWrapper, unequip, "(): снимает предмет и кладет в инвентарь тому, кто несет" )
{
    checkTarget( );
    
    if (target->wear_loc == wear_none || target->carried_by == NULL)
        throw Scripting::Exception("object already un-equipped");
    
    target->wear_loc->unequip( target );
    return Register( );
}


/*-----------------------------------------------------------------------*/

NMI_INVOKE( ObjectWrapper, isAffected, "(skill): находится ли предмет под воздействием аффекта с данным именем" )
{
    Skill *skill = args2skill(args);
    
    checkTarget( );

    if (skill)
        return target->isAffected( skill->getIndex( ) );
    else
        return false;
}

NMI_GET( ObjectWrapper, affected, "список (List) всех аффектов на предмете (структура .Affect)" )
{
    checkTarget();
    RegList::Pointer rc(NEW);

    if (!target->enchanted)
        for (auto &paf: target->pIndexData->affected) 
            rc->push_back( AffectWrapper::wrap( *paf ) );
    
    for (auto &paf: target->affected) 
        rc->push_back( AffectWrapper::wrap( *paf ) );
        
    Scripting::Object *sobj = &Scripting::Object::manager->allocate();
    sobj->setHandler(rc);

    return Register( sobj );
}

NMI_INVOKE( ObjectWrapper, affectAdd, "(aff): повесить на предмет указанный аффект (.Affect)" )
{
    checkTarget( );
    AffectWrapper *aw;
    
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
    
    aw = wrapper_cast<AffectWrapper>( args.front( ) );
    affect_to_obj( target, &(aw->getTarget()) );

    return Register( );
}

NMI_INVOKE( ObjectWrapper, affectJoin, "(aff): усилить существующий аффект или повесить новый (.Affect)" )
{
    checkTarget( );
    AffectWrapper *aw;
    
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
    
    aw = wrapper_cast<AffectWrapper>( args.front( ) );
    affect_enhance( target, &(aw->getTarget()) );

    return Register( );
}

NMI_INVOKE( ObjectWrapper, affectStrip, "(skill): снять с предмета все аффекты от умения по имени skill" )
{
    checkTarget( );
    Skill *skill = args2skill(args);
    
    affect_strip(target, skill->getIndex());
    return Register( );
}

NMI_INVOKE( ObjectWrapper, affectStripAll, "(): снять все аффекты" )
{
    checkTarget( );
    
    for (auto &paf: target->affected.clone())
        affect_remove_obj( target, paf );

    return Register( );
}

NMI_INVOKE( ObjectWrapper, random_obj_list, "([item_type]): случайный объект из списка, начинающегося с этого объекта. поиск ограничивается типом item_type, если задан")
{
    int itype = -1;
    ::Object *result = NULL;
    int cnt = 0;

    checkTarget( );

    if (!args.empty( )) 
        if (( itype = item_table.value( args2string( args ).c_str( ), true ) ) == NO_FLAG)
            throw Scripting::IllegalArgumentException( );

    for (::Object *obj = target; obj; obj = obj->next_content) 
        if (itype == -1 || obj->pIndexData->item_type == itype)
            if (number_range( 0, cnt++ ) == 0)
                result = obj;

    return wrap( result );
}

NMI_INVOKE(ObjectWrapper, hasWeaponFlag, "(flags): выставлен ли хотя бы один из флагов на оружии (таблица .tables.weapon_type2 или строка)")
{
    checkTarget( );
    if (target->item_type != ITEM_WEAPON)
        throw Scripting::Exception("Weapon flags queried on non-weapon");

    Register arg = argnum(args, 1);
    int flags;

    if (arg.type == Register::NUMBER) {
        flags = args2number(args);
    } else {
        DLString flagNames = args2string(args);
        flags = weapon_type2.bitstring(flagNames, true);
    }
    
    return Register(IS_WEAPON_STAT(target, flags) != 0);
}

NMI_INVOKE( ObjectWrapper, madeOfWood, "(): предмет сделан из дерева" )
{
    checkTarget( );
    return Register( material_is_typed( target, MAT_WOOD ) );
}

NMI_INVOKE( ObjectWrapper, madeOfMetal, "(): предмет сделан из металла" )
{
    checkTarget( );
    return Register( material_is_typed( target, MAT_METAL ) );
}

NMI_INVOKE( ObjectWrapper, materialBurns, "(): сколько тиков горит (-1 если тушит огонь)" )
{
    checkTarget( );
    return Register( material_burns( target ) );
}

NMI_INVOKE( ObjectWrapper, get_obj_content_vnum, "(vnum): поиск объекта внутри этого по внуму" )
{
    checkTarget( );

    int vnum = args2number( args );

    for (::Object *obj = target->contains; obj; obj = obj->next_content)
        if (obj->pIndexData->vnum == vnum)
            return wrap( obj );

    return Register( );
}

NMI_INVOKE( ObjectWrapper, list_obj_content_vnum, "(vnum): поиск списка (List) объектов внутри этого по внуму" )
{
    checkTarget( );
    RegList::Pointer rc(NEW);

    int vnum = args2number( args );

    for (::Object *obj = target->contains; obj; obj = obj->next_content)
        if (obj->pIndexData->vnum == vnum)
            rc->push_back( WrapperManager::getThis( )->getWrapper( obj ) );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( rc );

    return Register( sobj );
}

NMI_GET( ObjectWrapper, items, "список (List) всех предметов внутри этого" )
{
    checkTarget();
    RegList::Pointer rc(NEW);

    for (::Object *obj = target->contains; obj; obj = obj->next_content)
        rc->push_back( WrapperManager::getThis( )->getWrapper( obj ) );
    
    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( rc );

    return Register( sobj );
}

NMI_INVOKE( ObjectWrapper, api, "(): печатает этот API" )
{
    ostringstream buf;
    Scripting::traitsAPI<ObjectWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( ObjectWrapper, rtapi, "(): печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( ObjectWrapper, clear, "(): очистка всех runtime полей" )
{
    guts.clear( );
    self->changed();
    return Register( );
}

