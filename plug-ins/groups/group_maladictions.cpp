
/* $Id: group_maladictions.cpp,v 1.1.2.21.6.14 2010-08-24 20:31:55 rufina Exp $
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
#include "drink_utils.h"
#include "math_utils.h"
#include "damage.h"

#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "def.h"



SPELL_DECL(Anathema);
VOID_SPELL(Anathema)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;
    int strength = 0;

    if (victim->isAffected(sn)) {
        act_p("$C1 уже прокля$Gто|т|та.", ch,0,victim,TO_CHAR,POS_RESTING);
        return;
    }

    if (IS_GOOD(victim))
        strength = IS_EVIL(ch) ? 2 : (IS_GOOD(ch) ? 0 : 1);
    else if (IS_EVIL(victim))
        strength = IS_GOOD(ch) ? 2 : (IS_EVIL(ch) ? 0 : 1);
    else
        strength = (IS_GOOD(ch) || IS_EVIL(ch)) ? 1:0;

    if (!strength) {
        act_p("О, нет.. Кажется, $C1 нравится твоим Богам..", ch, NULL, victim, TO_CHAR, POS_RESTING);
        return;
    }
    
    level += (strength - 1) * 3;

    if (saves_spell( level, victim , DAM_HOLY, ch, DAMF_SPELL )) {
        ch->send_to("Твоя попытка закончилась неудачей.\n\r");
        return;
    }

    af.bitvector.setTable(&affect_flags);
    af.type         = sn;
    af.level        = level;
    af.duration     = 8 + level/10;
    af.location = APPLY_HITROLL;
    af.modifier     = - (level / 5) * strength;
    affect_to_char(victim, &af);

    af.location = APPLY_DAMROLL;
    af.modifier     = - (level / 5) * strength;
    affect_to_char(victim, &af);

    af.location = APPLY_SAVING_SPELL;
    af.modifier     = (level / 5) * strength;
    affect_to_char(victim, &af);

    af.location = APPLY_LEVEL;
    af.modifier     = -strength * 3;
    af.bitvector.setValue(AFF_CURSE);
    affect_to_char(victim, &af);
    
    act_p("Боги $c2 проклинают тебя!\r\nТы чувствуешь себя преотвратно.", 
            ch, 0, victim, TO_VICT, POS_RESTING);
    act_p("Твои Боги проклинают $C4!", ch, 0, victim, TO_CHAR, POS_RESTING);
    act_p("Боги $c2 проклинают $C4!", ch, 0, victim, TO_NOTVICT, POS_RESTING);

}

SPELL_DECL(BlackDeath);
VOID_SPELL(BlackDeath)::run( Character *ch, Room *room, int sn, int level ) 
{ 
  Affect af;

  if (IS_SET(room->room_flags, ROOM_LAW))
    {
      ch->send_to("Божественные Силы покровительствуют этому месту.\n\r");
      return;
    }
    if ( room->isAffected( sn ))
    {
     ch->send_to("Это место уже заражено чумой.\n\r");
     return;
    }

    af.bitvector.setTable(&raffect_flags);
    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = level / 15;
    
    af.modifier  = 0;
    af.bitvector.setValue(AFF_ROOM_PLAGUE);
    room->affectTo( &af );

    ch->send_to("Чума заражает все вокруг.\n\r");
    act_p("Чума заражает все вокруг.\n\r",ch,0,0,TO_ROOM,POS_RESTING);
}

AFFECT_DECL(BlackDeath);
VOID_AFFECT(BlackDeath)::entry( Room *room, Character *ch, Affect *paf )
{
     act("{yВоздух отравлен чумными миазмами.{x",ch, 0, 0, TO_CHAR);
}

VOID_AFFECT(BlackDeath)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "Чумные миазмы будут отравлять воздух еще {W%1$d{x ча%1$Iс|са|сов.",
                   paf->duration )
        << endl;
}

VOID_AFFECT(BlackDeath)::update( Room *room, Affect *paf )
{
    Affect plague;
    Character *vch;

    plague.bitvector.setTable(&affect_flags);
    plague.type                 = gsn_plague;
    plague.level                 = paf->level - 1;
    plague.duration                 = number_range(1,((plague.level/2)+1));
    
    plague.modifier                 = -5;
    plague.bitvector.setValue(AFF_PLAGUE);

    for (vch = room->people; vch != 0; vch = vch->next_in_room) {
        if ( !saves_spell(plague.level, vch, DAM_DISEASE, 0, DAMF_SPELL)
                && !is_safe_rspell(paf->level,vch)
                && !IS_AFFECTED(vch,AFF_PLAGUE)
                && number_bits(3) == 0)
        {
            act("Ты чувствуешь жар и легкую дрожь.", vch, 0, 0, TO_CHAR);
            act("$c1 дрожит и выглядит очень больн$gым|ым|ой.", vch, 0, 0,TO_ROOM);
            affect_join(vch,&plague);
        }
    }
}



SPELL_DECL(Blindness);
VOID_SPELL(Blindness)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        
        Affect af;

        if ( IS_AFFECTED(victim, AFF_BLIND) )
        {
    act_p("$c1 и так ничего не видит.",victim,0,0,TO_ROOM,POS_RESTING);
                return;
        }

        if ( saves_spell(level,victim,DAM_OTHER,ch, DAMF_SPELL) )
        {
                ch->send_to("Не получилось.\n\r");
                return;
        }

        af.bitvector.setTable(&affect_flags);
        af.type      = sn;
        af.level     = level;
        af.location = APPLY_HITROLL;
        af.modifier  = -4;
        af.duration  = 3+level / 15;
        af.bitvector.setValue(AFF_BLIND);
        affect_to_char( victim, &af );
        victim->send_to("Тебя ослепили!\n\r");
        act_p("$c1 теперь ничего не видит.",victim,0,0,TO_ROOM,POS_RESTING);
        return;

}


SPELL_DECL(Curse);
VOID_SPELL(Curse)::run( Character *ch, Object *obj, int sn, int level ) 
{
    Affect af;

    if (obj->behavior && obj->behavior->isLevelAdaptive( ))
    {
        act_p("$o1 отвергает твои попытки.",ch,obj,0,TO_CHAR,POS_RESTING);
        return;
    }
    if (IS_OBJ_STAT(obj,ITEM_EVIL))
    {
        act_p("$o1 уже полон дьявольской силы.",ch,obj,0,TO_CHAR,POS_RESTING);
        return;
    }

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
    {
        Affect *paf;

        paf = obj->affected.find(gsn_bless);
        if (!savesDispel(level,paf != 0 ? (int)paf->level : obj->level,0))
        {
            if (paf != 0)
                affect_remove_obj( obj, paf);
            act_p("Алая аура окружает $o4.",ch,obj,0,TO_ALL,POS_RESTING);
            REMOVE_BIT(obj->extra_flags,ITEM_BLESS);
            return;
        }
        else
        {
            act_p("Священная аура $o2 слишком могущественна для тебя.",
                   ch,obj,0,TO_CHAR,POS_RESTING);
            return;
        }
    }

    af.bitvector.setTable(&extra_flags);
    af.type         = sn;
    af.level        = level;
    af.duration     = (8 + level / 5);
    af.location = APPLY_SAVES;
    af.modifier     = +1;
    af.bitvector.setValue(ITEM_EVIL);
    affect_to_obj( obj, &af);

    act_p("Зловещая аура окружает $o4.",ch,obj,0,TO_ALL,POS_RESTING);
}

VOID_SPELL(Curse)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (IS_AFFECTED(victim,AFF_CURSE)) {
        if (ch == victim)
            act("Ты уже прокля$gто|т|та.", ch, 0, 0, TO_CHAR);
        else
            act("$C1 уже прокля$Gто|т|та.", ch, 0, victim, TO_CHAR);
        return;
    }
    
    if (saves_spell(level,victim,DAM_NEGATIVE,ch, DAMF_SPELL)) {
      ch->send_to("Не получилось...\n\r");        
      return;
    }
    
    af.bitvector.setTable(&affect_flags);
    af.type      = sn;
    af.level     = level;
    af.duration  = (8 + level / 10);
    af.location = APPLY_HITROLL;
    af.modifier  = -1 * (level / 8);
    af.bitvector.setValue(AFF_CURSE);
    affect_to_char( victim, &af );

    af.location = APPLY_SAVING_SPELL;
    af.modifier  = level / 8;
    affect_to_char( victim, &af );

        if (victim == ch)
          act("Ты чувствуешь себя проклят$gым|ым|ой.", victim, 0, 0, TO_CHAR);
        else
          act( "$C1 выглядит проклят$Gым|ым|ой.", ch,0,victim,TO_CHAR);
}

SPELL_DECL(CursedLands);
VOID_SPELL(CursedLands)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Affect af;

  if (IS_SET(room->room_flags, ROOM_LAW))
    {
      ch->send_to("Божественные Силы покровительствуют этому месту.\n\r");
      return;
    }
    if ( room->isAffected( sn ))
    {
     ch->send_to("Это место уже проклято!\n\r");
     return;
    }

    af.bitvector.setTable(&raffect_flags);
    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = level / 15;
    
    af.modifier  = 0;
    af.bitvector.setValue(AFF_ROOM_CURSE);
    room->affectTo( &af );

    ch->send_to("Божественное благословение покинуло это место.\n\r");
    act_p("Божественное благословение покинуло это место.\n\r",
           ch,0,0,TO_ROOM,POS_RESTING);


}


SPELL_DECL(DeadlyVenom);
VOID_SPELL(DeadlyVenom)::run( Character *ch, Room *room, int sn, int level ) 
{ 
        Affect af;

        if (IS_SET(room->room_flags, ROOM_LAW))
        {
                ch->send_to("Божественные Силы покровительствуют этому месту.\n\r");
                return;
        }
        if ( room->isAffected( sn ))
        {
                ch->send_to("Эта комната уже наполнена смертельным ядом.\n\r");
                return;
        }

        af.bitvector.setTable(&raffect_flags);
        af.type      = sn;
        af.level     = ch->getModifyLevel();
        af.duration  = level / 15;
        
        af.modifier  = 0;
        af.bitvector.setValue(AFF_ROOM_POISON);
        room->affectTo( &af );

        ch->send_to("Комната наполняется ядовитыми испарениями.\n\r");
        act_p("Комната наполняется ядовитыми испарениями.\n\r",
        ch,0,0,TO_ROOM,POS_RESTING);

}

AFFECT_DECL(DeadlyVenom);
VOID_AFFECT(DeadlyVenom)::entry( Room *room, Character *ch, Affect *paf )
{
     act("{yВ воздухе ощущаются ядовитые испарения.{x",ch, 0, 0, TO_CHAR);
}

VOID_AFFECT(DeadlyVenom)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "Ядовитые испарения исчезнут через {W%1$d{x ча%1$Iс|са|сов.",
                   paf->duration )
        << endl;
}

VOID_AFFECT(DeadlyVenom)::update( Room *room, Affect *paf )
{
    Affect af;
    Character *vch;

    af.bitvector.setTable(&affect_flags);
    af.type         = gsn_poison;
    af.level         = paf->level - 1;
    af.duration        = number_range(1,((af.level/5)+1));
    
    af.modifier        = -5;
    af.bitvector.setValue(AFF_POISON);

    for ( vch = room->people; vch != 0; vch = vch->next_in_room )
    {
        if ( !saves_spell(af.level ,vch,DAM_POISON, 0, DAMF_SPELL)
                && !is_safe_rspell(paf->level,vch)
                && !IS_AFFECTED(vch,AFF_POISON) && number_bits(3) == 0)
        {
            vch->send_to("Тебя подташнивает.\n\r");
            act_p("$c1 {gзеленеет{x лицом.",vch,0,0,TO_ROOM,POS_RESTING);
            affect_join(vch,&af);
        }
    }
}


SPELL_DECL(EnergyDrain);
VOID_SPELL(EnergyDrain)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam;

    if ( saves_spell( level, victim,DAM_NEGATIVE,ch, DAMF_SPELL) )
    {
        ch->send_to("Не получилось...\n\r");
        victim->send_to("Твоя душа холодеет.\n\r");
        return;
    }

    if( victim->getModifyLevel() <= 2 )
    {
        dam                 = ch->hit + 1;
    }
    else
    {
//        gain_exp( victim, 0 - number_range( level/5, 3 * level / 5 ) );
        victim->mana        /= 2;
        victim->move        /= 2;
        dam                 = dice(1, level);
        ch->hit                += dam;
    }

    victim->send_to("Ты чувствуешь, как твоя жизнь уходит!\n\r");
    ch->send_to("Ты безжалостно отбираешь жизнь!\n\r");
    damage_nocatch( ch, victim, dam, sn, DAM_NEGATIVE ,true, DAMF_SPELL);
}


SPELL_DECL(LethargicMist);
VOID_SPELL(LethargicMist)::run( Character *ch, Room *room, int sn, int level ) 
{ 
   Affect af;

  if (IS_SET(room->room_flags, ROOM_LAW))
    {
      ch->send_to("Божественные Силы покровительствуют этому месту.\n\r");
      return;
    }
    if ( room->isAffected( sn ))
    {
     ch->send_to("Летаргический туман уже окутал это место.\n\r");
     return;
    }

    af.bitvector.setTable(&raffect_flags);
    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = level / 15;
    
    af.modifier  = 0;
    af.bitvector.setValue(AFF_ROOM_SLOW);
    room->affectTo( &af );

    ch->send_to("Клубящийся летаргический туман заполняет это место.\n\r");
    act_p("Клубящийся летаргический туман заполняет это место.",
           ch,0,0,TO_ROOM,POS_RESTING);


}

AFFECT_DECL(LethargicMist);
VOID_AFFECT(LethargicMist)::entry( Room *room, Character *ch, Affect *paf )
{
     act("{yВ воздухе клубится летаргический туман.{x",ch, 0, 0, TO_CHAR);
}

VOID_AFFECT(LethargicMist)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "Летаргический туман, клубящийся в воздухе, развеется через {W%1$d{x ча%1$Iс|са|сов.",
                   paf->duration )
        << endl;
}

VOID_AFFECT(LethargicMist)::update( Room *room, Affect *paf )
{
    Affect af;
    Character *vch;

    af.bitvector.setTable(&affect_flags);
    af.type         = gsn_slow;
    af.level         = paf->level - 1;
    af.duration        = number_range(1,((af.level/5)+1));
    
    af.modifier        = -5;
    af.bitvector.setValue(AFF_SLOW);

    for (vch = room->people; vch != 0; vch = vch->next_in_room) {
        if ( !saves_spell(af.level ,vch,DAM_OTHER, 0, DAMF_SPELL|DAMF_WATER)
                && !is_safe_rspell(paf->level,vch)
                && !IS_AFFECTED(vch,AFF_SLOW) && number_bits(3) == 0 )
        {
            vch->send_to("Твои движения замедляются.\n\r");
            act_p("Движения $c2 замедляются.",vch,0,0,TO_ROOM,POS_RESTING);
            affect_join(vch,&af);
        }
    }
}


SPELL_DECL(Plague);
VOID_SPELL(Plague)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if (saves_spell(level,victim,DAM_DISEASE,ch, DAMF_SPELL) ||
        (victim->is_npc() && IS_SET(victim->act,ACT_UNDEAD)))
    {
        if (ch == victim)
          ch->send_to("Ты чувствуешь легкую слабость, но это проходит.\n\r");
        else
          act_p("$C1 не восприимчи$Gво|в|ва к болезни.",
                 ch,0,victim,TO_CHAR,POS_RESTING);
        return;
    }

    af.bitvector.setTable(&affect_flags);
    af.type          = sn;
    af.level         = level * 3/4;
    af.duration  = (10 + level / 10);
    af.location = APPLY_STR;
    af.modifier  = -1 * max(1,3 + level / 15);
    af.bitvector.setValue(AFF_PLAGUE);
    affect_join(victim,&af);

    victim->send_to("Ты кричишь от боли, когда кожа покрывается чумными язвами.\n\r");
    act_p("$c1 кричит от боли, когда кожа покрывается чумными язвами.",
           victim,0,0,TO_ROOM,POS_RESTING);

}

AFFECT_DECL(Plague);
VOID_AFFECT(Plague)::update( Character *ch, Affect *paf ) 
{
    Affect plague;
    Character *vch;
    int dam;
        
    DefaultAffectHandler::update( ch, paf );

    act_p("$c1 бьется в агонии, когда чума охватывает всю $s кожу.",
          ch,0,0,TO_ROOM,POS_RESTING);

    ch->send_to("Ты бьешься в агонии от чумы.\n\r");
    
    if (paf->level <= 1) 
       return; 

    plague.bitvector.setTable(&affect_flags);
    plague.type         = gsn_plague;
    plague.level         = paf->level - 1;
    plague.duration         = number_range(1,2 * plague.level);
    plague.location = APPLY_STR;
    plague.modifier         = -5;
    plague.bitvector.setValue(AFF_PLAGUE);

    for ( vch = ch->in_room->people; vch != 0; vch = vch->next_in_room) {
        if (!saves_spell(plague.level + 2,vch,DAM_DISEASE, 0, DAMF_SPELL)
                && !is_safe_rspell( vch )
                && !IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(2) == 0)
        {
            vch->send_to("Ты чувствуешь жар и легкие судороги.\n\r");
            act_p("$c1 дрожит и выглядит очень больн$gым|ым|ой.",
                  vch,0,0,TO_ROOM,POS_RESTING);
            affect_join(vch,&plague);
        }
    }

    dam = min( ch->getModifyLevel(), static_cast<short>( paf->level/5+1 ) );
    ch->mana -= dam;
    ch->move -= dam;

    float modifier = linear_interpolation(min(ch->getModifyLevel(),(short)103), 1, 103, 0.5, 2);
    
    int plague_damage = max(3,(int) (dam * modifier));

    damage_nocatch( ch, ch, plague_damage, gsn_plague,DAM_DISEASE,false, DAMF_SPELL);
    
    int plague_hp_damage = (max(ch->max_hit/20, 50) * modifier);

    if (number_range(1, 100) < 70 )
        damage_nocatch( ch, ch, plague_hp_damage, gsn_plague,DAM_DISEASE,true, DAMF_SPELL);
}
    
VOID_AFFECT(Plague)::entry( Character *ch, Affect *paf ) 
{
    Affect plague;
    Character *vch;

    DefaultAffectHandler::entry( ch, paf );

    if (paf->level <= 1)
        return;

    plague.bitvector.setTable(&affect_flags);
    plague.type                 = gsn_plague;
    plague.level                 = paf->level - 1;
    plague.duration = number_range(1,2 * plague.level);
    plague.location = APPLY_STR;
    plague.modifier = -5;
    plague.bitvector.setValue(AFF_PLAGUE);

    for (vch = ch->in_room->people; vch != 0; vch = vch->next_in_room)
        if ( !saves_spell(plague.level - 2,vch,DAM_DISEASE, 0, DAMF_SPELL)
                && !vch->is_immortal()
                && !IS_AFFECTED(vch,AFF_PLAGUE) 
                && number_bits(6) == 0)
        {
            vch->println( "Ты чувствуешь жар и лихорадку." );
            act_p("$c1 дрожит и выглядит болезненно.",vch,0,0,TO_ROOM,POS_RESTING);
            affect_join(vch,&plague);
        }
}

SPELL_DECL(Poison);
VOID_SPELL(Poison)::run( Character *ch, Object *obj, int sn, int level ) 
{
        Affect af;

        if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
        {
                if (drink_is_closed( obj, ch ))
                    return;

                if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
                {
                        act_p("Ты не можешь отравить $o1.",ch,obj,0,TO_CHAR,POS_RESTING);
                        return;
                }
                
                obj->value3(obj->value3() | DRINK_POISONED);
                act_p("Пары яда проникают в $o4.",ch,obj,0,TO_ALL,POS_RESTING);
                return;
        }

        if (obj->item_type == ITEM_WEAPON)
        {
                if ( IS_WEAPON_STAT(obj,WEAPON_FLAMING)
                        || IS_WEAPON_STAT(obj,WEAPON_FROST)
//                                || IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
//                                || IS_WEAPON_STAT(obj,WEAPON_SHARP)
                        || IS_WEAPON_STAT(obj,WEAPON_VORPAL)
                        || IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
                        || IS_WEAPON_STAT(obj,WEAPON_FADING)
                        || IS_WEAPON_STAT(obj,WEAPON_HOLY)
                        || IS_OBJ_STAT(obj,ITEM_BLESS)
                        || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
                {
                        act_p("Ты не можешь отравить $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
                        return;
                }

                if (IS_WEAPON_STAT(obj,WEAPON_POISON))
                {
                        act_p("Прикосновение $o2 уже ядовито.",ch,obj,0,TO_CHAR,POS_RESTING);
                        return;
                }

                af.bitvector.setTable(&weapon_type2);
                af.type         = sn;
                af.level         = level / 2;
                af.duration         = level/8;
                
                af.modifier         = 0;
                af.bitvector.setValue(WEAPON_POISON);
                affect_to_obj( obj, &af);

                act_p("Прикосновение $o2 становится ядовитым.",ch,obj,0,TO_ALL,POS_RESTING);
                return;
        }

        act_p("Ты не можешь отравить $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
}

VOID_SPELL(Poison)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        Affect af;

        if ( saves_spell( level, victim,DAM_POISON,ch, DAMF_SPELL) )
        {
                act_p("Кожа $c2 приобретает зеленоватый оттенок, но это сразу проходит.",
                        victim,0,0,TO_ROOM,POS_RESTING);
                victim->send_to("Ты чувствуешь легкое отравление, но это сразу проходит.\n\r");
                return;
        }

        af.bitvector.setTable(&affect_flags);
        af.type      = sn;
        af.level     = level;
        af.duration  = (10 + level / 10);
        af.location = APPLY_STR;
        af.modifier  = -2;
        af.bitvector.setValue(AFF_POISON);
        affect_join( victim, &af );
        victim->send_to("Ты чувствуешь себя очень болезненно.\n\r");
        act_p("$c1 выглядит очень болезненно.",victim,0,0,TO_ROOM,POS_RESTING);

}

AFFECT_DECL(Poison);
VOID_AFFECT(Poison)::update( Character *ch, Affect *paf ) 
{ 
    int poison_damage;

    DefaultAffectHandler::update( ch, paf );

    if (!IS_AFFECTED(ch, AFF_POISON) || IS_SLOW(ch))
        return;

    act_p("$c1 дрожит и испытывает боль.", ch, 0, 0, TO_ROOM, POS_RESTING);
    ch->send_to("Ты дрожишь и испытываешь боль.\n\r");
    
    poison_damage = paf->level * number_range(1,5);
    
    if (ch->getRealLevel( ) < 20)
        poison_damage = paf->level * number_range(1,2);
    else if (ch->getRealLevel( ) < 40)
        poison_damage = paf->level * number_range(1,4);
    
    float modifier = linear_interpolation(min(ch->getModifyLevel(),(short)103), 1, 103, 0.5, 2);

    poison_damage = max( 3, (int) (poison_damage*modifier) );
    
    damage_nocatch(ch, ch, poison_damage, gsn_poison, DAM_POISON, true, DAMF_SPELL);
}

SPELL_DECL(Slow);
VOID_SPELL(Slow)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( victim->isAffected(sn ) || IS_SLOW(victim))
    {
        if (victim == ch)
          ch->send_to("Ты не можешь двигаться медленнее, чем сейчас!\n\r");
        else
          act_p("$C1 не может двигаться медленнее, чем сейчас.",
                 ch,0,victim,TO_CHAR,POS_RESTING);
        return;
    }

    if (saves_spell(level,victim,DAM_OTHER,ch, DAMF_SPELL)
    ||  IS_SET(victim->imm_flags,IMM_SPELL))
    {
        if (victim != ch)
            act("Движения $C2 замедляются, но лишь на мгновение.", ch, 0, victim, TO_CHAR);
        victim->send_to("Ты чувствуешь себя немного сонно, но это сразу проходит.\n\r");
        return;
    }

    if (IS_AFFECTED(victim,AFF_HASTE))
    {
        if (checkDispel(level,victim,gsn_haste))
            return;
        
        if (victim != ch)
            act("Движения $C2 замедляются, но лишь на мгновение.", ch, 0, victim, TO_CHAR);

        victim->send_to("Твои движения замедляются, но лишь на мгновение.\n\r");
        return;
    }


    af.bitvector.setTable(&affect_flags);
    af.type      = sn;
    af.level     = level;
    af.duration  = (4 + level / 12);
    af.location = APPLY_DEX;
    af.modifier  = - max(2,level / 12);
    af.bitvector.setValue(AFF_SLOW);
    affect_to_char( victim, &af );
    victim->send_to("Твои движения замедляются...\n\r");
    act_p("Движения $c2 замедляются.",victim,0,0,TO_ROOM,POS_RESTING);
    return;

}

SPELL_DECL(Weaken);
VOID_SPELL(Weaken)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if (saves_spell( level, victim,DAM_OTHER,ch, DAMF_SPELL) ) {
      if (ch == victim)
          ch->send_to("Не получилось...\n\r");        
      else
          act("У тебя не получилось ослабить $C4.", ch, 0, victim, TO_CHAR);
      return;
    }

    if (victim->isAffected(sn )) {
        if (victim == ch)
            act("Ты уже ослабле$gно|н|на.", ch, 0, 0, TO_CHAR);
        else
            act("$C1 уже ослабле$Gно|н|на.", ch, 0, victim, TO_CHAR);
        return;
    }

    af.bitvector.setTable(&affect_flags);
    af.type      = sn;
    af.level     = level;
    af.duration  = (4 + level / 12);
    af.location = APPLY_STR;
    af.modifier  = -1 * (2 + level / 12);
    af.bitvector.setValue(AFF_WEAKEN);
    affect_to_char( victim, &af );
    victim->send_to("Ты чувствуешь, как силы покидают тебя.\n\r");
    act_p("$c1 слабеет на глазах.",victim,0,0,TO_ROOM,POS_RESTING);
    return;

}

SPELL_DECL(UnholyWord);
VOID_SPELL(UnholyWord)::run( Character *ch, Room *room, int sn, int level ) 
{ 

    int dam;
    
    if (!IS_EVIL(ch)) {
        ch->send_to( "Эта сила недоступна тебе.\r\n" );
        return;
    }
    
    act_p("$c1 произносит нечистые слова!", ch,0,0,TO_ROOM,POS_RESTING);
    ch->send_to("Ты произносишь нечистые слова!\n\r");

        for(auto &it : ch->in_room->getPeople())
        {
            
        if(!it->isDead() && it->in_room == ch->in_room){
            
            try{

        if (is_safe_spell(ch, it, true ))
            continue;

        if (it->is_mirror( ) && (number_percent( ) < 50)) 
            continue;
        
        if (is_safe( ch, it ))
            continue;
        
        if (IS_EVIL(it))
            continue;
        else if (IS_GOOD(it))
            dam = dice( level, 20 );
        else 
            dam = dice( level, 15 );
    
        if (saves_spell( level, it, DAM_NEGATIVE,ch, DAMF_SPELL )) {
            dam /= 2;
        }
        else if (!IS_AFFECTED( it, AFF_CURSE )) {
            Affect af;
            
            af.type  = sn;
            af.level = level;
            af.duration = 2 * level;
            af.location = APPLY_HITROLL;
            af.modifier = -1 * (level / 5);
            affect_to_char( it, &af );
            
            af.location = APPLY_SAVING_SPELL;
            af.modifier  = level / 8;
            af.bitvector.setTable(&affect_flags);
            af.bitvector.setValue(AFF_CURSE);
            affect_to_char( it, &af );

            it->send_to("Ты чувствуешь себя отвратительно.\n\r");
            
            if (ch != it)
                act("$C1 выглядит отвратительно.",ch,0,it,TO_CHAR);
        }

        it->send_to("Дьявольская сила повергает тебя!\n\r");

            if (ch->fighting != it && it->fighting != ch)
            yell_panic( ch, it );

        damage_nocatch( ch, it, dam, sn, DAM_NEGATIVE, true, DAMF_SPELL );
            }
                         catch (const VictimDeathException &) {
                             continue;
                    }
        }
    }
}

SPELL_DECL(BlackFeeble);
VOID_SPELL(BlackFeeble)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (ch->isAffected(sn )) {
        ch->send_to("С тебя довольно.\n\r");
        return;
    }

    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 30 ) + 3;
    affect_to_char( ch, &af );
    
    act_p( "Леденящий душу шепот проклятий окружает $c2, образуя защитную ауру.", ch, 0, 0, TO_ROOM,POS_RESTING);
    ch->send_to("Шепот проклятий окружает тебя, образуя защитную ауру.\n\r");
}

SPELL_DECL(Corruption);
VOID_SPELL(Corruption)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if (IS_AFFECTED(victim,AFF_CORRUPTION))
        {
         act_p("$C1 уже гниет заживо.",ch,0,victim,TO_CHAR,POS_RESTING);
         return;
        }

    if (saves_spell(level, victim, DAM_NEGATIVE, ch, DAMF_SPELL) ||
        (victim->is_npc() && IS_SET(victim->act,ACT_UNDEAD)))
    {
        if (ch == victim)
            act("Ты на мгновение почувствова$gло|л|ла себя болезненно.", ch, 0, 0, TO_CHAR);
        else
          act("$C1 кажется не подвластн$Gым|ым|ой разложению.",ch,0,victim,TO_CHAR);
        return;
    }

    af.bitvector.setTable(&affect_flags);
    af.type          = sn;
    af.level         = level * 3/4;
    af.duration  = (10 + level / 5);
    
    af.bitvector.setValue(AFF_CORRUPTION);
    affect_join(victim,&af);
    
    act("Ты вскрикиваешь в муках, начиная гнить заживо.", victim, 0, 0, TO_CHAR);
    act("$c1 вскрикивает в муках, начиная гнить заживо.", victim, 0, 0, TO_ROOM);
}
