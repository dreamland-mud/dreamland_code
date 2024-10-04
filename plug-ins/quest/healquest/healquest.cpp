/* $Id: healquest.cpp,v 1.1.2.25.6.10 2010/01/01 15:48:15 rufina Exp $
 *
 * ruffina, 2003
 */

#include "healquest.h"
#include "patientbehavior.h"
#include "scenarios.h"
#include "questexceptions.h"

#include "player_utils.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "affect.h"

#include "merc.h"
#include "handler.h"
#include "save.h"

#include "act.h"
#include "def.h"

GSN(observation);

/*--------------------------------------------------------------------
 * HealQuest
 *--------------------------------------------------------------------*/
void HealQuest::create( PCharacter *pch, NPCharacter *questman ) 
{
    NPCharacter *patient;
    HealScenarioList scenarios;
    unsigned int time, minLevel, maxLevel, count, mlevel;
    
    charName = pch->getName( );
    patient = getRandomClient( pch );
    registrator->getMyScenarios( pch, patient, scenarios );
    time = number_range( 15, 25 );
    mlevel = pch->getModifyLevel( );
    
    if (Player::isNewbie(pch)) {
        minLevel = max( 1, (int)(mlevel - 5) );
        maxLevel = mlevel + 5;
        count = 1;

    } else {
        minLevel = number_range(max(1, (int)mlevel-5), mlevel + 10);
        maxLevel = mlevel + 15;
        count = 1 + chance( 10 ) + chance( 3 );
    }

    if (scenarios.size( ) < count)
        throw QuestCannotStartException( );
    
    mode = count;

    while (!scenarios.empty( ) && maladies.size( ) < count) {
        int i = number_range( 0, scenarios.size( ) - 1 );
        HealScenario::Pointer scp = scenarios.at( i );
        unsigned int level = number_range( minLevel, maxLevel );

        scp->infect( patient, time + 1, level );
        maladies.setElement( scp->malady );
        scenarios.erase( scenarios.begin( ) + i );
        
        if (level >= mlevel + 10)
            mode++;
    }

    if (IS_AFFECTED(patient, AFF_HIDE|AFF_FADE)
        || patient->pIndexData->count > 5)
        mode++;

    assign<PatientBehavior>( patient );
    save_mobs( patient->in_room );
    mobName  = patient->getShortDescr( );
    roomName = patient->in_room->getName();
    areaName = patient->in_room->areaName();

    setTime( pch, time );

    tell_raw( pch, questman, "У меня есть для тебя срочное поручение!" );   
    tell_fmt( "{W%3$#^C1{G чем-то серьезно бол%3$Gьно|ен|ьна и нуждается в помощи лекаря.",
               pch, questman, patient );
    tell_fmt( "Место, где %3$P2 видели в последний раз - {W%4$s{x.",
              pch, questman, patient, patient->in_room->getName() );
    tell_fmt( "Это находится в районе под названием {W{hh%3$s{x.", 
              pch, questman, patient->in_room->areaName().c_str() );
    tell_fmt( "Поторопись, пока болезнь не доконала %3$P2!", pch, questman, patient );
    tell_fmt( "У тебя есть {Y%3$d{G мину%3$Iта|ты|т на выполнение задания.", pch, questman, time );

    wiznet( "", "cure %s [%d] from %d maladies, mode %d",
                patient->getNameP('1').c_str( ), patient->in_room->vnum,
                maladies.size( ), mode.getValue( ) );
}

void HealQuest::clear( NPCharacter *mob )
{
    ClientQuestModel::clear( mob );
    maladies.cleanup( mob );
}

void HealQuest::destroy( ) 
{
    clearMobile<PatientBehavior>( );
}

bool HealQuest::isComplete( ) 
{
    return (int)maladies.size( ) == maladies.successTotal( );
}

QuestReward::Pointer HealQuest::reward( PCharacter *ch, NPCharacter *questman ) 
{
    QuestReward::Pointer r( NEW );
    
    if (hint > 0 && !Player::isNewbie(ch)) {
        r->gold = number_range( 1, 2 );
        r->points = number_range( 1, 4 );

    } else {
        unsigned int m;
        
        m = max( 0, (int)(mode - (maladies.size( ) - maladies.successHero( ))) );

        switch (m) {
        case 0:
            r->gold = number_range( 5, 8 );
            r->points = number_range( 5, 8 );
            r->wordChance = 10;
            r->scrollChance = 7;
            break;
        case 1: 
            r->gold = number_range( 8, 12 );
            r->points = number_range( 8, 12 );
            r->prac = max( 0, number_range( -17, 2 ) );
            r->wordChance = 15;
            r->scrollChance = 10;
            break;
        case 2:
            r->gold = number_range( 12, 16 );
            r->points = number_range( 12, 16 );
            r->prac = max( 0, number_range( -13, 3 ) );
            r->wordChance = 25;
            r->scrollChance = 13;
            break;
        default:
            r->gold = number_range( 16, 20 );
            r->points = number_range( 16, 20 );
            r->prac = max( 0, number_range( -10, 4 ) );
            r->wordChance = 30;
            r->scrollChance = 15;
            break;
        }
    }

    if (ch->getClan( )->isDispersed( )) 
        r->points *= 2;
    else
        r->clanpoints = r->points;

    r->exp = (r->points + r->clanpoints) * 10;
    return r;
}

