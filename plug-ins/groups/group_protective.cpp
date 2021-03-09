
/* $Id: group_protective.cpp,v 1.1.2.19.6.15 2010-09-01 21:20:45 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/

#include "spelltemplate.h"
#include "profflags.h"

#include "affecthandler.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "object.h"
#include "affect.h"
#include "clanreference.h"

#include "magic.h"
#include "fight.h"
#include "act_move.h"
#include "gsn_plugin.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "def.h"


CLAN(none);
GSN(stardust);
GSN(dispel_affects);

static inline bool has_sanctuary_msg( Character *ch, Character *victim )
{
    if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
        if (victim == ch)
            act("Ты уже под защитой святилища.", ch, 0, 0, TO_CHAR);
        else
            act("$C1 уже под защитой святилища.", ch, 0, victim, TO_CHAR);
        return true;
    }

    if (victim->isAffected(gsn_dark_shroud)) {
        if (victim == ch)
            act("Ты уже под защитой темных богов.", ch, 0, 0, TO_CHAR);
        else
            act("$C1 уже под защитой темных богов.", ch, 0, victim, TO_CHAR);
        return true;
    }

    if (victim->isAffected(gsn_stardust)) {
        if (victim == ch)
            act("Звездная пыль уже кружится вокруг тебя.", ch, 0, 0, TO_CHAR);
        else
            act("Звездная пыль уже кружится вокруг $C2.", ch, 0, victim, TO_CHAR);
        return true;
    }

    return false;
}

SPELL_DECL(Armor);
VOID_SPELL(Armor)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (victim->isAffected(sn)) {
        if (victim == ch)
            act("Ты уже защище$gно|н|на заклинанием брони.", ch, 0, 0, TO_CHAR);
        else
            act("$C1 уже защище$Gно|н|на заклинанием брони.", ch, 0, victim, TO_CHAR);
        return;
    }

    af.type      = sn;
    af.level         = level;
    af.duration  = 7 + level / 6;
    af.modifier  = -1 * max(20,10 + level / 4); /* af.modifier  = -20;*/
    af.location = APPLY_AC;
    affect_to_char( victim, &af );
    
    if (ch->getProfession( )->getFlags( ch ).isSet(PROF_DIVINE)) {
        act("Священная броня окружает тебя.", victim, 0, 0, TO_CHAR);
        if (ch != victim)
            act("Священная броня окружает $C4.", ch, 0, victim, TO_CHAR);
    } else {
        act("Волшебная броня окружает тебя.", victim, 0, 0, TO_CHAR);
        if (ch != victim)
            act("Волшебная броня окружает $C4.", ch, 0, victim, TO_CHAR);
    }
}


SPELL_DECL(BarkSkin);
VOID_SPELL(BarkSkin)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( ch->isAffected(sn ) )
    {
        if (victim == ch)
          ch->pecho("Твоя кожа не может стать еще прочнее.");
        else
          act_p("Кожа $C2 не может стать еще прочнее.",
                 ch,0,victim,TO_CHAR,POS_RESTING);
        return;
    }

    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location = APPLY_AC;
    af.modifier  = -( int )(level * 1.5);
    affect_to_char( victim, &af );
    act_p( "Кожа $c2 покрывается корой.",
            victim, 0, 0, TO_ROOM,POS_RESTING);
    victim->pecho("Прочная защитная кора покрывает твою кожу.");
}

enum {
    CANCEL_ALWAYS,
    CANCEL_DISPEL,
    CANCEL_NEVER
};
static int can_cancel( Character *ch, Character *victim )
{
    if (ch->is_npc( ) && victim->is_npc( )) {
        if (!is_same_group( ch, victim ))
            return CANCEL_DISPEL;
        
        return CANCEL_ALWAYS;
    }

    if (!ch->is_npc( ) && !victim->is_npc( )) {
        if (ch == victim)
            return CANCEL_ALWAYS;

        if (ch->is_immortal( ))
            return CANCEL_ALWAYS;

        if (!IS_SET(victim->add_comm, PLR_NOCANCEL))
            return CANCEL_ALWAYS;

        if (ch->getClan( ) != victim->getClan( ))
            return CANCEL_DISPEL;

        if (ch->getClan( )->isDispersed( ))
            return CANCEL_DISPEL;

        return CANCEL_ALWAYS;
    }

    if (ch->is_npc( ) && !victim->is_npc( )) {
        return CANCEL_NEVER;
    }

    if (victim->getNPC( )->behavior
        && !victim->getNPC( )->behavior->canCancel( ch ))
        return CANCEL_NEVER;

    return CANCEL_ALWAYS;
}


