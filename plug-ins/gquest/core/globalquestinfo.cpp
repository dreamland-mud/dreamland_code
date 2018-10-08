/* $Id: globalquestinfo.cpp,v 1.1.2.1.6.5 2009/09/17 18:08:56 rufina Exp $
 * 
 * ruffina, 2003
 */

#include <sstream>
#include "logstream.h"

#include "globalquestinfo.h"
#include "globalquest.h"
#include "globalquestmanager.h"
#include "gqchannel.h"

#include "skillreference.h"
#include "pcharacter.h"
#include "room.h"
#include "descriptor.h"
#include "hometown.h"
#include "vnum.h"
#include "merc.h"
#include "def.h"

GSN(jail);
HOMETOWN(frigate);

/*---------------------------------------------------------------------------
 * GlobalQuestInfo
 *---------------------------------------------------------------------------*/
void GlobalQuestInfo::initialization( ) 
{
    GlobalQuestManager *manager = GlobalQuestManager::getThis( );

    try {
	manager->load( this );
	manager->registrate( this );	
	
	GlobalQuest::Pointer gq = manager->loadRT( this );

	if (gq) {
	    gq->resume( );
        }
	
    } catch( const Exception& ex ) {
        LogStream::sendError( ) << ex << endl;
    }
}

void GlobalQuestInfo::destruction( ) 
{
    GlobalQuestManager *manager = GlobalQuestManager::getThis( );
    
    manager->unregistrate( this );

    try {
	manager->save( this );
		
	GlobalQuest::Pointer gq = manager->findGlobalQuest( getQuestID( ) );

	if (gq) {
	    gq->suspend( );
	    manager->saveRT( *gq );
	}
	
    } catch( const Exception& ex ) {
        LogStream::sendError( ) << ex << endl;
    }
}

void GlobalQuestInfo::tryStart( const GlobalQuestInfo::Config &config )
{
    ostringstream buf;
    GlobalQuest::Pointer gq;
    GlobalQuestManager *manager = GlobalQuestManager::getThis( );
    
    if (manager->findGlobalQuest( getQuestID( ) ))
	throw GQAlreadyRunningException( getQuestID( ) );
	
    gq = getQuestInstance( );
    gq->setTotalTime( config.time );
    gq->setStartTime( );
    gq->setMinLevel( config.minLevel );
    gq->setMaxLevel( config.maxLevel );

    gq->create( config );
    
    gq->getQuestStartMessage( buf );

    if (!buf.str( ).empty( ))
	GQChannel::gecho( this, buf.str( ) );

    gq->resume( );
    manager->saveRT( *gq );
    setLastTime( dreamland->getCurrentTime( ) );
    manager->save( this );
}

bool GlobalQuestInfo::canParticipate( PCharacter *ch ) const
{
    if (ch->is_immortal( ))
	return false;
	
    if (ch->in_room->vnum == ROOM_VNUM_JAIL 
            || ch->in_room->vnum == 10 
	    || ch->isAffected( gsn_jail ))
	return false;

    if (ch->getAttributes( ).isAvailable( "nogq" ))
	return false;
    
    if (ch->getPC( )->getHometown( ) == home_frigate)
	return false;

    if (IS_SET(ch->in_room->room_flags, ROOM_NEWBIES_ONLY))
	return false;

    return true;
}

void GlobalQuestInfo::findParticipants( GlobalQuestInfo::PlayerList &players ) const
{
    for (Descriptor *d = descriptor_list; d; d = d->next) {
	Character *ch = d->character;
	
	if (!ch || d->connected != CON_PLAYING)
	    continue;

	if (ch->is_npc( ))
	    continue;

	if (canParticipate( ch->getPC( ) ))	
	    players.push_back( ch->getPC( ) );
    }	
}

/*---------------------------------------------------------------------------
 * GQuestInfoEveryone 
 *---------------------------------------------------------------------------*/
bool GQuestInfoEveryone::parseArguments( 
	const DLString &cArguments, Config &config, ostringstream &buf ) const
{
    DLString arguments = cArguments;

    if (!config.parseTime( arguments, getDefaultTime( ), buf )
            || !config.parseArguments( arguments, buf )
            || !config.parsePlayerCount( arguments, -1, buf ))
        return false;

    config.minLevel = config.maxLevel = 0;
    config.force = true;
    return true;
}

bool GQuestInfoEveryone::canAutoStart( const PlayerList &players, Config &config ) const
{
    if ((int)players.size( ) < minPlayers.getValue( ))
	return false;

    config.minLevel = config.maxLevel = 0;
    config.time = getDefaultTime( );
    config.force = false;
    return true;
}

/*---------------------------------------------------------------------------
 * GQuestInfoLevels
 *---------------------------------------------------------------------------*/
bool GQuestInfoLevels::parseArguments( 
	const DLString &cArguments, Config &config, ostringstream &buf ) const
{
    DLString arguments = cArguments;
        
    if (!config.parseLevels( arguments, buf )
            || !config.parseTime( arguments, getDefaultTime( ), buf )
            || !config.parseArguments( arguments, buf )
            || !config.parsePlayerCount( arguments, -1, buf ))
        return false;

    config.force = true;
    return true;
}

/*---------------------------------------------------------------------------
 * GlobalQuestInfo::Config
 *---------------------------------------------------------------------------*/
bool GlobalQuestInfo::Config::parseLevels( DLString &arguments, ostringstream &buf )
{
    try {
	minLevel = arguments.getOneArgument( ).toInt( );
	maxLevel = arguments.getOneArgument( ).toInt( );
	
	if (minLevel > maxLevel) {
	    buf << "Уровни не в том порядке." << endl;
	    return false;
	}
    
    } catch (const ExceptionBadType &e) {
	buf << "Неправильный диапазон уровней." << endl;
	return false;
    }
    return true;
}
bool GlobalQuestInfo::Config::parsePlayerCount( DLString &arguments, int defaultValue, ostringstream &buf )
{
    try {
        if (!arguments.empty( ))
            playerCnt = arguments.getOneArgument( ).toInt( );
        else 
            playerCnt = defaultValue;
    } catch (const ExceptionBadType &e) {
	buf << "Неправильное кол-во игроков." << endl;
	return false;
    }
    return true;
}
bool GlobalQuestInfo::Config::parseTime( DLString &arguments, int defaultValue, ostringstream &buf )
{
    try {
	if (!arguments.empty( ))
	    time = arguments.getOneArgument( ).toInt( );
	else
	    time = defaultValue; 
	
    } catch (const ExceptionBadType &e) {
	buf << "Неправильное время." << endl;
	return false;
    }
    return true;
}    

bool GlobalQuestInfo::Config::parseArguments( DLString &arguments, ostringstream &buf )
{
    this->arguments = arguments.getOneArgument( );
    return true;
}    
