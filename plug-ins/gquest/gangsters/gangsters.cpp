/* $Id: gangsters.cpp,v 1.1.2.3.6.7 2009/09/24 14:09:12 rufina Exp $
 * 
 * ruffina, 2003
 */

#include "dlscheduler.h"
#include "race.h"
#include "pcrace.h"
#include "room.h"
#include "object.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "pcharactermanager.h"

#include "globalquestmanager.h"
#include "gqchannel.h"
#include "gqexceptions.h"
#include "xmlattributereward.h"

#include "gangsters.h"
#include "gangstersinfo.h"
#include "objects.h"
#include "xmlattributegangsters.h"
#include "gangchef.h"
#include "gangmob.h"

#include "act_move.h"
#include "act.h"
#include "handler.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"


Gangsters* Gangsters::thisClass = NULL;

Gangsters::Gangsters( )
{
}

Gangsters::Gangsters( const DLString& id ) : GlobalQuest( id )
{
    thisClass = this;
}

Gangsters::~Gangsters( )
{
    thisClass = NULL;
}

void Gangsters::create( const Config& ) throw ( GQCannotStartException ) 
{
    AREA_DATA *area;
    AreaList areaList;
    
    for (area = area_first; area; area = area->next) {
	if (area->low_range <= minLevel 
	    && !IS_SET(area->area_flag, AREA_WIZLOCK|AREA_HOMETOWN|AREA_HIDDEN
	                                |AREA_NOQUEST|AREA_NOGATE) ) 
	{
	    areaList.push_back(area);
	} 
    }
    
    while (!areaList.empty( )) {
	Room *room;
	int msize;
	int areaIndex;
	MobileList people;
	RoomList mobRooms, portalRooms;

	areaIndex = number_range(0, areaList.size( ) - 1);
	area = areaList[ areaIndex ];
	areaList.erase( areaList.begin( ) + areaIndex );
	
	mobRoomVnums.clear( );
	portalRoomVnums.clear( );
	
	for (room = room_list; room; room = room->rnext) {
	    if (room->area != area) 
		continue;
		
	    if (checkRoom( room )) {
		mobRooms.push_back( room );
		mobRoomVnums.push_back( room->vnum );

		for (Character *ch = room->people; ch; ch = ch->next_in_room) {
		    if (!getActor( ch )->is_npc( ))
			continue;

		    if (ch->getNPC( )->pIndexData->area != area)
			continue;

		    if (ch->getRace( )->isPC( ))
			people.push_back( ch->getNPC( ) );
		}
	    }

	    if (GangPortal::canDrop( room ))
		portalRooms.push_back( room );
	}
	
	try {
	    msize = mobRooms.size( );
	    
	    if (!people.empty( ) && !portalRooms.empty( ) && msize > 10) {
		int numPortal, number;

		numPortal = number = (msize > 100 ? 3 : (msize > 40 ? 2 : 1));
		    
		while (!portalRooms.empty( ) && number > 0) 
		    if (createPortal( portalRooms ))
			number--;

		if (number == 0) {
		    log("good area: " << area->name);
		    populateArea( area, mobRooms, numPortal );
		    createFirstHint( people );
		    populateLair( );
		    return;
		}
	    }	
	}
	catch (const Exception& e) {
	    cleanup( false );
	    throw e;
	}
	
	cleanup( false );

    } 
    
    
    throw GQCannotStartException(minLevel, maxLevel);
}

