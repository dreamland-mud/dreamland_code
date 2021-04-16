/* $Id: class_paladin.cpp,v 1.1.2.13.6.10 2010-09-01 21:20:44 rufina Exp $
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

#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"
#include "spelltemplate.h"

#include "fleemovement.h"

#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "object.h"
#include "gsn_plugin.h"
#include "occupations.h"
#include "act_move.h"
#include "mercdb.h"

#include "magic.h"
#include "fight.h"
#include "damage.h"
#include "handler.h"
#include "vnum.h"
#include "merc.h"
#include "act.h"
#include "interp.h"
#include "def.h"
#include "skill_utils.h"

RACE(golem);
RACE(demon);

/*
 * 'layhands' skill command
 */

SKILL_RUNP( layhands )
{
    Character *victim;

    if ( ch->is_npc() || !gsn_lay_hands->usable( ch ) )
    {
        ch->pecho("Тебе недостает мастерства лечить других наложением рук.");
        return;
    }

    if ( (victim = get_char_room(ch,argument)) == 0) {
        ch->pecho("Ты не видишь здесь такого персонажа.");
        return;
    }

    if ( ch->isAffected(gsn_lay_hands)) {
        ch->pecho("Ты пока не можешь сосредоточиться.");
        return;
    }

    ch->setWait( gsn_lay_hands->getBeats(ch) );
    if ( number_percent() < gsn_lay_hands->getEffective( ch ) + skill_level_bonus(*gsn_lay_hands, ch) ) {
        postaffect_to_char(ch, gsn_lay_hands, 2);

        int slevel, chance, sbonus;
        slevel = skill_level(*gsn_lay_hands, ch);
        sbonus = skill_level_bonus(*gsn_lay_hands, ch);
        chance = gsn_holy_remedy->getEffective( ch );

        if (number_percent( ) < chance) {
            if (sbonus >= 0)
                sbonus += chance / 20;
            else
                sbonus = chance / 20;
            
            oldact("Свет на мгновение пронизывает твои ладони.", ch, 0, 0, TO_CHAR );
            oldact("Свет на мгновение пронизывает ладони $c2.", ch, 0, 0, TO_ROOM );
            gsn_holy_remedy->improve( ch, true );
        }
        
        victim->hit = min( victim->hit + slevel * 5 + sbonus * 20, (int)victim->max_hit );
        update_pos( victim );
    
        if (ch != victim) {
            oldact("Ты возлагаешь руки на $C4, и $M становится гораздо лучше.", ch, 0, victim, TO_CHAR);
            oldact("$c1 возлагает на тебя руки. Тепло наполняет твое тело.", ch, 0, victim, TO_VICT);
            oldact("$c1 возлагает руки на $C4. $C1 выглядит намного лучше.", ch, 0, victim, TO_NOTVICT);
        } else {
            oldact("Ты возлагаешь на себя руки и тебе становится гораздо лучше.", ch, 0, 0, TO_CHAR);
            oldact("$c1 возлагает на себя руки. $c1 выглядит намного лучше.", ch, 0, 0, TO_ROOM);
        }
    
        gsn_lay_hands->improve( ch, true, victim );        
    }
    else {
      if (ch != victim) {
          oldact("Ты возлагаешь руки на $C4, но ничего не происходит.", ch, 0, victim, TO_CHAR);
          oldact("$c1 картинным жестом возлагает на тебя руки... но ничего не происходит.", ch, 0, victim, TO_VICT);
          oldact("$c1 картинным жестом возлагает руки на $C4... но ничего не происходит.", ch, 0, victim, TO_NOTVICT);
      } else {
          oldact("Ты возлагаешь на себя руки, но ничего не происходит.", ch, 0, 0, TO_CHAR);
          oldact("$c1 картинным жестом возлагает на себя руки... но ничего не происходит.", ch, 0, 0, TO_ROOM);
      }
      gsn_lay_hands->improve( ch, false, victim );        
    }
    


}

SPELL_DECL(Banishment);
VOID_SPELL(Banishment)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    if (!victim->is_npc( ) 
        || (!IS_SET( victim->form, FORM_UNDEAD )
           && !IS_SET( victim->act, ACT_UNDEAD )
           && victim->getRace( ) != race_demon
           && victim->getRace( ) != race_golem))
    {
        oldact("К сожалению, $C1 -- не нечисть, не демон и не богомерзкий голем.", ch, 0, victim, TO_CHAR);
        return;
    }
    
    if (saves_spell(level, victim, DAM_HOLY, ch, DAMF_PRAYER)) {
        oldact("С $C5, кажется, ничего не происходит.", ch, 0, victim, TO_CHAR);
        return;
    }
    
    oldact_p("Возникает ослепительная вспышка, она поглощает $c4 и $e исчезает.",
            victim, 0, 0, TO_ROOM, POS_RESTING);
        
    raw_kill( victim, -1, ch, FKILL_MOB_EXTRACT );
}


