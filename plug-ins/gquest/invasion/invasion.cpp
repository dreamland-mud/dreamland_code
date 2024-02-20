/* $Id: invasion.cpp,v 1.1.2.3.6.5 2010-09-01 21:20:44 rufina Exp $
 * 
 * ruffina, 2004
 */

#include "invasion.h"
#include "invasioninfo.h"
#include "mobiles.h"
#include "xmlattributeinvasion.h"

#include "globalquestmanager.h"
#include "gqchannel.h"
#include "gqexceptions.h"
#include "xmlattributereward.h"

#include "class.h"

#include "room.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "object.h"

#include "act.h"

#include "merc.h"
#include "handler.h"
#include "descriptor.h"
#include "vnum.h"
#include "def.h"

InvasionGQuest* InvasionGQuest::thisClass = NULL;

InvasionGQuest::InvasionGQuest( )
{
}

InvasionGQuest::InvasionGQuest( const DLString& id ) : GlobalQuest( id )
{
    thisClass = this;
}

InvasionGQuest::~InvasionGQuest( )
{
    thisClass = NULL;
}

void InvasionGQuest::create( const GlobalQuest::Config &config )  
{
    int mobCnt, objCnt, helpCnt;
    std::vector<Room *> rooms;
    InvasionGQuestInfo::Scenarios::iterator iter;
    InvasionScenario *scen;
    
    if (config.force) {
        playerCnt = config.playerCnt;
    } else {
        GlobalQuestInfo::PlayerList players;
        InvasionGQuestInfo::getThis( )->findParticipants( players );

        playerCnt = players.size( );
    }

    if (playerCnt <= 0)
        throw GQCannotStartException( "no players on-line, please specify a <playerCnt> argument" );
    
    if (config.arguments.empty( )) {
        iter = InvasionGQuestInfo::getThis( )->getRandomScenariosEntry( );
        scenName = iter->first;
        scen = *(iter->second);
    } else {
        scenName = config.arguments;
        scen = InvasionGQuestInfo::getThis( )->findScenario( scenName );
        if (scen == NULL)
            throw GQCannotStartException( "Scenario not found" );
    }

    helpCnt = mobCnt = objCnt = 0;
    
    log("trying " << scenName << " scenario for " << playerCnt << " players");
    
    if (!scen->canStart( ))
        throw GQCannotStartException( "scenario doesnt want to start" );
    
    if (scen->vnumMobs.empty( ))
        objCnt = max( 40, number_range( playerCnt * 5, playerCnt * 7 ) );
    else if (scen->vnumObjs.empty( ))
        mobCnt = max( 40, number_range( playerCnt * 5, playerCnt * 7 ) );
    else {
        mobCnt = max( 40, number_range( playerCnt * 2, playerCnt * 4 ) );
        objCnt = max( 40, number_range( playerCnt * 2, playerCnt * 4 ) );
    }
        
    if (!scen->vnumHelpers.empty( ))
        helpCnt = max( 20, number_range( playerCnt * 2, playerCnt * 3 ) );
    
    scen->collectRooms( rooms, mobCnt + objCnt + helpCnt);
    
    if ((int)rooms.size( ) < mobCnt + objCnt + helpCnt)
        throw GQCannotStartException("not enough rooms");
        
    log("mobiles " << mobCnt << ", objects " << objCnt << ", helpers " << helpCnt << ", rooms " << rooms.size( ));
    
    try {
        int i;
        NPCharacter *mob;

        while (!rooms.empty( ) && mobCnt > 0) {
            mob = createMob( );
            i = number_range( 0, rooms.size( ) - 1 );
            char_to_room( mob, rooms[i] );

            if (mob->canEnter( rooms[i] )) 
                mobCnt--;
            else 
                extract_char( mob );
            
            rooms.erase( rooms.begin( ) + i );
        }

        while (!rooms.empty( ) && objCnt > 0) {
            i = number_range( 0, rooms.size( ) - 1 );
            obj_to_room( createObj( ), rooms[i] );
            objCnt--;
            rooms.erase( rooms.begin( ) + i );
        }

        while (!rooms.empty( ) && helpCnt > 0) {
            mob = createHelper( );
            i = number_range( 0, rooms.size( ) - 1 );
            char_to_room( mob, rooms[i] );

            if (mob->canEnter( rooms[i] )) 
                helpCnt--;
            else 
                extract_char( mob );
            
            rooms.erase( rooms.begin( ) + i );
        }
    }
    catch (const Exception& e) {
        cleanup( false );
        throw e;
    }
}

