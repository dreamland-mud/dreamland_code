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
#include "msgformatter.h"
#include "loadsave.h"
#include "act.h"
#include "save.h"

#include "def.h"
#include "l10n.h"

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

        // Capture every name per language so info() answers in the reader's own;
        // getForLang() falls back to Russian where a language has none.
        // customerArea has to be filled before getRandomRoomClient() below, which
        // reads it back through checkRoomClient to keep the errand out of the
        // customer's own zone.
        for (int l = LANG_MIN; l < LANG_MAX; l++) {
            lang_t lang = (lang_t)l;

            customerName[lang] = customer->getShortDescr( lang );
            customerRoom[lang] = customer->in_room->getName( lang );
            customerArea[lang] = customer->in_room->areaName( lang );
        }

        if (getScenario( ).needsEndPoint( )) {
            endPoint = getRandomRoomClient( pch );

            for (int l = LANG_MIN; l < LANG_MAX; l++)
                targetArea[(lang_t)l] = endPoint->areaName( (lang_t)l );
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

    tell_fmt( _("{W%3$#^C1{G хочет отыскать некоторые принадлежащие %3$P3 вещи."),  
              pch, questman, customer );
    // The frame is translated per recipient, but these two names are resolved at
    // call time -- so they need the recipient's language explicitly, or a UA
    // player is told about a room in Russian inside a Ukrainian sentence.
    lang_t plang = viewerLang( pch );

    tell_fmt( _("%3$#^P1 ждет тебя в районе {W%4$s{G ({W{hh%5$s{hx{G)."),
               pch, questman, customer,
               customer->in_room->getName( plang ),
               customer->in_room->areaName( plang ).c_str( ) );
    tell_fmt( _("У тебя есть {Y%3$d{G мину%3$Iта|ты|т, чтобы добраться туда и узнать подробности."),
               pch, questman, time );

    // Wiznet is an immortal channel and deliberately stays Russian.
    wiznet( scenName.getValue( ).c_str( ),
            "customer [%s], item [%s], count %d, path from [%d] to [%d]",
            customer->getNameP( '1' ).c_str( ),
            russian_case( itemName.get( LANG_DEFAULT ), '1' ).c_str( ),
            total.getValue( ),
            customer->in_room->vnum, (endPoint ? endPoint->vnum : 0) );
}

bool LocateQuest::isComplete( ) 
{
    return state == QSTAT_FINISHED;
}

void LocateQuest::info( std::ostream &buf, PCharacter *ch )
{
    lang_t lang = viewerLang( ch );

    switch (state.getValue( )) {
    case QSTAT_INIT:
        buf << fmt( ch, _("%1$s хочет отыскать кое-какие свои вещи."),
                    russian_case( customerName.getForLang( lang ), '1' ).c_str( ) ) << endl
            << fmt( ch, _("Тебя с нетерпением ждут в районе %1$s."),
                    customerRoom.getForLang( lang ).c_str( ) ) << endl
            << fmt( ch, _("Это находится в местности под названием {hh%1$s{hx."),
                    customerArea.getForLang( lang ).c_str( ) ) << endl;
        break;
    case QSTAT_SEARCH:
        getScenario( ).getLegend( ch, this, buf );

        if (delivered > 0)
            buf << fmt( ch, _("Тобой уже доставлено {Y%1$d{x из них."),
                        delivered.getValue( ) ) << endl;

        buf << fmt( ch, _("Заказчик ждет тебя в районе %1$s."),
                    customerRoom.getForLang( lang ).c_str( ) ) << endl
            << fmt( ch, _("Это находится в местности под названием {hh%1$s{hx."),
                    customerArea.getForLang( lang ).c_str( ) ) << endl;

        break;
    case QSTAT_FINISHED:
        // Same two sentences the base class already carries, word for word.
        infoComplete( buf, ch );
        break;
    default:
        break;
    }
}

void LocateQuest::shortInfo( std::ostream &buf, PCharacter *ch )
{
    lang_t lang = viewerLang( ch );

    switch (state.getValue( )) {
    case QSTAT_INIT:
        buf << fmt( ch, _("Помочь %1$s из %2$s (%3$s) отыскать свои вещи."),
                    russian_case( customerName.getForLang( lang ), '3' ).c_str( ),
                    customerRoom.getForLang( lang ).c_str( ),
                    customerArea.getForLang( lang ).c_str( ) );
        break;
    case QSTAT_SEARCH:
        // The count suffix moves into the frame as %1$I so each language brings
        // its own plural rule, instead of Russian endings glued onto a
        // translated word.
        buf << fmt( ch, _("Найти %1$d штук%1$Iу|и| %2$s для %3$s из %4$s (%5$s)."),
                    total.getValue( ),
                    russian_case( itemMltName.getForLang( lang ), '2' ).c_str( ),
                    russian_case( customerName.getForLang( lang ), '2' ).c_str( ),
                    customerRoom.getForLang( lang ).c_str( ),
                    customerArea.getForLang( lang ).c_str( ) );
        break;
    case QSTAT_FINISHED:
        shortInfoComplete( buf, ch );
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
    // Both sides pinned to Russian on purpose: this is a comparison, not display
    // text. Letting either side follow the viewer would make the same room match
    // or not depending on who is reading.
    if (!customerArea.emptyValues( ) && customerArea.get( LANG_DEFAULT ) == room->areaName( ))
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

    // getForLang() mirrors Russian into any slot the scenario data has not
    // translated yet, which is what the reader needs: an empty slot would fall
    // through to the bare prototype instead.
    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;

        itemName[lang] = itemScen.shortDesc.getForLang( lang );
        itemMltName[lang] = itemScen.shortMlt.getForLang( lang );
    }

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