void Gangsters::cleanup( bool performance )
{
    Character *ch, *ch_next;
    Object *obj, *obj_next;
    
    for (obj = object_list; obj; obj = obj_next) {
	obj_next = obj->next;

	if (!obj->behavior)
	    continue;
	
	if (obj->behavior.getDynamicPointer<GangPortal>( ))
	    extract_obj( obj );
	else {
	    GangKey::Pointer behavior = obj->behavior.getDynamicPointer<GangKey>( );

	    if (behavior) {
		behavior->needsReset = false;
		extract_obj( obj );
	    }
	}
    }
    
    if (!mobRoomVnums.empty( ))
	REMOVE_BIT(get_room_index( mobRoomVnums.front( ) )->area->area_flag, AREA_NOGATE);
    
    for (ch = char_list; ch; ch = ch_next) {
	ch_next = ch->next;
	
	if (!ch->is_npc() || !ch->getNPC()->behavior || !ch->getNPC()->behavior.getDynamicPointer<GangMob>( ))
	    continue;
	    
	if (performance) {
	    if (ch->position >= POS_RESTING) 
		act_p("$c1 произносит '{gHasta la vista, baby!{x'", ch, 0, 0, TO_ROOM, POS_RESTING);
	    
	    if (ch->position >= POS_MORTAL)	
		act_p("$c1 исчезает в клубе дыма.", ch, 0, 0, TO_ROOM, POS_RESTING);
	}
	
	extract_char( ch );
    }
    
    wipeRoom( get_room_index( GangstersInfo::getThis( )->vnumLair ) );
}

void Gangsters::destroy( ) 
{
    cleanup( true );
    
    switch (state.getValue( )) {
    case ST_CHEF_KILLED:
	rewardChefKiller( );
	break;
    case ST_BROKEN:
	rewardNobody( );
	break;
    default:
	rewardLeader( );
	break;
    }
}

class GangKeysResetTask: public SchedulerTask {
public:
    typedef ::Pointer<GangKeysResetTask> Pointer;
    
    GangKeysResetTask( Gangsters::Pointer gq ) : gquest( gq ) 
    {
    }
    virtual void run( )
    {
	gquest->resetKeys( );
    }
    virtual int getPriority( ) const
    {
	int prio = DLScheduler::getThis( )->getPriority( );

	prio = max( prio, (int)SCDP_INITIAL ) + 100;
	return prio;
    }

private:
    Gangsters::Pointer gquest;
};

void Gangsters::resume( )
{
    int rtime = getTaskTime( );
    
    if (rtime <= 0) 
	scheduleDestroy( );
    else {
	DLScheduler::getThis( )->putTaskInSecond( rtime * 60, Gangsters::Pointer( this ) );
	GlobalQuestManager::getThis( )->activate( this );
	DLScheduler::getThis( )->putTaskNOW( GangKeysResetTask::Pointer( NEW, this ) );
    }
}

void Gangsters::after( )
{
    if (state != ST_NONE && state != ST_NO_MORE_HINTS)
	return;

    switch (hintCount++) {
    case 0:
	if (state == ST_NONE)
	    if (!createSecondHint( ))
		createThirdHint( );
	
	break;
	
    case 1:
	if (state == ST_NONE)
	    createThirdHint( );
	
	break;

    default:
	scheduleDestroy( );
	return;
    }
    
    GlobalQuest::after( );
}

int Gangsters::getTaskTime( ) const
{
    int r = getRemainedTime( ) / (3 - hintCount.getValue( ));
    log("::getTaskTime: remained " << getRemainedTime( ) << ", task " << r);
    return r;
}

void Gangsters::report( std::ostringstream &buf, PCharacter *ch ) const
{
    if (isLevelOK( ch )) {
	XMLAttributeGangsters::Pointer attr;
	
	attr = ch->getAttributes( ).findAttr<XMLAttributeGangsters>( getQuestID( ) );
	
	if (attr && attr->getKilled( ) > 0)
	    buf << "Число убитых тобой преступников: " 
		<< GQChannel::BOLD <<  attr->getKilled( ) << GQChannel::NORMAL << endl;
	
	buf << "До конца охоты остается ";
	printRemainedTime( buf );
	buf << "." << endl;
    }
}

void Gangsters::progress( std::ostringstream &buf ) const
{
    PCharacterMemoryList::const_iterator i;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );

    for (i = pcm.begin( ); i != pcm.end( ); i++) {
	XMLAttributeGangsters::Pointer attr; 
	
	attr = i->second->getAttributes( ).findAttr<XMLAttributeGangsters>( getQuestID( ) );
	
	if (!attr || attr->getKilled( ) <= 0)
	    continue;
	
	buf << GQChannel::NORMAL
	    << dlprintf( "%-15s", i->second->getName( ).c_str( ) ) << " "
	    << GQChannel::BOLD << dlprintf( "%-4d", attr->getKilled( ) )
	    << GQChannel::NORMAL << endl;
    }
}

