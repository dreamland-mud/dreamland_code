/* $Id: kidnapquest.cpp,v 1.1.2.32.6.7 2009/01/01 14:13:18 rufina Exp $
 *
 * ruffina, 2004
 */

#include "kidnapquest.h"
#include "kidnapquestregistrator.h"
#include "king.h"
#include "prince.h"
#include "bandit.h"
#include "objects.h"
#include "questexceptions.h"

#include "selfrate.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "mercdb.h"
#include "merc.h"
#include "handler.h"
#include "save.h"
#include "act.h"
#include "def.h"

#define OBJ_VNUM_CANOE 3051

KidnapQuest::KidnapQuest( ) 
                  : debug( false )
{
}

void KidnapQuest::create( PCharacter *pch, NPCharacter *questman )
{
    NPCharacter *prince, *king;
    Room *room;
    int time;

    charName = pch->getName( );
    state = QSTAT_INIT;

    try {
        scenName = getReg( )->getRandomScenario( pch );

        king = createKing( pch );
        kingVnum = king->pIndexData->vnum;
        kingRoom = king->in_room->getName();
        kingArea = king->in_room->areaName();
        kingName = king->getShortDescr( );

        room = findRefuge( pch, king );
        princeArea = room->areaName();
        princeRoom = room->getName();

        prince = createPrince( king, room );
        princeName = prince->getShortDescr( );

    } catch (const QuestCannotStartException &e) {
        destroy( );
        throw e;
    }
    
    time = std::max( 6, range / 10 );

    if (rated_as_newbie( pch ))
        time *= 2;

    setTime( pch, time );
    
    getScenario( ).onQuestStart( pch, questman, king );
    tell_raw( pch, questman, "У тебя есть {Y%d{G минут%s, чтобы добраться туда и узнать, в чем дело.",
                  time, GET_COUNT(time,"а","ы","") ); 

    wiznet( scenName.c_str( ), "%s in [%d], kid in [%d]",
                 king->getNameP('1').c_str( ),
                 king->in_room->vnum,
                 prince->in_room->vnum );

    if (pch->isCoder( ))
        debug = true;
}

void KidnapQuest::destroy( ) 
{
    clearMobile<KidnapKing>( );
    destroyMobile<KidnapPrince>( );
    destroyMobiles<KidnapBandit>( );
    destroyItems<KidnapMark>( );
}

Quest::Reward::Pointer KidnapQuest::reward( PCharacter *ch, NPCharacter *questman ) 
{
    Reward::Pointer r( NEW );
    
    r->points = number_range( 18, 25 );
    r->points += ambushes * 25;
    if(!IS_TOTAL_NEWBIE(ch)){
    r->points -= hint * 10;
    }
    r->points = std::max( 10, r->points );

    r->gold = number_fuzzy( r->points );
    r->wordChance = 10 * (ambushes + 1);
    r->scrollChance = number_range( 5, 5 * ambushes );

    if (chance( 5 * ambushes ))
        r->prac = number_range( 1, 3 );
    
    if (!ch->getClan( )->isDispersed( )) {
        r->points /= 2;
        r->clanpoints = r->points;
    }

    r->exp = (r->points + r->clanpoints) * 10;
    return Reward::Pointer( r );
}

void KidnapQuest::info( std::ostream &buf, PCharacter *ch ) 
{
    switch (state.getValue( )) {
    case QSTAT_INIT:
        buf << "Тебе нужно попасть в " << kingRoom << " ({hh" << kingArea << "{hx), "
            << "найти там " << russian_case( kingName, '4' ) 
            << " и узнать, какая помощь от тебя требуется." << endl;
        break;

    case QSTAT_MARK_RCVD:
        buf << "Тебе нужно отыскать " << russian_case( princeName, '4' ) 
            << " в местности под названием {hh" << princeArea << "{x." << endl;
        break;
    
    case QSTAT_KID_FOUND:
        buf << "Тебе необходимо отвести " << russian_case( princeName, '4' )
            << " к " << russian_case( kingName, '3' ) << "." << endl
            << "Это находится в " << kingRoom << " ({hh" << kingArea << "{hx)." << endl;
        break;
        
    case QSTAT_KING_ACK_WAITING:
        buf << "Твое задание почти выполнено!" << endl
            << "Вернись к " << russian_case( kingName, '3' ) << " за благодарностью." << endl;
        break;

    case QSTAT_FINISHED:
        buf << "Твое задание выполнено!" << endl
            << "Вернись к тому, кто тебе его дал, до того, как выйдет время!" << endl;
        break;
    }
}

void KidnapQuest::shortInfo( std::ostream &buf, PCharacter *ch )
{
    switch (state.getValue( )) {
    case QSTAT_INIT:
        buf << "Узнать, что случилось у " << russian_case( kingName, '2') << " в "
            << kingRoom << " (" << kingArea << ").";
        break;

    case QSTAT_MARK_RCVD:
        buf << "Найти " << russian_case( princeName, '4' ) << " в "
            << princeArea << ".";
        break;
    
    case QSTAT_KID_FOUND:
        buf << "Отвести " << russian_case( princeName, '4' ) 
            << " к " << russian_case( kingName, '3' ) << " в " << kingRoom << " (" << kingArea << ").";
        break;
        
    case QSTAT_KING_ACK_WAITING:
        buf << "Вернуться к " << russian_case( kingName, '3' ) << " за благодарностью.";
        break;

    case QSTAT_FINISHED:
        buf << "Вернуться к квестору за наградой.";
        break;
    }
}

