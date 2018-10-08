/* $Id: globalquestmanager.cpp,v 1.1.2.3.6.9 2009/09/24 14:09:12 rufina Exp $
 * 
 * ruffina, 2003
 */

#include <sstream>
#include "logstream.h"

#include "globalquestmanager.h"
#include "globalquest.h"
#include "globalquestinfo.h"
#include "gqexceptions.h"
#include "xmlattributeglobalquest.h"
#include "xmlattributereward.h"

#include "xmldocument.h"
#include "dlscheduler.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "dreamland.h"
#include "mercdb.h"

using namespace std;

GlobalQuestManager* GlobalQuestManager::thisClass = NULL;
const DLString GlobalQuestManager::TABLE_NAME= "gquests";
const DLString GlobalQuestManager::NODE_NAME_QINFO = "GlobalQuestInfo";
const DLString GlobalQuestManager::NODE_NAME_QUEST = "GlobalQuest";

GlobalQuestManager::GlobalQuestManager( ) 
               : dbioTable( dreamland->getTableDir( ), TABLE_NAME ),
                 dbioRuntime( dreamland->getDbDir( ), TABLE_NAME )
{
    thisClass = this;
    dbioTable.open( );
    dbioRuntime.open( );
}

GlobalQuestManager::~GlobalQuestManager( )
{
    thisClass = NULL;
}

   
void GlobalQuestManager::run( ) 
{
    if (dreamland->getRebootCounter( ) < 36 && dreamland->getRebootCounter( ) != -1) 
        return;
        
    for (RegistryList::iterator iter = registry.begin( );
                            iter != registry.end( );
                            iter++)
    {
        GlobalQuestInfo::Config config;
        GlobalQuestInfo::Pointer gqi = iter->second;
        GlobalQuestInfo::PlayerList players;
        
        if (!gqi->getAutostart( ))
            continue;

        if (gqi->getLastTime( ) + gqi->getWaitingTime( ) > dreamland->getCurrentTime( )) 
            continue;

        gqi->findParticipants( players );        

        if (!gqi->canAutoStart( players, config ))
            continue;

        try {
            gqi->tryStart( config );
        } 
        catch (const GQAlreadyRunningException &) {
        } 
        catch (const Exception &e) { 
            LogStream::sendNotice( ) << e.what( ) << endl;
        }
    }
}

void GlobalQuestManager::after( )
{
    DLScheduler::getThis( )->putTaskInSecond( 60, GlobalQuestManager::Pointer( this ) );    
}

int GlobalQuestManager::getPriority( ) const
{
    return SCDP_ROUND + 100;
}

void GlobalQuestManager::registrate( GlobalQuestInfo *gqi ) 
{
    registry[ gqi->getQuestID( ) ] = gqi;
}

void GlobalQuestManager::unregistrate( GlobalQuestInfo *gqi ) 
{
    RegistryList::iterator i = registry.find( gqi->getQuestID( ) );

    if (i != registry.end( ))
        registry.erase( i );
}

void GlobalQuestManager::activate( GlobalQuest *gq )
{
    running[ gq->getQuestID( ) ] = gq;
}

void GlobalQuestManager::deactivate( GlobalQuest *gq )
{
    RunList::iterator i = running.find( gq->getQuestID( ) );

    if (i != running.end( ))
        running.erase( i );
}

GlobalQuestManager::RunList & GlobalQuestManager::getRunning( ) 
{
    return running;
}

GlobalQuestManager::RegistryList & GlobalQuestManager::getRegistry( ) 
{
    return registry;
}

GlobalQuest::Pointer 
GlobalQuestManager::findGlobalQuest( const DLString &id ) 
{
    RunList::iterator i = running.find( id );
    
    if (i == running.end( ))
        return GlobalQuest::Pointer( );
    
    return i->second;
}

GlobalQuestInfo::Pointer 
GlobalQuestManager::findGlobalQuestInfo( const DLString &id ) 
{
    RegistryList::iterator i = registry.find( id );
    
    if (i == registry.end( ))
        return GlobalQuestInfo::Pointer( );

    return i->second;
}

void GlobalQuestManager::rewardChar( PCMemoryInterface *pcm, XMLReward &r ) 
{
    XMLAttributeGlobalQuest::Pointer attribute;
    XMLAttributeReward::Pointer attr;
    
    attribute = pcm->getAttributes( ).getAttr<XMLAttributeGlobalQuest>( "gquest" ); 

    if (attribute && attribute->getNoExp( ) == true) 
        r.experience = 0;

    attr = pcm->getAttributes( ).getAttr<XMLAttributeReward>( "reward" );
    attr->addReward( r );
    
    if (pcm->isOnline( )) 
        attr->reward( pcm->getPlayer( ) );

    PCharacterManager::saveMemory( pcm );
}

void GlobalQuestManager::load( GlobalQuestInfo *gqi )
{
    XMLDocument::Pointer doc( NEW );
    XMLNode::Pointer node( NEW );

    try {
        DBIO::DBNode dbNode = dbioTable.select( gqi->getQuestID( ) );
        basic_istringstream<char> istr( dbNode.getXML( ).c_str( ) );
        
        doc->load( istr );
        node = doc->getFirstNode( );
        
        if (!node.isEmpty( )) 
            gqi->fromXML( node );
                    
    } catch (const Exception &e) {
        LogStream::sendWarning( ) << e.what( ) << endl;
    }
}

GlobalQuest::Pointer GlobalQuestManager::loadRT( GlobalQuestInfo *gqi )
{
    GlobalQuest::Pointer gq;
    XMLDocument::Pointer doc( NEW );
    XMLNode::Pointer node( NEW );

    try {
        DBIO::DBNode dbNode = dbioRuntime.select( gqi->getQuestID( ) );
        basic_istringstream<char> istr( dbNode.getXML( ).c_str( ) );

        doc->load( istr );
        node = doc->getFirstNode( );

        if (!node.isEmpty( )) {
            gq = gqi->getQuestInstance( );
            gq->fromXML( node );
        }
                    
    } catch (...) {
    }

    return gq;
}

void GlobalQuestManager::save( GlobalQuestInfo *gqi )
{
    basic_ostringstream<char> ostr;
    XMLDocument::Pointer doc( NEW );
    XMLNode::Pointer node( NEW, NODE_NAME_QINFO );
    
    try {
        doc->appendChild( node );
        gqi->toXML( node );
        doc->save( ostr );

        dbioTable.insert( gqi->getQuestID( ), ostr.str( ) );
    
    } catch (const Exception& ex) {
        throw ex;
    }
}

void GlobalQuestManager::saveRT( GlobalQuest *gq )
{
    basic_ostringstream<char> ostr;
    XMLDocument::Pointer doc( NEW );
    XMLNode::Pointer node( NEW, NODE_NAME_QUEST );
    
    try {
        doc->appendChild( node );
        gq->toXML( node );
        doc->save( ostr );

        dbioRuntime.insert( gq->getQuestID( ), ostr.str( ) );
    
    } catch (const Exception& ex) {
        throw ex;
    }
}

void GlobalQuestManager::removeRT( GlobalQuest *gq )
{
    try {
        dbioRuntime.remove( gq->getQuestID( ) );
    
    } catch (...) {
    }
}

