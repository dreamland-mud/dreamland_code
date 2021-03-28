/* $Id: group_benedictions.cpp,v 1.1.2.18.6.14 2009/09/11 11:24:54 rufina Exp $
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
#include "affecthandlertemplate.h"

#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "objectbehavior.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "act_move.h"
#include "gsn_plugin.h"
#include "fight_exception.h"

#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "def.h"


GSN(inspiration);
PROF(paladin);
PROF(anti_paladin);

SPELL_DECL(Benediction);
VOID_SPELL(Benediction)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;
    int strength = 0;

    if (victim->isAffected(sn)) {
        if (victim == ch)
            oldact("Ты уже наслаждаешься благостью Богов.", ch,0,0,TO_CHAR);
        else
            oldact("$C1 уже наслаждается благостью Богов.", ch,0,victim,TO_CHAR);

        return;
    }

    if (IS_EVIL(victim))
            strength = IS_EVIL(ch) ? 2 : (IS_GOOD(ch) ? 0 : 1);
    if (IS_GOOD(victim))
            strength = IS_GOOD(ch) ? 2 : (IS_EVIL(ch) ? 0 : 1);
    if (IS_NEUTRAL(victim))
            strength = IS_NEUTRAL(ch) ? 2 : 1;

    if (!strength) {
        oldact("Похоже, твои Боги не благосклонны к $C3.", ch, NULL, victim, TO_CHAR);
        return;
    }

    af.type         = sn;
    af.level        = level;
    af.duration     = 5 + level/2;
    af.location = APPLY_HITROLL;
    af.modifier     = (level / 8) * strength;
    affect_to_char(victim, &af);

    af.location = APPLY_SAVING_SPELL;
    af.modifier     = 0 - (level / 8) * strength;
    affect_to_char(victim, &af);

    if (victim != ch) {
        oldact("Ты одаряешь $C3 благостью своих Богов.", ch,0,victim,TO_CHAR);
        oldact("$c1 одаряет тебе благостью своих Богов.", ch,0,victim,TO_VICT);
    }
    else
        victim->pecho("Ты наслаждаешься божественной благостью.");

}

SPELL_DECL(Bless);
VOID_SPELL(Bless)::run( Character *ch, Object *obj, int sn, int level ) 
{
    Affect af;

    if (obj->behavior && obj->behavior->isLevelAdaptive( ))
    {
        oldact("$o1 отвергает твои попытки благословения.",ch,obj,0,TO_CHAR);
        return;
    }
    if (obj->is_obj_stat(ITEM_BLESS))
    {
        oldact("$o1 уже имеет священную ауру.",ch,obj,0,TO_CHAR);
        return;
    }
    
    if (obj->is_obj_stat(ITEM_EVIL))
    {
        Affect *paf;

        paf = obj->affected.find(gsn_curse);
        if (!savesDispel(level,paf != 0 ? (int)paf->level : obj->level,0))
        {
            if (paf != 0)
                affect_remove_obj( obj, paf, true);
            oldact("Священная аура окружает $o4.",ch,obj,0,TO_ALL);
            REMOVE_BIT(obj->extra_flags,ITEM_EVIL);
            return;
        }
        else
        {
            oldact_p("Дьявольская сила $o2 более могущественна, чем твое благословение.",
                   ch,obj,0,TO_CHAR,POS_RESTING);
            return;
        }
    }
    // let's check, may be a permanet affect
    if (number_percent() < level/4 )
    {  // permanet
      af.bitvector.setTable(&extra_flags);
      af.type                = sn;
      af.level        = level;
      af.duration        = -1;
      af.location = APPLY_SAVES;
      af.modifier        = -1;
      af.bitvector.setValue(ITEM_BLESS);
      affect_to_obj( obj, &af);
            oldact_p("$o1 начинает светиться ровным голубым светом.",
            ch,obj,0,TO_ALL,POS_RESTING);
    }
    else // not a permanent effect
    {
      af.bitvector.setTable(&extra_flags);
      af.type                = sn;
      af.level                = level;
      af.duration        = (6 + level / 2);
      af.location = APPLY_SAVES;
      af.modifier        = ch->isAffected( gsn_inspiration ) ? -3 : -1;
      af.bitvector.setValue(ITEM_BLESS);
      affect_to_obj( obj, &af);
      oldact("Священная аура окружает $o4.",ch,obj,0,TO_ALL);
    }
}

VOID_SPELL(Bless)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if ( victim->isAffected(sn ) ||
         victim->isAffected(gsn_warcry ) )
    {
        if (victim == ch)
          oldact("Ты уже благословле$gно|н|на.", ch,0, 0,TO_CHAR);
        else
          oldact("$C1 уже благословле$Gно|н|на.", ch,0,victim,TO_CHAR);
        return;
    }

    af.type      = sn;
    af.level         = level;
    af.duration  = (6 + level / 2);
    af.location = APPLY_HITROLL;
    af.modifier  = level / 8;
    affect_to_char( victim, &af );

    af.location = APPLY_SAVING_SPELL;
    af.modifier  = 0 - level / 8;
    affect_to_char( victim, &af );
    victim->pecho("Ты чувствуешь божественное благословение.");
    if ( ch != victim )
        oldact("Ты даришь $C3 благословение своих богов.", ch,0,victim,TO_CHAR);

}

SPELL_DECL(Calm);
VOID_SPELL(Calm)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *vch;
    int mlevel = 0;
    int count = 0;
    short high_level = 0;
    int chance;
    Affect af;
    bool found = false;

    /* get sum of all mobile levels in the room */
    for (vch = room->people; vch != 0; vch = vch->next_in_room)
    {
        if (vch->position == POS_FIGHTING)
        {
            count++;
            if (vch->is_npc())
              mlevel += vch->getModifyLevel();
            else
              mlevel += vch->getModifyLevel() / 2;
            high_level = max(high_level, vch->getModifyLevel() );
        }
    }

    /* compute chance of stopping combat */
    chance = 4 * level - high_level + 2 * count;

    if (ch->is_immortal()) /* always works */
      mlevel = 0;

    if (number_range(0, chance) >= mlevel)  /* hard to stop large fights */
    {
        for (vch = room->people; vch != 0; vch = vch->next_in_room)
           {
            if (vch->is_npc() && (IS_SET(vch->imm_flags,IMM_SPELL) ||
                                IS_SET(vch->act,ACT_UNDEAD)))
              continue;

            if (IS_AFFECTED(vch,AFF_CALM) || IS_AFFECTED(vch,AFF_BERSERK)
            ||  vch->isAffected(gsn_frenzy))
              continue;

            vch->pecho("Волна спокойствия окутывает тебя.");
            oldact("Волна спокойствия окутывает $c4.", vch, 0, 0, TO_ROOM );

            if (vch->fighting || vch->position == POS_FIGHTING)
              stop_fighting(vch,false);


            af.type = sn;
            af.level = level;
            af.duration = level/4;
            af.location = APPLY_HITROLL;
            if (!vch->is_npc())
              af.modifier = -5;
            else
              af.modifier = -2;
            affect_to_char(vch,&af);

            af.bitvector.setTable(&affect_flags);
            af.bitvector.setValue(AFF_CALM);
            af.location = APPLY_DAMROLL;
            affect_to_char(vch,&af);
            
            found = true;
        }
    }

    if (!found)
        ch->pecho("Тебе тут некого успокаивать.");
}