SPELL_DECL(Cancellation);
VOID_SPELL(Cancellation)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    AffectHandler::Pointer affect;
    bool found = false;

    switch (can_cancel( ch, victim )) {
    case CANCEL_ALWAYS:
            break;
    case CANCEL_NEVER:
            ch->pecho("Отмена здесь бессильна, используй вместо этого снятие чар.");
            return;
    case CANCEL_DISPEL:
            if (!is_safe_spell( ch, victim, false ))        
                spell(gsn_dispel_affects, level, ch, victim);
            return;
    }

    level += 2;
    
    /* unlike dispel affects, the victim gets NO save */

    for (int sn = 0; sn < skillManager->size( ); sn++) {
        affect = skillManager->find( sn )->getAffect( );

        if (affect && affect->isCancelled( ))
            if (checkDispel( level, victim, sn )) 
                found = true;
    }

    if (found)
        ch->pecho("Получилось!");
    else
        ch->pecho("Твоя попытка отменить воздействия закончилась неудачей.");
}


SPELL_DECL(DarkShroud);
VOID_SPELL(DarkShroud)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;
    
    if (has_sanctuary_msg( ch, victim ))
        return;

    if (IS_GOOD(victim)) // Not for good !!!
    {
       if (victim == ch)
          act("Темные боги не будут защищать тебя!!!", ch, 0, 0, TO_CHAR);
       else
                 act("Темные боги не будут защищать $C4!!!", ch, 0, victim, TO_CHAR);
        return;
    }

    af.type      = sn;
    af.level     = level;
    af.duration  = level / 6;
    affect_to_char( victim, &af );
    act("{DТемная аура{x окружает $c4.", victim, 0, 0, TO_ROOM);
    act("{DТемная аура{x окружает тебя.", victim, 0, 0, TO_CHAR);
}


SPELL_DECL(DispelAffects);
VOID_SPELL(DispelAffects)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    AffectHandler::Pointer affect;
    bool found = false;
    
    if (IS_CHARMED(ch))
        return;

    if (saves_spell(level, victim,DAM_OTHER, ch, DAMF_SPELL)) {
        victim->pecho("Ты чувствуешь легкий звон в ушах.");
        ch->pecho("Твоя попытка развеять чары закончилась неудачей.");
        return;
    }

    for (int sn = 0; sn < skillManager->size( ); sn++) {
        affect = skillManager->find( sn )->getAffect( );

        if (affect && affect->isDispelled( ))
            if (checkDispel( level, victim, sn )) 
                found = true;
    }

    if (found)
        ch->pecho("Получилось!");
    else
        ch->pecho("Твоя попытка развеять чары закончилась неудачей.");
}


SPELL_DECL(DragonSkin);
VOID_SPELL(DragonSkin)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  
  Affect af;

  if ( victim->isAffected(sn ) )
    {
      if (victim == ch)
               ch->pecho("Твоя кожа уже тверда, как драконья.");
      else
               act_p("Кожа $C2 уже тверда, как драконья.",
               ch,0,victim,TO_CHAR,POS_RESTING);
      return;
    }
  
  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location = APPLY_AC;
  af.modifier  = - (2 * level);
  
  affect_to_char( victim, &af );
  act_p( "Кожа $c2 становится тверже драконьей.",
          victim,0,0,TO_ROOM,POS_RESTING );
  victim->pecho("Твоя кожа становится тверже драконьей.");
  return;

}

SPELL_DECL(EnhancedArmor);
VOID_SPELL(EnhancedArmor)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( victim->isAffected(sn ) )
    {
        if (victim == ch)
          ch->pecho("Силовое поле уже защищает тебя.");
        else
          act("Силовое поле уже окружает $C4.",ch,0,victim,TO_CHAR);
        return;
    }
    
    af.type      = sn;
    af.level         = level;
    af.duration  = 24;
    af.modifier  = -60;
    af.location = APPLY_AC;
    
    affect_to_char( victim, &af );
    victim->pecho("Силовое поле окружает тебя.");
    if ( ch != victim )
        act("Силовое поле окружает $C4.",ch,0,victim,TO_CHAR);
    return;

}

