#include "spelltemplate.h"

#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "act_move.h"
#include "gsn_plugin.h"
#include "effects.h"
#include "damage.h"
#include "loadsave.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"

SPELL_DECL(DragonStrength);
VOID_SPELL(DragonStrength)::run( Character *ch, Character *, int sn, int level ) 
{ 
  Affect af;

  if (ch->isAffected(sn))
    {
      ch->pecho("Сила Дракона уже переполняет тебя.");
      return;
    }

  af.type = sn;
  af.level = level;
  af.duration = ch->getModifyLevel() / 3;

  af.modifier = max( ch->getModifyLevel() / 10, 2 );
  af.location = APPLY_HITROLL;
  affect_to_char(ch, &af);

  af.modifier = max( ch->getModifyLevel() / 10, 2 );
  af.location = APPLY_DAMROLL;
  affect_to_char(ch, &af);

  af.modifier = -max( ch->getModifyLevel() / 10, 2 );
  af.location = APPLY_AC;
  affect_to_char(ch, &af);

  af.modifier = max( ch->getModifyLevel() / 30, 1 );
  af.location = APPLY_STR;
  affect_to_char(ch, &af);

  af.modifier = -max( ch->getModifyLevel() / 30, 1 );;
  af.location = APPLY_DEX;
  affect_to_char(ch, &af);

  ch->pecho("Сила Дракона пронизывает тебя.");
  act("%^C1 становится сильнее.", ch, 0, 0,TO_ROOM);

}




SPELL_DECL(AcidBreath);
VOID_SPELL(AcidBreath)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam,hp_dam,dice_dam,hpch;

    act("%1$^C1 брызгает кислотой на %2$C4.",ch,victim,0,TO_NOTVICT);
    oldact("$c1 брызгает струей едкой кислоты на тебя.", ch,0,victim,TO_VICT);
    act("Ты брызгаешь кислотой на %2$C4.",ch,victim,0,TO_CHAR);

    hpch = max(12,(int)ch->hit);
    hp_dam = number_range(hpch/11 + 1, hpch/6);
    dice_dam = dice(level,16);

    dam = max(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

    if (saves_spell(level,victim,DAM_ACID, ch, DAMF_MAGIC))
    {
        acid_effect(victim,level/2,dam/4,TARGET_CHAR, DAMF_MAGIC);
        damage_nocatch(ch,victim,dam/2,sn,DAM_ACID,true, DAMF_MAGIC);
    }
    else
    {
        acid_effect(victim,level,dam,TARGET_CHAR, DAMF_MAGIC);
        damage_nocatch(ch,victim,dam,sn,DAM_ACID,true, DAMF_MAGIC);
    }

}



SPELL_DECL(FireBreath);
VOID_SPELL(FireBreath)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam,hp_dam,dice_dam;
    int hpch;
    
    act("%^C1 выдыхает воронку бушующего огня.", ch,victim,0,TO_NOTVICT);
    act("%^C1 выдыхает на тебя воронку бушующего огня!", ch,victim,0,TO_VICT);
    act("Ты выдыхаешь воронку бушующего огня.", ch,0,0,TO_CHAR);

    hpch = max( 10, (int)ch->hit );
    hp_dam  = number_range( hpch/9+1, hpch/5 );
    dice_dam = dice(level,20);

    dam = max(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
    fire_effect(victim->in_room,level,dam/2,TARGET_ROOM, DAMF_MAGIC);

    for ( auto &vch : victim->in_room->getPeople())
    {
       if(!vch->isDead() && vch->in_room == victim->in_room){

        if ( vch->is_mirror()
            && ( number_percent() < 50 ) ) continue;


        if (is_safe_spell(ch,vch,true)
        ||  ( vch->is_npc() && ch->is_npc()
        &&  (ch->fighting != vch && vch->fighting != ch )))
            continue;
        if ( is_safe(ch, vch) )
          continue;

        if (vch == victim) /* full damage */
        {
                try{
            if (saves_spell(level,vch,DAM_FIRE, ch, DAMF_MAGIC))
            {
                fire_effect(vch,level/2,dam/4,TARGET_CHAR, DAMF_MAGIC);
                damage_nocatch(ch,vch,dam/2,sn,DAM_FIRE,true, DAMF_MAGIC);
            }
            else
            {
                fire_effect(vch,level,dam,TARGET_CHAR, DAMF_MAGIC);
                damage_nocatch(ch,vch,dam,sn,DAM_FIRE,true, DAMF_MAGIC);
            }
                }
                catch (VictimDeathException &){
                        continue;
                }
        }
        else /* partial damage */
        {
                try{
            if (saves_spell(level - 2,vch,DAM_FIRE, ch, DAMF_MAGIC))
            {
                fire_effect(vch,level/4,dam/8,TARGET_CHAR, DAMF_MAGIC);
                damage_nocatch(ch,vch,dam/4,sn,DAM_FIRE,true, DAMF_MAGIC);
            }
            else
            {
                fire_effect(vch,level/2,dam/4,TARGET_CHAR, DAMF_MAGIC);
                damage_nocatch(ch,vch,dam/2,sn,DAM_FIRE,true, DAMF_MAGIC);
            }
                }
                catch (const VictimDeathException &){
                        continue;
                }
        }
       }
    }

}