SPELL_DECL(Frenzy);
VOID_SPELL(Frenzy)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if (victim->isAffected(sn) || IS_AFFECTED(victim,AFF_BERSERK))
    {
        if (victim == ch)
          ch->pecho("Ты уже в ярости!");
        else
          oldact("$C1 уже в ярости!",ch,0,victim,TO_CHAR);
        return;
    }

    if (victim->isAffected(gsn_calm))
    {
        if (victim == ch)
          ch->pecho("Сейчас тебя ничто не может разозлить.");
        else
          oldact_p("Сейчас ничто не может разозлить $C4.",
                 ch,0,victim,TO_CHAR,POS_RESTING);
        return;
    }

    if ((IS_GOOD(ch) && !IS_GOOD(victim)) ||
        (IS_NEUTRAL(ch) && !IS_NEUTRAL(victim)) ||
        (IS_EVIL(ch) && !IS_EVIL(victim))
       )
    {
        oldact("Твои боги не благосклонны к $C3.",ch,0,victim,TO_CHAR);
        return;
    }

    af.type          = sn;
    af.level         = level;
    af.duration         = level / 3;

    af.modifier  = level / 6;
    af.location = APPLY_HITROLL;
    affect_to_char(victim,&af);

    af.location = APPLY_DAMROLL;
    affect_to_char(victim,&af);

    af.modifier  = 10 * (level / 12);
    af.location = APPLY_AC;
    affect_to_char(victim,&af);

    victim->pecho("Дикая ярость наполняет тебя!");
    oldact_p("В глазах $c2 вспыхивает дикая ярость!",
           victim,0,0,TO_ROOM,POS_RESTING);

}