void InvasionGQuest::cleanup( bool performance )
{
    Character *ch, *ch_next;
    Object *obj, *obj_next;
    
    for (ch = char_list; ch; ch = ch_next) {
        ch_next = ch->next;
        
        if (ch->is_npc( )) {
            NPCharacter *mob = ch->getNPC( );
            
            if (mob->behavior) {
                if (mob->behavior.getDynamicPointer<InvasionMob>( )) {
                    if (performance) 
                        oldact("$c1 лопается.", ch, 0, 0, TO_ROOM);
                    
                    extract_char( mob );
                }
                else if (mob->behavior.getDynamicPointer<InvasionHelper>( )) 
                    extract_char( mob );
            }
        }
    }

    for (obj = object_list; obj; obj = obj_next) {
        obj_next = obj->next;

        if (obj->behavior) {
            if (obj->behavior.getDynamicPointer<InvasionObj>( )) {
                if (performance && obj->in_room)
                    oldact("$o1 исчезает.", obj->in_room->people, obj, 0, TO_ROOM);

                extract_obj( obj );
            }
            else if (obj->behavior.getDynamicPointer<InvasionInstrument>( )) 
                extract_obj( obj );
        }
    }
}

void InvasionGQuest::destroy( ) 
{
    cleanup( true );
    rewardLeader( );
}

void InvasionGQuest::report( std::ostringstream &buf, PCharacter *ch ) const
{
    XMLAttributeInvasion::Pointer attr;
    
    attr = ch->getAttributes( ).findAttr<XMLAttributeInvasion>( getQuestID( ) );
    
    if (attr && attr->getKilled( ) > 0)
        buf << "Число уничтоженных тобой безобразий: " 
            << GQChannel::BOLD <<  attr->getKilled( ) << GQChannel::NORMAL << endl;
    
    buf << "До конца охоты остается ";
    printRemainedTime( buf );
    buf << "." << endl;
}

void InvasionGQuest::progress( std::ostringstream &ostr ) const
{
    PCharacterMemoryList::const_iterator i;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );

    for (i = pcm.begin( ); i != pcm.end( ); i++) {
        XMLAttributeInvasion::Pointer attr; 
        
        attr = i->second->getAttributes( ).findAttr<XMLAttributeInvasion>( getQuestID( ) );
        
        if (!attr || attr->getKilled( ) <= 0)
            continue;

        ostr << fmt(0, "%s%-15s %s%-4d%s",
                     GQChannel::NORMAL, i->second->getName( ).c_str( ),         
                     GQChannel::BOLD, attr->getKilled( ), GQChannel::NORMAL);

        ostr << endl;
    }
}

void InvasionGQuest::getQuestDescription( std::ostringstream &buf ) const
{
    buf << getScenario( )->getInfoMsg( ) << endl;
}

void InvasionGQuest::getQuestStartMessage( std::ostringstream &buf ) const
{
    buf << getScenario( )->getStartMsg( ) << endl;
}

/*****************************************************************************/

/*
 *  rewards
 */

