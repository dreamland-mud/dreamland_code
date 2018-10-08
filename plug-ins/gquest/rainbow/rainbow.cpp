/* $Id: rainbow.cpp,v 1.1.2.3.6.2 2007/09/11 00:34:11 rufina Exp $
 * 
 * ruffina, 2004
 */

#include <vector>
#include <list>
#include <map>

#include "rainbow.h"
#include "rainbowinfo.h"
#include "mobiles.h"
#include "objects.h"
#include "scenarios.h"

#include "globalquestmanager.h"
#include "gqchannel.h"
#include "gqexceptions.h"
#include "xmlattributereward.h"
#include "xmlattributeglobalquest.h"

#include "class.h"

#include "mobilebehaviormanager.h"
#include "room.h"
#include "object.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "pcharactermanager.h"

#include "act.h"
#include "mercdb.h"
#include "handler.h"
#include "save.h"
#include "merc.h"
#include "descriptor.h"
#include "vnum.h"
#include "def.h"

RainbowGQuest* RainbowGQuest::thisClass = NULL;

RainbowGQuest::RainbowGQuest( )
{
}

RainbowGQuest::RainbowGQuest( const DLString& id ) : GlobalQuest( id )
{
    thisClass = this;
}

RainbowGQuest::~RainbowGQuest( )
{
    thisClass = NULL;
}

void RainbowGQuest::create( const Config& config ) throw ( GQCannotStartException ) 
{
    Character *ch;
    NPCharacter *mob;
    vector<NPCharacter *> mobiles;
    list<NPCharacter *> bringers;
    unsigned int total;
    RainbowGQuestInfo::Scenarios::iterator iter;
    RainbowScenario *scenario;

    if (config.arguments.empty( )) {
        iter = RainbowGQuestInfo::getThis( )->getRandomScenariosEntry( );
        scenName = iter->first;
        scenario = *(iter->second);
    } else {
        scenName = config.arguments;
        scenario = RainbowGQuestInfo::getThis( )->findScenario( scenName );
        if (scenario == NULL)
            throw GQCannotStartException( "Scenario not found" );
    }

    if (!config.force)
        scenario->canStart( );


    LogStream::sendNotice( ) << "Trying to start rainbow, scenario " << scenName << endl;
    for (ch = char_list; ch; ch = ch->next) {
	if (!ch->is_npc( ))
	    continue;
	
	mob = ch->getNPC( );
	
	if (scenario->checkArea( mob->in_room->area )
	    && scenario->checkRoom( mob->in_room )
	    && scenario->checkMobile( mob ))
	{
	    mobiles.push_back( mob );
	}
    }
    
    LogStream::sendNotice( ) << "Found " << mobiles.size( ) << " candidates." << endl;
    total = scenario->getPiecesCount( );

    try {
	while (!mobiles.empty( ) && bringers.size( ) < total) {
	    list<NPCharacter *>::iterator j;
	    int i = number_range( 0, mobiles.size( ) - 1 );

	    mob = mobiles[i];
	    mobiles.erase( mobiles.begin( ) + i );
	    
	    for (j = bringers.begin( ); j != bringers.end( ); j++)
		if ((*j)->in_room->area == mob->in_room->area)
		    break;
	    
	    if (j != bringers.end( )) 
		continue;
	    
	    RainbowMob::Pointer behavior( NEW );
	    behavior->setChar( mob );
	    behavior->number = bringers.size( );
	    mob->behavior.setPointer( *behavior );

	    save_mobs( mob->in_room );
	    bringers.push_back( mob );

	    roomVnums.push_back( mob->in_room->vnum );
	    SET_BIT(mob->in_room->area->area_flag, AREA_NOGATE);
	}

	if (bringers.size( ) < total) 
	    throw GQCannotStartException( "not found enough mobiles" );

    }
    catch (const GQCannotStartException& e0) {
        LogStream::sendNotice( ) << "Caught exception: " << e0.what( ) << endl;
	cleanup( );
	throw e0;
    }
    catch (const Exception& e) {
        LogStream::sendNotice( ) << "Caught exception: " << e.what( ) << endl;
	cleanup( );
	throw GQCannotStartException( e.what( ) );
    }
    
    LogStream::sendNotice( ) << "Configured for " << bringers.size( ) << " candidates." << endl;
    scenario->onQuestInit( );
    LogStream::sendNotice( ) << "Rainbow quest init done." << endl;
}

void RainbowGQuest::cleanup( )
{
    Character *ch, *ch_next;
    Object *obj, *obj_next;
    NPCharacter *mob;
    unsigned int i;
    
    LogStream::sendNotice( ) << "Rainbow cleanup." << endl;

    for (i = 0; i < roomVnums.size( ); i++)
	REMOVE_BIT(get_room_index(roomVnums[i])->area->area_flag, AREA_NOGATE);
	
    for (ch = char_list; ch; ch = ch_next) {
	ch_next = ch->next;
	
	if (!ch->is_npc( ))
	    continue;

	mob = ch->getNPC( );
	 
	if (mob->behavior && mob->behavior.getDynamicPointer<RainbowMob>( )) {
	    MobileBehaviorManager::assignBasic( mob );
	    save_mobs( mob->in_room );
	}
    }

    for (obj = object_list; obj; obj = obj_next) {
	obj_next = obj->next;

	if (obj->behavior && obj->behavior.getDynamicPointer<RainbowPiece>( ))
	    extract_obj( obj );
    }
}

void RainbowGQuest::destroy( ) 
{
    cleanup( );

    switch (state.getValue( )) {
    case ST_FINISHED:
	rewardWinner( );
	break;
    case ST_RUNNING:
	rewardNobody( );
	break;
    }
}