SPELL_DECL(GroupDefense);
VOID_SPELL(GroupDefense)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *gch;
    Affect af;

    for( gch=room->people; gch != 0; gch=gch->next_in_room)
    {
        if( !is_same_group( gch, ch))
            continue;

        if (spellbane( ch, gch ))
            continue;

        if( !gch->isAffected(gsn_armor ) ) {
            af.type      = gsn_armor;
            af.level     = level;
            af.duration  = level;
            af.location = APPLY_AC;
            af.modifier  = -20;
            affect_to_char( gch, &af );

            oldact("Священная броня окружает тебя.", gch, 0, 0, TO_CHAR);
            oldact("Священная броня окружает $c4.", gch, 0, 0, TO_ROOM);
        } else {
            if( gch == ch)
               oldact("Ты уже защище$gно|н|на заклинанием брони.", ch, 0, 0, TO_CHAR);
            else
                oldact("$C1 уже защище$Gно|н|на заклинанием брони.", ch, 0, gch, TO_CHAR);
        }

        if( gch->isAffected(gsn_shield ) )
            {
            if (gch == ch)
              oldact("Ты уже защище$gно|н|на заклинанием щита.", ch, 0, 0, TO_CHAR);
            else
              oldact("$C1 уже защище$Gно|н|на заклинанием щита.", ch, 0, gch, TO_CHAR);
          continue;
        }

        af.type      = gsn_shield;
        af.level     = level;
        af.duration  = level;
        af.location = APPLY_AC;
        af.modifier   = -20;
        affect_to_char( gch, &af );

        oldact("Божественная энергия окружает тебя щитом.", gch, 0, 0, TO_CHAR);
        oldact("Божественная энергия окружает $c4 щитом.", gch, 0, 0, TO_ROOM);
    }
}


SPELL_DECL(HealingLight);
VOID_SPELL(HealingLight)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Affect af;

    if ( room->isAffected( sn ))
    {
        ch->pecho("Эта комната уже освещена излечивающим светом.");
        return;
    }

    af.type      = sn;
    af.level     = level;
    af.duration  = level / 25;
    af.location.setTable(&apply_room_table);
    af.location = APPLY_ROOM_HEAL;
    af.modifier  = level;
    room->affectTo( &af );

    postaffect_to_char(ch, sn, level / 10);
    ch->pecho("Комната освещается излечивающим светом.");
    oldact_p("$c1 освещает комнату излечивающим светом.",
           ch,0,0,TO_ROOM,POS_RESTING);
}

AFFECT_DECL(HealingLight);
VOID_AFFECT(HealingLight)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "Все вокруг освещено излечивающим светом, который ускорит "
                   "восстановление здоровья на {W%2$d{x в течение {W%1$d{x ча%1$Iса|сов|сов.",
                   paf->duration, paf->modifier )
        << endl;
}

SPELL_DECL(HolyWord);
VOID_SPELL(HolyWord)::run( Character *ch, Room *room, int sn, int level ) 
{ 

    int dam;
    int bless_num, curse_num, frenzy_num;

    bless_num = gsn_bless;
    curse_num = gsn_curse;
    frenzy_num = gsn_frenzy;

    oldact_p("$c1 произносит заклинание {WБожественной Силы{x!",
           ch,0,0,TO_ROOM,POS_RESTING);
    ch->pecho("Ты произносишь заклинание {WБожественной Силы{x!");

    for ( auto &vch : room->getPeople() )
    {

        if(!vch->isDead() && vch->in_room == room){
        
        if (vch->is_mirror() && number_percent() < 50) 
            continue;

        try {
            if ((IS_GOOD(ch) && IS_GOOD(vch)) ||
                (IS_EVIL(ch) && IS_EVIL(vch)) ||
                (IS_NEUTRAL(ch) && IS_NEUTRAL(vch)) )
            {
                if (spellbane( ch, vch ))
                    continue;

              vch->pecho("Ты чувствуешь себя более могущественно.");
              spell(frenzy_num,level,ch, vch);
              spell(bless_num,level,ch, vch);
            }

            else if ((IS_GOOD(ch) && IS_EVIL(vch)) ||
                     (IS_EVIL(ch) && IS_GOOD(vch)) )
            {
              if (!is_safe_spell(ch,vch,true))
              {
                if (ch->fighting != vch && vch->fighting != ch)
                    yell_panic( ch, vch );

                spell(curse_num,level,ch, vch);
                vch->pecho("Божественная сила повергает тебя!");
                dam = dice(level,6);
                try{
                damage_nocatch(ch,vch,dam,sn,DAM_HOLY, true, DAMF_PRAYER);
                }
                catch (const VictimDeathException &){
                    continue;
                }
              }
            }

            else if (IS_NEUTRAL(ch))
            {
              if (!is_safe_spell(ch,vch,true))
              {
                if (ch->fighting != vch && vch->fighting != ch)
                    yell_panic( ch, vch );

                spell(curse_num,level/2,ch, vch);
                vch->pecho("Божественная сила повергает тебя!");
                dam = dice(level,4);
                try{
                damage_nocatch(ch,vch,dam,sn,DAM_HOLY, true, DAMF_PRAYER);
                }
                catch (const VictimDeathException &){
                    continue;
                }
              }
            }
        } catch (const VictimDeathException &ex) {
            ch->pecho("Ты чувствуешь себя опустошенно.");
            ch->move /= (4/3);
            ch->hit /= (4/3);
            throw ex;
        }
    }
    }

    ch->pecho("Ты чувствуешь себя опустошенно.");
/*    gain_exp( ch, -1 * number_range(1,10) * 5);           */
    ch->move /= (4/3);
    ch->hit /= (4/3);
}


