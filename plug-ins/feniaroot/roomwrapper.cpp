/* $Id: roomwrapper.cpp,v 1.1.4.21.6.22 2014-09-19 11:39:39 rufina Exp $
 *
 * ruffina, 2004
 */

#include <sys/time.h>
#include <iostream>

#include "logstream.h"
#include "skillmanager.h"
#include "affect.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "save.h"                                                               
#include "merc.h"
#include "profiler.h"

#include "structwrappers.h"
#include "objectwrapper.h"
#include "roomwrapper.h"
#include "characterwrapper.h"
#include "wrappermanager.h"
#include "reglist.h"
#include "wrap_utils.h"
#include "nativeext.h"
#include "subr.h"

#include "roomtraverse.h"
#include "def.h"

using namespace std;
using namespace Scripting;

DLString regfmt(Character *to, const RegisterList &argv);

NMI_INIT(RoomWrapper, "комната")

RoomWrapper::RoomWrapper( ) : target( NULL )
{
}

void RoomWrapper::setSelf( Scripting::Object *s )
{
    WrapperBase::setSelf( s );

    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void RoomWrapper::extract( bool count )
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        LogStream::sendError() << "Room wrapper: extract without target" << endl;
    }

    GutsContainer::extract( count );
}

void RoomWrapper::setTarget( ::Room *r )
{
    target = r;
    id = ROOM_VNUM2ID(r->vnum);
}

void RoomWrapper::checkTarget( ) const throw( Scripting::Exception )
{
    if (zombie.getValue())
        throw Scripting::Exception( "Room is dead" );
        
    if (target == NULL)
        throw Scripting::Exception( "Room is offline" );
}

Room * RoomWrapper::getTarget( ) const
{
    checkTarget();
    return target;
}


#define GETWRAP(x, h) NMI_GET(RoomWrapper, x, h) { \
    checkTarget(); \
    return WrapperManager::getThis( )->getWrapper(target->x); \
}

GETWRAP( rnext, "указывает на след. комнату в глобальном списке .room_list" )
GETWRAP( contents, "указывает на первый предмет на полу комнаты" )
GETWRAP( people, "указывает на первого чара в комнате" )

NMI_GET( RoomWrapper, vnum , "номер комнаты в арии")
{
    checkTarget( );
    return Register( target->vnum );
}

NMI_GET( RoomWrapper, name , "название комнаты")
{
    checkTarget( );
    return Register( target->name );
}

NMI_GET( RoomWrapper, areaname , "имя арии")
{
    checkTarget( );
    return Register( target->area->name );
}

NMI_GET( RoomWrapper, area, "экземпляр Area для этой комнаты")
{
    checkTarget( );
    return AreaWrapper::wrap( target->area->area_file->file_name );
}

NMI_GET(RoomWrapper, ppl, "список (List) всех чаров в комнате")
{
    checkTarget();
    RegList::Pointer rc(NEW);

    Character *rch;
    
    for(rch = target->people; rch; rch = rch->next_in_room)
        rc->push_back( WrapperManager::getThis( )->getWrapper( rch ) );
    
    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}

NMI_GET( RoomWrapper, items, "список (List) всех предметов на полу" )
{
    checkTarget();
    RegList::Pointer rc(NEW);

    for (::Object *obj = target->contents; obj; obj = obj->next_content)
        rc->push_back( WrapperManager::getThis( )->getWrapper( obj ) );
    
    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( rc );

    return Register( sobj );
}

NMI_GET( RoomWrapper, sector_type , "значение типа местности (таблица .sector_table)")
{
    checkTarget( );
    return Register( target->sector_type );
}

NMI_GET( RoomWrapper, affected_by, "биты аффектов на комнате (таблица .tables.affect_flags)" )
{
    checkTarget( );
    return (int)target->affected_by;
}

NMI_SET( RoomWrapper, affected_by, "биты аффектов на комнате (таблица .tables.affect_flags)" )
{
    checkTarget( );
    target->affected_by = arg.toNumber();
}

NMI_GET( RoomWrapper, room_flags, "флаги комнаты (таблица .tables.room_flags)" )
{
    checkTarget( );
    return (int)target->room_flags;
}

NMI_SET( RoomWrapper, room_flags, "флаги комнаты (таблица .tables.room_flags)" )
{
    checkTarget( );
    target->room_flags = arg.toNumber();
}

