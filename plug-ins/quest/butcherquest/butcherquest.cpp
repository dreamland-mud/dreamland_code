/* $Id: butcherquest.cpp,v 1.1.2.27.6.8 2009/11/08 17:39:52 rufina Exp $
 *
 * ruffina, 2003
 */
#include "butcherquest.h"
#include "steakcustomer.h"
#include "questmanager.h"
#include "questexceptions.h"

#include "player_utils.h"
#include "skillreference.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "roomutils.h"
#include "race.h"

#include "merc.h"
#include "act.h"
#include "loadsave.h"
#include "save.h"
#include "def.h"

GSN(butcher);

void ButcherQuest::create( PCharacter *pch, NPCharacter *questman ) 
{
    MobIndexMap games;
    MOB_INDEX_DATA *pGameIndex;
    NPCharacter *customer;
    int time;

    charName = pch->getName( );

    findVictims( pch, games );
    pGameIndex  = getRandomMobIndex( games );
    raceName    = pGameIndex->race;
    raceRusName = pGameIndex->short_descr.get(LANG_DEFAULT);
    areaName    = pGameIndex->area->getName();
    
    if (Player::isNewbie(pch))
        ordered = URANGE( 1, games[pGameIndex].size( ) * 3 / 2, 6 );
    else 
        ordered = URANGE( 4, games[pGameIndex].size( ) * 3 / 2, 12 );
        
    customer = getRandomClient( pch );
    customerName = customer->getNameP( '1' );
    customerName = customerName.upperFirstCharacter( );
    customerArea = customer->in_room->areaName();
    assign<SteakCustomer>( customer );
    save_mobs( customer->in_room );

    time = number_range( 15, 25 );
    setTime( pch, time );

    tell_raw( pch, questman, "У меня есть для тебя срочное поручение!" );
    tell_raw( pch, questman, 
        "{W%s{G из местности {W{hh%s{hx{G хочет подать к столу {W%d{G кус%s мяса {W%s{G из местности {W{hh%s{hx{G.", 
        customerName.c_str( ),
        customerArea.c_str( ),
        ordered.getValue( ),
        GET_COUNT(ordered.getValue( ), "ок", "ка", "ков"),
        raceRusName.ruscase( '2' ).c_str( ),
        areaName.c_str( ));

    tell_raw( pch, questman, "Доставь мясо заказчику и вернись сюда за вознаграждением." );
    tell_raw( pch, questman, "У тебя есть {Y%d{G минут%s на выполнение задания.",
                  time, GET_COUNT(time,"а","ы","") ); 

    wiznet( "", "%d steaks of %s from %s, customer %s.",
                ordered.getValue( ),
                raceName.c_str( ),
                areaName.c_str( ),
                customerName.c_str( ) );
}

bool ButcherQuest::isComplete( ) 
{
    return (delivered.getValue( ) >= ordered.getValue( ));
}

void ButcherQuest::info( std::ostream &buf, PCharacter *ch ) 
{
    if (isComplete( ))
        buf << "Твое задание {YВЫПОЛНЕНО{x!" << endl
            << "Вернись за вознаграждением, до того как выйдет время!" << endl;
    else { 
        buf << customerName << " из {hh" << customerArea
            << "{hx просит тебя доставить к столу "
            << ordered << " кус" << GET_COUNT(ordered.getValue( ), "ок", "ка", "ков")
            << " мяса " << raceRusName.ruscase( '2' ) 
            << " из местности {hh" << areaName << "{hx." << endl;
            
        if (delivered > 0)
            buf << "Доставлено кусков: " << delivered << "." << endl;    
    }        
}

void ButcherQuest::shortInfo( std::ostream &buf, PCharacter *ch )
{
    if (isComplete( ))
        buf << "Вернуться к квестору за наградой.";
    else { 
        buf << customerName << " из " << customerArea << " заказал "
            << ordered << " кус" << GET_COUNT(ordered.getValue( ), "ок", "ка", "ков")
            << " мяса " << raceRusName.ruscase( '2' )  << " из " << areaName << ".";
    }        
}

QuestReward::Pointer ButcherQuest::reward( PCharacter *ch, NPCharacter *questman ) 
{
    QuestReward::Pointer r( NEW );
    int n;
    
    n = ordered.getValue( ) * 2;
    r->gold = number_fuzzy( 5 + n );
    r->points = number_fuzzy( 5 + n );
    r->prac = std::max( 0, number_range( -10, 2 ) );
    r->wordChance = n * 3 / 2;
    r->scrollChance = number_range( 5, 10 );

    if (ch->getClan( )->isDispersed( )) 
        r->points *= 2;
    else
        r->clanpoints = r->points;

    r->exp = (r->points + r->clanpoints) * 10;
    return r;
}

void ButcherQuest::destroy( ) 
{
    clearMobile<SteakCustomer>( );
}

bool ButcherQuest::checkMobileVictim( PCharacter *pch, NPCharacter *mob )
{
    if (!VictimQuestModel::checkMobileVictim( pch, mob ))
        return false;

    if (mob->getRealLevel( ) > pch->getModifyLevel( ) + 10)
        return false;
    
    if (mob->size <= SIZE_TINY)
        return false;

    if (!IS_SET(mob->form, FORM_EDIBLE))
        return false;

    if (mob->in_room->areaIndex() != mob->pIndexData->area)
        return false;
    
    return ButcherQuestRegistrator::getThis( )->races.hasElement( mob->getRace( )->getName( ) );
}

bool ButcherQuest::checkMobileClient( PCharacter *pch, NPCharacter *mob )
{
    if (!ClientQuestModel::checkMobileClient( pch, mob ))
        return false;
        
    if (ButcherQuestRegistrator::getThis( )->cooks.hasName( mob ))
        return true;

    return false;
}

bool ButcherQuest::checkRoomVictim( PCharacter *pch, Room *room, NPCharacter *victim )
{
    if (room->areaIndex()->low_range > pch->getModifyLevel( ))
        return false;
    
    if (!RoomUtils::isNature(room))
        return false;

    return VictimQuestModel::checkRoomVictim( pch, room, victim );
}

/* 
 * ButcherQuestRegistrator
 */
ButcherQuestRegistrator * ButcherQuestRegistrator::thisClass = NULL;

ButcherQuestRegistrator::ButcherQuestRegistrator( )
{
    thisClass = this;
}

ButcherQuestRegistrator::~ButcherQuestRegistrator( )
{
    thisClass = NULL;
}

bool ButcherQuestRegistrator::applicable( PCharacter *pch, bool fAuto ) const 
{
    if (!QuestRegistratorBase::applicable(pch, fAuto))
        return false;

    return (gsn_butcher->getEffective( pch ) >= 25);
}