SPELL_DECL(Inspire);
VOID_SPELL(Inspire)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *gch;
    Affect af;

    for( gch=room->people; gch != 0; gch=gch->next_in_room )
    {
            if( !is_same_group( gch, ch) )
                    continue;

            if (spellbane( ch, gch ))
                continue;

            if ( gch->isAffected(sn ) )
            {
              if(gch == ch)
                  ch->pecho("Ты уже воодушевле%Gно|н|на.", ch);
              else
                  ch->pecho("%1$^C1 уже воодушевле%1$Gно|н|на.", gch);
              continue;
            }

            af.type      = sn;
            af.level     = level;
            af.duration  = 6 + level;

            af.location = APPLY_HITROLL;
            af.modifier  = level/12;
            affect_to_char( gch, &af );

            af.location = APPLY_SAVING_SPELL;
            af.modifier  = 0 - level/12;
            affect_to_char( gch, &af );

            gch->pecho("Ты чувствуешь воодушевление!");
            if( ch != gch )
                oldact("Ты воодушевляешь $C4 силой Создателя!", ch, 0, gch, TO_CHAR);

    }
}

SPELL_DECL(RayOfTruth);
VOID_SPELL(RayOfTruth)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam, align;

    if (IS_EVIL(ch) )
    {
        victim = ch;
        ch->pecho("Энергия взрывается внутри тебя!");
    }

    if (victim != ch)
    {
        oldact_p("$c1 взмахивает руками, посылая ослепительный луч света!",
               ch,0,0,TO_ROOM,POS_RESTING);
        ch->pecho("Ты взмахиваешь руками, посылая ослепительный луч света!");
    }

    if (IS_GOOD(victim))
    {
        oldact_p("Ослепительный луч света не может повредить $c3.",
               victim,0,victim,TO_ROOM,POS_RESTING);
        victim->pecho("Ослепительный луч света не может повредить тебе.");
        return;
    }

    dam = dice( level, 10 );

        if( ch->getProfession( ) == prof_paladin ||
        ch->getProfession( ) == prof_anti_paladin )
                dam = dam + dam / 2;

    if ( saves_spell( level, victim,DAM_HOLY, ch, DAMF_PRAYER) )
        dam /= 2;
    
    if (victim->is_npc( ))
        align = victim->alignment;
    else if (IS_NEUTRAL(victim))
        align = 0;
    else
        align = -1000;
        
    align -= 350;

    if (align < -1000)
        align = -1000 + (align + 1000) / 3;

    dam = (dam * align * align) / 750000;
    
    if (!IS_AFFECTED(victim, AFF_BLIND))
        spell(gsn_blindness, 3 * level / 4, ch, victim);

    damage_nocatch( ch, victim, dam, sn, DAM_HOLY ,true, DAMF_PRAYER);
}


SPELL_DECL(RestoringLight);
VOID_SPELL(RestoringLight)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int mana_add;

    if (IS_AFFECTED(victim,AFF_BLIND))
        {
         spell(gsn_cure_blindness,level,ch,victim);
        }
    if (IS_AFFECTED(victim,AFF_CURSE))
        {
         spell(gsn_remove_curse,level,ch,victim);
        }
    if (IS_AFFECTED(victim,AFF_POISON))
        {
         spell(gsn_cure_poison,level,ch,victim);
        }
    if (IS_AFFECTED(victim,AFF_PLAGUE))
        {
         spell(gsn_cure_disease,level,ch,victim);
        }

    if (victim->hit != victim->max_hit)
        {
             mana_add = min( (victim->max_hit - victim->hit), (int)ch->mana );
             victim->hit = min( victim->hit + mana_add, (int)victim->max_hit );
             ch->mana -= mana_add;
        }
    update_pos( victim );
    victim->pecho("Волна тепла согревает твое тело.");
    if ( ch != victim )
        ch->pecho("Ok.");
    return;

}


