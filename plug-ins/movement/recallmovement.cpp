/* $Id$
 *
 * ruffina, 2004
 */
#include "recallmovement.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "loadsave.h"
#include "affectflags.h"
#include "skill_utils.h"
#include "merc.h"

#include "def.h"
#include "l10n.h"

RecallMovement::RecallMovement( Character *ch )
                 : JumpMovement( ch )
{
}

RecallMovement::RecallMovement( Character *ch, Character *actor, Room *to_room )
                 : JumpMovement( ch, actor, to_room )
{
}

bool RecallMovement::moveAtomic( )
{
    if (ch == actor)
        msgOnStart( );

    return JumpMovement::moveAtomic( );
}

void RecallMovement::msgOnStart( )
{
}

bool RecallMovement::applyFightingSkill( Character *wch, SkillReference &skill )
{
    int chance;
    
    if (!wch->fighting) {
        skill->improve(wch, true);
        return true;
    }

    if (wch != ch) {
        msgSelf( ch, "But %2$C1 is fighting!", "Но %2$C1 сражается!", "Але %2$C1 б'ється!" );
        return false;
    }

    wch->pecho( _("Ты долж%1$Gно|ен|на сражаться!"), wch );
    chance = skill->getEffective( wch );
    chance = 80 * chance / 100 + skill_level_bonus(*skill, ch);
    
    if (number_percent( ) > chance) {
        skill->improve( wch, false );
        wch->setWaitViolence( 1 );
        wch->pecho( _("Тебе не удается сбежать из боя!") );
        return false;
    }

    skill->improve( wch, true );
    msgSelfParty( wch, "You flee the battlefield!", "Ты убегаешь с поля битвы!", "Ти тікаєш з поля бою!",
                   "%2$^C1 flees the battlefield!", "%2$^C1 убегает с поля битвы!", "%2$^C1 тікає з поля бою!" );
    return true;
}

bool RecallMovement::checkMount( )
{
    if (actor->is_npc( ) && actor == ch && actor->mount) {
        msgSelfMaster( actor, "You can't do that.", "Ты не сможешь этого сделать.", "Ти не зможеш цього зробити.",
                      "%3$^C1 can't do that.", "%3$^C1 не сможет этого сделать.", "%3$^C1 не зможе цього зробити." );
        return false;
    }

    if (!horse)
        return true;

    if (!canOrderHorse( )) {
        ch->pecho( _("Тебе необходимо сначала спешиться.") );
        return false;
    }

    return true;
}

bool RecallMovement::checkShadow( )
{
    if (!ch->is_npc( ) && ch->getPC( )->shadow != -1) {
        ch->pecho( _("Твои молитвы о возвращении тонут в пустоте твоей {Dтени{x.") );
        return false;
    }

    return true;
}

bool RecallMovement::checkBloody( Character *wch )
{
    if (IS_BLOODY(wch)) {
        msgSelfParty( wch, 
                      "Богам нет дела до тебя, смертн%1$Gое|ый|ая.",
                      "Богам нет дела до %1$C2." );
        return false;
    }

    return true;
}

bool RecallMovement::checkForsaken( Character *wch )
{
    if (!checkNorecall( wch ) || !checkCurse( wch )) {
        msgSelfParty( wch, 
                      "Боги покинули тебя.",
                      "Боги покинули %1$C4." );
        return false;
    }

    return true;
}

bool RecallMovement::checkNorecall( Character *wch )
{
    if (IS_SET(from_room->room_flags, ROOM_NO_RECALL)) {
        wch->pecho(_("Эта местность проклята, здесь боги не услышат твою молитву о возвращении."));
        return false;
    }

    if (wch->death_ground_delay > 0
        && wch->trap.isSet( TF_NO_RECALL )) 
    {
        wch->pecho(_("Ты в ловушке! Отсюда не удастся спастись молитвой о возвращении."));
        return false;
    }

    return true;
}

bool RecallMovement::checkCurse( Character *wch )
{
    if (IS_AFFECTED(wch, AFF_CURSE))
    { 
        wch->pecho(_("На тебе висит проклятие, препятствующее молитве о возвращении."));
        return false;
    }

    if (IS_ROOM_AFFECTED(from_room, AFF_ROOM_CURSE))
    {  
        wch->pecho(_("На этой местности висит временное проклятие, препятствующее молитве о возвращении."));
        return false;
    }

    return true;
}

bool RecallMovement::applyInvis( Character *wch )
{
    strip_camouflage( wch );
    return true;
}

bool RecallMovement::applyMovepoints( )
{
    ch->move /= 2;
    return true;
}

bool RecallMovement::checkSameRoom( )
{
    if (from_room == to_room) {
        ch->pecho(_("Но ты уже здесь!"));
        return false;
    }

    return true;
}

void RecallMovement::moveFollowers( Character *wch ) 
{
    NPCharacter *pet;
    
    if (!wch || wch->is_npc( ))
        return;
    
    if (!( pet = wch->getPC( )->pet ))
        return;

    if (pet->in_room == to_room)
        return;

    movePet( pet );        
}

bool RecallMovement::checkPumped( )
{
    if (ch->getLastFightDelay( ) < FIGHT_DELAY_TIME) {
        ch->pecho( _("Твой пульс стучит слишком часто, ты не можешь сосредоточиться на молитве.") );
        return false;
    }

    return true;
}

bool RecallMovement::applyWaitstate( )
{
    return true;
}

