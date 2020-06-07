/* $Id$
 *
 * ruffina, 2004
 */
#include "clanmobiles.h"
#include "clanreference.h"

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
#include "handler.h"
#include "gsn_plugin.h"
#include "interp.h"
#include "act.h"
#include "arg_utils.h"
#include "act_move.h"
#include "mercdb.h"
#include "def.h"

CLAN(battlerager);
CLAN(none);


/*--------------------------------------------------------------------------
 * Clan Mobile (base class) 
 *-------------------------------------------------------------------------*/
ClanArea::Pointer ClanMobile::getClanArea( )
{
    ClanArea::Pointer clanArea;
    AREA_DATA *area;

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
 * Clan Healer 
 *-------------------------------------------------------------------------*/
ClanHealer::ClanHealer( ) 
{
}

bool ClanHealer::specIdle( )
{
    Character *victim = 0;
    int count = 0;
    int sn;
    
    if (Healer::specIdle( ))
        return true;

    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room) {
        if (rch == ch)
            continue;
        if (!ch->can_see( rch ))
            continue;
        if (rch->is_npc( ) && !IS_CHARMED(rch))
            continue;

        if (!rch->is_npc( )) {
            if (rch->getClan( ) != clan)
                continue;
        } else {
            if (!healPets)
                continue;
            if (!rch->master || rch->master->is_npc( ))
                continue;
            if (rch->master->getClan( ) != clan)
                continue;
        }
        
        if (number_range( 0, count++ ) == 0) 
            victim = rch;
    }

    if (victim == 0)
        return false;
    
    sn = -1;        
    if (victim->isAffected(gsn_plague ))
        sn = gsn_cure_disease;
    else if (victim->isAffected(gsn_blindness ))
        sn = gsn_cure_blindness;
    else if (victim->isAffected(gsn_poison ))
        sn = gsn_cure_poison;
    else if (IS_AFFECTED( victim, AFF_CURSE ))
        sn = gsn_remove_curse;
    else if (!victim->isAffected(gsn_armor ))
        sn = gsn_armor;
    else if (!victim->isAffected(gsn_bless ) && !victim->isAffected(gsn_warcry ))
        sn = gsn_bless;
    else if ((victim->hit < victim->max_hit) && (victim->move < victim->max_move))
    {
        if (number_percent( ) < 50)
            sn = gsn_heal;
        else
            sn = gsn_refresh; 
    } 
    else if (victim->hit < victim->max_hit)
        sn = gsn_heal;
    else if (victim->move < victim->max_move)
        sn = gsn_refresh; 
    
    return ::spell( sn, ch->getModifyLevel( ), ch, victim, FSPELL_VERBOSE|FSPELL_BANE );
}

bool ClanHealer::canServeClient( Character *client )
{
    if (!Healer::canServeClient( client ))
        return false;
    
    if ((!client->is_npc( ) && client->getClan( ) != clan)
        || (client->is_npc( ) && (!client->master 
                                  || client->master->is_npc( )
                                  || client->master->getClan( ) != clan)))
    {
        say_act( client, ch, "Я не буду помогать тебе." );
        return false;
    }
    
    return true;
}

int ClanHealer::getOccupation( )
{
    return Healer::getOccupation( ) | (1 << OCC_PRACTICER);
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

    DLString what = fmt(0, "{WКлан %s не смог удержать оборону.{x", clanArea->getClan()->getShortName().c_str());
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
        doPetitionOutsider( pch );

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
//        extract_obj(obj);
        return;
    }
    
    if (checkPush( pch ))
        return;
    
    if (checkGhost( pch ))
        return;
    
    actIntruder( pch );
    doNotify();
    doPetitionOutsider( pch );
    doAttack( pch );
}