SPELL_DECL(SanctifyLands);
VOID_SPELL(SanctifyLands)::run( Character *ch, Room *room, int sn, int level ) 
{ 
  if (number_bits(1) == 0)
    {
      ch->pecho("Твоя попытка закончилась неудачей.");
      return;
    }

    bool clean = true;

  if (IS_ROOM_AFFECTED(room,AFF_ROOM_CURSE))
        {
         clean = false;
         room->affectStrip( gsn_cursed_lands);
         ch->pecho("Ты развеиваешь проклятие, висевшее над этой местностью.");
         oldact_p("Проклятие, висевшее над этой местностью, развеивается.\n\r",
                ch,0,0,TO_ROOM,POS_RESTING);
        }
  if (IS_ROOM_AFFECTED(room,AFF_ROOM_POISON))
        {
         clean = false;
         room->affectStrip( gsn_deadly_venom);
         ch->pecho("Ядовитые пары, клубившиеся в этой местности, рассеиваются.");
         oldact_p("Ядовитые пары, клубившиеся в этой местности, рассеиваются.\n\r",
                ch,0,0,TO_ROOM,POS_RESTING);
        }
  if (IS_ROOM_AFFECTED(room,AFF_ROOM_SLEEP))
        {
         clean = false;
         ch->pecho("Усыпляющие чары, висевшие над этой местностью, развеиваются.");
         oldact_p("Усыпляющие чары, висевшие над этой местностью, развеиваются.\n\r",
                ch,0,0,TO_ROOM,POS_RESTING);
         room->affectStrip( gsn_mysterious_dream);
        }
  if (IS_ROOM_AFFECTED(room,AFF_ROOM_PLAGUE))
        {
         clean = false;
         ch->pecho("Чумные миазмы, клубившиеся в этой местности, рассеиваются.");
         oldact_p("Чумные миазмы, клубившиеся в этой местности, рассеиваются.\n\r",
                ch,0,0,TO_ROOM,POS_RESTING);
         room->affectStrip( gsn_black_death);
        }
  if (IS_ROOM_AFFECTED(room,AFF_ROOM_SLOW))
        {
         clean = false;
         ch->pecho("Летаргический туман, клубившийся в этой местности, рассеивается.");
         oldact_p("Летаргический туман, клубившийся в этой местности, рассеивается.\n\r",
                ch,0,0,TO_ROOM,POS_RESTING);
         room->affectStrip( gsn_lethargic_mist);
        }

    if (clean)
        ch->pecho("Эта местность не нуждается в очищении.");
}

SPELL_DECL(Wrath);
VOID_SPELL(Wrath)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam;
    Affect af;

    if ( !ch->is_npc() && IS_EVIL(ch) )
        victim = ch;

    if ( IS_GOOD(victim) ) {
        oldact("Силы Света защищают $c4.", victim, 0, 0, TO_ROOM);
        oldact("Силы Света защищают тебя.", victim, 0, 0, TO_CHAR);
        return;
    }

    if ( IS_NEUTRAL(victim) ) {
        oldact("Это заклинание не действует на $C4.", ch, 0, victim, TO_CHAR);
        return;
    }

    dam = dice(level,14);

    if ( saves_spell( level, victim, DAM_HOLY,ch, DAMF_PRAYER ) )
        dam /= 2;

    if (!IS_AFFECTED(victim, AFF_CURSE) && !saves_spell( level, victim, DAM_HOLY, ch, DAMF_PRAYER ) )
    {
        af.type      = sn;
        af.level     = level;
        af.duration  = 2*level;
        af.location = APPLY_HITROLL;
        af.modifier  = -1 * (level / 8);
        affect_to_char( victim, &af );

        af.bitvector.setTable(&affect_flags);
        af.bitvector.setValue(AFF_CURSE);
        af.location = APPLY_SAVING_SPELL;
        af.modifier  = level / 8;
        affect_to_char( victim, &af );

        victim->pecho("Ты чувствуешь себя отвратительно.");

        if ( ch != victim )
            oldact("$C1 выглядит отвратительно.",ch,0,victim,TO_CHAR);
    }

    damage_nocatch( ch, victim, dam, sn, DAM_HOLY, true, DAMF_PRAYER );
}