NMI_GET( RoomWrapper, light, "количество источников света в комнате" )
{
    checkTarget( );
    return (int)target->light;
}

NMI_GET( RoomWrapper, description, "описание комнаты" )
{
    checkTarget( );
    return Register( target->description );
}

NMI_SET( RoomWrapper, light, "количество источников света в комнате" )
{
    checkTarget( );
    target->light = arg.toNumber();
}

NMI_GET( RoomWrapper, clan, "имя клана, которому принадлежит комната" )
{
    checkTarget();
    return Register( target->clan->getShortName( ) );
}

static Scripting::Register get_direction( Room *r, int dir )
{
    if (r->exit[dir])
        return WrapperManager::getThis( )->getWrapper(r->exit[dir]->u1.to_room);
    else 
        return Scripting::Register( );
}

static int get_door_argument( const RegisterList &args )
{
    int door;
    DLString doorName;
    
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
    
    doorName = args.front( ).toString( );
    if (( door = direction_lookup( doorName.c_str( ) ) ) == -1)
        door = args.front( ).toNumber( );

    if (door < 0 || door >= DIR_SOMEWHERE)
        throw Scripting::IllegalArgumentException( );

    return door;
}

NMI_GET( RoomWrapper, north, "комната на север отсюда или null")
{
    checkTarget( );
    return get_direction( target, DIR_NORTH );
}
NMI_GET( RoomWrapper, south, "комната на юг отсюда или null")
{
    checkTarget( );
    return get_direction( target, DIR_SOUTH );
}
NMI_GET( RoomWrapper, east, "комната на восток отсюда или null")
{
    checkTarget( );
    return get_direction( target, DIR_EAST );
}
NMI_GET( RoomWrapper, west, "комната на запад отсюда или null")
{
    checkTarget( );
    return get_direction( target, DIR_WEST );
}
NMI_GET( RoomWrapper, up, "комната вверх отсюда или null")
{
    checkTarget( );
    return get_direction( target, DIR_UP );
}
NMI_GET( RoomWrapper, down, "комната вниз отсюда или null")
{
    checkTarget( );
    return get_direction( target, DIR_DOWN );
}

/*
 * METHODS
 */

NMI_INVOKE( RoomWrapper, exits, "(ch): список номеров всех доступных выходов для персонажа ch")
{
    RegList::Pointer list(NEW);
    
    checkTarget( );
    Scripting::Object *listObj = &Scripting::Object::manager->allocate( );
    listObj->setHandler( list );
    Character *ch = args2character(args);

    for (int door = 0; door < DIR_SOMEWHERE; door++) {
        EXIT_DATA *pexit;
        Room *room;
        if (!( pexit = target->exit[door] ))
            continue;
        if (!( room = pexit->u1.to_room ))
            continue;
        if (!ch->can_see( room ))
            continue;
        list->push_back(Register(door));
    }

    return Register( listObj );
}

NMI_INVOKE(RoomWrapper, doorTo, "(room): номер двери, ведущей из этой комнаты в room" )
{
    Room *room;
    
    checkTarget( );
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    room = wrapper_cast<RoomWrapper>( args.front( ) )->getTarget( );

    for (int d = 0; d < DIR_SOMEWHERE; d++)
        if (target->exit[d] && target->exit[d]->u1.to_room == room)
            return d;

    return -1;
}

NMI_INVOKE( RoomWrapper, getRoom, "(имя или номер выхода): комната по этому направлению" )
{
    int door;
    
    checkTarget( );
    door = get_door_argument( args );
    return get_direction( target, door );
}

NMI_INVOKE( RoomWrapper, getRevDoor, "(имя или номер выхода): вернет номер противоположного направления" )
{
    checkTarget( );
    return dirs[get_door_argument( args )].rev;
}

NMI_INVOKE( RoomWrapper, doorNumber, "(имя выхода): вернет номер выхода" )
{
    checkTarget( );
    return get_door_argument( args );
}

NMI_INVOKE( RoomWrapper, doorName, "(номер выхода): вернет имя выхода" )
{
    checkTarget( );
    return dirs[get_door_argument( args )].name;
}

NMI_INVOKE( RoomWrapper, dirMsgLeave, "(имя или номер выхода): сообщение при уходе через этот выход (на север, на восток)" )
{
    checkTarget( );
    return dirs[get_door_argument( args )].leave;
}