void Gangsters::getQuestDescription( std::ostringstream &buf ) const
{
    getQuestStartMessage( buf );
    buf << endl	<< getHint( ) << endl;
}

void Gangsters::getQuestStartMessage( std::ostringstream &buf ) const
{
    buf << "Шайка преступников атаковала мирных жителей. "
	<< "Ищутся храбрецы " 
	<< GQChannel::BOLD << minLevel << "-" << maxLevel << GQChannel::NORMAL
	<< " уровней для уничтожения бандитов и их главаря.";
}

/*****************************************************************************/

/*
 *  rewards
 */

void Gangsters::rewardLeader( ) 
{
    std::basic_ostringstream<char> buf;
    PCharacterMemoryList::const_iterator i;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
    std::list<PCMemoryInterface *> leaders;
    int max = 0, killed;

    for (i = pcm.begin( ); i != pcm.end( ); i++) {
	XMLAttributeGangsters::Pointer attr;
	
	attr = i->second->getAttributes( ).findAttr<XMLAttributeGangsters>( getQuestID( ) );
	
	if (!attr) 
	    continue;

	killed = attr->getKilled( );
	if (killed && killed > max) {
	    max = killed;
	    leaders.clear( );
	    leaders.push_back( i->second );
	} else if (killed && killed == max)
	    leaders.push_back( i->second );
    }
    
    buf << "Главаря шайки так никто и не убил.";
    GQChannel::gecho( this, buf );

    if (leaders.empty( )) {
	buf << "Более того, ни один бандит не пострадал."  << endl;
    }
    else { 
	XMLReward reward;
	
	reward.qpoints = max * number_range( 10, 15 ) + number_fuzzy( 10 );
	reward.gold = max * number_range( 10, 15 );
	reward.experience = max * number_fuzzy( 50 );
	reward.reason = DLString( "За убийство самого большого количества бандитов ты получаешь: " );
	reward.id = getQuestID( );
	
	if (leaders.size( ) == 1)
	    buf << "Самый лучший охотник за бандитами:" << GQChannel::BOLD;
	else
	    buf << "Самые успешные охотники за бандитами:" << GQChannel::BOLD;
	
	while (!leaders.empty( )) {
	    PCMemoryInterface * pci;

	    pci = leaders.back( );
	    leaders.pop_back( );

	    buf << " " << pci->getName( );
	    if (!leaders.empty( ))
		buf << ",";

	    log("reward leader " << pci->getName( ));
	    GlobalQuestManager::getThis( )->rewardChar( pci, reward );
	}
    }

    GQChannel::gecho( this, buf );
}

void Gangsters::rewardChefKiller( ) 
{
    std::basic_ostringstream<char> buf;
    XMLReward r;
    PCMemoryInterface *pci = PCharacterManager::find( chefKiller );
    
    r.gold = number_range( getMaxLevel( ), 2 * getMaxLevel( ) );
    r.qpoints = number_range( 200, 250 );
    r.experience = number_range( 300, 500 );
    r.practice = number_range( -6, 3 );
    r.reason = DLString( "Поздравляем! Шеф убит и все бандиты разбежались. В награду ты получаешь: " );
    r.id = getQuestID( );

    GlobalQuestManager::getThis( )->rewardChar( pci, r );

    buf << GQChannel::BOLD << pci->getName( ) << GQChannel::NORMAL 
	<< " уничтожил" << GET_SEX(pci, "", "о", "а") <<" главаря шайки!";

    GQChannel::gecho( this, buf );
    
    pci->getAttributes( ).getAttr<XMLAttributeGlobalQuest>( "gquest" )
		    ->rememberVictory( getQuestID( ) );
}

void Gangsters::rewardNobody( ) 
{
    GQChannel::gecho( this, "Шефа банды убила противоборствующая группировка.");
}

