/* $Id: scenarios.cpp,v 1.1.2.3.6.4 2009/02/15 01:43:52 rufina Exp $
 * 
 * ruffina, 2004
 * some scenarios by Miyamoto, Cotton
 */

#include <map>

#include "scenarios.h"

#include "affect.h"
#include "room.h"
#include "npcharacter.h"
#include "pcharacter.h"

#include "mercdb.h"
#include "act.h"
#include "merc.h"
#include "descriptor.h"
#include "def.h"


/*--------------------------------------------------------------------------
 * Invasion Scenario 
 *-------------------------------------------------------------------------*/
bool InvasionScenario::canStart( )
{
    return true;
}

bool InvasionScenario::checkRoom( Room *room )
{
    return !IS_SET(room->room_flags, ROOM_NO_QUEST|ROOM_MANSION|ROOM_NO_MOB|ROOM_NO_DAMAGE|ROOM_SAFE|ROOM_SOLITARY)
        && !IS_SET(room->room_flags, ROOM_NEWBIES_ONLY|ROOM_GODS_ONLY|ROOM_NOWHERE);
}

bool InvasionScenario::checkArea( AREA_DATA *area )
{
    if (IS_SET(area->area_flag, AREA_WIZLOCK|AREA_NOQUEST|AREA_HIDDEN))
	return false;
	
    if (area->low_range > 20)
	return false;

    return true;
}


/*--------------------------------------------------------------------------
 * Sparse Scenario 
 *-------------------------------------------------------------------------*/
void InvasionSparseScenario::collectRooms( vector<Room *>& rooms, int mobCnt )
{
    Room *room;
    
    for (room = room_list; room; room = room->rnext) {
	if (!checkArea( room->area ))
	    continue;
	
	if (!checkRoom( room ))
	    continue;
	
	rooms.push_back( room );
    }
}   

/*--------------------------------------------------------------------------
 * Dense Scenario 
 *-------------------------------------------------------------------------*/
void InvasionDenseScenario::collectRooms( vector<Room *>& rooms, int mobCnt )
{
    Room *room;
    typedef map<AREA_DATA *, vector<Room *> > RoomsByArea;
    RoomsByArea goodRooms;
    int areaCnt;
    
    for (room = room_list; room; room = room->rnext) {
	if (!checkArea( room->area )) 
	    continue;
	    
	if (!checkRoom( room ))
	    continue;
	
	goodRooms[room->area].push_back( room );
    }

    areaCnt = std::max( 3, number_range( mobCnt / 20, mobCnt / 7 ) );
    
    while (!goodRooms.empty( ) && areaCnt > 0) {
	unsigned int j;
	RoomsByArea::iterator it;
	int i = number_range( 0, goodRooms.size( ) - 1 );
	
	for (it = goodRooms.begin( ); i > 0 && it != goodRooms.end( ); it++, i--) 
	    ;
	
	if ((int)it->second.size( ) >= mobCnt / areaCnt) {
	    for (j = 0; j < it->second.size( ); j++)
		rooms.push_back( it->second[j] );

	    areaCnt--;
	}
	
	goodRooms.erase( it );
    }
}

/*--------------------------------------------------------------------------
 * Locust Scenario 
 *-------------------------------------------------------------------------*/
bool InvasionLocustScenario::checkRoom( Room *room )
{
    if (IS_SET(room->room_flags, ROOM_INDOORS))
	return false;
	
    switch (room->sector_type) {
    case SECT_FIELD: case SECT_FOREST: case SECT_HILLS: case SECT_MOUNTAIN:
	return InvasionScenario::checkRoom( room );
    default:
	return false;
    }
}

/*--------------------------------------------------------------------------
 * Bubbles Scenario 
 *-------------------------------------------------------------------------*/
void InvasionBubblesMob::actDeath( Character *killer )
{
    Descriptor *d;
    DLString s;
    char buf[256];

    s = ch->getNPC( )->getShortDescr( );
    
    if (s.length( ) >= 2 && s.at( 0 ) == '{')
	sprintf(buf, "{%c", s.at( 1 ));
    else
	sprintf(buf, "{x");
   
    strcat(buf+strlen(buf), "(*)!(*)!(*) ЧПОК !!! (*)!(*)!(*){x\r\n");
    
    for (d = descriptor_list; d != 0; d = d->next) 
	if (d->connected == CON_PLAYING && d->character)
	    d->character->send_to(buf);
}

/*--------------------------------------------------------------------------
 * Football Scenario 
 *-------------------------------------------------------------------------*/
bool InvasionFootballScenario::checkRoom( Room *room )
{
    if (IS_SET(room->room_flags, ROOM_INDOORS))
	return false;
	
    switch (room->sector_type) {
    case SECT_FIELD: case SECT_HILLS: case SECT_MOUNTAIN: 
	return InvasionScenario::checkRoom( room );
    default:
	return false;
    }
}

