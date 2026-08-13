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
#include "msgformatter.h"
#include "merc.h"
#include "act.h"
#include "loadsave.h"
#include "save.h"
#include "def.h"
#include "l10n.h"

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

    // Capture the name per language so info() answers in the reader's own.
    // What lands in a slot is firstNonEmpty(instance, prototype, lang), so an
    // untranslated language yields either Russian or nothing; getForLang() on
    // read covers both and nothing can render blank.
    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;

        raceRusName[lang] = pGameIndex->short_descr.getForLang( lang );
        areaName[lang] = pGameIndex->area->getName( lang, '1' );
    }
    
    if (Player::isNewbie(pch))
        ordered = URANGE( 1, games[pGameIndex].size( ) * 3 / 2, 6 );
    else 
        ordered = URANGE( 4, games[pGameIndex].size( ) * 3 / 2, 12 );
        
    customer = getRandomClient( pch );
    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;

        customerName[lang] = customer->getNameP( '1', lang ).upperFirstCharacter( );
        customerArea[lang] = customer->in_room->areaName( lang );
    }
    assign<SteakCustomer>( customer );
    save_mobs( customer->in_room );

    time = number_range( 15, 25 );
    setTime( pch, time );

    tell_raw( pch, questman, _("У меня есть для тебя срочное поручение!") );
    lang_t plang = viewerLang( pch );
    tell_raw( pch, questman,
        _("{W%1$s{G из местности {W{hh%2$s{hx{G хочет подать к столу {W%3$d{G кус%3$Iок|ка|ков мяса {W%4$s{G из местности {W{hh%5$s{hx{G."),
        customerName.getForLang( plang ).c_str( ),
        customerArea.getForLang( plang ).c_str( ),
        ordered.getValue( ),
        raceRusName.getForLang( plang ).ruscase( '2' ).c_str( ),
        areaName.getForLang( plang ).c_str( ));

    tell_raw( pch, questman, _("Доставь мясо заказчику и вернись сюда за вознаграждением.") );
    tell_raw( pch, questman, _("У тебя есть {Y%1$d{G минут%1$Iа|ы| на выполнение задания."), time ); 

    wiznet( "", "%d steaks of %s from %s, customer %s.",
                ordered.getValue( ),
                raceName.c_str( ),
                areaName.get( LANG_DEFAULT ).c_str( ),
                customerName.get( LANG_DEFAULT ).c_str( ) );
}

bool ButcherQuest::isComplete( ) 
{
    return (delivered.getValue( ) >= ordered.getValue( ));
}

void ButcherQuest::info( std::ostream &buf, PCharacter *ch ) 
{
    if (isComplete( )) {
        infoComplete( buf, ch );
        return;
    }

    lang_t lang = viewerLang( ch );
    buf << fmt( ch, _("%1$s из {hh%2$s{hx просит тебя доставить к столу %3$d кус%3$Iок|ка|ков мяса %4$s из местности {hh%5$s{hx."),
                customerName.getForLang( lang ).c_str( ),
                customerArea.getForLang( lang ).c_str( ),
                ordered.getValue( ),
                raceRusName.getForLang( lang ).ruscase( '2' ).c_str( ),
                areaName.getForLang( lang ).c_str( ) ) << endl;

    if (delivered > 0)
        buf << fmt( ch, _("Доставлено кусков: %1$d."), delivered.getValue( ) ) << endl;
}

void ButcherQuest::shortInfo( std::ostream &buf, PCharacter *ch )
{
    if (isComplete( )) {
        shortInfoComplete( buf, ch );
        return;
    }

    lang_t lang = viewerLang( ch );
    buf << fmt( ch, _("%1$s из %2$s заказал %3$d кус%3$Iок|ка|ков мяса %4$s из %5$s."),
                customerName.getForLang( lang ).c_str( ),
                customerArea.getForLang( lang ).c_str( ),
                ordered.getValue( ),
                raceRusName.getForLang( lang ).ruscase( '2' ).c_str( ),
                areaName.getForLang( lang ).c_str( ) );
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