SPELL_DECL(Fortitude);
VOID_SPELL(Fortitude)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (ch->isAffected(sn )) {
        act("Ты уже гото$gво|в|ва противостоять холоду и темным чарам.", ch, 0, 0, TO_CHAR);
        return;
    }

    af.bitvector.setTable(&res_flags);
    af.type = sn;
    af.duration = level / 10;
    af.level = ch->getModifyLevel();
    af.bitvector.setValue(RES_COLD|RES_NEGATIVE);
    
    af.modifier = 0;
    affect_to_char(ch, &af);
    
    act("Свет дарует тебе сопротивляемость к могильному холоду и темным чарам!", ch, 0, 0, TO_CHAR);

}



SPELL_DECL(SpellResistance);
VOID_SPELL(SpellResistance)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        Affect af;

        if (!ch->isAffected(sn))
        {
                ch->pecho("Ты обретаешь сопротивляемость к магии!");

                af.bitvector.setTable(&res_flags);
                af.type = sn;
                af.duration = level / 10;
                af.level = ch->getModifyLevel();
                af.bitvector.setValue(RES_SPELL);
                
                af.modifier = 0;
                affect_to_char(ch, &af);
        }
        else
                ch->pecho("Ты уже имеешь сопротивляемость к магии.");
        return;

}



SPELL_DECL(MassSanctuary);
VOID_SPELL(MassSanctuary)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *gch;
    Affect af;

    for( gch=room->people; gch != 0; gch=gch->next_in_room)
    {
        if (!is_same_group( gch, ch ))
            continue;

        if (spellbane( ch, gch ))
            continue;
        
        if (has_sanctuary_msg( ch, gch ))
            continue;

        af.bitvector.setTable(&affect_flags);
        af.type      = gsn_sanctuary;
        af.level     = level;
        af.duration  = number_fuzzy( level/6 );
        af.bitvector.setValue(AFF_SANCTUARY);
        affect_to_char( gch, &af );

        act("{WБелая аура{x окружает тебя.", gch, 0, 0, TO_CHAR);
        if (ch != gch)
            act("{WБелая аура{x окружает $C4.", ch, 0, gch, TO_CHAR);
    }
}


SPELL_DECL(ProtectionCold);
VOID_SPELL(ProtectionCold)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( victim->isAffected(gsn_protection_cold) )
    {
        if (victim == ch)
          act("Ты уже защище$gно|н|на от холода.", ch,0, 0,TO_CHAR);
        else
          act("$C1 уже защище$Gно|н|на от холода.", ch,0,victim,TO_CHAR);
        return;
    }

    if ( victim->isAffected(gsn_protection_heat) )
    {
        if (victim == ch)
          act("Ты уже защище$gно|н|на от огня.", ch,0, 0,TO_CHAR);
        else
          act("$C1 уже защище$Gно|н|на от огня.", ch,0,victim,TO_CHAR);
        return;
    }

    if ( victim->isAffected(gsn_make_shield) )
    {
        if (victim == ch)
          act("Ты уже защище$gно|н|на ледяным щитом.", ch,0,0,TO_CHAR);
        else
          act("$C1 уже защище$Gно|н|на ледяным щитом.", ch,0,victim,TO_CHAR);
        return;
    }
    
    af.type      = gsn_protection_cold;
    af.level     = level;
    af.duration  = 24;
    af.location = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    
    affect_to_char( victim, &af );
    victim->pecho("Твоя защищенность от воздействия низких температур повышается.");
    if ( ch != victim )
        act_p("Защищенность $C2 от воздействия низких температур повышается.",
              ch,0,victim,TO_CHAR,POS_RESTING);
    return;

}