SPELL_DECL(Prayer);
VOID_SPELL(Prayer)::run( Character *ch, char *, int sn, int level ) 
{ 
    Affect af;
    int punish_lvl, sk, roll;
    punish_lvl = number_range(102, 110);    // negative effect level -- punished by gods
    level = number_range(level, 110);  // positive effect level
    roll = number_percent();
    sk = gsn_liturgy->getEffective( ch );

    if (ch->hit  < ch->max_hit  / 10 ||
        ch->mana < ch->max_mana / 10 ||
        ch->move < ch->max_move / 10) {
        oldact("Ты слишком истоще$gно|н|на для молитвы.", ch, 0, 0, TO_CHAR);
        return;
    }

    ch->mana -= ch->max_mana  / 10; 
    ch->move -= ch->max_move / 10;
    ch->hit  -= ch->max_hit / 10;
    update_pos(ch);

    // 20% chance to fail with max skill
    if (ch->isAffected(sn) || roll > sk * 4 / 5 )
    {         
        oldact("Ты разгнева$gло|л|ла Богов своими молитвами!", ch, 0, 0, TO_CHAR);
        
        if (!ch->isAffected(gsn_weaken)) {
            af.type = gsn_weaken;
            af.level = punish_lvl;
            af.duration = punish_lvl / 15;
            af.location = APPLY_STR;
            af.modifier = -1 * (punish_lvl / 4);
            affect_to_char(ch, &af);
            ch->pecho("Ты чувствуешь, как сила уходит из тебя.");
            oldact("$c1 выглядит слаб$gым|ым|ой и уставш$gим|им|ой.", ch, NULL, NULL, TO_ROOM);
        }
        else if (!IS_AFFECTED(ch, AFF_CURSE) && !IS_SET(ch->imm_flags, IMM_NEGATIVE)) {
            af.bitvector.setTable(&affect_flags);
            af.type = gsn_curse;
            af.level = punish_lvl;
            af.duration = punish_lvl / 10;
            af.location = APPLY_HITROLL;
            af.modifier = -1 * (punish_lvl / 7);
            af.bitvector.setValue(AFF_CURSE);
            affect_to_char(ch, &af);
            af.location = APPLY_SAVING_SPELL;
            af.modifier = punish_lvl / 7;
            affect_to_char(ch, &af);
            oldact("Ты чувствуешь себя проклят$gым|ым|ой.", ch, 0, 0, TO_CHAR);
        }
        else {
            if (ch->fighting) {
                ch->pecho("Твои мускулы перестают тебе повиноваться...");
                ch->setDazeViolence( 3 );
                ch->setWaitViolence( 1 );
            }
            else {
                af.bitvector.setTable(&affect_flags);
                af.type = gsn_sleep;
                af.level = punish_lvl;
                af.duration = 3;
                af.bitvector.setValue(AFF_SLEEP);
                affect_join(ch, &af);

                if (IS_AWAKE(ch)) {
                    ch->pecho("Ты засыпаешь....");
                    oldact("$c1 засыпает.", ch, NULL, NULL, TO_ROOM);
                    ch->position = POS_SLEEPING;
                }
            }
        }
        return;
    }

    // 80% chance to not fail with max skill   
    if (ch->fighting) {
        // 10% chance to fizzle with max skill
        if (roll < sk / 10 ) {
            ch->pecho("Боги слишком заняты, чтобы снизойти до твоих молитв...");
            return;
        }
        
        postaffect_to_char(ch, sn, level/10 + 1);
        
        // 10% chance to just get the affect
        if (roll < sk / 5 ) {
            ch->pecho("Твоя молитва не увенчалась успехом.");
            return;
        }

        ch->pecho("{WБлагословение Богов снизошло на тебя!{x");

        switch (number_range(0, 3)) {
        case 0:
            if (IS_GOOD(ch) && IS_EVIL(ch->fighting))
                sn = gsn_ray_of_truth;
            else if (IS_EVIL(ch) && IS_GOOD(ch->fighting))
                sn = gsn_demonfire;
            else
                sn = gsn_flamestrike;
            break;
        case 1:
            sn = gsn_curse;
            break;
        case 2:
            sn = gsn_blindness;
            break;
        case 3:
            sn = gsn_severity_force;
            break;
        default:
            break;
        }
 
        spell( sn, level, ch, ch->fighting );
        return;
    }
    else {
        // 25% chance to fizzle with max skill
        if (roll < sk / 4 ) {
            ch->pecho("Боги слишком заняты, чтобы снизойти до твоих молитв...");
            return;
        }
        
        ch->pecho("{WБлагословение Богов снизошло на тебя!{x");

        af.bitvector.setTable(&affect_flags);
        af.type         = sn;
        af.level        = level;
        af.duration     = level / 10 + 1;

        af.location = APPLY_LEARNED;
        af.modifier     = level / 10 + 1;
        affect_to_char(ch, &af);

        af.location = APPLY_LEVEL;
        af.modifier     = level / 40 + 1;
        affect_to_char(ch, &af);
        return;
    }
}


