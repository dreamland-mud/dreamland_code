/* $Id: staffquest.cpp,v 1.1.2.24.6.4 2009/01/01 14:13:18 rufina Exp $
 *
 * ruffina, 2003
 */

#include "staffquest.h"
#include "questexceptions.h"
#include "staffbehavior.h"

#include "player_utils.h"

#include "pcharacter.h"
#include "object.h"
#include "room.h"
#include "roomutils.h"
#include "loadsave.h"
#include "merc.h"
#include "msgformatter.h"
#include "act.h"
#include "def.h"
#include "l10n.h"

/*
 * StaffQuest
 */
void StaffQuest::create( PCharacter *pch, NPCharacter *questman ) 
{
    Object *eyed;
    int time;

    charName = pch->getName( );

    try {
        scenName = StaffQuestRegistrator::getThis( )->getRandomScenario( pch );
        eyed = createStaff( getRandomRoomClient( pch ) );
    } 
    catch (const QuestCannotStartException &e) {
        destroy( );
        throw e;
    }

    // Capture the name per language so info() answers in the reader's own.
    // What lands in a slot is firstNonEmpty(instance, prototype, lang), so an
    // untranslated language yields either Russian (a dressed item, whose slots
    // PR #982 mirrored) or nothing. getForLang() on read covers both, and
    // nothing can render blank.
    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;

        areaName[lang] = eyed->in_room->areaName( lang );
        roomName[lang] = eyed->in_room->getName( lang );
        objName[lang] = eyed->getShortDescr( lang );
    }
    
    time = number_range( 15, 25 ); 
    setTime( pch, time );
    
    getScenario( ).onQuestStart( pch, questman );
    tell_raw( pch, questman, _("Придворные волшебники определили, где спрятано украденное сокровище.") );
    tell_raw( pch, questman, _("Тебе поручается доставить его мне!") );        
    lang_t plang = viewerLang( pch );
    tell_raw( pch, questman, _("Место, где оно спрятано, называется {W%s{G"), roomName.getForLang( plang ).c_str( ) );
    tell_raw( pch, questman, _("И находится это место в районе под названием - {W{hh%s{hx{G"), areaName.getForLang( plang ).c_str( ) );
    tell_raw( pch, questman, _("У тебя есть {Y%1$d{G минут%1$Iа|ы| на выполнение задания."), time ); 
    
    wiznet( scenName.c_str( ), "in room \"%s\" area \"%s\"",
                               roomName.get( LANG_DEFAULT ).c_str( ),
                               areaName.get( LANG_DEFAULT ).c_str( ) );
}

bool StaffQuest::isComplete( ) 
{
    PCharacter *ch = getHeroWorld( );
    
    if (!ch)
        return false;

    return getItemList<StaffBehavior>( ch->carrying ) != NULL;
}

Room * StaffQuest::helpLocation( )
{
    Object *obj = getItemWorld<StaffBehavior>( );
    
    return (obj ? obj->in_room : NULL);
}

void StaffQuest::info( std::ostream &buf, PCharacter *ch ) 
{
    if (isComplete( )) {
        infoComplete( buf, ch );
        return;
    }

    lang_t lang = viewerLang( ch );
    buf << fmt( ch, _("У тебя задание - вернуть %1$s."),
                russian_case( objName.getForLang( lang ), '4' ).c_str( ) ) << endl
        << fmt( ch, _("Место, где спрятано сокровище, называется %1$s."),
                roomName.getForLang( lang ).c_str( ) ) << endl
        << fmt( ch, _("И находится это место в районе под названием {hh%1$s{hx."),
                areaName.getForLang( lang ).c_str( ) ) << endl;
}

void StaffQuest::shortInfo( std::ostream &buf, PCharacter *ch )
{
    if (isComplete( )) {
        shortInfoComplete( buf, ch );
        return;
    }

    lang_t lang = viewerLang( ch );
    buf << fmt( ch, _("Доставить квестору %1$s из %2$s (%3$s)."),
                russian_case( objName.getForLang( lang ), '4' ).c_str( ),
                roomName.getForLang( lang ).c_str( ),
                areaName.getForLang( lang ).c_str( ) );
}

QuestReward::Pointer StaffQuest::reward( PCharacter *ch, NPCharacter *questman ) 
{
    QuestReward::Pointer r( NEW );

    if (hint.getValue( ) && !Player::isNewbie(ch)) {
        r->gold = number_range( 1, 2 );
        r->points = number_range( 1, 4 );
    }
    else {
        r->gold = number_range( 5, 10 );
        r->points = number_range( 5, 10 );
        r->wordChance = 2 * r->points;
        r->scrollChance = number_range( 3, 7 );

        if (chance( 10 ))
            r->prac = 1;
    }
    

    if (ch->getClan( )->isDispersed( )) 
        r->points *= 2;
    else
        r->clanpoints = r->points;

    r->exp = (r->points + r->clanpoints) * 10;

    // $w instead of $n4: the room broadcast reaches viewers of every language,
    // and a plain text arg would show all of them the actor-side Russian. The
    // accusative each language needs is picked here, once per slot, because $w
    // substitutes a finished word rather than declining one. The three strings
    // must outlive the synchronous oldact calls below.
    DLString objEn = objName.getForLang( LANG_EN ).ruscase( '4' );
    DLString objRu = objName.getForLang( LANG_RU ).ruscase( '4' );
    DLString objUa = objName.getForLang( LANG_UA ).ruscase( '4' );
    LangText objText = { objEn.c_str( ), objRu.c_str( ), objUa.c_str( ) };

    oldact(_("Ты передаешь $w $C3."), ch, &objText, questman, TO_CHAR);
    oldact(_("$c1 передает $w $C3."), ch, &objText, questman, TO_ROOM);

    return r;
}

void StaffQuest::destroy( ) 
{
    destroyItem<StaffBehavior>( );
}

bool StaffQuest::checkRoomClient( PCharacter *pch, Room *room )
{
    if (room->areaIndex()->high_range + 20 < pch->getModifyLevel( ))
        return false;

    if (RoomUtils::isWaterOrAir(room))
        return false;

    if (!ItemQuestModel::checkRoomClient( pch, room ))
        return false;

    return true;
}

const StaffScenario & StaffQuest::getScenario( ) const
{
    return *(StaffQuestRegistrator::getThis( )->getMyScenario<StaffScenario>( scenName ));
}

Object * StaffQuest::createStaff( Room *room )
{
    Object *eyed;
    
    eyed = createItem<StaffBehavior>( StaffQuestRegistrator::getThis( )->objVnum );
    getScenario( ).dress( eyed );
    obj_to_room( eyed, room );
    return eyed;
}

/*
 * StaffScenario
 */
bool StaffScenario::applicable( PCharacter *pch )  const
{
    return true;
}

void StaffScenario::onQuestStart( PCharacter *pch, NPCharacter *questman ) const
{
    if (msg.empty( ))
        tell_raw( pch, questman, _("Из королевской сокровищницы похитили {W%s{G!"),
                  shortDesc.getForLang( viewerLang( pch ) ).ruscase( '4' ).c_str( ) );
    else
        tell_raw( pch, questman, msg.c_str( ) );
}

/*
 * StaffQuestRegistrator
 */
StaffQuestRegistrator * StaffQuestRegistrator::thisClass = NULL;

StaffQuestRegistrator::StaffQuestRegistrator( )
{
    thisClass = this;
}

StaffQuestRegistrator::~StaffQuestRegistrator( )
{
    thisClass = NULL;
}

