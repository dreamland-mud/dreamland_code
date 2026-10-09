/* $Id$
 *
 * ruffina, 2004
 */
#include "portalmovement.h"
#include "move_utils.h"

#include "feniamanager.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "fenia_utils.h"

#include "skillreference.h"
#include "clanreference.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"
#include "core/object.h"

#include "fight_exception.h"
#include "damage_impl.h"
#include "rageoath.h"
#include "damageflags.h"
#include "interp.h"
#include "act.h"
#include "loadsave.h"
#include "merc.h"

#include "def.h"
#include "l10n.h"


PortalMovement::PortalMovement( Character *ch, Object *portal, Room *leaderRoom )
                  : Walkment( ch )
{
    this->portal = portal;
    this->leaderRoom = leaderRoom;
}

static DLString oprog_portal_location( Object *portal, Character *ch )
{
    FENIA_STR_CALL( portal, "PortalLocation", "C", ch );
    FENIA_NDX_STR_CALL( portal, "PortalLocation", "OC", portal, ch );
    return "";
}

bool PortalMovement::findTargetRoom( )
{
    DLString rc = oprog_portal_location( portal, ch );
    int targetVnum = (rc.empty( ) || !rc.isNumber( ) ? 0 : rc.toInt( ));

    if (targetVnum != 0) {
        to_room = get_room_instance( targetVnum );
    }
    else if (leaderRoom) {
        // Followers land where the leader did, not on a random roll of their own.
        to_room = leaderRoom;
    }
    else if (IS_SET(portal->value2(),GATE_RANDOM) || portal->value3() == -1) {
        to_room = pickRandomRoom( );
        if (to_room)
            portal->value3(to_room->vnum); /* keeps record */
    }
    else if (IS_SET(portal->value2(), GATE_BUGGY) && (number_percent( ) < 5)) {
        to_room = pickRandomRoom( );
    }
    else {
        to_room = get_room_instance( portal->value3() );
    }

    if (to_room == 0 && (IS_SET(portal->value2(), GATE_RANDOM|GATE_BUGGY) || portal->value3() == -1)) {
        msgSelfParty( ch,
                      "%4$^O1 flickers but leads nowhere.",
                      "%4$^O1 мерцает, но никуда не ведет.",
                      "%4$^O1 мерехтить, але нікуди не веде.",
                      "%4$^O1 flickers but leads nowhere.",
                      "%4$^O1 мерцает, но никуда не ведет.",
                      "%4$^O1 мерехтить, але нікуди не веде." );
        return false;
    }

    if (to_room == 0) {
         msgSelfParty( ch,
                       "%4$^O1 не может находиться тут.", 
                       "%4$^O1 не может находиться тут." );
         return false;
    }

    return true;
}

/*
 * A random destination obeys the same rules as the teleport spell: Fenia's
 * onPortalRandomRoom vetoes a roll by returning true. With no handler every
 * roll passes and this is the old get_random_room.
 */
Room * PortalMovement::pickRandomRoom( )
{
    for (int i = 0; i < 10; i++) {
        Room *room = get_random_room( ch );

        if (!gprog( "onPortalRandomRoom", "COR", ch, portal, room ))
            return room;
    }

    return 0;
}

bool PortalMovement::isNormalExit( )
{
    return !IS_SET(portal->value2(), GATE_NOCHECK_EXIT);
}

bool PortalMovement::canLeaveMaster( Character *wch )
{
    return !isNormalExit( ) || Walkment::canLeaveMaster( wch );
}

bool PortalMovement::canMove( Character *wch )
{
    return checkCharges( )
            && Walkment::canMove( wch )
            && checkScripts( wch )
            && checkOath( wch );
}

bool PortalMovement::checkOath( Character *wch )
{
    if (!IS_SET(portal->extra_flags, ITEM_MAGIC) || rage_magic_allowed( wch, RAGE_PORTAL ))
        return true;

    rage_magic_refuse( wch );
    return false;
}

bool PortalMovement::tryMove( Character *wch )
{
    return Walkment::tryMove( wch )
            && applySpellbane( wch );
}

bool PortalMovement::checkCharges( )
{
    return portal->value0() >= 0;
}

int PortalMovement::move( )
{
    if (!moveRecursive( )) 
        return RC_MOVE_FAIL;

    if (portal->value0() == -1) {
        portal->getRoom( )->echo( POS_RESTING, 
                                  _("%^O1 медленно исчезает в дымке."), 
                                  portal );
        extract_obj( portal );
    }

    return RC_MOVE_OK;
}

/*
 * A player can't be led into a portal hidden from them by magic (invisible,
 * or shown only to detect magic). Blindness and darkness don't count: a group
 * still follows its leader by the hand. Pets go on a leash.
 */
