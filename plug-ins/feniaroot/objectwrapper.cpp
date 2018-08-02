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
#include "mercdb.h"

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

void ObjectWrapper::checkTarget( ) const throw( Scripting::Exception )
{
    if (zombie.getValue())
	throw Scripting::Exception( "Non existent object referenced" );

    if (target == NULL) 
	throw Scripting::Exception( "Object is offline" );
}

NMI_GET( ObjectWrapper, online, "" )
{
    return Register( target != NULL );
}

NMI_GET( ObjectWrapper, dead, "" )
{
    return Register( zombie.getValue() );
}

#define GETWRAP(x) NMI_GET(ObjectWrapper, x, "") { \
    checkTarget(); \
    return WrapperManager::getThis( )->getWrapper(target->x); \
}

NMI_GET( ObjectWrapper, id , "")
{
    return DLString(id.getValue( ));
}

NMI_GET( ObjectWrapper, vnum , "") 
{ 
    checkTarget( ); 
    return target->pIndexData->vnum;
}

NMI_GET( ObjectWrapper, short_descr , "")
{
    checkTarget( );
    return Register( target->getShortDescr( ) );
}

NMI_SET( ObjectWrapper, short_descr , "")
{
    DLString d = arg.toString( );
    
    checkTarget( );
    target->setShortDescr( d.c_str( ) );
}

NMI_GET( ObjectWrapper, real_short_descr , "")
{
    checkTarget( );
    return Register( target->getRealShortDescr( ) );
}

NMI_GET( ObjectWrapper, enchanted, "")
{
    checkTarget( );
    return Register( target->enchanted );
}

NMI_SET( ObjectWrapper, enchanted , "")
{
    checkTarget( );
    target->enchanted = arg.toNumber( );
}

NMI_GET( ObjectWrapper, description , "")
{
    checkTarget( );
    return Register( target->getDescription( ) );
}

NMI_SET( ObjectWrapper, description , "")
{
    DLString d = arg.toString( );

    checkTarget( );
    target->setDescription( d.c_str( ) );
}

NMI_GET( ObjectWrapper, material, "")
{
    checkTarget( );
    return Register( target->getMaterial( ) );
}

NMI_SET( ObjectWrapper, material, "")
{
    DLString d = arg.toString( );

    checkTarget( );
    target->setMaterial( d.c_str( ) );
}

NMI_GET( ObjectWrapper, name , "")
{
    checkTarget( );
    return Register( target->getName( ) );
}

NMI_SET( ObjectWrapper, name , "")
{
    DLString d = arg.toString( );

    checkTarget( );
    target->setName( d.c_str() );
}

NMI_GET( ObjectWrapper, pocket, "")
{
    checkTarget( );
    return Register( target->pocket );
}

NMI_SET( ObjectWrapper, pocket, "")
{
    checkTarget( );
    target->pocket = arg.toString( );
}

NMI_GET( ObjectWrapper, level , "")
{
    checkTarget( );
    return Register( target->level );
}

NMI_SET( ObjectWrapper, level , "")
{
    checkTarget( );
    target->level = arg.toNumber( );
}

NMI_GET( ObjectWrapper, condition , "")
{
    checkTarget( );
    return Register( target->condition );
}

NMI_SET( ObjectWrapper, condition , "")
{
    checkTarget( );
    target->condition = arg.toNumber( );
}

NMI_GET( ObjectWrapper, cost , "")
{
    checkTarget( );
    return Register( target->cost );
}

NMI_SET( ObjectWrapper, cost , "")
{
    checkTarget( );
    target->cost = arg.toNumber( );
}

NMI_GET( ObjectWrapper, extra_flags, "")
{
    checkTarget( );
    return Register( target->extra_flags );
}

NMI_SET( ObjectWrapper, extra_flags, "")
{
    checkTarget( );
    target->extra_flags = arg.toNumber( );
}

NMI_GET( ObjectWrapper, wear_flags, "")
{
    checkTarget( );
    return Register( target->wear_flags);
}