SPELL_DECL(ProtectionEvil);
VOID_SPELL(ProtectionEvil)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL)
    ||   IS_AFFECTED(victim, AFF_PROTECT_GOOD))
    {
        if (victim == ch)
          act("Ты уже защище$gно|н|на.", ch,0, 0,TO_CHAR);
        else
          act("$C1 уже защище$Gно|н|на.", ch,0,victim,TO_CHAR);
        return;
    }

    if (IS_NEUTRAL(victim))
    {
        if (victim == ch)
          act("Ты не можешь получить защиту от злых существ.", ch,0, 0,TO_CHAR);
        else
          act("$C1 не может получить защиту от злых существ.", ch,0,victim,TO_CHAR);
        return;
    }

    af.bitvector.setTable(&affect_flags);
    af.type      = sn;
    af.level     = level;
    af.duration  = (10 + level / 5);
    af.location = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector.setValue(AFF_PROTECT_EVIL);
    affect_to_char( victim, &af );
    victim->pecho("Ты получаешь защиту от злых существ.");
    if ( ch != victim )
        act_p("$C1 получает защиту от злых существ.",
               ch,0,victim,TO_CHAR,POS_RESTING);
    return;

}


SPELL_DECL(ProtectionGood);
VOID_SPELL(ProtectionGood)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD)
    ||   IS_AFFECTED(victim, AFF_PROTECT_EVIL))
    {
        if (victim == ch)
          act("Ты уже защище$gно|н|на.", ch,0, 0,TO_CHAR);
        else
          act("$C1 уже защище$Gно|н|на.", ch,0,victim,TO_CHAR);
        return;
    }

    if (IS_NEUTRAL(victim))
    {
        if (victim == ch)
          act("Ты не можешь получить защиту от добрых существ.", ch,0, 0,TO_CHAR);
        else
          act("$C1 не может получить защиту от добрых существ.", ch,0,victim,TO_CHAR);
        return;
    }

    af.bitvector.setTable(&affect_flags);
    af.type      = sn;
    af.level     = level;
    af.duration  = (10 + level / 5);
    af.location = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector.setValue(AFF_PROTECT_GOOD);
    affect_to_char( victim, &af );
    victim->pecho("Ты получаешь защиту от добрых существ.");
    if ( ch != victim )
        act_p("$C1 получает защиту от добрых существ.",
               ch,0,victim,TO_CHAR,POS_RESTING);
    return;

}

SPELL_DECL(ProtectionHeat);
VOID_SPELL(ProtectionHeat)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( victim->isAffected(gsn_protection_heat) )
    {
        if (victim == ch)
          act("Ты уже защище$gно|н|на от огня.", ch,0, 0,TO_CHAR);
        else
          act("$C1 уже защище$Gно|н|на от огня.", ch,0,victim,TO_CHAR);
        return;
    }

    if ( victim->isAffected(gsn_protection_cold) )
    {
        if (victim == ch)
          act("Ты уже защище$gно|н|на от холода.", ch,0, 0,TO_CHAR);
        else
          act("$C1 уже защище$Gно|н|на от холода.", ch,0,victim,TO_CHAR);
        return;
    }

    if ( victim->isAffected(gsn_make_shield) )
    {
        if (victim == ch)
          act("Ты уже защище$gно|н|на огненным щитом.", ch,0,0,TO_CHAR);
        else
          act("$C1 уже защище$Gно|н|на огненным щитом.", ch,0,victim,TO_CHAR);
        return;
    }

    
    af.type      = gsn_protection_heat;
    af.level     = level;
    af.duration  = 24;
    af.location = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    
    affect_to_char( victim, &af );
    victim->pecho("Твоя защищенность от воздействия высоких температур повышается.");
    if ( ch != victim )
        act_p("Защищенность $C2 от воздействия высоких температур повышается.",
               ch,0,victim,TO_CHAR,POS_RESTING);
    return;

}

SPELL_DECL(ProtectionNegative);
VOID_SPELL(ProtectionNegative)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  Affect af;

    if (!ch->isAffected(sn))
    {
      ch->pecho("Ты приобретаешь сопротивляемость к темной магии и молитвам.");

      af.bitvector.setTable(&res_flags);
      af.type = sn;
      af.duration = level / 4;
      af.level = ch->getModifyLevel();
      af.bitvector.setValue(RES_NEGATIVE);
      
      af.modifier = 0;
      affect_to_char(ch, &af);
    }
  else
      ch->pecho("У тебя уже есть сопротивляемость к темной магии и молитвам.");
 return;

}


