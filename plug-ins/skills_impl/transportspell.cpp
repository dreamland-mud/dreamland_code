/* $Id$
 *
 * ruffina, 2004
 */
#include "transportspell.h"

#include "clanreference.h"
#include "skillreference.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "dreamland.h"
#include "interp.h"
#include "save.h"
#include "occupations.h"
#include "fight.h"
#include "magic.h"
#include "handler.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

CLAN(none);
GSN(mental_block);
GSN(spellbane);

/*------------------------------------------------------------------------
 * GateMovement 
 *-----------------------------------------------------------------------*/
GateMovement::GateMovement( Character *ch, Character *victim, GateSpell::Pointer spell, Character *actor, int level )
               : JumpMovement( ch, actor, victim->in_room )
{
    this->victim = victim;
    this->spell = spell;
    this->level = level;
}

GateMovement::GateMovement( Character *ch, Character *victim )
               : JumpMovement( ch, ch, victim->in_room )
{
    this->victim = victim;
    level = -1;
}

GateMovement::~GateMovement( )
{
}

bool GateMovement::moveAtomic( )
{
    if (!JumpMovement::moveAtomic( )) {
        ch->pecho( "Твоя попытка закончилась неудачей." );
        return false;
    }

    return true;
}

bool GateMovement::canMove( Character *wch )
{
    if (ch != actor)
        return true;
    else if (wch == ch->mount)
        return checkCaster( wch );
    else
        return checkLevel( )
               && checkCaster( wch )
               && checkCasterRoom( )
               && checkVictimRoom( )
               && checkVictim( );
}

bool GateMovement::tryMove( Character *wch )
{
    if (ch != actor)
        return true;
    else if (wch == ch->mount)
        return applyViolent( wch );
    else
        return applyViolent( wch )
               && applySavesSpell( );
}

bool GateMovement::checkLevel( )
{
    if (level < 0 || !spell)
        return true;
        
    if (victim->getModifyLevel( ) < level + spell->levelDiff)
        return true;

    return false;
}

bool GateMovement::checkCaster( Character *wch )
{
    if (wch->fighting) {
        wch->pecho( "Адреналин в крови мешает тебе сконцентрироваться." );
        return false;
    }
        
    if (IS_AFFECTED(wch, AFF_CURSE)) {
        wch->pecho( "Проклятие на тебе блокирует транспортные заклинания." );
        return false;
    }
    
    if (spell && !spell->gateShadow && HAS_SHADOW( wch )) {
        wch->pecho( "Вторая тень блокирует транспортные заклинания." );
        return false;
    }

    if (wch->is_npc( ) && wch == actor && wch->mount)
        return false;

    return true;
}

bool GateMovement::checkCasterRoom( )
{
    if (IS_SET(from_room->room_flags, ROOM_SAFE|ROOM_NO_RECALL|ROOM_NOSUMMON)) {
        ch->pecho( "Транспортные заклинания в этой местности запрещены Богами." );
        return false;
    }

    if (IS_ROOM_AFFECTED(from_room, AFF_ROOM_CURSE)) {
        ch->pecho( "На этой местности висит временное проклятие." );
        return false;
    }
  
    return true;
}

bool GateMovement::checkVictim( )
{
    if (victim == ch || victim == ch->mount) {
        ch->pecho( "Перейти на сам{Smого{Sfу{Sx себя? Но как узнать, где настоящ{Smий{Sfая{Sx ты -- здесь или там?" );
        ch->pecho( "А вдруг тебя вообще не существует?! Вопросы, вопросы..." );      
        return false;
    }

    if (victim->is_immortal( ) && !ch->is_immortal( )) {
        ch->pecho( "Таким способом Бессмертных лучше не беспокоить." );      
        return false;
    }

    if (DIGGED(victim)) {
        ch->pecho( "Непроницаемый экран защищает твою цель, тебе не удается установить ментальную связь." );      
        return false;
    }

    if (victim->is_npc( )) {
        if (IS_SET(victim->getNPC( )->pIndexData->area->area_flag, AREA_NOGATE)) {
            ch->pecho( "Местность, в которой находится твоя цель, защищена Богами." );
            return false;
        }
      
        if (IS_SET(victim->imm_flags, IMM_SUMMON)) {
            ch->pecho( "Непроницаемый экран защищает твою цель, тебе не удается установить ментальную связь." );      
            return false;
        }
            
        if (IS_CHARMED(victim)) {
            ch->pecho( "Разум твоей жертвы во власти кого-то другого, тебе не удается установить ментальную связь." );
            return false;
        }
    } 
    else {
        if (!ch->is_npc( ) 
            && !ch->getClan( )->isDispersed( ) 
            && ch->getClan( ) == victim->getClan( ))
            return true; 
        
        if (!is_safe(ch, victim) 
            && IS_SET(victim->act, PLR_NOSUMMON) 
            && spell 
            && from_room->area != to_room->area) {
            ch->pecho( "Открыть портал на твою цель удастся только в пределах одной зоны." );
            return false;
        }  
        
        if (is_safe(ch, victim) && IS_SET(victim->act, PLR_NOSUMMON))
            return false;
    }
    
    return true;
}