bool KidnapQuest::isComplete( )
{
    return (state == QSTAT_FINISHED);
}

Room * KidnapQuest::helpLocation( )
{
    return findMobileRoom<KidnapPrince>( );
}

bool KidnapQuest::help( PCharacter *ch, NPCharacter *questman ) 
{
    Room *room = helpLocation( );

    if (state == QSTAT_INIT) {
        tell_fmt( "Извини, но на этом этапе задания тебе придется искать путь само%1$Gму|му|ой.", ch, questman );
        return true;
    }
    else if(!room){
        tell_fmt( "Извини, я сейчас не могу тебе помочь - придется искать путь само%1$Gму|му|ой.", ch, questman );
        wiznet( "find", "failure" );
        return true;
    }

    if (hint.getValue( ) > 5 && !IS_TOTAL_NEWBIE(ch)) {
        if (number_percent( ) < 30)
            tell_fmt( "%1$^C1, тебе необходимо следовать по такому пути: eeeennnwwnewseesennnnnnnnwwnnn.", ch, questman ); 
        else
            tell_fmt( "О, %1$^C1, как же ты меня утоми%1$Gло|л|ла.. Ступай... Ищи са%1$Gмо|м|ма.", ch, questman );
        
        wiznet( "find", "failure" );
        return true;
    }
    if(!IS_TOTAL_NEWBIE(ch))
    tell_raw( ch, questman,  "Я помогу тебе, но награда будет не так велика.");

    if (rated_as_guru( ch ))
        tell_raw( ch, questman, 
             "Последний раз {W%s{G видели неподалеку от {W%s{G.", 
             russian_case( princeName, '4' ).c_str( ),
             room->getName() );
    else
        tell_raw( ch, questman, 
             "Последний раз {W%s{G видели в местности {W{hh%s{hx{G.", 
             russian_case( princeName, '4' ).c_str( ),
             room->areaName() );
     
    hint++;
    wiznet( "find", "success, attempt #%d", hint.getValue( ) );
    return true;
}

void KidnapQuest::destroyBandits( ) 
{
    destroyMobiles<KidnapBandit>( );
}

KidnapQuestRegistrator * KidnapQuest::getReg( ) const
{
    return KidnapQuestRegistrator::getThis( );                         
}

const KidnapScenario & KidnapQuest::getScenario( ) const
{
    return *(getReg()->getMyScenario<KidnapScenario>(scenName));
}

Room * KidnapQuest::findRefuge( PCharacter *hero, NPCharacter *king ) 
{
    RoomList rooms;
    const KidnapScenario &scenario = getScenario( );
    
    if (!scenario.refuges.empty( )) 
        findClientRooms( hero, rooms, scenario.refuges );
    else 
        findClientRooms( hero, rooms );
    
    return getDistantRoom( hero, rooms, king->in_room, 30, 3 );
}

NPCharacter * KidnapQuest::createKing( PCharacter *hero )
{
    NPCharacter *king;

    king = getRandomClient( hero );
    ClientQuestModel::assign<KidnapKing>( king );
    save_mobs( king->in_room );
    return king;
}

NPCharacter * KidnapQuest::createPrince( NPCharacter *king, Room *room )
{
    NPCharacter *kid;

    kid = createMobile<KidnapPrince>( getReg( )->princeVnum );
    getScenario( ).kid.dress( kid, king );
    char_to_room( kid, room );
    return kid;
}

NPCharacter * KidnapQuest::createBandit( )
{
    NPCharacter *bandit;
    Object *canoe;
    
    bandit = createMobile<KidnapBandit>( getReg( )->banditVnum );
    getScenario( ).bandit.dress( bandit );

    canoe = create_object( get_obj_index( OBJ_VNUM_CANOE ), 0 );
    obj_to_char( canoe, bandit );

    return bandit;
}

Object * KidnapQuest::createMark( ) 
{
    Object *mark;
    
    mark = createItem<KidnapMark>( getReg( )->markVnum );
    getScenario( ).mark.dress( mark );
    return mark;
}

bool KidnapQuest::checkMobileClient( PCharacter *pch, NPCharacter *mob )
{
    return getScenario( ).kings.hasElement( mob->pIndexData->vnum )
           && ClientQuestModel::checkMobileClient( pch, mob );
}

bool KidnapQuest::checkRoomClient( PCharacter *pch, Room * room ) 
{
    if (!ClientQuestModel::checkRoomClient( pch, room ))
        return false;
        
    if (IS_WATER(room) || room->getSectorType() == SECT_AIR)
        return false;
    
    if (!kingArea.empty( ) && kingArea == room->areaName())
        return false;

    return true;
}