SPELL_DECL(TurnUndead);
VOID_SPELL(TurnUndead)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *vch, *vch_next;
    int dam;

    oldact("$c1 чертит в воздухе священный символ.", ch, 0, 0, TO_ROOM);
    oldact("Ты чертишь в воздухе священный символ.", ch, 0, 0, TO_CHAR);

    for (vch = room->people; vch != NULL; vch = vch_next) {
        vch_next = vch->next_in_room;

        if (is_safe_spell( ch, vch, true ))
            continue;
        
        if (!IS_SET( vch->form, FORM_UNDEAD ))
            continue;
        
        if (!IS_SET( vch->act, ACT_UNDEAD ))
            continue;
        
        if (saves_spell( level, vch, DAM_HOLY, ch, DAMF_PRAYER )) {
            oldact("$C1 игнорирует твою слабую попытку изгнания и бросается в атаку!", ch, 0, vch, TO_CHAR);
            oldact("Ты игнорируешь слабую попытку изгнания.", ch, 0, vch, TO_VICT);
            damage_nocatch( ch, vch, 0, sn, DAM_HOLY, true, DAMF_PRAYER );
        }
        else {
            oldact("$c5 овладевают священные силы, заставляя в ужасе обратиться в бегство.", vch, 0, ch, TO_ROOM);
            oldact("Священные силы овладевают тобой, заставляя в ужасе обратиться в бегство.", ch, 0, vch, TO_VICT);

            dam = dice( level, 12 ); 
            damage_nocatch( ch, vch, dam, sn, DAM_HOLY, true, DAMF_PRAYER );
            FleeMovement( vch ).move( );
        }
    }

}

SPELL_DECL(Turn);
VOID_SPELL(Turn)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *victim, *victim_next;

    if ( ch->isAffected(sn ) )
    {
        ch->pecho("Это заклинание использовалось совсем недавно.");
        return;
    }

    postaffect_to_char(ch, sn, 5);

    for (victim = room->people; victim != 0; victim = victim_next)
    {
        int dam, align, level;

        victim_next = victim->next_in_room;
        level = ch->getModifyLevel( );

        if (is_safe_spell(ch,victim,true))
            continue;
        if (is_safe(ch, victim))
          continue;

        if (IS_EVIL(ch) ) {
            victim = ch;
            ch->pecho("Энергия взрывается внутри тебя!");
        }

        if (victim != ch) {
            oldact_p("$c1 высоко вздымает руки, посылая ослепительный луч света!",
                   ch,0,0,TO_ROOM,POS_RESTING);
            ch->pecho("Ты высоко вздымаешь руки, посылая ослепительный луч света!");
        }

        if (IS_GOOD(victim) || IS_NEUTRAL(victim)) {
            oldact_p("Свет не может причинить вреда $c3.",
                   victim,0,victim,TO_ROOM,POS_RESTING);
            victim->pecho("Свет не может причинить тебе вреда.");
            continue;
        }
        
        if (victim->is_npc( )
            && victim->getNPC( )->behavior
            && IS_SET(victim->getNPC( )->behavior->getOccupation( ), (1 << OCC_CLANGUARD)))
        {
            oldact("$c1 не может покинуть свой пост.", victim, 0, 0, TO_ROOM);
            continue;
        }
                             
        dam = dice( level, 10 );
        if ( saves_spell( level, victim,DAM_HOLY, ch, DAMF_PRAYER ) )
            dam /= 2;

        align = victim->alignment;
        align -= 350;

        if (align < -1000)
            align = -1000 + (align + 1000) / 3;

        dam = (dam * align * align) / 1000000;

        damage_nocatch( ch, victim, dam, sn, DAM_HOLY, true, DAMF_PRAYER);

        if (victim->in_room == 0)
            continue;
        
        if (FleeMovement( victim ).move( ))
            break;
    }
}
