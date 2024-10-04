/* $Id: locatequest.cpp,v 1.1.2.15.6.12 2010-09-01 21:20:46 rufina Exp $
 *
 * ruffina, 2004
 */

#include "locatequest.h"
#include "scenarios.h"
#include "objects.h"
#include "mobiles.h"

#include "questexceptions.h"

#include "skillreference.h"
#include "clanreference.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "player_utils.h"

#include "handler.h"
#include "act.h"
#include "save.h"

#include "def.h"

CLAN(battlerager);
GSN(locate_object);
GSN(find_object);


/*-----------------------------------------------------------------------------
 * LocateQuest
 *----------------------------------------------------------------------------*/
LocateQuest::LocateQuest( )
{
}

void LocateQuest::create( PCharacter *pch, NPCharacter *questman ) 
{
    int time;
    NPCharacter *customer;
    Room *endPoint = 0;
    
    charName = pch->getName( );
    state = QSTAT_INIT;

    try {
        scenName = LocateQuestRegistrator::getThis( )->getRandomScenario( pch );
        customer = getRandomClient( pch );
        customerName = customer->getShortDescr( );
        customerRoom = customer->in_room->getName();
        customerArea = customer->in_room->areaName();

        if (getScenario( ).needsEndPoint( )) {
            endPoint = getRandomRoomClient( pch );
            targetArea = endPoint->areaName();
        }

        scatterItems( pch, endPoint, customer );
        ClientQuestModel::assign<LocateCustomer>( customer );
        save_mobs( customer->in_room );
    } 
    catch (const QuestCannotStartException &e) {
        destroy( );
        throw e;
    }

    time = number_range( 5, 10 ); 

    if (Player::isNewbie( pch ))
        time *= 2;

    setTime( pch, time );

    tell_fmt( "{W%3$#^C1{G хочет отыскать некоторые принадлежащие %3$P3 вещи.",  
              pch, questman, customer );
    tell_fmt( "%3$#^P1 ждет тебя в районе {W%4$s{G ({W{hh%5$s{hx{G).", 
               pch, questman, customer, customer->in_room->getName(), customer->in_room->areaName().c_str() );
    tell_fmt( "У тебя есть {Y%3$d{G мину%3$Iта|ты|т, чтобы добраться туда и узнать подробности.",  
               pch, questman, time );
    
    wiznet( scenName.getValue( ).c_str( ), 
            "customer [%s], item [%s], count %d, path from [%d] to [%d]",
            customer->getNameP( '1' ).c_str( ), 
            itemName.ruscase( '1' ).c_str( ),
            total.getValue( ),
            customer->in_room->vnum, (endPoint ? endPoint->vnum : 0) );
}

bool LocateQuest::isComplete( ) 
{
    return state == QSTAT_FINISHED;
}

void LocateQuest::info( std::ostream &buf, PCharacter *ch ) 
{
    switch (state.getValue( )) {
    case QSTAT_INIT:
        buf << customerName.ruscase( '1' ) <<  " хочет отыскать кое-какие свои вещи." << endl
            << "Тебя с нетерпением ждут в районе " << customerRoom << "." << endl
            << "Это находится в местности под названием {hh" << customerArea << "{hx." << endl;
        break;
    case QSTAT_SEARCH:
        getScenario( ).getLegend( ch, this, buf );

        if (delivered > 0)
            buf << "Тобой уже доставлено {Y" << delivered << "{x из них." << endl;
        
        buf << "Заказчик ждет тебя в районе " << customerRoom << "." << endl
            << "Это находится в местности под названием {hh" << customerArea << "{hx." << endl;

        break;
    case QSTAT_FINISHED:
        buf << "Твое задание {YВЫПОЛНЕНО{x!" << endl
            << "Вернись за вознаграждением, до того как выйдет время!" << endl;
        break;
    default:
        break;
    }
}

void LocateQuest::shortInfo( std::ostream &buf, PCharacter *ch )
{
    switch (state.getValue( )) {
    case QSTAT_INIT:
        buf << "Помочь " << customerName.ruscase( '3' ) << " из " << customerRoom
            << " (" << customerArea << ") отыскать свои вещи.";
        break;
    case QSTAT_SEARCH:
        buf << "Найти " << total << " штук" << GET_COUNT(total, "у", "и", "") << " "
            << russian_case( itemMltName.getValue( ), '2' ) << " для "
            << russian_case( customerName.getValue( ), '2' ) << " из " << customerRoom 
            << " (" << customerArea << ").";
        break;
    case QSTAT_FINISHED:
        buf << "Вернуться к квестору за наградой.";
        break;
    default:
        break;
    }
}