static bool portal_hidden_from( Character *fch, Object *portal )
{
    if (fch->is_npc( ) || fch->can_see( portal ))
        return false;

    if (IS_AFFECTED(fch, AFF_BLIND))
        return false;

    if (fch->in_room->isDark( ) && !IS_AFFECTED(fch, AFF_INFRARED) && !IS_OBJ_STAT(portal, ITEM_GLOW))
        return false;

    return true;
}

int PortalMovement::moveOneFollower( Character *wch, Character *fch )
{
    if (portal_hidden_from( fch, portal )) {
        msgSelf( fch,
                 "You can't see where to follow.",
                 "Ты не видишь, куда идти следом.",
                 "Ти не бачиш, куди йти слідом." );
        return RC_MOVE_FAIL;
    }

    oldact(_("Ты следуешь за $C5."), fch, 0, wch, TO_CHAR );
    // Movement::move, not PortalMovement::move: the leader's move already
    // handles the portal's charges and gowith.
    return PortalMovement( fch, portal, to_room ).Movement::move( );
}

void PortalMovement::place( Character *wch )
{
    // A gowith portal arrives before its first passenger, so the arrival
    // look already shows it lying there.
    if (IS_SET(portal->value2(), GATE_GOWITH) && portal->in_room == from_room) {
        obj_from_room( portal );
        obj_to_room( portal, to_room );
    }

    Walkment::place( wch );
    from_room->history.record( wch, portal );
}

bool PortalMovement::moveAtomic( )
{
    if (!Walkment::moveAtomic( ))
        return false;

    if (portal->value0() > 0) {
        portal->value0(portal->value0() - 1);
        if (portal->value0() == 0)
            portal->value0(-1);
    }

    return true;
}

int PortalMovement::getDoorStatus(Character *wch)
{
    if (!IS_SET(portal->value1(), EX_CLOSED))
        return RC_MOVE_OK;
        
    if (wch->get_trust( ) >= ANGEL)
        return RC_MOVE_OK;

    if (IS_GHOST( wch ))
        return RC_MOVE_OK;
    
    return RC_MOVE_CLOSED;
}

bool PortalMovement::checkClosedDoor( Character *wch )
{
    rc = getDoorStatus(wch);
    
    if (rc == RC_MOVE_OK)
        return true;

    msgSelfParty( wch,
                  "It's closed here, try typing {y{hcopen %4$O1{x.",
                  "Тут закрыто, попробуй написать {y{hcоткрыть %4$O1{x.",
                  "Тут зачинено, спробуй написати {y{hcвідкрити %4$O1{x.",
                  "It's closed here, try typing {y{hcopen %4$O1{x.",
                  "Тут закрыто, попробуй написать {y{hcоткрыть %4$O1{x.",
                  "Тут зачинено, спробуй написати {y{hcвідкрити %4$O1{x." );
    return false;
}

bool PortalMovement::checkSafe( Character *wch )
{
    return !isNormalExit( ) || Walkment::checkSafe( wch );
}

bool PortalMovement::checkAir( Character *wch )
{
    return !isNormalExit( ) || Walkment::checkAir( wch );
}

bool PortalMovement::checkWater( Character *wch )
{
    return !isNormalExit( ) || Walkment::checkWater( wch );
}

/*
 * Who may enter which portal is decided in Fenia (global onPortalEnter,
 * .tmp.transport.checkPortal): curses, no-recall rooms, prisoners, and the
 * transport-spell rules for magic and portable portals. A refusing handler
 * returns true and has already told the walker and its master why.
 * A mount carrying a rider is not asked: the rider's answer covers both.
 * With no handler registered, the old curse checks below still stand guard.
 */
bool PortalMovement::checkScripts( Character *wch )
{
    if (RIDDEN(wch))
        return true;

    if (!gprog_registered( "onPortalEnter" ))
        return checkCurse( wch );

    if (!gprog( "onPortalEnter", "COR", wch, portal, to_room ))
        return true;

    rc = RC_MOVE_EXPLAINED;
    return false;
}