NMI_SET( ObjectWrapper, wear_flags, "")
{
    checkTarget( );
    target->wear_flags = arg.toNumber( );
}

NMI_GET( ObjectWrapper, timer, "")
{
    checkTarget( );
    return Register( target->timer );
}

NMI_SET( ObjectWrapper, timer, "")
{
    checkTarget( );
    target->timer = arg.toNumber( );
}

NMI_GET( ObjectWrapper, owner , "")
{
    checkTarget( );
    const char *o = target->getOwner( );

    if (o == NULL)
	return Register( "" );
    else
	return Register( o );
}

NMI_SET( ObjectWrapper, owner , "")
{
    DLString d = arg.toString( );
    checkTarget( );
    target->setOwner( d.c_str() );
}

NMI_GET( ObjectWrapper, item_type, "")
{
    checkTarget( );
    return Register( target->item_type);
}
NMI_GET( ObjectWrapper, wear_loc, "")
{
    checkTarget( );
    return Register( target->wear_loc->getName( ) );
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

#define SETGETVALUE(x) \
    NMI_GET( ObjectWrapper, value##x, "") { \
	checkTarget( ); \
	return Register( target->value[x]); \
    } \
    NMI_SET( ObjectWrapper, value##x, "") { \
	checkTarget( ); \
	target->value[x] = arg.toNumber(); \
    }
	
SETGETVALUE(0)
SETGETVALUE(1)
SETGETVALUE(2)
SETGETVALUE(3)
SETGETVALUE(4)

GETWRAP( pIndexData )
GETWRAP( next )
GETWRAP( next_content )
GETWRAP( contains )
GETWRAP( in_obj )
GETWRAP( carried_by )
GETWRAP( in_room )

/*
 * Methods
 */
NMI_INVOKE( ObjectWrapper, getCarrier, "")
{
    checkTarget();
    Character *ch = target->getCarrier();

    if (ch)
	return WrapperManager::getThis( )->getWrapper(ch); 
    else
	return Register();
}
NMI_INVOKE( ObjectWrapper, getRoom, "")
{
    checkTarget();
    Room *r = target->getRoom();

    if (r) 
	return WrapperManager::getThis( )->getWrapper(r); 
    else
	return Register();
}

static void obj_from_anywhere( ::Object *obj )
{
    if (obj->in_room)
	obj_from_room( obj );
    else if (obj->carried_by)
	obj_from_char( obj );
    else if (obj->in_obj)
	obj_from_obj( obj );
}

NMI_INVOKE( ObjectWrapper, obj_from_char , "deprecated")
{
    return Register( );
}

NMI_INVOKE( ObjectWrapper, obj_to_char , "")
{
    CharacterWrapper *chWrap;
    
    checkTarget();
    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );
    
    chWrap = wrapper_cast<CharacterWrapper>(args.front( ));
    
    obj_from_anywhere( target );
    ::obj_to_char( target, chWrap->getTarget( ) );
    return Register( );
}

NMI_INVOKE( ObjectWrapper, obj_from_room , "deprecated")
{
    return Register( );
}

NMI_INVOKE( ObjectWrapper, obj_to_room , "")
{
    RoomWrapper *roomWrap;
    
    checkTarget();
    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );
    
    roomWrap = wrapper_cast<RoomWrapper>(args.front( ));
    
    obj_from_anywhere( target );
    ::obj_to_room( target, roomWrap->getTarget( ) );
    return Register( );
}

NMI_INVOKE( ObjectWrapper, obj_from_obj , "deprecated")
{
    return Register( );
}

NMI_INVOKE( ObjectWrapper, obj_to_obj , "")
{
    ObjectWrapper *objWrap;
    
    checkTarget();
    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );
    
    objWrap = wrapper_cast<ObjectWrapper>(args.front( ));
    
    obj_from_anywhere( target );
    ::obj_to_obj( target, objWrap->getTarget( ) );
    return Register( );
}

NMI_INVOKE( ObjectWrapper, extract , "")
{
    Register thiz( self );
    bool count = true;
    
    checkTarget();
    
    if (!args.empty())
	count = args.front().toNumber();
    
    ::extract_obj_1( target, count );
    return Register();
}