NMI_INVOKE( RoomWrapper, dirMsgEnter, "(имя или номер выхода): сообщение при заходе через этот выход (с юга, с запада)" )
{
    checkTarget( );
    return dirs[get_door_argument( args )].enter;
}

NMI_INVOKE( RoomWrapper, getExitFlags, "(имя или номер выхода): флаги этого выхода (таблица .tables.exit_flags)" )
{
    EXIT_DATA *pExit;
    
    checkTarget( );

    pExit = target->exit[get_door_argument( args )];
    if (pExit)
        return Register( pExit->exit_info );
    else
        return Register( 0 );
}

NMI_INVOKE( RoomWrapper, exitKeyword, "(имя или номер выхода): ключевые слова, на которые откликается эта дверь или выход" )
{
    EXIT_DATA *pExit;
    
    checkTarget( );

    pExit = target->exit[get_door_argument( args )];
    if (pExit)
        return Register(pExit->keyword);
    else
        return Register("");
}

NMI_INVOKE( RoomWrapper, exitShortDescr, "(имя или номер выхода): название выхода с падежами" )
{
    EXIT_DATA *pExit;
    
    checkTarget( );

    pExit = target->exit[get_door_argument( args )];
    if (pExit)
        return Register(pExit->short_descr);
    else
        return Register("");
}

static void update_door_flags( Room *room, const RegisterList &args, int flags, bool fSet )
{
    EXIT_DATA *pExit = room->exit[get_door_argument( args )];

    if (pExit) {
        if (fSet)
            SET_BIT(pExit->exit_info, flags );
        else
            REMOVE_BIT(pExit->exit_info, flags );
    }
}

NMI_INVOKE(RoomWrapper, close, "(имя или номер выхода): закрыть дверь по указанному направлению")
{
    checkTarget( );
    update_door_flags( target, args, EX_CLOSED, true ); 
    return Register( ); 
}

NMI_INVOKE(RoomWrapper, open, "(имя или номер выхода): открыть дверь по указанному направлению")
{
    checkTarget( );
    update_door_flags( target, args, EX_CLOSED|EX_LOCKED, false ); 
    return Register( ); 
}

NMI_INVOKE(RoomWrapper, lock, "(имя или номер выхода): запереть дверь по указанному направлению")
{
    checkTarget( );
    update_door_flags( target, args, EX_CLOSED|EX_LOCKED, true ); 
    return Register( ); 
}

NMI_INVOKE(RoomWrapper, unlock, "(имя или номер выхода): отпереть дверь по указанному направлению")
{
    checkTarget( );
    update_door_flags( target, args, EX_LOCKED, false ); 
    return Register( ); 
}


NMI_INVOKE(RoomWrapper, isDark, "(): true если в комнате темно" )
{
    checkTarget( );
    return Register( target->isDark( ) );
}

NMI_INVOKE(RoomWrapper, isCommon, "(): true если комната доступна всем (т.е. не приватная/клановая/newbie-only/...)" )
{
    checkTarget( );
    return Register( target->isCommon( ) );
}

NMI_INVOKE( RoomWrapper, echo, "(fmt, args): выводит отформатированную строку всем неспящим в комнате" )
{
    checkTarget( );
    target->echo( POS_RESTING, regfmt( NULL, args ).c_str( ) );
    return Register( );
}

NMI_INVOKE( RoomWrapper, echoAround, "(fmt, args): выводит отформатированную строку всем неспящим в прилегающие комнаты" )
{
    checkTarget( );
    target->echoAround( POS_RESTING, regfmt( NULL, args ).c_str( ) );
    return Register( );
}

NMI_INVOKE(RoomWrapper, zecho, "(msg): выведет сообщение msg для всех в этой арии" )
{
    Character *wch;
    const char *msg;
    
    checkTarget( );

    if (args.size( ) != 1)
        throw Scripting::NotEnoughArgumentsException( );

    msg = args.front( ).toString( ).c_str( );
    
    for (wch = char_list; wch; wch = wch->next) 
        if (wch->in_room->area == target->area) 
            wch->println( msg );

    return Register( );
}

NMI_INVOKE(RoomWrapper, get_obj_vnum, "(vnum): поиск первого объекта в комнате по его внуму" )
{
    int vnum;
    ::Object *obj;

    checkTarget( );

    if (args.size( ) != 1)
        throw Scripting::NotEnoughArgumentsException( );
    
    vnum = args.front( ).toNumber( );

    for (obj = target->contents; obj; obj = obj->next_content)
        if (obj->pIndexData->vnum == vnum)
            return WrapperManager::getThis( )->getWrapper(obj); 

    return Register( );
}