QuestReward::Pointer LocateQuest::reward( PCharacter *ch, NPCharacter *questman ) 
{
    QuestReward::Pointer r( NEW );
    
    if (hint && !Player::isNewbie(ch)) {
        r->points = number_range( 3, 9 );
        r->gold = number_fuzzy( r->points );
    } else {
        if (total <= 5)
            r->points = 10;
        else
            r->points = 20;

        r->points += number_range( 3 * total, 4 * total );
        r->gold = number_fuzzy( r->points );
        r->wordChance = 3 * total;
        r->scrollChance = number_fuzzy( total );

        if (chance( total ))
            r->prac = number_range( 1, 3 );
    
        if (!ch->getClan( )->isDispersed( )) {
            r->points /= 2;
            r->clanpoints = r->points;
        }
    }

    r->exp = (r->points + r->clanpoints) * 10;
    return r;
}

void LocateQuest::destroy( ) 
{
    destroyItems<LocateItem>( );
    clearMobile<LocateCustomer>( );
}

/*-----------------------------------------------------------------------------
 * LocateQuest: local methods
 *----------------------------------------------------------------------------*/
const LocateScenario & LocateQuest::getScenario( ) const
{
    return *(LocateQuestRegistrator::getThis( )->getMyScenario<LocateScenario>( scenName ));
}

bool LocateQuest::checkMobileClient( PCharacter *pch, NPCharacter *mob )
{
    return getScenario( ).customers.hasElement( mob->pIndexData->vnum )
           && ClientQuestModel::checkMobileClient( pch, mob );
}

bool LocateQuest::checkRoomClient( PCharacter *pch, Room *room )
{
    if (!customerArea.empty( ) && customerArea == room->areaName())
        return false;

    return ClientQuestModel::checkRoomClient( pch, room );
}

void LocateQuest::scatterItems( PCharacter *pch, Room *endPoint, NPCharacter *customer )
{
    Object *obj;
    OBJ_INDEX_DATA *pObjIndex;
    unsigned int i, count;
    LocateAlgo::Rooms rooms;
    const LocateScenario &scen = getScenario( );

    if (scen.items.empty( ))
        throw QuestCannotStartException( );
    
    scen.findRooms( pch, customer->in_room, endPoint, rooms );
    count = scen.getCount( pch );

    if (!count || count > rooms.size( ))
        throw QuestCannotStartException( );
    
    const LSItemData &itemScen = scen.items[number_range( 0, scen.items.size( ) - 1 )];
    itemName = itemScen.shortDesc;
    itemMltName = itemScen.shortMlt;

    pObjIndex = get_obj_index( LocateQuestRegistrator::getThis( )->itemVnum );
        
    while (!rooms.empty( ) && total < (int)count) {
        i = number_range( 0, rooms.size( ) - 1 );
        obj = createItem<LocateItem>( pObjIndex );
        itemScen.dress( obj );
        obj_to_room( obj, rooms[i] );
        total++;
        rooms.erase( rooms.begin( ) + i );
    }

    if (!total)
        throw QuestCannotStartException( );
}

/*-----------------------------------------------------------------------------
 * LocateQuestRegistrator
 *----------------------------------------------------------------------------*/
LocateQuestRegistrator * LocateQuestRegistrator::thisClass = NULL;

LocateQuestRegistrator::LocateQuestRegistrator( )
{
    thisClass = this;
}

LocateQuestRegistrator::~LocateQuestRegistrator( )
{
    thisClass = NULL;
}

bool LocateQuestRegistrator::applicable( PCharacter *pch, bool fAuto ) const
{
    if (!QuestRegistratorBase::applicable(pch, fAuto))
        return false;
    if (pch->getClan( ) == clan_battlerager)
        return false;

    return gsn_locate_object->getEffective( pch ) >= 50
            || gsn_find_object->getEffective( pch ) >= 50;
}