SPELL_DECL(ProtectiveShield);
VOID_SPELL(ProtectiveShield)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  
  Affect af;

  if (victim->isAffected(sn)) {
      if (victim == ch)
               ch->pecho("Охранный щит уже окружает тебя.");
      else
               act("Охранный щит уже окружает $C4.",ch,0,victim,TO_CHAR);
      return;
  }

  
  af.type      = sn;
  af.level     = level;
  af.duration  = number_fuzzy( level / 30 ) + 3;
  af.location = APPLY_AC;
  af.modifier  = -20;
  
  affect_to_char( victim, &af );
  if (chance(1)) {
      act( "Предохранительный щит окружает $c4.",victim,0,0,TO_ROOM);
      victim->pecho("Предохранительный щит окружает тебя.");
  } else {
      act( "Охранный щит окружает $c4.",victim,0,0,TO_ROOM);
      victim->pecho("Охранный щит окружает тебя.");
  }
}


SPELL_DECL(Resilience);
VOID_SPELL(Resilience)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (!ch->isAffected(sn)) {
      ch->pecho("Ты приобретаешь устойчивость к энергетическим атакам.");

      af.bitvector.setTable(&res_flags);
      af.type = sn;
      af.duration = level / 10;
      af.level = ch->getModifyLevel();
      af.bitvector.setValue(RES_ENERGY);
      
      af.modifier = 0;
      affect_to_char(ch, &af);
    }
  else
      ch->pecho("У тебя уже есть устойчивость к энергетическим атакам.");

}

SPELL_DECL(Sanctuary);
VOID_SPELL(Sanctuary)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (has_sanctuary_msg( ch, victim ))
        return;

    af.bitvector.setTable(&affect_flags);
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 6;
    af.bitvector.setValue(AFF_SANCTUARY);
    affect_to_char( victim, &af );
    act("{WБелая аура{x окружает $c4.", victim, 0, 0, TO_ROOM);
    act("{WБелая аура{x окружает тебя.", victim, 0, 0, TO_CHAR);
}

SPELL_DECL(Stardust);
VOID_SPELL(Stardust)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (has_sanctuary_msg( ch, victim ))
        return;

    af.type      = sn;
    af.level     = level;
    af.duration  = level / 6;
    affect_to_char( victim, &af );
    act("Мерцающая {Wз{wве{Wзд{wная {Wп{wыль закружилась вокруг $c2.", victim, 0, 0, TO_ROOM);
    act("Мерцающая {Wз{wве{Wзд{wная {Wп{wыль закружилась вокруг тебя.", victim, 0, 0, TO_CHAR);
}

SPELL_DECL(Shield);
VOID_SPELL(Shield)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if ( victim->isAffected(sn ) )
    {
        if (victim == ch)
            act("Ты уже под воздействием щита.", ch, 0, 0, TO_CHAR);
        else
            act("$C1 уже под воздействием щита.", ch, 0, victim, TO_CHAR);
        return;
    }

    
    af.type      = sn;
    af.level     = level;
    af.duration  = (8 + level / 3);
    af.location = APPLY_AC;
    af.modifier  = -1 * max(20,10 + level / 3); /* af.modifier  = -20;*/
    
    affect_to_char( victim, &af );

    if (ch->getProfession( )->getFlags( ch ).isSet(PROF_DIVINE)) {
        act("Божественная энергия окружает тебя щитом.", victim, 0, 0, TO_CHAR);
        act("Божественная энергия окружает $c4 щитом.", victim, 0, 0, TO_ROOM);
    } else {
        act("Волшебный щит окружает тебя.", victim, 0, 0, TO_CHAR);
        act("Волшебный щит окружает $c4.", victim, 0, 0, TO_ROOM);
    }
}


SPELL_DECL(StoneSkin);
VOID_SPELL(StoneSkin)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( ch->isAffected(sn ) )
    {
        if (victim == ch)
          ch->pecho("Твоя кожа уже тверда как камень.");
        else
          act_p("Кожа $C2 уже тверда как камень.",
                 ch,0,victim,TO_CHAR,POS_RESTING);
        return;
    }

    
    af.type      = sn;
    af.level     = level;
    af.duration  = (10 + level / 5);
    af.location = APPLY_AC;
    af.modifier  = -1 * max(40,20 + level / 2);  /*af.modifier=-40;*/
    
    affect_to_char( victim, &af );
    act_p( "Кожа $c2 становится тверже камня.",
            victim, 0, 0, TO_ROOM,POS_RESTING);
    victim->pecho("Твоя кожа становится тверже камня.");
}
