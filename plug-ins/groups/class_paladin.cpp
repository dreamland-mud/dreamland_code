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
        ch->println("Тебе недостает мастерства лечить других наложением рук.");
        return;
    }

    if ( (victim = get_char_room(ch,argument)) == 0) {
        ch->send_to("Ты не видишь здесь такого персонажа.\n\r");
        return;
    }

    if ( ch->isAffected(gsn_lay_hands)) {
        ch->send_to("Ты пока не можешь сосредоточиться.\n\r");
        return;
    }

    ch->setWait( gsn_lay_hands->getBeats( ) );

    postaffect_to_char(ch, gsn_lay_hands, 2);

    victim->hit = min( victim->hit + ch->getModifyLevel() * 5, (int)victim->max_hit );
    update_pos( victim );
    
    if (ch != victim) {
        act( "Ты возлагаешь руки на $C4, и $M становится гораздо лучше.", ch, 0, victim, TO_CHAR);
        act( "$c1 возлагает на тебя руки. Тепло наполняет твое тело.", ch, 0, victim, TO_VICT);
        act( "$c1 возлагает руки на $C4. $C1 выглядит намного лучше.", ch, 0, victim, TO_NOTVICT);
    } else {
        act( "Ты возлагаешь на себя руки: тебе становится гораздо лучше.", ch, 0, 0, TO_CHAR);
        act( "$c1 возлагает на себя руки. $c1 выглядит намного лучше.", ch, 0, 0, TO_ROOM);
    }
    
    gsn_lay_hands->improve( ch, true, victim );
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
        act_p("$C1 - вовсе не нечисть и не демон.", ch, 0, victim, TO_CHAR, POS_RESTING);
        return;
    }

    if (saves_spell(level, victim, DAM_HOLY, ch, DAMF_SPELL)) {
        act_p("С $C5, кажется, ничего не происходит.", ch, 0, victim, TO_CHAR, POS_RESTING);
        return;
    }
    
    act_p("Возникает ослепительная вспышка, она поглощает $c4 и $e исчезает.",
            victim, 0, 0, TO_ROOM, POS_RESTING);
        
    raw_kill( victim, -1, ch, FKILL_MOB_EXTRACT );
}


SPELL_DECL(Prayer);
VOID_SPELL(Prayer)::run( Character *ch, char *, int sn, int level ) 
{ 
    Affect af;
    int lvl = max(ch->getRealLevel( ) + 10, 110);

    level = number_range(102, 110);

    if (ch->hit < ch->getRealLevel( ) || ch->mana < ch->getRealLevel( ) || ch->move < ch->getRealLevel( )) {
        act_p("Ты слишком истоще$gно|н|на для молитвы.", ch, 0, 0, TO_CHAR, POS_RESTING);
        return;
    }

    ch->mana -= ch->getModifyLevel( );
    ch->move -= ch->getModifyLevel( );
    ch->hit -= ch->getModifyLevel( );
    update_pos(ch);
    ch->setWaitViolence( 1 );

    if (ch->isAffected(sn) 
        || (number_percent() < number_fuzzy(1) + 50 - ch->getSkill( sn ) / 2))
    {
        // bad 
        act_p("Ты разгнева$gло|л|ла Богов своими молитвами!", ch, 0, 0, TO_CHAR, POS_RESTING);

        if (!ch->isAffected(gsn_weaken)) {
            af.type = gsn_weaken;
            af.level = lvl;
            af.duration = lvl / 15;
            af.location = APPLY_STR;
            af.modifier = -1 * (lvl / 4);
            affect_to_char(ch, &af);
            ch->send_to("Ты чувствуешь, как сила уходит из тебя.\n\r");
            act_p("$c1 выглядит слаб$gым|ым|ой и уставш$gим|им|ой.", ch, NULL, NULL, TO_ROOM, POS_RESTING);
        }
        else if (!IS_AFFECTED(ch, AFF_CURSE) && !IS_SET(ch->imm_flags, IMM_NEGATIVE)) {
            af.bitvector.setTable(&affect_flags);
            af.type = gsn_curse;
            af.level = lvl;
            af.duration = lvl / 10;
            af.location = APPLY_HITROLL;
            af.modifier = -1 * (lvl / 7);
            af.bitvector.setValue(AFF_CURSE);
            affect_to_char(ch, &af);
            af.location = APPLY_SAVING_SPELL;
            af.modifier = lvl / 7;
            affect_to_char(ch, &af);
            act_p("Ты чувствуешь себя проклят$gым|ым|ой.", ch, 0, 0, TO_CHAR, POS_RESTING);
        }
        else {
            if (ch->position == POS_FIGHTING) {
                ch->send_to("Твои мускулы перестают тебе повиноваться...\n\r");
                ch->setDazeViolence( 3 );
                ch->setWaitViolence( 1 );
            }
            else {
                af.bitvector.setTable(&affect_flags);
                af.type = gsn_sleep;
                af.level = lvl;
                af.duration = 3;
                af.bitvector.setValue(AFF_SLEEP);
                affect_join(ch, &af);

                if (IS_AWAKE(ch)) {
                    ch->send_to("Ты засыпаешь....\n\r");
                    act_p("$c1 засыпает.", ch, NULL, NULL, TO_ROOM, POS_RESTING);
                    ch->position = POS_SLEEPING;
                }
            }
        }
        return;
    }

    if (number_percent() > ch->getSkill( sn ) - 5 ) {
        // nothing 
        ch->send_to("Боги слишком заняты, чтобы снизойти до твоих молитв...\n\r");
        return;
    }

    // you did it! 

    ch->send_to("Благословение Богов снизошло на тебя!\n\r");

    postaffect_to_char(ch, sn, number_fuzzy(1 + ch->getRealLevel( ) / 8));

    // random effects 
    sn = -1;

    if (ch->position == POS_FIGHTING && ch->fighting != NULL) {
        switch (number_range(0, 7)) {
        case 0:
            if (IS_GOOD(ch) && IS_EVIL(ch->fighting))
                sn = gsn_ray_of_truth;
            else if (IS_EVIL(ch) && IS_GOOD(ch->fighting))
                sn = gsn_demonfire;
            else
                sn = gsn_flamestrike;
            break;
        case 2:
            sn = gsn_curse;
            break;
        case 5:
            sn = gsn_blindness;
            break;
        case 7:
            sn = gsn_cause_critical;
            break;
        default:
            break;
        }

        if (sn != -1)
            spell( sn, level, ch, ch->fighting );

        ch->setWaitViolence( 1 );
    }
    else {
        switch (number_range(0, 31)) {
        case 0:            sn = gsn_sanctuary;                    break;
        case 1:            sn = gsn_bless;                    break;
        case 2:
        case 3:            sn = gsn_heal;                    break;
        case 4:            sn = gsn_haste;                    break;
        case 5:
        case 6:            sn = gsn_refresh;               break;
        case 7:            sn = gsn_benediction;            break;
        case 8:            sn = gsn_shield;                    break;
        case 9:                
        case 10:    sn = gsn_stone_skin;            break;
        case 11:
        case 12:    sn = gsn_armor;                    break;
        case 13:        
            if (IS_EVIL( ch ))
                sn = gsn_protection_good;
            break;
        case 14:
            if (IS_GOOD( ch ))
                sn = gsn_protection_evil;
            break;
        case 15:
        case 16:    sn = gsn_giant_strength;            break;
        case 17:    sn = gsn_protective_shield;            break;
        case 18:    sn = gsn_frenzy;                break;
        case 19:    sn = gsn_enhanced_armor;            break;
        default:
            break;
        }
        if (sn != -1)
            spell( sn, level, ch, ch );
    }
    

}