NMI_INVOKE( ObjectWrapper, get_extra_descr , "")
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

NMI_INVOKE( ObjectWrapper, set_extra_descr , "")
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


NMI_INVOKE( ObjectWrapper, equip, "оденется в указанное место тому кто нас несет" )
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

NMI_INVOKE( ObjectWrapper, unequip, "снимает шмотку и кладет в инвентарь" )
{
    checkTarget( );
    
    if (target->wear_loc == wear_none || target->carried_by == NULL)
	throw Scripting::Exception("object already un-equipped");
    
    target->wear_loc->unequip( target );
    return Register( );
}


/*-----------------------------------------------------------------------*/


NMI_GET( ObjectWrapper, affected, "" )
{
    checkTarget();
    RegList::Pointer rc(NEW);
    Affect *paf;

    if (!target->enchanted)
	for (paf = target->pIndexData->affected; paf != 0; paf = paf->next) 
	    rc->push_back( AffectWrapper::wrap( *paf ) );
    
    for (paf = target->affected; paf != 0; paf = paf->next) 
	rc->push_back( AffectWrapper::wrap( *paf ) );
	
    Scripting::Object *sobj = &Scripting::Object::manager->allocate();
    sobj->setHandler(rc);

    return Register( sobj );
}

NMI_INVOKE( ObjectWrapper, affectAdd, "" )
{
    checkTarget( );
    AffectWrapper *aw;
    Affect af;
    
    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );
    
    aw = wrapper_cast<AffectWrapper>( args.front( ) );
    aw->toAffect( af );
    affect_to_obj( target, &af );

    return Register( );
}

NMI_INVOKE( ObjectWrapper, affectStrip, "" )
{
    checkTarget( );
    Skill *skill;
    int sn;
    Affect *paf, *paf_next;
    
    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );

    skill = skillManager->findExisting( args.front( ).toString( ) );
    
    if (!skill)
	throw Scripting::IllegalArgumentException( );
    
    sn = skill->getIndex( );

    for (paf = target->affected; paf; paf = paf_next) {
	paf_next = paf->next;

	if (paf->type == sn)
	    affect_remove_obj( target, paf );
    }
    
    return Register( );
}

NMI_INVOKE( ObjectWrapper, affectStripAll, "" )
{
    checkTarget( );
    
    while (target->affected)
	affect_remove_obj( target, target->affected );

    return Register( );
}

NMI_INVOKE( ObjectWrapper, random_obj_list, "случайный объект из списка, начинающегося с этого объекта. может брать в параметры item type")
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

NMI_INVOKE( ObjectWrapper, madeOfWood, "предмет сделан из дерева" )
{
    checkTarget( );
    return Register( material_is_typed( target, MAT_WOOD ) );
}

NMI_INVOKE( ObjectWrapper, madeOfMetal, "предмет сделан из металла" )
{
    checkTarget( );
    return Register( material_is_typed( target, MAT_METAL ) );
}

NMI_INVOKE( ObjectWrapper, materialBurns, "сколько тиков горит (-1 если тушит огонь)" )
{
    checkTarget( );
    return Register( material_burns( target ) );
}

NMI_INVOKE( ObjectWrapper, get_obj_content_vnum, "поиск объекта внутри этого по внуму" )
{
    checkTarget( );

    int vnum = args2number( args );

    for (::Object *obj = target->contains; obj; obj = obj->next_content)
	if (obj->pIndexData->vnum == vnum)
	    return wrap( obj );

    return Register( );
}

NMI_INVOKE( ObjectWrapper, list_obj_content_vnum, "поиск списка объектов внутри этого по внуму" )
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

NMI_INVOKE( ObjectWrapper, api, "печатает этот API" )
{
    ostringstream buf;
    Scripting::traitsAPI<ObjectWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( ObjectWrapper, rtapi, "печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( ObjectWrapper, clear, "очистка всех runtime полей" )
{
    guts.clear( );
    self->changed();
    return Register( );
}

