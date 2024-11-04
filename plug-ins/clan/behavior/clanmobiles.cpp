/* $Id$
 *
 * ruffina, 2004
 */
#include "clanmobiles.h"
#include "clanreference.h"
#include "skillreference.h"
#include "occupations.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"

#include "dreamland.h"
#include "fight.h"
#include "magic.h"
#include "damage.h"
#include "infonet.h"
#include "messengers.h"
#include "merc.h"
#include "loadsave.h"

#include "interp.h"
#include "act.h"
#include "arg_utils.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"

#include "def.h"

CLAN(battlerager);
CLAN(none);
CLAN(flowers);
GSN(armor);
GSN(bless);
GSN(blindness);
GSN(cure_blindness);
GSN(cure_disease);
GSN(cure_poison);
GSN(heal);
GSN(plague);
GSN(poison);
GSN(refresh);
GSN(remove_curse);
GSN(warcry);


/*--------------------------------------------------------------------------
 * Clan Mobile (base class) 
 *-------------------------------------------------------------------------*/
ClanArea::Pointer ClanMobile::getClanArea( )
{
    ClanArea::Pointer clanArea;
    AreaIndexData *area;

    area = getChar( )->pIndexData->area;

    if (area->behavior) 
        clanArea = area->behavior.getDynamicPointer<ClanArea>( );

    return clanArea;
}

void ClanMobile::setChar( NPCharacter *mob )
{
    MobileBehavior::setChar( mob );
    ch->setClan( clan->getName( ) );
}

void ClanMobile::unsetChar( )
{
    ch->setClan( clan_none );
    MobileBehavior::unsetChar( );
}

/*--------------------------------------------------------------------------
 * Clan Guard 
 *-------------------------------------------------------------------------*/
ClanGuard::ClanGuard( )
{
}

bool ClanGuard::death( Character *killer )
{
    if (!killer || killer->is_immortal())
        return false;

    ClanArea::Pointer clanArea = getClanArea( );
    if (!clanArea)
        return false;

    DLString what = clanArea->getClan()->getRussianName( ).ruscase('3') + " {Wне удалось удержать оборону.{x";
    infonet(0, 0, "{CТихий голос из $o2: ", what.c_str());
    send_discord_clan(what);
    send_telegram(what);

    return false;
}

/*--------------------------------------------------------------------------
 * clan guard aggression
 *-------------------------------------------------------------------------*/
bool ClanGuard::aggress( )
{
    Character *rch, *rch_next;
    ClanArea::Pointer clanArea;

    clanArea = getClanArea( );

    if (!clanArea)
        return false;
        
    for (rch = ch->in_room->people; rch; rch = rch_next) {
        PCharacter *pch = rch->getPC( );
        
        rch_next = rch->next_in_room;

        if (rch->is_npc( ))
            continue;
        if (rch->is_immortal( ))
            continue;
        if (rch->getClan( ) == clan)
            continue;
        if (rch->fighting)
            continue;
        if (clanArea->findInvitation( pch ))
            continue;
        if (checkPush( pch ) || checkGhost( pch ))
            continue;
        
        actIntruder( pch );
        doNotify();

        try {
            doAttack( pch );
        } catch (const VictimDeathException &) {
        }
    }

    return true;
}

/*--------------------------------------------------------------------------
 * clan guard greet program
 *-------------------------------------------------------------------------*/
void ClanGuard::greet( Character *wch )
{
    Object *obj;
    PCharacter *pch;
    ClanArea::Pointer clanArea;

    if (wch->is_npc( ) || wch->is_immortal( ))
        return;
        
    clanArea = getClanArea( );

    if (!clanArea)
        return;

    pch = wch->getPC( );

    if (pch->getClan( ) == clan) {
        actGreet( pch );
        return;
    }
    
    if (( obj = clanArea->findInvitation( pch ) )) {
        actInvited( pch, obj );
        return;
    }
    
    if (checkPush( pch ))
        return;
    
    if (checkGhost( pch ))
        return;
    
    actIntruder( pch );
    doNotify();
    doAttack( pch );
}

void ClanGuard::doNotify()
{
    time_t now = dreamland->getCurrentTime();
    if (lastNotified > now - Date::SECOND_IN_MINUTE)
        return;

    ClanArea::Pointer clanArea = getClanArea();
    DLString msg = "Территория " + clanArea->getClan()->getRussianName( ).ruscase('2') + "{x атакована!";
    send_discord_clan(msg);
    send_telegram(msg);
    lastNotified = now;
}

bool ClanGuard::checkPush( PCharacter* wch ) 
{
    Room * location;

    if (!IS_SET(wch->act, PLR_CONFIRMED)
        || (wch->getClan( ) != ch->getClan( ) && wch->getRealLevel( ) <= 50)
        || (wch->getClan( ) != ch->getClan( ) && !dreamland->hasOption( DL_PK ))
        || wch->getClan( ) == clan_flowers)
    {
        actPush( wch );

        if (!( location = get_room_instance( wch->getHometown( )->getRecall( ) ) )) 
            location = get_room_instance( 1 );
        
        transfer_char( wch, ch, location,
                       NULL, NULL, "%1$^C1 внезапно появляется." );
        return true;
    }

    return false;
}