int RainbowGQuest::getTaskTime( ) const
{
    int itime = getScenario( )->getInitTime( );

    switch (state.getValue( )) {
    case ST_INIT:
	return itime;
    case ST_RUNNING:
	return getRemainedTime( ) - itime;
    default:
	return 0;
    }
}

void RainbowGQuest::after( )
{
    if (state == ST_INIT) {
	GQChannel::gecho( this, getScenario( )->getStartMsg( ) );
	state = ST_RUNNING;
    }
    
    GlobalQuest::after( );
}

void RainbowGQuest::report( std::ostringstream &buf, PCharacter *ch ) const
{
    int cnt;
    
    if (isHidden( ))
	return;

    cnt = countPieces( ch );

    if (cnt > 0)
	getScenario( )->printCount( cnt, buf );

    getScenario( )->printTime( buf );
}

void RainbowGQuest::progress( std::ostringstream &ostr ) const
{
    char buf[MAX_STRING_LENGTH];
    int cnt;
    Character *ch;

    if (isHidden( ))
	return;

    for (Descriptor *d = descriptor_list; d; d = d->next) {
	if (!d->character || d->character->is_npc( ) || d->connected != CON_PLAYING)
	    continue;
	
	ch = d->character;
	cnt = countPieces( ch );
	
	if (cnt > 0) {
	    sprintf(buf, "%s%-15s %s%-4d%s",
			 GQChannel::NORMAL, ch->getName( ).c_str( ), 	
			 GQChannel::BOLD, cnt, GQChannel::NORMAL);
	    ostr << buf << endl;
	}
    }
}

void RainbowGQuest::getQuestDescription( std::ostringstream &str ) const
{
    char buf[MAX_STRING_LENGTH];
    Character *ch;
    NPCharacter *mob;
    int t = getRemainedTime( );
    
    if (isHidden( ))
	return;
   
    str << getScenario( )->getInfoMsg( ) << endl << endl;
    
    for (ch = char_list; ch; ch = ch->next) {
	if (!ch->is_npc( ))
	    continue;
	
	mob = ch->getNPC( );

	if (!mob->behavior || !mob->behavior.getDynamicPointer<RainbowMob>( ))
	    continue;
	
	
	sprintf(buf, "%s%-30s%s из %s%s",
		     GQChannel::NORMAL, ch->getNameP( '1' ).c_str( ), 
		     GQChannel::NORMAL, ch->in_room->name, GQChannel::NORMAL);
	str << buf;
	
	if (t <= 5)
	    str << " (" << ch->in_room->area->name << ")" << GQChannel::NORMAL;
	
	str << endl;
    }

    str << endl;
}

void RainbowGQuest::getQuestStartMessage( std::ostringstream &buf ) const
{
}

bool RainbowGQuest::isHidden( ) const
{
    return state.getValue( ) == ST_INIT;
}

/*****************************************************************************/

void RainbowGQuest::rewardNobody( ) 
{
    GQChannel::gecho( getDisplayName( ), getScenario( )->getNoWinnerMsg( ) );
}

void RainbowGQuest::rewardWinner( )
{
    std::basic_ostringstream<char> buf;
    XMLReward r;
    PCMemoryInterface *pci = PCharacterManager::find( winnerName );

    r.reason = getScenario( )->getWinnerMsg( );
    r.gold = number_range( 400, 700 );
    r.qpoints = number_range( 250, 350 );
    r.id = getQuestID( );

    GlobalQuestManager::getThis( )->rewardChar( pci, r );

    getScenario( )->printWinnerMsgOther( pci->getName( ), buf );

    GQChannel::gecho( this, buf.str( ), dynamic_cast<PCharacter *>( pci ) );

    pci->getAttributes( ).getAttr<XMLAttributeGlobalQuest>( "gquest" )
		    ->rememberVictory( getQuestID( ) );
}

/*****************************************************************************/

int RainbowGQuest::countPieces( Character *ch )
{
    Object *obj;
    RainbowPiece::Pointer bhv;
    map<int, int> pieces;

    for (obj = ch->carrying; obj; obj = obj->next_content) 
	if (obj->behavior) {
	    bhv = obj->behavior.getDynamicPointer<RainbowPiece>( );

	    if (bhv) 
		pieces[bhv->number]++;
	}

    return pieces.size( );
}

Object * RainbowGQuest::createPiece( int number )
{
    Object *piece;
    OBJ_INDEX_DATA *pObjIndex;
    RainbowPiece::Pointer behavior( NEW );
    
    if (!(pObjIndex = get_obj_index( getScenario( )->getPieceVnum( ) )))
	throw GQRuntimeException( "no obj index for rainbow piece" );
	
    piece = create_object( pObjIndex, 0 );
    behavior->setObj( piece );
    behavior->config( number );
    piece->behavior.setPointer( *behavior );

    return piece;
}

void RainbowGQuest::recreateMob( RainbowMob *oldBhv )
{
    NPCharacter *newMob, *oldMob;
    RainbowMob::Pointer newBhv( NEW );
    
    oldMob = oldBhv->getChar( );
    newMob = create_mobile( oldMob->pIndexData );
    
    newBhv->setChar( newMob );
    newBhv->number = oldBhv->number;
    newMob->behavior.setPointer( *newBhv );
    
    char_to_room( newMob, oldMob->in_room );
}

RainbowScenario * RainbowGQuest::getScenario( ) const
{
    return RainbowGQuestInfo::getThis( )->getScenario( scenName.getValue( ) );
}

const DLString & RainbowGQuest::getDisplayName( ) const
{
    return getScenario( )->getDisplayName( );
}