bool GateMovement::checkVictimRoom( )
{
    if (!ch->can_see( to_room ))
        return false;

    if (IS_SET(to_room->room_flags, 
               ROOM_SAFE|ROOM_PRIVATE|ROOM_SOLITARY|ROOM_NOSUMMON))
        return false;

    return true;
}

bool GateMovement::applySavesSpell( )
{
    return !saves_spell( ch->getModifyLevel( ), victim, DAM_OTHER, ch, DAMF_SPELL );
}

bool GateMovement::applyViolent( Character *wch )
{
    if (IS_BLOODY( wch )) {
        int chance = 0;

        if (spell && spell->gateViolent) {
            chance = PK_TIME_VIOLENT - wch->getPC( )->PK_time_v;
            chance = 100 * chance / PK_TIME_VIOLENT;
        }

        return number_percent( ) < chance;
    }

    return true;
}

void GateMovement::msgOnMove( Character *wch, bool fLeaving )
{
    if (!spell)
        return;

    if (fLeaving) {
        if (ch != actor && !spell->msgGroupLeave.empty( ))
            msgSelf( wch, spell->msgGroupLeave.c_str( ) );
        else {
            msgSelf( wch, spell->msgSelfLeave.c_str( ) );
            msgRoomNoParty( wch, spell->msgRoomLeave.c_str( ) );     
        }
    }
    else {
        if (ch != actor && !spell->msgGroupEnter.empty( ))
            msgSelf( wch, spell->msgGroupEnter.c_str( ) );
        else {
            msgSelf( wch, spell->msgSelfEnter.c_str( ) );
            msgRoomNoParty( wch, spell->msgRoomEnter.c_str( ) );
        }
    }
}

void GateMovement::moveFollowers( Character *wch )
{
    if (!wch)
        return;

    if (!spell)
        return;
    
    if (from_room == to_room)
        return;

    if (spell->takePet) {
        NPCharacter *pet;

        if (!wch->is_npc( ) && ( pet = wch->getPC( )->pet ))
            GateMovement( pet, victim, spell, actor, level ).moveRecursive( );
    }
        
    if (spell->takeGroup) {
        GroupMembers group = group_members_room( ch, from_room );

        for (GroupMembers::iterator g = group.begin( ); g != group.end( ); g++)
            if ((*g)->in_room == from_room)
                GateMovement( *g, victim, spell, actor, level ).moveRecursive( );
    }
}

void GateMovement::msgEcho( Character *listener, Character *wch, const char *msg )
{
    if (IS_AWAKE(listener))
        listener->pecho( msg, wch, victim, actor );
}

/*------------------------------------------------------------------------
 * GateSpell
 *-----------------------------------------------------------------------*/
void GateSpell::run( Character *ch, Character *victim, int sn, int level )
{
    GateMovement( ch, victim, Pointer( this ), ch, level ).move( );
}



/*------------------------------------------------------------------------
 * SummonMovement 
 *-----------------------------------------------------------------------*/
SummonMovement::SummonMovement( Character *ch, Character *caster, SummonSpell::Pointer spell, int level )
                  : JumpMovement( ch, ch, caster->in_room )
{
    this->caster = caster;
    this->spell = spell;
    this->level = level;
}

SummonMovement::~SummonMovement( )
{
}

bool SummonMovement::moveAtomic( )
{
    if (!JumpMovement::moveAtomic( )) {
        caster->pecho( "Твоя попытка закончилась неудачей." );
        return false;
    }

    return true;
}

bool SummonMovement::canMove( Character *wch )
{
    if (wch == ch->mount)
        return true;
        
    return checkCaster( ) 
           && checkVictim( );
}

bool SummonMovement::tryMove( Character *wch )
{
    if (wch == ch->mount)
        return true;

    return applySavesSpell( ) 
           && applySpellbane( )
           && applyLazy( );
}

bool SummonMovement::applyLazy( )
{
    if (caster->in_room->area == ch->in_room->area) {
        caster->pecho( "Может, пора пройтись пешком?" );
        return false;
    }

    return true;
}