bool ClanGuard::checkGhost( PCharacter *wch ) 
{
    Room * location;

    if (IS_SLAIN( wch ) || IS_DEATH_TIME( wch )) {
        actGhost( wch );

        if (!( location = get_room_instance( wch->getHometown( )->getRecall( ) ) )) 
            location = get_room_instance( 1 );
        
        transfer_char( wch, ch, location,
                       NULL, NULL, "%1$^C1 внезапно появляется." );
        return true;
    }

    return false;
}

void ClanGuard::actGhost( PCharacter *wch )
{
    do_say( ch, "Призраки не имеют права входить сюда." );
}

void ClanGuard::actIntruder( PCharacter *wch )
{
    interpret_raw( ch, "cb", "Внимание, обнаружены лазутчики! Посторонним вход запрещен!" );
}

void ClanGuard::doAttack( PCharacter *wch )
{
    if (wch->is_immortal( ))
        return;

    switch (ch->position.getValue( )) {
    case POS_FIGHTING:
        SET_BIT( ch->off_flags, OFF_AREA_ATTACK );
        one_hit_nocatch( ch, wch );
        break;
    default:
        multi_hit_nocatch( ch, wch );
        break;
    }
}

void ClanGuard::actInvited( PCharacter *wch, Object *obj )
{
    do_say( ch, "Тебя пригласили -- только это оправдывает твое присутствие здесь!" );
}

void ClanGuard::actPush( PCharacter *wch )
{
}

void ClanGuard::actGreet( PCharacter *wch )
{
}

/*--------------------------------------------------------------------------
 * clan guard speech programm 
 *-------------------------------------------------------------------------*/
void ClanGuard::speech( Character *wch, const char *cspeech )
{
    DLString speech = cspeech;
    Object * obj;
    ClanArea::Pointer clanArea = getClanArea( );
    
    if (!clanArea)
        return;

    if (wch->is_npc( ))
        return;
            
    obj = NULL;

    if (speech ^ "I need key") {
        if (clanArea->keyVnum > 0) {
            if (wch->getClan( ) != ch->getClan( )) {
                do_say(ch, "ЭТОГО ты не получишь!");
                return;
            }

            obj = create_object(get_obj_index(clanArea->keyVnum), 0);
            obj->timer = 120;
            oldact("$c1 снимает с шеи $o4.", ch, obj, 0, TO_ROOM );
            oldact("Ты снимаешь с шеи $o4.", ch, obj, 0, TO_CHAR );
        }
    }
    else if (speech ^ "I need invitation" || speech ^ "Мне нужно приглашение") {
        if (clanArea->invitationVnum > 0) {
            if (wch->getClan( ) != ch->getClan( )) {
                do_say(ch, "Я не обязан приглашать тебя в свой клан!");
                return;
            }

            obj = create_object(get_obj_index( clanArea->invitationVnum ), 0);
            obj->timer = 15;
            actGiveInvitation( wch->getPC( ), obj );            
        }
    }
    else if (speech ^ "I need book") {
        if (clanArea->bookVnum > 0) {
            if (wch->getClan( ) != ch->getClan( )) {
                do_say(ch, "Эта книга не для твоих глаз!");
                return;
            }
            
            if (!wch->getClan( )->isRecruiter( wch->getPC( ) )) {
                do_say(ch, "Эту книгу я отдам только руководителям клана!");
                return;
            }

            obj = create_object(get_obj_index( clanArea->bookVnum ), 0);
        }
    }

    if (obj) {
        oldact("$C1 дает $o4 $c3.", wch, obj, ch, TO_ROOM );
        oldact("$C1 дает тебе $o4.", wch, obj, ch, TO_CHAR );
        obj_to_char(obj, wch);
    }
}

void ClanGuard::actGiveInvitation( PCharacter *wch, Object *obj )
{
    oldact("$c1 пишет на $o6.", ch, obj, 0, TO_ROOM );
    oldact("Ты пишешь на $o6.", ch, obj, 0, TO_CHAR );
}

/*--------------------------------------------------------------------------
 * clan guard special programm 
 *-------------------------------------------------------------------------*/
bool ClanGuard::specFight( )
{
    Character *victim;

    if (ch->wait > 0)
        return false;

    if (!( victim = getVictim( ) ))
        return true;
    
    spec_cast( victim );
    return true;
}

bool ClanGuard::spec_cast( Character *victim )
{
    return ::spell( getCast( victim ), 
                    ch->getRealLevel( ), 
                    ch, 
                    victim, 
                    FSPELL_VERBOSE );
}

int ClanGuard::getCast( Character * )
{
    return -1;
}

Character * ClanGuard::getVictim( ) 
{
    Character *victim, *v_next;

    for (victim = ch->in_room->people; victim; victim = v_next) {
        v_next = victim->next_in_room;

        if (victim->fighting == ch && number_bits( 1 ) == 0)
            return victim;
    }

    return NULL;
}

bool ClanGuard::specAdrenaline( )
{
    return false;
}

int ClanGuard::getOccupation( )
{
    return BasicMobileDestiny::getOccupation( ) 
            | (1 << OCC_PRACTICER)
            | (1 << OCC_CLANGUARD);
}

/*--------------------------------------------------------------------------
 * Clan summoned creatures 
 *-------------------------------------------------------------------------*/
ClanSummonedCreature::~ClanSummonedCreature( )
{
}

