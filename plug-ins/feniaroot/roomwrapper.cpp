/* $Id: roomwrapper.cpp,v 1.1.4.21.6.22 2014-09-19 11:39:39 rufina Exp $
 *
 * ruffina, 2004
 */

#include <sys/time.h>
#include <iostream>
#include <string.h>

#include "logstream.h"
#include "skillmanager.h"
#include "affect.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "roomutils.h"
#include "save.h"
#include "loadsave.h"                                                               
#include "merc.h"
#include "profiler.h"
#include "behavior.h"
#include "json_utils_ext.h"

#include "structwrappers.h"
#include "areaindexwrapper.h"
#include "affectwrapper.h"
#include "objectwrapper.h"
#include "roomwrapper.h"
#include "characterwrapper.h"
#include "wrappermanager.h"
#include "regcontainer.h"
#include "lex.h"
#include "reglist.h"
#include "wrap_utils.h"
#include "nativeext.h"
#include "subr.h"

#include "roomtraverse.h"
#include "def.h"

using namespace std;
using namespace Scripting;

DLString regfmt(Character *to, const RegisterList &argv);

/** Resolve door argument that can be either door number or direction name. */
static int get_door_argument( const RegisterList &args )
{
    return args2door(args);
}

/** Populate either exit or extra exit data based on exit number or keyword. Return false if not found. */
static bool resolve_exits(const RegisterList &args, Room *room, EXIT_DATA *&pExit, EXTRA_EXIT_DATA *&pExtraExit)
{
    pExit = 0;
    pExtraExit = 0;

    if (args.empty())
        throw Scripting::NotEnoughArgumentsException();

    Register arg = args.front();
    int door = arg2door(arg);

    if (door < 0) {
        pExtraExit = room->extra_exits.find(arg.toString());
    } else {
        pExit = room->exit[door];
    }

    return pExtraExit != 0 || pExit != 0;
}

/** Find an exit and change exit flags on it. */
static void update_door_flags( Room *room, const RegisterList &args, int flags, bool fSet )
{
    EXIT_DATA *pExit;
    EXTRA_EXIT_DATA *pExtraExit;

    if (!resolve_exits(args, room, pExit, pExtraExit))
        throw Scripting::IllegalArgumentException();

    int &exit_info = pExtraExit ? pExtraExit->exit_info : pExit->exit_info;
    if (fSet)
        SET_BIT(exit_info, flags );
    else
        REMOVE_BIT(exit_info, flags );
}

/** Translate door name into a room wrapper. */
static Scripting::Register get_direction( Room *r, int dir )
{
    if (r->exit[dir])
        return WrapperManager::getThis( )->getWrapper(r->exit[dir]->u1.to_room);
    else 
        return Scripting::Register( );
}


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

void RoomWrapper::checkTarget( ) const 
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
    return Register( target->getName() );
}

NMI_GET( RoomWrapper, areaname , "имя арии в именительном падеже")
{
    checkTarget( );
    return Register( target->areaName() );
}

NMI_GET( RoomWrapper, area, "экземпляр AreaIndex для этой комнаты")
{
    checkTarget( );
    return WrapperManager::getThis()->getWrapper(target->areaIndex());
}

NMI_GET(RoomWrapper, smell, "строка с запахом комнаты")
{
    checkTarget();
    return target->pIndexData->smell;
}

NMI_SET(RoomWrapper, smell, "строка с запахом комнаты")
{
    checkTarget();
    target->pIndexData->smell = arg.toString();
    target->areaIndex()->changed = true;
}

NMI_GET(RoomWrapper, sound, "строка со звуком в комнате")
{
    checkTarget();
    return target->pIndexData->sound;
}

NMI_SET(RoomWrapper, sound, "строка со звуком в комнате")
{
    checkTarget();
    target->pIndexData->sound = arg.toString();
    target->areaIndex()->changed = true;
}

NMI_GET(RoomWrapper, ppl, "список (List) всех чаров в комнате")
{
    checkTarget();
    RegList::Pointer rc(NEW);

    Character *rch;
    
    for(rch = target->people; rch; rch = rch->next_in_room)
        rc->push_back( WrapperManager::getThis( )->getWrapper( rch ) );
    
    return wrap(rc);
}