bool SummonMovement::checkCaster( )
{
    if (caster->is_npc( ) && IS_CHARMED(caster))
        return false;

    if (IS_VIOLENT( caster )) {
        caster->pecho( "Адреналин в крови мешает тебе сконцентрироваться." );
        return false;
    }
      
    if (IS_SET(caster->in_room->room_flags, ROOM_SAFE|ROOM_NOSUMMON)) {
        caster->pecho( "Призыв в эту местность запрещен Богами." );
        return false;
    }
    
    if (caster->in_room->getCapacity( ) == 0) {
        caster->pecho( "Рядом с тобой в этой комнате уже никто не уместится." );
        return false;
    }

    return true;
}    

bool SummonMovement::checkVictim( )
{
    if (ch == caster) {
        caster->pecho( "Ты с глубоким удовлетворением призываешь сам{Smого{Sfу{Sx себя." );
        caster->pecho( "К немедленному ответу за бесцельно прожитые годы." );      
        return false;
    }
    
    if (ch->is_immortal( ) && !caster->is_immortal( )) {
        caster->pecho( "Призывать Бессмертных лучше другими способами." );
        return false;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE|ROOM_NOSUMMON)) {
        caster->pecho( "Местность, в которой находится твоя жертва, защищена Богами." );
        return false;
    }

    if (ch->fighting != 0) {
        caster->pecho( "Разум твоей жертвы в смятении, тебе не удается установить ментальную связь." );
        return false;
    }

    if (ch->isAffected( gsn_mental_block )) {
        caster->pecho( "Непроницаемый экран защищает твою жертву, тебе не удается установить ментальную связь." );
        return false;
    }

    if (ch->isAffected( gsn_spellbane )) {
        caster->pecho( "Ненависть к магии защищает твою жертву, тебе не удается установить ментальную связь." );
        return false;
    }

    if (IS_SET(ch->imm_flags, IMM_SUMMON)) {
        caster->pecho( "Непроницаемый экран защищает твою жертву, тебе не удается установить ментальную связь." );
        return false;
    }
  
    if (ch->is_npc( )) {
        if (IS_SET(ch->act,ACT_AGGRESSIVE) || IS_AFFECTED(ch,AFF_BLOODTHIRST)) {
            caster->pecho( "Призывать агрессивных существ запрещено Богами." );
            return false;
        }
            
        if (ch->getNPC( )->behavior && IS_SET(ch->getNPC( )->behavior->getOccupation( ), (1 << OCC_SHOPPER))) {
            caster->pecho( "Призывать продавцов запрещено Богами." );
            return false;
        }
            
        if (is_safe( caster, ch ))
            return false;            
    }
    else {      
        if (!caster->is_npc( )) {      
            if (is_safe( caster, ch ))
                return false;
          
            if ( (ch->getClan( )->isDispersed( )) && (caster->getClan( ) != ch->getClan( )) ) {
                caster->pecho( "Хочешь напасть на клановика? Сделай это лицом к лицу." );
                return false;              
            }
          
            if ( (!IS_SET(ch->act,PLR_NOSUMMON)) || (caster->getClan( ) == ch->getClan( )) )
                return true;    
        }
    }

    return true;
}

bool SummonMovement::applySavesSpell( )
{
    return !saves_spell( level, ch, DAM_OTHER, caster, DAMF_SPELL );
}

bool SummonMovement::applySpellbane( )
{
    return !spell->spellbane( caster, ch );
}

void SummonMovement::msgEcho( Character *listener, Character *wch, const char *msg )
{
    if (IS_AWAKE(listener))
        listener->pecho( msg, 
                           (RIDDEN(wch) ? wch->mount : wch),
                           (MOUNTED(wch) ? wch->mount : wch),
                           caster );
}

void SummonMovement::msgOnMove( Character *wch, bool fLeaving )
{
    if (fLeaving) {
        msgRoomNoParty( wch, "%1$^C1 внезапно исчезает." );     
        msgSelfParty( wch, 
                      "%3$^C1 призывает тебя!",
                      "%3$^C1 призывает %2$^C4!" );
    }
    else {
        msgRoomNoParty( wch, "%1$^C1 появляется рядом с тобой." );
    }
}

/*------------------------------------------------------------------------
 * SummonSpell
 *-----------------------------------------------------------------------*/
void SummonSpell::run( Character *ch, Character *victim, int sn, int level )
{
    SummonMovement( victim, ch, Pointer( this ), level ).move( );
}