SPELL_DECL(FrostBreath);
VOID_SPELL(FrostBreath)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam,hp_dam,dice_dam, hpch;
    
    act("%^C1 выдыхает леденящую воронку инея!", ch, victim, 0,TO_NOTVICT);
    act("%^C1 выдыхает на тебя леденящую воронку инея!", ch, victim, 0,TO_VICT);
    act("Ты выдыхаешь воронку инея.", ch, victim, 0,TO_CHAR);

    hpch = max(12,(int)ch->hit);
    hp_dam = number_range(hpch/11 + 1, hpch/6);
    dice_dam = dice(level,16);

    dam = max(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    cold_effect(victim->in_room,level,dam/2,TARGET_ROOM, DAMF_MAGIC);

    for ( auto &vch : victim->in_room->getPeople())
    {
        if(!vch->isDead() && vch->in_room == victim->in_room){

        if ( vch->is_mirror()
            && ( number_percent() < 50 ) ) continue;

        if (is_safe_spell(ch,vch,true)
        ||  (vch->is_npc() && ch->is_npc()
        &&   (ch->fighting != vch && vch->fighting != ch)))
            continue;
        if ( is_safe(ch, vch) )
          continue;


        if (vch == victim) /* full damage */
        {
                try{
            if (saves_spell(level,vch,DAM_COLD, ch, DAMF_MAGIC))
            {
                cold_effect(vch,level/2,dam/4,TARGET_CHAR, DAMF_MAGIC);
                damage_nocatch(ch,vch,dam/2,sn,DAM_COLD,true, DAMF_MAGIC);
            }
            else
            {
                cold_effect(vch,level,dam,TARGET_CHAR, DAMF_MAGIC);
                damage_nocatch(ch,vch,dam,sn,DAM_COLD,true, DAMF_MAGIC);
            }
                }
                catch (const VictimDeathException &){
                        continue;
                }
        }
        else
        {
                try{
            if (saves_spell(level - 2,vch,DAM_COLD, ch, DAMF_MAGIC))
            {
                cold_effect(vch,level/4,dam/8,TARGET_CHAR, DAMF_MAGIC);
                damage_nocatch(ch,vch,dam/4,sn,DAM_COLD,true, DAMF_MAGIC);
            }
            else
            {
                cold_effect(vch,level/2,dam/4,TARGET_CHAR, DAMF_MAGIC);
                damage_nocatch(ch,vch,dam/2,sn,DAM_COLD,true, DAMF_MAGIC);
            }
                }
                catch (const VictimDeathException &){
                        continue;
                }
        }
        }
    }

}


SPELL_DECL(GasBreath);
VOID_SPELL(GasBreath)::run( Character *ch, Room *room, int sn, int level ) 
{ 

    int dam,hp_dam,dice_dam,hpch;

    act("%^C1 выдыхает воронку ядовитого газа!", ch, 0, 0,TO_ROOM);
    act("Ты выдыхаешь воронку ядовитого газа.", ch, 0, 0,TO_CHAR);

    hpch = max(16,(int)ch->hit);
    hp_dam = number_range(hpch/15+1,8);
    dice_dam = dice(level,12);

    dam = max(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    poison_effect(room,level,dam,TARGET_ROOM, DAMF_MAGIC);

    for ( auto &vch : room->getPeople())
    {
        if(!vch->isDead() && vch->in_room == room){

        if ( vch->is_mirror()
            && ( number_percent() < 50 ) ) continue;

        if (is_safe_spell(ch,vch,true)
        ||  (ch->is_npc() && vch->is_npc()
        &&   (ch->fighting != vch && vch->fighting != ch)))
            continue;
        if ( is_safe(ch, vch) )
          continue;

        if (ch->fighting != vch && vch->fighting != ch)
            yell_panic( ch, vch );

        try{

        if (saves_spell(level,vch,DAM_POISON, ch, DAMF_MAGIC))
        {
            poison_effect(vch,level/2,dam/4,TARGET_CHAR, DAMF_MAGIC);
            damage_nocatch(ch,vch,dam/2,sn,DAM_POISON,true, DAMF_MAGIC);
        }
        else
        {
            poison_effect(vch,level,dam,TARGET_CHAR, DAMF_MAGIC);
            damage_nocatch(ch,vch,dam,sn,DAM_POISON,true, DAMF_MAGIC);
        }

        }

        catch (const VictimDeathException &){
                continue;
        }
        }
    }

}


SPELL_DECL(LightningBreath);
VOID_SPELL(LightningBreath)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    int dam,hp_dam,dice_dam,hpch;

    act("Выдох %1$C2 ударяет по %2$C3 разрядом молнии.", ch, victim, 0,TO_NOTVICT);
    act("Выдох %C2 ударяет по тебе разрядом молнии!", ch, victim, 0,TO_VICT);
    act("Твой выдох ударяет по %2$C3 разрядом молнии.", ch, victim, 0,TO_CHAR);

    hpch = max(10,(int)ch->hit);
    hp_dam = number_range(hpch/9+1,hpch/5);
    dice_dam = dice(level,20);

    dam = max(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

    if (saves_spell(level,victim,DAM_LIGHTNING, ch, DAMF_MAGIC))
    {
        shock_effect(victim,level/2,dam/4,TARGET_CHAR, DAMF_MAGIC);
        damage_nocatch(ch,victim,dam/2,sn,DAM_LIGHTNING,true, DAMF_MAGIC);
    }
    else
    {
        shock_effect(victim,level,dam,TARGET_CHAR, DAMF_MAGIC);
        damage_nocatch(ch,victim,dam,sn,DAM_LIGHTNING,true, DAMF_MAGIC);
    }

}