NMI_INVOKE(RoomWrapper, get_obj_type, "(type): поиск первого объекта в комнате по его типу (таблица .tables.item_table)" )
{
    checkTarget( );

    int itemType = args2number(args);
    ::Object *obj = get_obj_room_type(target, itemType);
    if (!obj)
        return Register();
    return WrapperManager::getThis( )->getWrapper(obj); 
}

NMI_INVOKE( RoomWrapper, list_obj_vnum, "(vnum): поиск списка объектов в комнате по внуму" )
{
    checkTarget( );
    RegList::Pointer rc(NEW);

    int vnum = args2number( args );

    for (::Object *obj = target->contents; obj; obj = obj->next_content)
        if (obj->pIndexData->vnum == vnum)
            rc->push_back( WrapperManager::getThis( )->getWrapper( obj ) );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( rc );

    return Register( sobj );
}

NMI_INVOKE(RoomWrapper, get_mob_vnum, "(vnum): поиск первого моба в комнате по его внуму" )
{
    int vnum;
    Character *rch;

    checkTarget( );

    if (args.size( ) != 1)
        throw Scripting::NotEnoughArgumentsException( );
    
    vnum = args.front( ).toNumber( );

    for (rch = target->people; rch; rch = rch->next_in_room)
        if (rch->is_npc( ) && rch->getNPC( )->pIndexData->vnum == vnum)
            return WrapperManager::getThis( )->getWrapper( rch ); 

    return Register( );
}

NMI_INVOKE( RoomWrapper, list_mob_vnum, "(vnum): поиск списка мобов в комнате по внуму" )
{
    checkTarget( );
    RegList::Pointer rc(NEW);

    int vnum = args2number( args );

    for (Character *rch = target->people; rch; rch = rch->next_in_room)
        if (rch->is_npc( ) && rch->getNPC( )->pIndexData->vnum == vnum)
            rc->push_back( WrapperManager::getThis( )->getWrapper( rch ) );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( rc );

    return Register( sobj );
}

/*---------------------------------------------------------
 * fenia traverse
 *--------------------------------------------------------*/
struct FeniaDoorFunc {
    FeniaDoorFunc( Character *w = 0, bitstring_t sa = 0, bitstring_t sd = 0 )
                  : walker( w ), sectorsAllow( sa ), sectorsDeny( sd )
    {
    }
    bool operator () ( Room *const room, EXIT_DATA *exit ) const
    {
        if (IS_SET(exit->exit_info, EX_LOCKED))
            return false;
        
        Room *toRoom = exit->u1.to_room;
        bitstring_t mysector = (1 << toRoom->sector_type);

        if (sectorsAllow != 0 && !IS_SET(sectorsAllow, mysector))
            return false;

        if (sectorsDeny != 0 && IS_SET(sectorsDeny, mysector))
            return false;

        if (!toRoom->isCommon( ))
            return false;
            
        if (walker && !walker->canEnter( toRoom ))
            return false;
        
        return true;
    }
     
    Character *walker;
    bitstring_t sectorsAllow, sectorsDeny;
};

struct FeniaExtraExitFunc {
    bool operator () ( Room *const room, EXTRA_EXIT_DATA *eexit ) const
    {
        return false;
    }
};

struct FeniaPortalFunc {
    bool operator () ( Room *const room, ::Object *portal ) const
    {
        return false;
    }
};

typedef RoomRoadsIterator<FeniaDoorFunc, FeniaExtraExitFunc, FeniaPortalFunc> 
                      FeniaHookIterator;

struct PathWithDepthComplete {
    typedef NodesEntry<RoomTraverseTraits> MyNodesEntry;
    PathWithDepthComplete( int d, RegList::Pointer r ) : depth( d ), rooms( r ) 
    { 
    }

    inline bool operator () ( const MyNodesEntry *const head, bool last ) 
    {
        if (head->generation < depth && !last)
            return false;

        for (const MyNodesEntry *i = head; i->prev; i = i->prev) 
            rooms->push_front( WrapperManager::getThis( )->getWrapper( i->node ) );

        return true;
    }

    int depth;
    RegList::Pointer rooms;
};