void Gangsters::rewardMobKiller( PCharacter *killer, Character *mob )
{
    XMLReward r;
    int diff = mob->getRealLevel( ) - killer->getModifyLevel( );

    r.experience = number_range( 10, 30 );
    r.qpoints = number_range( diff, 8 );
    r.gold = number_range( diff, 8 );
    r.reason = "Твоя награда за уничтожение преступника составляет: ";
    r.id = getQuestID( );
    GlobalQuestManager::getThis( )->rewardChar( killer, r );	

    XMLAttributeGangsters::Pointer attr = killer->getAttributes( ).getAttr<XMLAttributeGangsters>( getQuestID( ) );
    attr->setKilled( attr->getKilled( ) + 1 );
    
    if (state == ST_NONE)
	state = ST_NO_MORE_HINTS;
}

/*****************************************************************************/

/*
 * hint messages
 */
void Gangsters::createFirstHint( MobileList &people )
{
    std::basic_ostringstream<char> buf;
    NPCharacter *informer, *mob;
    DLString name;
    
    informer = people[ number_range( 0, people.size( ) - 1 ) ];
    mob = createMob( );
    char_to_room( mob, informer->in_room );
   
    name = informer->getNameP('1');
    name.upperFirstCharacter( );
    informerName = name;
    informerRoom = informer->in_room->name;

    buf << name	<< " сообщил" << GET_SEX( informer, "", "о", "а" );
    
    switch (number_range(1, 4)) {
    case 1: case 2: buf << " Хассану"; break;
    case 3: case 4: buf << " Валькирии"; break;
    }
    
    buf << ", что видел" << GET_SEX( informer, "", "о", "а" )
	<< " бандитов возле местности под названием " << informer->in_room->name << ". ";
    setHint( buf.str( ) );
}	    

Room * Gangsters::findHintRoom( std::ostringstream &buf )
{
    Room *room = NULL;

    for (unsigned int i = 0; i < mobRoomVnums.size( ); i++) {
	room = get_room_index( mobRoomVnums[i] );
	
	if (informerRoom.getValue( ) == room->name)
	    continue;

	for (Character *ch = room->people; ch; ch = ch->next_in_room) {
	    DLString name;

	    if (!getActor( ch )->is_npc( ))
		continue;

	    if (ch->getNPC()->pIndexData->area != room->area) 
		continue;
	    
	    if (!ch->getRace( )->isPC( ))
		continue;
	    
	    name = ch->getNameP('1');
	    if (name == informerName.getValue( ))
		continue;

	    /* from the same area but not informer */
	    
	    name.upperFirstCharacter( );
	    buf	<< name << " столкнул" << GET_SEX( ch, "ся", "ось", "ась" )
		<< " с гангстерами возле " << room->name << ".";

	    return room;
	}
    }
    
    /* cannot find mob, give hint only about a room they're in */
    if (room) 
	buf << "Гангстеры были также замечены неподалеку от " 
	    << room->name << ".";
    
    return room;
}

bool Gangsters::createSecondHint( )
{
    std::basic_ostringstream<char> buf;
    Room *room = findHintRoom( buf );

    if (room) {
	GQChannel::gecho( this, buf );
	
	setHint( getHint( ) + " " + buf.str( ) );
	char_to_room( createMob( ), room );
    }
    
    return (room != NULL);
}

void Gangsters::createThirdHint( )
{
    std::basic_ostringstream<char> buf;

    buf << "Больше всего от руки бандитов пострадала местность " << areaName << ".";
    setHint( buf.str( ) );
    GQChannel::gecho( this, buf );

    state = ST_NO_MORE_HINTS;
}

/*****************************************************************************/


NPCharacter * Gangsters::createMob( )
{
    NPCharacter *ch;
    MOB_INDEX_DATA *pMobIndex;
    GangMember::Pointer behavior( NEW );
    int vnum = GangstersInfo::getThis( )->vnumMob;
    
    if (!(pMobIndex = get_mob_index( vnum )))
	throw MobileNotFoundException( vnum );
	
    ch = create_mobile( pMobIndex );
    behavior->setChar( ch );
    behavior->config( number_range( minLevel, maxLevel ) );
    ch->behavior.setPointer( *behavior );
    
    return ch;
}

