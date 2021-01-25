/* $Id$
 *
 * ruffina, 2004
 */
#include "recallmovement.h"
#include "move_utils.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "affectflags.h"
#include "skill_utils.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

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
        msgSelf( ch, "Но %2$C1 сражается!" );
        return false;
    }

    wch->pecho( "Ты долж%1$Gно|ен|на сражаться!", wch );
    chance = skill->getEffective( wch );
    chance = 80 * chance / 100 + skill_level(*skill, ch);
    
    if (number_percent( ) > chance) {
        skill->improve( wch, false );
        wch->setWaitViolence( 1 );
        wch->pecho( "Тебе не удается сбежать из боя!" );
        return false;
    }

    skill->improve( wch, true );
    msgSelfParty( wch, "Ты убегаешь с поля битвы!", "%2$^C1 убегает с поля битвы!" );
    return true;
}

bool RecallMovement::checkMount( )
{
    if (actor->is_npc( ) && actor == ch && actor->mount) {
        msgSelfMaster( actor, "Ты не сможешь этого сделать.", "%3$^C1 не сможет этого сделать." );
        return false;
    }

    if (!horse)
        return true;

    if (!canOrderHorse( )) {
        ch->pecho( "Тебе необходимо сначала спешиться." );
        return false;
    }

    return true;
}

bool RecallMovement::checkShadow( )
{
    if (!ch->is_npc( ) && ch->getPC( )->shadow != -1) {
        ch->pecho( "Твои молитвы о возвращении тонут в пустоте твоей {Dтени{x." );
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
        wch->pecho("Эта местность проклята, здесь боги не услышат твою молитву о возвращении.");
        return false;
    }

    if (wch->death_ground_delay > 0
        && wch->trap.isSet( TF_NO_RECALL )) 
    {
        wch->pecho("Ты в ловушке! Отсюда не удастся спастись молитвой о возвращении.");
        return false;
    }

    return true;
}

bool RecallMovement::checkCurse( Character *wch )
{
    if (IS_AFFECTED(wch, AFF_CURSE))
    { 
        wch->pecho("На тебе висит проклятие, препятствующее молитве о возвращении.");
        return false;
    }

    if (IS_ROOM_AFFECTED(from_room, AFF_ROOM_CURSE))
    {  
        wch->pecho("На этой местности висит временное проклятие, препятствующее молитве о возвращении.");
        return false;
    }

    return true;
}

bool RecallMovement::applyInvis( Character *wch )
{
    if (IS_AFFECTED( wch, AFF_HIDE|AFF_FADE ))
        REMOVE_BIT(wch->affected_by, AFF_HIDE|AFF_FADE);

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
        ch->pecho("Но ты уже здесь!");
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
        ch->pecho( "Твой пульс стучит слишком часто, ты не можешь сосредоточиться на молитве." );
        return false;
    }

    return true;
}

bool RecallMovement::applyWaitstate( )
{
    ch->setWaitViolence( 1 );
    return true;
}