NMI_GET( RoomWrapper, items, "список (List) всех предметов на полу" )
{
    checkTarget();
    RegList::Pointer rc(NEW);

    for (::Object *obj = target->contents; obj; obj = obj->next_content)
        rc->push_back( WrapperManager::getThis( )->getWrapper( obj ) );
    
    return wrap(rc);
}

NMI_GET(RoomWrapper, players, "список (List) всех игроков в комнате")
{
    checkTarget();
    RegList::Pointer rc(NEW);

    for(Character *rch = target->people; rch; rch = rch->next_in_room)
        if (!rch->is_npc())
            rc->push_back( WrapperManager::getThis( )->getWrapper( rch ) );
    
    return wrap(rc);
}

NMI_GET( RoomWrapper, sector_type , "значение типа местности (таблица .sector_table)")
{
    checkTarget( );
    return Register( target->getSectorType() );
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

NMI_GET( RoomWrapper, liquid, "жидкость комнаты (.Liquid)" )
{
    checkTarget();
    return LiquidWrapper::wrap(target->getLiquid()->getName());
}

NMI_GET( RoomWrapper, description, "описание комнаты" )
{
    checkTarget( );
    return Register( target->getDescription() );
}

NMI_GET( RoomWrapper, clan, "клан, которому принадлежит комната (структура .Clan)" )
{
    checkTarget();
    return ClanWrapper::wrap( target->pIndexData->clan->getName( ) );
}

NMI_GET( RoomWrapper, guilds, "гильдии в этой комнате" )
{
    checkTarget();
    if (!target->pIndexData->guilds.empty( ))
        return Register( target->pIndexData->guilds.toString().c_str() );
    else 
        return Register("");
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
NMI_INVOKE(RoomWrapper, saveItems, "(): сохраняет все предметы на полу в комнате на диск")
{
    checkTarget();
    save_mobs(target);
    return Register();
}

NMI_INVOKE(RoomWrapper, saveMobs, "(): сохраняет всех мобов в комнате на диск")
{
    checkTarget();
    save_items(target);
    return Register();
}

NMI_INVOKE(RoomWrapper, playersWithPosition, "(pos): список (List) всех игроков в комнате в определенном положении")
{
    checkTarget();
    RegList::Pointer rc(NEW);
    int pos = argnum2flag(args, 1, position_table);

    for(Character *rch = target->people; rch; rch = rch->next_in_room)
        if (!rch->is_npc() && rch->position == pos)
            rc->push_back( WrapperManager::getThis( )->getWrapper( rch ) );
    
    return wrap(rc);
}


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

NMI_INVOKE( RoomWrapper, getRoom, "(имя или номер выхода): комната по этому направлению или null" )
{
    checkTarget( );

    int door = get_door_argument( args );
    if (door == -1)
        return Register();

    return get_direction( target, door );
}

NMI_INVOKE( RoomWrapper, getRevDoor, "(имя или номер выхода): вернет номер противоположного направления" )
{
    checkTarget( );

    int door = get_door_argument(args);
    if (door == -1)
        return Register();

    return dirs[door].rev;
}

NMI_INVOKE( RoomWrapper, doorNumber, "(имя выхода): вернет номер выхода или -1" )
{
    checkTarget( );
    return get_door_argument( args );
}

NMI_INVOKE( RoomWrapper, doorName, "(номер выхода): вернет имя выхода или null" )
{
    checkTarget( );

    int door = get_door_argument( args );
    if (door == -1)
        return Register();

    return dirs[door].name;
}

Room *check_place(Character *ch, Room *start_room, const DLString &cArg)
{
    char arg[MAX_INPUT_LENGTH], argument[MAX_INPUT_LENGTH];
    EXIT_DATA *pExit;
    Room *dest_room;
    int number, door;
    int range = (ch->getModifyLevel() / 10) + 1;

    strcpy(argument, cArg.c_str());
    number = number_argument(argument, arg);
    if ((door = direction_lookup(arg)) < 0)
        return 0;

    dest_room = start_room;

    while (number > 0) {
        number--;

        if (--range < 1)
            return 0;

        if ((pExit = dest_room->exit[door]) == 0 
            || !ch->can_see(pExit) 
            || IS_SET(pExit->exit_info, EX_CLOSED)) 
        {
            break;
        }

        dest_room = pExit->u1.to_room;
        if (number < 1)
            return dest_room;
    }

    return 0;
}

NMI_INVOKE(RoomWrapper, roomAt, "(ch,arg): доступная ch комната по направлению, указанному в аргументе (2.n, 3.e и т.д.)")
{
    checkTarget();
    Character *ch = argnum2character(args, 1);
    DLString arg = argnum2string(args, 2);

    Room *dest = check_place(ch, target, arg);
    if (!dest)
        return Register();

    return wrap(dest);
}

NMI_INVOKE( RoomWrapper, dirMsgLeave, "(имя или номер выхода): сообщение при уходе через этот выход (на север, на восток)" )
{
    checkTarget( );

    int door = get_door_argument( args );
    if (door == -1)
        return Register();

    return dirs[door].leave;
}

NMI_INVOKE( RoomWrapper, dirMsgWhere, "(имя или номер выхода): где находится направление (на севере, внизу, на востоке)" )
{
    checkTarget( );

    int door = get_door_argument( args );
    if (door == -1)
        return Register();

    return dirs[door].where;
}

NMI_INVOKE( RoomWrapper, dirMsgEnter, "(имя или номер выхода): сообщение при заходе через этот выход (с юга, с запада)" )
{
    checkTarget( );

    int door = get_door_argument( args );
    if (door == -1)
        return Register();

    return dirs[door].enter;
}

NMI_INVOKE( RoomWrapper, getExitFlags, "(номер выхода, имя экстра/выхода): флаги этого выхода (таблица .tables.exit_flags)" )
{
    EXIT_DATA *pExit;
    EXTRA_EXIT_DATA *pExtraExit;
    
    checkTarget();
    if (!resolve_exits(args, target, pExit, pExtraExit))
        throw Scripting::IllegalArgumentException();

    return pExtraExit ? pExtraExit->exit_info : pExit->exit_info;
}

NMI_INVOKE( RoomWrapper, exitKeyword, "(номер выхода, имя экстра/выхода): ключевые слова, на которые откликается эта дверь или выход" )
{
    EXIT_DATA *pExit;
    EXTRA_EXIT_DATA *pExtraExit;
    
    checkTarget();
    if (!resolve_exits(args, target, pExit, pExtraExit))
        throw Scripting::IllegalArgumentException();

    return pExtraExit ? pExtraExit->keyword : pExit->keyword;
}

NMI_INVOKE( RoomWrapper, exitShortDescr, "(номер выхода, имя экстра/выхода): название выхода с падежами" )
{
    EXIT_DATA *pExit;
    EXTRA_EXIT_DATA *pExtraExit;
    
    checkTarget();
    if (!resolve_exits(args, target, pExit, pExtraExit))
        throw Scripting::IllegalArgumentException();

    DLString desc = pExtraExit ? pExtraExit->short_desc_from : pExit->short_descr;
    if (desc.empty())
        desc = "двер|ь|и|и|ь|ью|и";

    return desc;
}


NMI_INVOKE(RoomWrapper, close, "(номер выхода, имя экстра/выхода): закрыть дверь по указанному направлению")
{
    checkTarget( );
    update_door_flags( target, args, EX_CLOSED, true ); 
    return Register( ); 
}

NMI_INVOKE(RoomWrapper, open, "(номер выхода, имя экстра/выхода): открыть дверь по указанному направлению")
{
    checkTarget( );
    update_door_flags( target, args, EX_CLOSED|EX_LOCKED, false ); 
    return Register( ); 
}

NMI_INVOKE(RoomWrapper, lock, "(номер выхода, имя экстра/выхода): запереть дверь по указанному направлению")
{
    checkTarget( );
    update_door_flags( target, args, EX_CLOSED|EX_LOCKED, true ); 
    return Register( ); 
}

NMI_INVOKE(RoomWrapper, unlock, "(номер выхода, имя экстра/выхода): отпереть дверь по указанному направлению")
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

    for (Character *to = target->people; to; to = to->next_in_room) {
        to->pecho(POS_RESTING, regfmt(to, args).c_str());
    }

    return Register( );
}

NMI_INVOKE( RoomWrapper, echoAround, "(fmt, args): выводит отформатированную строку всем неспящим в прилегающие комнаты" )
{
    checkTarget( );

    for (auto room: RoomUtils::adjancentRooms(target)) {
        for (Character *to = room->people; to; to = to->next_in_room) {
            to->pecho(POS_RESTING, regfmt(to, args).c_str());
        }
    }

    return Register( );
}

NMI_INVOKE(RoomWrapper, zecho, "(msg): выведет сообщение msg для всех в этой арии" )
{
    Character *wch;
    DLString msg = args2string(args);
    
    checkTarget( );
    
    for (wch = char_list; wch; wch = wch->next) 
        if (wch->in_room->area == target->area) 
            wch->pecho(msg);

    return Register( );
}

NMI_INVOKE(RoomWrapper, get_obj_vnum, "(vnum[,owner]): поиск первого объекта в комнате по его внуму (с владельцем owner)" )
{
    checkTarget( );

    int vnum = argnum2number(args, 1);
    DLString owner;

    if (args.size() > 1)
        owner = argnum2string(args, 2);

    for (::Object *obj = target->contents; obj; obj = obj->next_content)
        if (obj->pIndexData->vnum == vnum)
            if (owner.empty() || owner == obj->getOwner())
                return wrap(obj); 

    return Register();
}

NMI_INVOKE(RoomWrapper, get_obj_type, "(type): поиск первого объекта в комнате по его типу (имя или номер из .tables.item_table)" )
{
    checkTarget( );

    int itemType = argnum2flag(args, 1, item_table);
    ::Object *obj = get_obj_room_type(target, itemType);
    if (!obj)
        return Register();
    return WrapperManager::getThis( )->getWrapper(obj); 
}

NMI_INVOKE(RoomWrapper, list_obj_type, "(type): поиск списка объектов в комнате по его типу (имя или номер из .tables.item_table)" )
{
    checkTarget( );

    int itemType = argnum2flag(args, 1, item_table);
    RegList::Pointer rc(NEW);

    for (::Object *obj = target->contents; obj; obj = obj->next_content)
        if (obj->item_type == itemType)
            rc->push_back( WrapperManager::getThis( )->getWrapper( obj ) );

    return wrap(rc);
}

NMI_INVOKE( RoomWrapper, list_obj_vnum, "(vnum): поиск списка объектов в комнате по внуму" )
{
    checkTarget( );
    RegList::Pointer rc(NEW);

    int vnum = args2number( args );

    for (::Object *obj = target->contents; obj; obj = obj->next_content)
        if (obj->pIndexData->vnum == vnum)
            rc->push_back( WrapperManager::getThis( )->getWrapper( obj ) );

    return wrap(rc);
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

    return wrap(rc);
}

NMI_INVOKE( RoomWrapper, count_enemies, "(ch): кол-во персонажей, сражающихся с ch")
{
    checkTarget( );
    Character *ch = args2character(args);
    int count = 0;

    for (Character *rch = target->people; rch; rch = rch->next_in_room)
    if (rch != ch && rch->fighting == ch)
        count++;

    return Register(count);
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
        bitstring_t mysector = (1 << toRoom->getSectorType());

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

    return wrap(rooms);
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

    return wrap(rooms);
}

NMI_GET( RoomWrapper, resetMobiles, "список внумов мобов, которые ресетятся в этой комнате") 
{
    RegList::Pointer rc(NEW);
    
    checkTarget( );
    
    for (auto &pReset: target->pIndexData->resets)
        if (pReset->command == 'M')
            rc->push_back( Register( pReset->arg1 ) );

    return wrap(rc);
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

NMI_INVOKE(RoomWrapper, isWater, "(): является ли эта комната водной или подводной")
{
    checkTarget();
    return RoomUtils::isWater(target);
}

NMI_INVOKE(RoomWrapper, isOutside, "(): находится ли комната снаружи помещения")
{
    checkTarget();
    return RoomUtils::isOutside(target);
}

NMI_INVOKE(RoomWrapper, hasWaterParticles, "(): достаточно ли водяных паров в комнате")
{
    checkTarget();
    return RoomUtils::hasWaterParticles(target);
}

NMI_INVOKE(RoomWrapper, isNature, "(): является ли комната дикой местностью")
{
    checkTarget();
    return RoomUtils::isNature(target);
}

NMI_INVOKE(RoomWrapper, hasDust, "(): достаточно ли пыли или песка в комнате")
{
    checkTarget();
    return RoomUtils::hasDust(target);
}

NMI_INVOKE(RoomWrapper, hasParticles, "(): достаточно ли разных частиц в комнате")
{
    checkTarget();
    return RoomUtils::hasParticles(target);
}

NMI_INVOKE( RoomWrapper, isAffected, "(skill): находится ли комната под воздействием аффекта с данным именем" )
{
    Skill *skill = args2skill(args);
    
    checkTarget( );

    if (skill)
        return target->isAffected( skill->getIndex( ) );
    else
        return false;
}

NMI_GET( RoomWrapper, affected, "список (List) всех аффектов на комнате (структура .Affect)" )
{
    checkTarget();
    RegList::Pointer rc(NEW);

    for (auto &paf: target->affected) 
        rc->push_back( AffectWrapper::wrap( *paf ) );
        
    return wrap(rc);
}

NMI_INVOKE( RoomWrapper, affectAdd, "(aff): повесить новый аффект (.Affect)" )
{
    checkTarget( );
    AffectWrapper *aw;
    
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
    
    aw = wrapper_cast<AffectWrapper>( args.front( ) );
    target->affectTo(&(aw->getTarget()));
    return Register( );
}

NMI_INVOKE( RoomWrapper, affectJoin, "(aff): усилить существующий аффект или повесить новый (.Affect)" )
{
    checkTarget( );
    AffectWrapper *aw;
    
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
    
    aw = wrapper_cast<AffectWrapper>( args.front( ) );
    target->affectJoin(&(aw->getTarget()));
    return Register( );
}

NMI_INVOKE( RoomWrapper, affectStrip, "(skill[,verbose]): снять с комнаты все аффекты от умения по имени skill, показав сообщение о спадании (verbose)" )
{
    checkTarget( );
    Skill *skill = argnum2skill(args, 1);
    bool verbose = args.size() > 1 ? argnum2boolean(args, 2) : false;

    target->affectStrip(skill->getIndex(), verbose);
    return Register( );
}

NMI_INVOKE( RoomWrapper, affectReplace, "(.Affect): удалить все аффекты этого типа и повесить новый" )
{
    checkTarget( );
    AffectWrapper *aw;

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    aw = wrapper_cast<AffectWrapper>( args.front( ) );        
    target->affectStrip(aw->getTarget().type);
    target->affectJoin( &(aw->getTarget()) );
    return Register( );
}

NMI_INVOKE( RoomWrapper, affectFind, "(skill,ch): найти аффект с данным именем и владельцем-персонажем" )
{
    Skill *skill = argnum2skill(args, 1);
    Character *owner = argnum2character(args, 2);
    
    checkTarget();

    for (auto &paf: target->affected) 
        if (paf->type.getElement() == skill && paf->sources.getOwner() == owner)
            return AffectWrapper::wrap(*paf);

    return Register();
}

NMI_GET(RoomWrapper, behaviors, "список имен всех поведений")
{
    checkTarget();

    RegList::Pointer rc(NEW);

    for (auto &b: target->pIndexData->behaviors.toSet()) 
        rc->push_back(
            Register(
                behaviorManager->find(b)->getName()));

    return ::wrap(rc);
}

NMI_SET(RoomWrapper, behaviors, "список имен всех поведений")
{
    checkTarget();
    arg2globalBitvector<Behavior>(arg, target->pIndexData->behaviors);
    target->pIndexData->areaIndex->changed = true;
}

NMI_INVOKE(RoomWrapper, hasBehavior, "(bhvName): true если среди поведений комнаты есть указанное")
{
    checkTarget();

    DLString bhvName = args2string(args);
    Behavior *bhv = behaviorManager->findExisting(bhvName);
    if (!bhv)
        throw IllegalArgumentException();

    return Register(target->pIndexData->behaviors.isSet(bhv->getIndex()));
}

NMI_GET(RoomWrapper, props, "Map (структура) из свойств поведения, ключ - имя поведения") 
{
    checkTarget();
    return JsonUtils::toRegister(target->pIndexData->props);
}

NMI_INVOKE(RoomWrapper, setProperty, "(key,subkey,value): установить значение props[key][subkey] в value")
{
    checkTarget();
    DLString key = argnum2string(args, 1);
    DLString subkey = argnum2string(args, 2);
    DLString value = argnum2string(args, 3);

    if (value.isNumber()) {
        target->pIndexData->props[key][subkey] = value.toInt();
    } else {
        target->pIndexData->props[key][subkey] = value;
    }

    target->area->pIndexData->changed = true;

    return Register();
}