NPCharacter * Gangsters::createChef( )
{
    NPCharacter *ch;
    MOB_INDEX_DATA *pMobIndex;
    GangChef::Pointer behavior( NEW );
    int vnum = GangstersInfo::getThis( )->vnumChef;
    
    if (!(pMobIndex = get_mob_index( vnum )))
	throw MobileNotFoundException( vnum );
	
    ch = create_mobile( pMobIndex );
    behavior->setChar( ch );
    behavior->config( maxLevel );
    ch->behavior.setPointer( *behavior );

    return ch;
}

Object * Gangsters::createKey( )
{
    Object *key;
    OBJ_INDEX_DATA *pObjIndex;
    GangKey::Pointer behavior( NEW );
    int vnum = GangstersInfo::getThis( )->vnumKey;
    
    if (!(pObjIndex = get_obj_index( vnum )))
	throw ObjectNotFoundException( vnum );
	
    key = create_object( pObjIndex, 0 );
    behavior->setObj( key );
    key->behavior.setPointer( *behavior );

    return key;
}

void Gangsters::resetKeys( )
{
    std::vector<NPCharacter *> mobiles;
    Character *ch;
    NPCharacter *mob;
    Object *obj;
    int keyCnt = 0;
   
    if (!keyCount)
	return;

    for (obj = object_list; obj; obj = obj->next) 
	if (obj->behavior && obj->behavior.getDynamicPointer<GangKey>( ))
	    keyCnt++;
    
    if (keyCnt >= keyCount)
	return;

    for (ch = char_list; ch; ch = ch->next) {
	if (!ch->is_npc( ))
	    continue;

	mob = ch->getNPC( );
	
	if (mob->in_room->vnum == GangstersInfo::getThis( )->vnumLair)
	    continue;

	if (!mob->behavior || !mob->behavior.getDynamicPointer<GangMob>( ))
	    continue;
	    
	for (obj = mob->carrying; obj; obj = obj->next_content) 
	    if (obj->pIndexData->vnum == GangstersInfo::getThis( )->vnumKey)
		break;

	if (obj)
	    continue;
	
	mobiles.push_back( mob );
    }
    
    while (keyCnt++ < keyCount) {
	if (mobiles.empty( )) {
	    mob = createMob( );
	    char_to_room( mob, pickRandomRoom( ) );
	}
	else {
	    int i = number_range( 0, mobiles.size( ) - 1 );
	    
	    mob = mobiles[i];
	    mobiles.erase( mobiles.begin( ) + i );
	}
	
	obj_to_char( createKey( ), mob );
	log("new key to mob in room " << mob->in_room->name ); 
    }
}

Object * Gangsters::createPortal( RoomList &portalRooms ) 
{
    int i; 
    Room *room;
    Object *portal = NULL;
    
    i = number_range(0, portalRooms.size( ) - 1);
    room = portalRooms[i]; 
    portalRooms.erase( portalRooms.begin( ) + i );
    
    switch (room->sector_type) {
    case SECT_FOREST:
    case SECT_FIELD:
    case SECT_DESERT:
    case SECT_HILLS:
    case SECT_MOUNTAIN:
	portal = create_object( get_obj_index( GangstersInfo::getThis( )->vnumPortalForest ), 0 );
	break;
    case SECT_CITY:
    case SECT_UNUSED:
    default:
	portal = create_object( get_obj_index( GangstersInfo::getThis( )->vnumPortalCity ), 0 );
	break;
    }
    
    if (!portal->behavior)
	throw BadObjectBehaviorException( portal->pIndexData->vnum );

    SET_BIT( portal->value[1], EX_ISDOOR|EX_CLOSED|EX_LOCKED|EX_NOPASS|EX_PICKPROOF );
    portal->value[4] = GangstersInfo::getThis( )->vnumKey;

    obj_to_room( portal, room );
    portalRoomVnums.push_back( room->vnum );
    log("put portal in " << room->name);

    return portal;
}