bool PortalMovement::checkCurse( Character *wch )
{
    if (wch->get_trust( ) >= ANGEL)
        return true;
    
    if (IS_AFFECTED(wch, AFF_CURSE) && !IS_SET(portal->value2(), GATE_CURSE_ALLOWED))
    {
        msgSelfParty( wch,
                      "Your curse keeps you from entering %4$O4.",
                      "Твое проклятье мешает тебе войти в %4$O4.",
                      "Твоє прокляття заважає тобі увійти в %4$O4.",
                      "%2$C2's curse keeps %2$P3 from entering %4$O4.",
                      "Проклятье %2$C2 мешает %2$P3 войти в %4$O4.",
                      "Прокляття %2$C2 заважає %2$P3 увійти в %4$O4." );
        return false;
    }
    
    if ((IS_SET(from_room->room_flags, ROOM_NO_RECALL)
         || IS_ROOM_AFFECTED(from_room, AFF_ROOM_CURSE)) && !IS_SET(portal->value2(), GATE_FROM_NO_RECALL))
    {
        msgSelfParty( wch,
                      "The curse of this place keeps you from leaving it.",
                      "Проклятье этого места мешает тебе его покинуть.",
                      "Прокляття цього місця заважає тобі його покинути.",
                      "The curse of this place keeps %2$C3 from leaving it.",
                      "Проклятье этого места мешает %2$C3 его покинуть.",
                      "Прокляття цього місця заважає %2$C3 його покинути." );
        return false;
    }
    
    return true;
}

bool PortalMovement::applyWeb( Character *wch )
{
    return !isNormalExit( ) || Walkment::applyWeb( wch );
}

bool PortalMovement::applySpellbane( Character *wch )
{
    if (!IS_SET(portal->extra_flags, ITEM_MAGIC) || !rage_member( wch ))
        return true;

    // The oath still tolerates a portal at low ranks, but the aura rolls against it.
    if (!rage_own_magic( wch ))
        return true;

    oldact(_("Магия $o2 гаснет, и проход не пропускает тебя."), wch, portal, 0, TO_CHAR);
    oldact(_("Магия $o2 гаснет перед $c5."), wch, portal, 0, TO_ROOM);
    return false;
}

bool PortalMovement::applyMovepoints( Character *wch )
{
    return !isNormalExit( ) || Walkment::applyMovepoints( wch );
}

int PortalMovement::getMoveCost( Character *wch )
{
    return 1;
}

void PortalMovement::msgOnMove( Character *wch, bool fLeaving )
{
    DLString msg;
    
    if (MOUNTED(wch))
        return;

    if (wch->is_mirror( ))
        return;

    if (ch->invis_level >= LEVEL_HERO || wch->invis_level >= LEVEL_HERO)
        return;
    
    if (fLeaving) {
        if (RIDDEN(wch))
            msgRoomNoParty( wch,
                            "%1$^C1 ride%1$ns| into %4$O4 on %2$C6.",
                            "%1$^C1 въезжа%1$nет|ют в %4$O4 верхом на %2$C6.",
                            "%1$^C1 в'їжджа%1$nє|ють у %4$O4 верхи на %2$C6." );
        else
            msgRoomNoParty( wch,
                            "%1$^C1 step%1$ns| into %4$O4.",
                            "%1$^C1 вход%1$nит|ят в %4$O4.",
                            "%1$^C1 вход%1$nить|ять у %4$O4." );

        const char *sEn, *sRu, *sUa;
        if (isNormalExit( )) {
            sEn = "You step into %4$O4.";
            sRu = "Ты входишь в %4$O4.";
            sUa = "Ти входиш у %4$O4.";
        }
        else {
            sEn = "You step into %4$O4 and are swept away to another place...";
            sRu = "Ты входишь в %4$O4 и переносишься в другое место...";
            sUa = "Ти входиш у %4$O4 і переносишся в інше місце...";
        }

        msgSelf( wch, sEn, sRu, sUa );
        if (RIDDEN(wch))
            msgSelf( RIDDEN(wch), sEn, sRu, sUa );
    }
    else {
        DLString en, uk;
        if (isNormalExit( )) {
            en  = "%1$^C1 appear%1$ns|";
            msg = "%1$^C1 появля%1$nется|ются";
            uk  = "%1$^C1 з'явля%1$nється|ються";
        }
        else {
            en  = "%1$^C1 appear%1$ns| out of %4$O2";
            msg = "%1$^C1 появля%1$nется|ются из %4$O2";
            uk  = "%1$^C1 з'явля%1$nється|ються з %4$O2";
        }

        if (RIDDEN(wch)) {
            en  << " on %2$C6";
            msg << " верхом на %2$C6";
            uk  << " верхи на %2$C6";
        }

        en  << ".";
        msg << ".";
        uk  << ".";
        msgRoomNoParty( wch, en.c_str( ), msg.c_str( ), uk.c_str( ) );
    }
}

void PortalMovement::msgEcho( Character *victim, Character *wch, const char *msg )
{
    if (canHear( victim, wch ))
        victim->pecho( msg, 
                       (RIDDEN(wch) ? wch->mount : wch),
                       (MOUNTED(wch) ? wch->mount : wch),
                       wch, 
                       portal );
}

