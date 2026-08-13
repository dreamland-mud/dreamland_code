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

#include "player_utils.h"
#include "roomutils.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "msgformatter.h"
#include "merc.h"
#include "loadsave.h"
#include "save.h"
#include "act.h"
#include "def.h"
#include "l10n.h"

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
        room = findRefuge( pch, king );
        prince = createPrince( king, room );

    // Capture the name per language so info() answers in the reader's own;
    // getForLang() on read falls back to Russian where a language has none.
        for (int l = LANG_MIN; l < LANG_MAX; l++) {
            lang_t lang = (lang_t)l;

            kingRoom[lang] = king->in_room->getName( lang );
            kingArea[lang] = king->in_room->areaName( lang );
            kingName[lang] = king->getShortDescr( lang );
            princeArea[lang] = room->areaName( lang );
            princeRoom[lang] = room->getName( lang );
            princeName[lang] = prince->getShortDescr( lang );
        }

    } catch (const QuestCannotStartException &e) {
        destroy( );
        throw e;
    }
    
    time = std::max( 6, range / 10 );

    if (Player::isNewbie( pch ))
        time *= 2;

    setTime( pch, time );
    
    getScenario( ).onQuestStart( pch, questman, king );
    tell_raw( pch, questman, _("У тебя есть {Y%d{G минут%s, чтобы добраться туда и узнать, в чем дело."),
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

QuestReward::Pointer KidnapQuest::reward( PCharacter *ch, NPCharacter *questman ) 
{
    QuestReward::Pointer r( NEW );
    
    r->points = number_range( 18, 25 );
    r->points += ambushes * 25;

    if (!Player::isNewbie(ch)) {
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
    return r;
}

void KidnapQuest::info( std::ostream &buf, PCharacter *ch ) 
{
    lang_t lang = viewerLang( ch );

    switch (state.getValue( )) {
    case QSTAT_INIT:
        buf << fmt( ch, _("Тебе нужно попасть в %1$s ({hh%2$s{hx), найти там %3$N4 и узнать, какая помощь от тебя требуется."),
                    kingRoom.getForLang( lang ).c_str( ),
                    kingArea.getForLang( lang ).c_str( ),
                    kingName.getForLang( lang ).c_str( ) ) << endl;
        break;

    case QSTAT_MARK_RCVD:
        buf << fmt( ch, _("Тебе нужно отыскать %1$N4 в местности под названием {hh%2$s{x."),
                    princeName.getForLang( lang ).c_str( ),
                    princeArea.getForLang( lang ).c_str( ) ) << endl;
        break;
    
    case QSTAT_KID_FOUND:
        buf << fmt( ch, _("Тебе необходимо отвести %1$N4 к %2$N3."),
                    princeName.getForLang( lang ).c_str( ),
                    kingName.getForLang( lang ).c_str( ) ) << endl
            << fmt( ch, _("Это находится в %1$s ({hh%2$s{hx)."),
                    kingRoom.getForLang( lang ).c_str( ),
                    kingArea.getForLang( lang ).c_str( ) ) << endl;
        break;
        
    case QSTAT_KING_ACK_WAITING:
        buf << fmt( ch, _("Твое задание почти выполнено!") ) << endl
            << fmt( ch, _("Вернись к %1$N3 за благодарностью."),
                    kingName.getForLang( lang ).c_str( ) ) << endl;
        break;

    case QSTAT_FINISHED:
        buf << fmt( ch, _("Твое задание выполнено!") ) << endl
            << fmt( ch, _("Вернись к тому, кто тебе его дал, до того, как выйдет время!") ) << endl;
        break;
    }
}

void KidnapQuest::shortInfo( std::ostream &buf, PCharacter *ch )
{
    lang_t lang = viewerLang( ch );

    switch (state.getValue( )) {
    case QSTAT_INIT:
        buf << fmt( ch, _("Узнать, что случилось у %1$N2 в %2$s (%3$s)."),
                    kingName.getForLang( lang ).c_str( ),
                    kingRoom.getForLang( lang ).c_str( ),
                    kingArea.getForLang( lang ).c_str( ) );
        break;

    case QSTAT_MARK_RCVD:
        buf << fmt( ch, _("Найти %1$N4 в %2$s."),
                    princeName.getForLang( lang ).c_str( ),
                    princeArea.getForLang( lang ).c_str( ) );
        break;
    
    case QSTAT_KID_FOUND:
        buf << fmt( ch, _("Отвести %1$N4 к %2$N3 в %3$s (%4$s)."),
                    princeName.getForLang( lang ).c_str( ),
                    kingName.getForLang( lang ).c_str( ),
                    kingRoom.getForLang( lang ).c_str( ),
                    kingArea.getForLang( lang ).c_str( ) );
        break;
        
    case QSTAT_KING_ACK_WAITING:
        buf << fmt( ch, _("Вернуться к %1$N3 за благодарностью."),
                    kingName.getForLang( lang ).c_str( ) );
        break;

    case QSTAT_FINISHED:
        shortInfoComplete( buf, ch );
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
        tell_fmt( _("Извини, но на этом этапе задания тебе придется искать путь само%1$Gму|му|ой."), ch, questman );
        return true;
    }
    else if(!room){
        tell_fmt( _("Извини, я сейчас не могу тебе помочь - придется искать путь само%1$Gму|му|ой."), ch, questman );
        wiznet( "find", "failure" );
        return true;
    }

    if (hint.getValue( ) > 5 && !Player::isNewbie(ch)) {
        if (number_percent( ) < 30)
            tell_fmt( _("%1$^C1, тебе необходимо следовать по такому пути: eeeennnwwnewseesennnnnnnnwwnnn."), ch, questman ); 
        else
            tell_fmt( _("О, %1$^C1, как же ты меня утоми%1$Gло|л|ла.. Ступай... Ищи са%1$Gмо|м|ма."), ch, questman );
        
        wiznet( "find", "failure" );
        return true;
    }

    if (!Player::isNewbie(ch))
        tell_raw( ch, questman,  _("Я помогу тебе, но награда будет не так велика."));

    lang_t hlang = viewerLang( ch );
    tell_raw( ch, questman,
            _("Последний раз {W%s{G видели в местности {W{hh%s{hx{G."),
            russian_case( princeName.getForLang( hlang ), '4' ).c_str( ),
            room->areaName( hlang ).c_str() );
     
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
        
    if (RoomUtils::isWaterOrAir(room))
        return false;
    
    // Both sides stay Russian: this drives room selection, not display.
    if (!kingArea.emptyValues( ) && kingArea.get( LANG_DEFAULT ) == room->areaName())
        return false;

    return true;
}