struct PathToTargetComplete {
    typedef NodesEntry<RoomTraverseTraits> MyNodesEntry;
    
    PathToTargetComplete( Room *t, RegList::Pointer r ) : target( t ), rooms( r ) 
    { 
    }

    inline bool operator () ( const MyNodesEntry *const head, bool last ) 
    {
        if (head->node != target)
            return false;
        
        for (const MyNodesEntry *i = head; i->prev; i = i->prev) 
            rooms->push_front( WrapperManager::getThis( )->getWrapper( i->node ) );

        return true;
    }
    
    Room *target;
    RegList::Pointer rooms;
};

NMI_INVOKE( RoomWrapper, traverse, "(depth, walker, sectorsAllow, sectorsDeny): построит путь (список комнат) для чара walker глубины depth, с разрешенными-запрещенными типами местности в виде битовых масок" )
{
    bitstring_t sectorsAllow, sectorsDeny;
    int depth;
    Character *walker;
    Scripting::RegisterList::const_iterator i = args.begin( );

    checkTarget( );
    
    depth = (i == args.end( ) ? 4000 : (i++)->toNumber( ));
    walker = (i == args.end( ) || i->type == Register::NUMBER) ? 0 : wrapper_cast<CharacterWrapper>( *i++ )->getTarget( );
    sectorsAllow = (i == args.end( ) ? 0 : (i++)->toNumber( ));
    sectorsDeny= (i == args.end( ) ? 0 : (i++)->toNumber( ));
    
    FeniaDoorFunc df( walker, sectorsAllow, sectorsDeny );
    FeniaExtraExitFunc eef;
    FeniaPortalFunc pf;
    FeniaHookIterator iter( df, eef, pf, 5 );
    
    RegList::Pointer rooms( NEW );
    PathWithDepthComplete complete( depth, rooms );

    room_traverse( target, iter, complete, 10000 );

    Scripting::Object *obj = &Scripting::Object::manager->allocate( );
    obj->setHandler( rooms );

    return Scripting::Register( obj );
}

NMI_INVOKE( RoomWrapper, traverseTo, "(target, walker, sectorsAllow, sectorsDeny): построит путь до цели target для чара walker, с разрешенными-запрещенными типами местности в виде битовых масок" )
{
    bitstring_t sectorsAllow, sectorsDeny;
    Room *targetRoom;
    Character *walker;
    Scripting::RegisterList::const_iterator i = args.begin( );

    checkTarget( );
    
    targetRoom = (i == args.end( ) ? target : wrapper_cast<RoomWrapper>( *i++ )->getTarget( ));
    walker = (i == args.end( ) || i->type == Register::NUMBER) ? 0 : wrapper_cast<CharacterWrapper>( *i++ )->getTarget( );
    sectorsAllow = (i == args.end( ) ? 0 : (i++)->toNumber( ));
    sectorsDeny= (i == args.end( ) ? 0 : (i++)->toNumber( ));
    
    FeniaDoorFunc df( walker, sectorsAllow, sectorsDeny );
    FeniaExtraExitFunc eef;
    FeniaPortalFunc pf;
    FeniaHookIterator iter( df, eef, pf, 5 );
    
    RegList::Pointer rooms( NEW );
    PathToTargetComplete complete( targetRoom, rooms );

    room_traverse( target, iter, complete, 10000 );

    Scripting::Object *obj = &Scripting::Object::manager->allocate( );
    obj->setHandler( rooms );

    return Scripting::Register( obj );
}

NMI_GET( RoomWrapper, resetMobiles, "список внумов мобов, которые ресетятся в этой комнате") 
{
    RESET_DATA *pReset;
    RegList::Pointer rc(NEW);
    
    checkTarget( );
    
    for (pReset = target->reset_first; pReset; pReset = pReset->next)
        if (pReset->command == 'M')
            rc->push_back( Register( pReset->arg1 ) );

    Scripting::Object *obj = &Scripting::Object::manager->allocate( );
    obj->setHandler( rc );

    return Register( obj );
}    

NMI_INVOKE( RoomWrapper, api, "(): печатает этот API" )
{
    ostringstream buf;
    Scripting::traitsAPI<RoomWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( RoomWrapper, rtapi, "(): печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( RoomWrapper, clear, "(): очистка всех runtime полей" )
{
    guts.clear( );
    self->changed();
    return Register( );
}