Room * Gangsters::recursiveWalk( Room *room, int depth, int maxDepth ) 
{
    Room *pRoom; 
    int j, i;
    Room * targets [DIR_SOMEWHERE];

    if (depth >= maxDepth) 
	return room;
    
    for (i = 0, j = 0; i < DIR_SOMEWHERE; i++) {
	EXIT_DATA *door;
	if (!room->exit[i])
	    continue;
	
	pRoom = room->exit[i]->u1.to_room;
	if (!pRoom)
	    continue;

	if (IS_SET(pRoom->room_flags, ROOM_MARKER))
	    continue;
	
	door = pRoom->exit[dirs[i].rev];
	if (!door || door->u1.to_room != room)
	    continue;

	targets[j++] = pRoom;
    }

    for (i = 0; i < j; i++) {
	int i0 = number_mm( ) % j;
	pRoom = targets[i];
	targets[i] = targets[i0];
	targets[i0] = pRoom;
    }
	
    SET_BIT(room->room_flags, ROOM_MARKER);
    pRoom = NULL;
    
    for (i = 0; i < j; i++) {
	pRoom = recursiveWalk( targets[i], depth + 1, maxDepth );
	if (pRoom) 
	    break;	    
    }
    
    REMOVE_BIT(room->room_flags, ROOM_MARKER);
    return pRoom;
}

Room * Gangsters::pickRandomRoom( )
{
    int i = number_range( 0, mobRoomVnums.size( ) - 1 );
    return get_room_index( mobRoomVnums[i] );
}

DLString Gangsters::lairHint( ) 
{
    if (!portalRoomVnums.empty( )) {
	int i = number_range(0, portalRoomVnums.size( ) - 1);
	int vnum = portalRoomVnums[i];
	Room *room = get_room_index( vnum );

	if (room && (room = recursiveWalk( room, 0, number_range( 1, 2 ) )))
	    return room->name;
    }
    
    return "";
}

void Gangsters::populateArea( AREA_DATA *area, RoomList &mobRooms, int numPortal )
{
    int number;
    
    areaName = area->name;
    SET_BIT( area->area_flag, AREA_NOGATE );
    
    number = number_fuzzy( mobRooms.size( ) / 5 );

    for (int j = 0; j <= number; j++) {
	Object *key;
	Character *mob;
	
	mob = createMob( );
	char_to_room( mob, mobRooms[number_range( 0, mobRooms.size( ) - 1 )] );
	
	if (numPortal-- > 0) {
	    key = createKey( );
	    keyCount++;
	    obj_to_char( key, mob );
	    log("key to mob in room " << mob->in_room->name ); 
	}
    }
}

void Gangsters::populateLair( )
{
    Room *lair;
    int number;

    lair = get_room_index( GangstersInfo::getThis( )->vnumLair );
    wipeRoom( lair );
    char_to_room( createChef( ), lair );

    number = number_range( 2, 3 );

    while (number-- > 0) 
	char_to_room( createMob( ), lair );
}
    
bool Gangsters::isPoliceman( Character *ch ) 
{
    NPCharacter *mob;
    char *name;
    
    if (!ch->is_npc( ))
	return false;
    
    mob = ch->getNPC();

    if (IS_SET( mob->off_flags, ASSIST_GUARD ) ||
	 mob->spec_fun.name == "spec_guard" || 
	 mob->spec_fun.name == "spec_patrolman")
	return true;
    
    name = mob->pIndexData->player_name;

    if (is_name("guard", name) || is_name("guardian", name) ||
	is_name("shiriff", name) || is_name("bodyguard", name) ||
	is_name("cityguard", name) || is_name("стражник", name)  ||
	is_name("шериф", name)  || is_name("охранник", name)  ||
	is_name("телохранитель", name))
	return true;

    return false;
}

bool Gangsters::checkRoom( Room *const pRoomIndex )
{
    if (pRoomIndex->sector_type == SECT_AIR || IS_WATER(pRoomIndex))
	return false;
    
    if (IS_SET(pRoomIndex->room_flags, ROOM_SAFE|ROOM_NO_QUEST|ROOM_NO_MOB))
	return false;
	
    if (!pRoomIndex->isCommon( ))
	return false;

    return true;
}

    