SPELL_DECL(TurnUndead);
VOID_SPELL(TurnUndead)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *vch, *vch_next;
    int dam;

    act_p( "$c1 чертит в воздухе священный символ.", ch, 0, 0, TO_ROOM, POS_RESTING);
    act_p( "Ты чертишь в воздухе священный символ.", ch, 0, 0, TO_CHAR, POS_RESTING);

    for (vch = room->people; vch != NULL; vch = vch_next) {
        vch_next = vch->next_in_room;

        if (is_safe_spell( ch, vch, true ))
            continue;
        
        if (!IS_SET( vch->form, FORM_UNDEAD ))
            continue;
        
        if (!IS_SET( vch->act, ACT_UNDEAD ))
            continue;
        
        if (saves_spell( level, vch, DAM_HOLY, ch, DAMF_SPELL )) {
            act("$C1 игнорирует твою слабую попытку изгнания и бросается в атаку!", ch, 0, vch, TO_CHAR);
            act_p("Ты игнорируешь слабую попытку изгнания.", ch, 0, vch, TO_VICT, POS_RESTING);
            damage_nocatch( ch, vch, 0, sn, DAM_HOLY, true, DAMF_SPELL );
        }
        else {
            act_p( "$c5 овладевают священные силы, заставляя в ужасе обратиться в бегство.", vch, 0, ch, TO_ROOM, POS_RESTING);
            act_p( "Священные силы овладевают тобой, заставляя в ужасе обратиться в бегство.", ch, 0, vch, TO_VICT, POS_RESTING);

            dam = dice( level, 12 ); 
            damage_nocatch( ch, vch, dam, sn, DAM_HOLY, true, DAMF_SPELL );
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
        ch->send_to("Это заклинание использовалось совсем недавно.");
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
            ch->send_to("Энергия взрывается внутри тебя!\n\r");
        }

        if (victim != ch) {
            act_p("$c1 высоко вздымает руки, посылая ослепительный луч света!",
                   ch,0,0,TO_ROOM,POS_RESTING);
            ch->send_to("Ты высоко вздымаешь руки, посылая ослепительный луч света!\n\r");
        }

        if (IS_GOOD(victim) || IS_NEUTRAL(victim)) {
            act_p("Свет не может причинить вреда $c3.",
                   victim,0,victim,TO_ROOM,POS_RESTING);
            victim->send_to("Свет не может причинить тебе вреда.\n\r");
            continue;
        }
        
        if (victim->is_npc( )
            && victim->getNPC( )->behavior
            && IS_SET(victim->getNPC( )->behavior->getOccupation( ), (1 << OCC_CLANGUARD)))
        {
            act_p("$c1 не может покинуть свой пост.", victim, 0, 0, TO_ROOM, POS_RESTING);
            continue;
        }
                             
        dam = dice( level, 10 );
        if ( saves_spell( level, victim,DAM_HOLY, ch, DAMF_SPELL ) )
            dam /= 2;

        align = victim->alignment;
        align -= 350;

        if (align < -1000)
            align = -1000 + (align + 1000) / 3;

        dam = (dam * align * align) / 1000000;

        damage_nocatch( ch, victim, dam, sn, DAM_HOLY, true, DAMF_SPELL);

        if (victim->in_room == 0)
            continue;
        
        if (FleeMovement( victim ).move( ))
            break;
    }
}