void InvasionGQuest::rewardLeader( ) 
{
    std::basic_ostringstream<char> buf;
    PCharacterMemoryList::const_iterator i;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
    std::list<PCMemoryInterface *> leaders;
    int max = 0, killed;
    InvasionScenario *scen = getScenario( );

    for (i = pcm.begin( ); i != pcm.end( ); i++) {
        XMLAttributeInvasion::Pointer attr;
        
        attr = i->second->getAttributes( ).findAttr<XMLAttributeInvasion>( getQuestID( ) );
        
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
    
    if (leaders.empty( )) {
        buf << scen->getNoWinnerMsg( ) << endl;
    }
    else { 
        XMLReward reward;

        reward.qpoints = 200;
        reward.gold = number_range( 150, 250 );
        reward.experience = number_range( 100, 400 );
        reward.practice = number_range( -6, 3 );
        reward.reason = scen->getWinnerMsg( );
        reward.id = getQuestID( );

        if (leaders.size() == 1) {
            PCMemoryInterface *winner = leaders.front();
            winner->getAttributes( ).getAttr<XMLAttributeGlobalQuest>( "gquest" )
                            ->rememberVictory( getQuestID( ) );
        }

        if (leaders.size( ) > 1 && !scen->getWinnerMsgOtherMlt( ).empty( ))
            buf << scen->getWinnerMsgOtherMlt( );
        else
            buf << scen->getWinnerMsgOther( );

        buf << GQChannel::BOLD;

        while (!leaders.empty( )) {
            PCMemoryInterface * pci;

            pci = leaders.back( );
            leaders.pop_back( );

            buf << " " << pci->getName( );
            if (!leaders.empty( ))
                buf << ",";

            log("InvasionGQuest: reward winner " << pci->getName( ));
            GlobalQuestManager::getThis( )->rewardChar( pci, reward );
        }

    }

    GQChannel::gecho( this, buf );
}

void InvasionGQuest::rewardKiller( PCharacter *killer )
{
    XMLReward r;
    XMLAttributeInvasion::Pointer attr;
    
    if (!killer)
        return;

    attr = killer->getAttributes( ).getAttr<XMLAttributeInvasion>( getQuestID( ) );
    
    if (attr->isPunished( ))
        return;

    attr->setKilled( attr->getKilled( ) + 1 );

    r.qpoints = 1;
    r.reason = getScenario( )->getRewardMsg( );
    r.id = getQuestID( );
    GlobalQuestManager::getThis( )->rewardChar( killer, r );        
}


/*****************************************************************************/


NPCharacter * InvasionGQuest::createMob( )
{
    NPCharacter *ch;
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    InvasionScenario::VnumList &mobs = getScenario( )->vnumMobs;

    vnum = mobs[number_range( 0, mobs.size( ) - 1 )];

    if (!( pMobIndex = get_mob_index( vnum ) ))
        throw MobileNotFoundException( vnum );
        
    ch = create_mobile( pMobIndex );
    
    if (!ch->behavior || !ch->behavior.getDynamicPointer<InvasionMob>( ))
        throw BadMobileBehaviorException( vnum );
        
    return ch;
}

NPCharacter * InvasionGQuest::createHelper( )
{
    NPCharacter *ch;
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    InvasionScenario::VnumList &mobs = getScenario( )->vnumHelpers;
    
    vnum = mobs[number_range( 0, mobs.size( ) - 1 )];

    if (!( pMobIndex = get_mob_index( vnum ) ))
        throw MobileNotFoundException( vnum );
        
    ch = create_mobile( pMobIndex );
    
    if (!ch->behavior || !ch->behavior.getDynamicPointer<InvasionHelper>( ))
        throw BadMobileBehaviorException( vnum );
        
    return ch;
}

Object * InvasionGQuest::createObj( )
{
    Object *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    InvasionScenario::VnumList &objs= getScenario( )->vnumObjs;

    vnum = objs[number_range( 0, objs.size( ) - 1 )];

    if (!( pObjIndex = get_obj_index( vnum ) ))
        throw ObjectNotFoundException( vnum );

    obj = create_object( pObjIndex, 0 );
        
    if (!obj->behavior || !obj->behavior.getDynamicPointer<InvasionObj>( ))
        throw BadObjectBehaviorException( vnum );

    return obj;
}

Object * InvasionGQuest::createInstrument( )
{
    Object *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    InvasionScenario::VnumList &objs= getScenario( )->vnumInstruments;

    vnum = objs[number_range( 0, objs.size( ) - 1 )];

    if (!( pObjIndex = get_obj_index( vnum ) ))
        throw ObjectNotFoundException( vnum );

    obj = create_object( pObjIndex, 0 );
        
    if (!obj->behavior || !obj->behavior.getDynamicPointer<InvasionInstrument>( ))
        throw BadObjectBehaviorException( vnum );

    return obj;
}

int InvasionGQuest::countInstruments( PCharacter *ch )
{
    Object *obj;
    int cnt = 0;
    InvasionScenario::VnumList &objs= getScenario( )->vnumInstruments;
    InvasionScenario::VnumList::iterator i;
    
    for (obj = object_list; obj; obj = obj->next) 
        if (obj->behavior
            && obj->behavior.getDynamicPointer<InvasionInstrument>( )
            && obj->hasOwner( ch ))
        {
            for (i = objs.begin( ); i != objs.end( ); i++)
                if (*i == obj->pIndexData->vnum) {
                    cnt++;
                    break;
                }
        }

    return cnt;
}

InvasionScenario * InvasionGQuest::getScenario( ) const
{
    return InvasionGQuestInfo::getThis( )->getScenario( scenName.getValue( ) );
}