void ClanGuard::doNotify()
{
    time_t now = dreamland->getCurrentTime();
    if (lastNotified > now - Date::SECOND_IN_MINUTE)
        return;

    ClanArea::Pointer clanArea = getClanArea();
    DLString msg = "Клан " + clanArea->getClan()->getShortName() + " атакован!";
    send_discord_clan(msg);
    send_telegram(msg);
    lastNotified = now;
}

bool ClanGuard::checkPush( PCharacter* wch ) 
{
    Room * location;

    if (!IS_SET(wch->act, PLR_CONFIRMED)
        || (wch->getClan( ) == clan_none && wch->getRealLevel( ) <= 80)
        || (wch->getClan( ) != clan_none && wch->getClan( ) != ch->getClan( ) && wch->getRealLevel( ) <= 15)
        || (wch->getClan( ) != ch->getClan( ) && !dreamland->hasOption( DL_PK )))
    {
        actPush( wch );

        if (!( location = get_room_index( wch->getHometown( )->getRecall( ) ) )) 
            location = get_room_index( 1 ); // В limbo потвор
        
        transfer_char( wch, ch, location,
                       NULL, NULL, "%1$^C1 внезапно появляется." );
        return true;
    }

    return false;
}

bool ClanGuard::checkGhost( PCharacter *wch ) 
{
    if (IS_SLAIN( wch ) || IS_DEATH_TIME( wch )) {
        actGhost( wch );
        act( "You slay $C4 in cold blood!", ch, 0, wch, TO_CHAR );
        act( "$c1 slays you in cold blood!", ch, 0, wch, TO_VICT );
        act( "$c1 slays $C4 in cold blood!", ch, 0, wch, TO_NOTVICT );
        raw_kill( wch, -1, 0, FKILL_CRY|FKILL_GHOST|FKILL_CORPSE );
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
    interpret_raw( ch, "cb", "Посторонние... Посторонним вход запрещен!" );
}

void ClanGuard::doPetitionOutsider( PCharacter *wch )
{
    if (wch->getClan( ) == clan_none)
        interpret( wch, "clan petition outsider" );
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
    do_say( ch, "Тебя пригласили - только это оправдывает твое присутствие здесь!" );
//    act( "$C1 забирает $o4 у $c2.", wch, obj, ch, TO_ROOM );
//    act( "$C1 забирает у тебя $o4.", wch, obj, ch, TO_CHAR );
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
void ClanGuard::speech( Character *wch, const char *speech )
{
    Object * obj;
    ClanArea::Pointer clanArea = getClanArea( );
    
    if (!clanArea)
        return;

    if (wch->is_npc( ))
        return;
            
    obj = NULL;

    if (!str_cmp( speech, "I need key" )) {
        if (clanArea->keyVnum > 0) {
            if (wch->getClan( ) != ch->getClan( )) {
                do_say(ch, "ЭТОГО ты не получишь!");
                return;
            }

            obj = create_object(get_obj_index(clanArea->keyVnum), 0);
            obj->timer = 120;
            act( "$c1 снимает с шеи $o4.", ch, obj, 0, TO_ROOM );
            act( "Ты снимаешь с шеи $o4.", ch, obj, 0, TO_CHAR );
        }
    }
    else if (!str_cmp( speech, "I need invitation" ) || !str_cmp(speech, "Мне нужно приглашение")) {
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
    else if (!str_cmp( speech, "I need book" )) {
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
        act( "$C1 дает $o4 $c3.", wch, obj, ch, TO_ROOM );
        act( "$C1 дает тебе $o4.", wch, obj, ch, TO_CHAR );
        obj_to_char(obj, wch);
    }
}

void ClanGuard::actGiveInvitation( PCharacter *wch, Object *obj )
{
    act( "$c1 пишет на $o6.", ch, obj, 0, TO_ROOM );
    act( "Ты пишешь на $o6.", ch, obj, 0, TO_CHAR );
}

/*--------------------------------------------------------------------------
 * clan guard special programm 
 *-------------------------------------------------------------------------*/
bool ClanGuard::specFight( )
{
    Character *victim;

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