void HealQuest::info( std::ostream &buf, PCharacter *ch ) 
{
    if (isComplete( ))
        buf << "Твое задание {YВЫПОЛНЕНО{x!" << endl
            << "Вернись за вознаграждением, до того как выйдет время!" << endl;
    else 
        buf << "У тебя задание - вылечить " << russian_case( mobName, '4' ) << "!" << endl
            << "Место, где пациента видели в последний раз - " << roomName << endl
            << "Это находится в районе под названием {hh" << areaName << "{hx." << endl;
}

void HealQuest::shortInfo( std::ostream &buf, PCharacter *ch )
{
    if (isComplete( ))
        buf << "Вернуться к квестору за наградой.";
    else 
        buf << "Вылечить " << russian_case( mobName, '4' ) << " из "
            << roomName << " (" << areaName << ").";
}

Room * HealQuest::helpLocation( )
{
    return findMobileRoom<PatientBehavior>( );
}


bool HealQuest::checkMobileClient( PCharacter *pch, NPCharacter *mob )
{
    int diff, mlevel;

    if (!ClientQuestModel::checkMobileClient( pch, mob ))
        return false;

    if (IS_SET( mob->imm_flags, IMM_SPELL ))
        return false;

    mlevel = pch->getModifyLevel( );
    diff = abs( mlevel - mob->getRealLevel( ) );
    
    if (diff > mlevel / 4)
        return false;
    
    return true;
}

bool HealQuest::checkRoomClient( PCharacter *pch, Room *room )
{
    if (IS_SET( room->room_flags, ROOM_NO_CAST ))
        return false;

    if (!ClientQuestModel::checkRoomClient( pch, room ))
        return false;
        
    return true;
}

/*--------------------------------------------------------------------
 * HealQuestRegistrator
 *--------------------------------------------------------------------*/
HealQuestRegistrator *registrator = 0;
HealQuestRegistrator::HealQuestRegistrator( )
{
    registrator = this;
}

HealQuestRegistrator::~HealQuestRegistrator( )
{
    registrator = 0;
}

bool HealQuestRegistrator::applicable( PCharacter *pch, bool fAuto ) const
{
    if (!QuestRegistratorBase::applicable(pch, fAuto))
        return false;

    Scenarios::const_iterator s;
    for (s = scenarios.begin(); s != scenarios.end(); s++) {
        if (s->second->applicable(pch))
            return true;
    }

    return false;
}

/*--------------------------------------------------------------------
 * HealMaladies
 *--------------------------------------------------------------------*/
void HealMaladies::cleanup( NPCharacter *mob )
{
    if (!mob)
        return;

    for (iterator i = begin( ); i != end( ); i++)
        if (i->second == HSTAT_INIT) 
            affect_strip( mob, skillManager->find( i->first )->getIndex( ) );
}

void HealMaladies::setElement( const SkillReference &skill ) 
{
    (*this)[skill.getName( )] = HSTAT_INIT;
}

bool HealMaladies::hasKey( int sn ) const
{
    return hasKey( skillManager->find( sn )->getName( ) );
}

bool HealMaladies::hasKey( const DLString &name ) const
{
    return find( name ) != end( );
}

void HealMaladies::setAttempts( int sn )
{
    for (iterator i = begin( ); i != end( ); i++)
        if (i->second == HSTAT_INIT)
            if (registrator->getMyScenario<HealScenario>( i->first )->healedBy( sn ))
                i->second = HSTAT_ATTEMPT;
}

bool HealMaladies::checkSuccess( int sn, NPCharacter *patient )
{
    return checkSuccessAny( sn, patient, HSTAT_SUCCESS );
}

bool HealMaladies::checkSuccessOther( int sn, NPCharacter *patient )
{
    return checkSuccessAny( sn, patient, HSTAT_SUCCESS_OTHER );
}

bool HealMaladies::checkSuccessAny( int sn, NPCharacter *patient, int successState )
{
    bool result = false;

    for (iterator i = begin( ); i != end( ); i++)
        if (i->second == HSTAT_ATTEMPT) {
            if (registrator->getMyScenario<HealScenario>( i->first )->isInfected( patient ))
                i->second = HSTAT_INIT;
            else {
                i->second = successState;
                result = true;
            }
        }

    return result;
}

int HealMaladies::countStates( int state ) const
{
    int count = 0;

    for (const_iterator i = begin( ); i != end( ); i++)
        if (i->second == state)
            count++;

    return count;
}

int HealMaladies::successHero( ) const
{
    return countStates( HSTAT_SUCCESS );
}

int HealMaladies::successTotal( ) const
{
    return countStates( HSTAT_SUCCESS ) + countStates( HSTAT_SUCCESS_OTHER );
}

