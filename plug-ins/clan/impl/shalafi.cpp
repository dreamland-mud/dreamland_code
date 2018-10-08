/* $Id: shalafi.cpp,v 1.1.6.7.6.12 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2005
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

#include "shalafi.h"

#include "spelltemplate.h"                                                 
#include "skillcommandtemplate.h"
#include "skill.h"
#include "skillmanager.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "affect.h"

#include "damage.h"
#include "fight.h"
#include "magic.h"
#include "merc.h"
#include "gsn_plugin.h"
#include "mercdb.h"
#include "vnum.h"
#include "handler.h"
#include "save.h"
#include "interp.h"
#include "act.h"
#include "act_move.h"
#include "def.h"

GSN(dispel_affects);

#define OBJ_VNUM_POTION_SILVER        43
#define OBJ_VNUM_POTION_GOLDEN        44
#define OBJ_VNUM_POTION_SWIRLING      45

/*--------------------------------------------------------------------------
 * Seneschal 
 *-------------------------------------------------------------------------*/
void ClanGuardShalafi::actGreet( PCharacter *wch )
{
    do_say(ch, "Приветствую тебя, мудрец.");
}
void ClanGuardShalafi::actPush( PCharacter *wch )
{
    act( "$C1 бросает на тебя мимолетный взгляд.\n\rИ тут же ты чувствуешь, как некая магическая сила вышвыривает тебя вон.", wch, 0, ch, TO_CHAR );
    act( "$C1 бросает на $c4 мимолетный взгляд и $c1 мгновенно исчезает.", wch, 0, ch, TO_ROOM );
}
int ClanGuardShalafi::getCast( Character *victim )
{
        int sn = -1;

        switch ( dice(1,16) )
        {
        case  0:
                sn = gsn_blindness;
                break;
        case  1:
                if (!victim->isAffected( gsn_spellbane ))
                    sn = gsn_dispel_affects;
                break;
        case  2:
                sn = gsn_weaken;
                break;
        case  3:
                sn = gsn_blindness;
                break;
        case  4:
                sn = gsn_acid_arrow;
                break;
        case  5:
                sn = gsn_caustic_font;
                break;
        case  6:
                sn = gsn_energy_drain;
                break;
        case  7:
        case  8:
        case  9:
                sn = gsn_acid_blast;
                break;
        case 10:
                sn = gsn_plague;
                break;
        case 11:
                sn = gsn_acid_blast;
                break;
        case 12:  
        case 13:
                sn = gsn_lightning_breath;
                break;
        case 14:
        case 15:
                sn = gsn_mental_knife;
                break;
        default:
                sn = -1;
                break;
        }

        return sn;
}

SPELL_DECL(Brew);
VOID_SPELL(Brew)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    Object *potion;
    Object *vial;
    int spell;

    if (obj->item_type != ITEM_TRASH 
        && obj->item_type != ITEM_TREASURE
        && obj->item_type != ITEM_LOCKPICK
        && obj->item_type != ITEM_KEY)
      {
        ch->send_to("Эта вещь не может быть превращена в зелье.\n\r");
        return;
      }

    if (obj->wear_loc != wear_none)
      {
        ch->send_to("Вещь должна находиться в списке инвентаря.\n\r");
        return;
      }

    for( vial=ch->carrying; vial != 0; vial=vial->next_content )
      if ( vial->pIndexData->vnum == OBJ_VNUM_POTION_VIAL )
        break;
    if (  vial == 0 )  {
        ch->send_to("У тебя нет сосуда, необходимого для изготовления зелья.\n\r");
        return;
    }


    if (number_percent() < 50)
      {
        ch->send_to("Твоя попытка закончилась неудачей, разбив сосуд.\n\r");
        extract_obj(obj);
        return;
      }
        
    if (obj->item_type == ITEM_TRASH)
      potion = create_object( get_obj_index(OBJ_VNUM_POTION_SILVER), level);
    else if (obj->item_type == ITEM_TREASURE)
      potion = create_object( get_obj_index(OBJ_VNUM_POTION_GOLDEN), level);
    else
      potion = create_object( get_obj_index(OBJ_VNUM_POTION_SWIRLING), level);

    spell = 0;

    potion->value[0] = level;

    if (obj->item_type == ITEM_TRASH)
      {
        if (number_percent() < 20)
          spell = gsn_fireball;
        else if (number_percent() < 40)
          spell = gsn_cure_poison;
        else if (number_percent() < 60)
          spell = gsn_cure_blindness;
        else if (number_percent() < 80)
          spell = gsn_cure_disease;
        else
          spell = gsn_word_of_recall;
      }
    else if (obj->item_type == ITEM_TREASURE)
      {
        switch(number_bits(3)) {
        case 0:
          spell = gsn_cure_critical;
          break;
        case 1:
          spell = gsn_haste;
          break;
        case 2:
          spell = gsn_frenzy;
          break;
        case 3:
          spell = gsn_create_spring;
          break;
        case 4:
          spell = gsn_holy_word;
          break;
        case 5:
          spell = gsn_invisibility;
          break;
        case 6:
          spell = gsn_cure_light;
          break;
        case 7:
          spell = gsn_cure_serious;
          break;
        
        }
      }
    else
      {
        if (number_percent() < 20)
          spell = gsn_detect_magic;
        else if (number_percent() < 40)
          spell = gsn_detect_invis;
        else if (number_percent() < 65)
          spell = gsn_pass_door;
        else
          spell = gsn_acute_vision;
      }

    potion->value[1] = spell;
    extract_obj(obj);
    act_p("Используя магические силы, ты изготавливаешь $o4!",
           ch, potion, 0, TO_CHAR,POS_RESTING);
    act_p("Используя магические силы $c1 изготовляет $o4!",
           ch, potion, 0, TO_ROOM,POS_RESTING);

    obj_to_char(potion, ch);
    extract_obj( vial );
}


SPELL_DECL_T(DemonSummon, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, DemonSummon)::createMobile( Character *ch, int level ) const 
{
    return createMobileAux( ch, ch->getModifyLevel( ), 
                         ch->hit, 
                         (ch->is_npc( ) ? ch->max_mana : ch->getPC( )->perm_mana),
                         number_range(level/15, level/10),
                         number_range(level/3, level/2),
                         number_range(level/8, level/6) );
} 

void ShalafiDemon::conjure( )
{   
    int chance;
    Character *mch = ch->master;
    
    ClanSummonedCreature::conjure( );

    if (!mch)
        return;

    chance = mch->getCurrStat(STAT_INT) * 3 
           + mch->getModifyLevel( ) / 10 
           + mch->getCurrStat(STAT_WIS) / 2;
    
    if (chance >= number_percent( ))
        return;
    
    REMOVE_BIT( ch->affected_by, AFF_CHARM );
    ch->leader = ch->master = NULL;

    if (ch->can_see( mch ))
        do_say(ch, "Ты рискнул нарушить мой покой?!!!");
    else
        do_say(ch, "Кто рискнул нарушить мой покой?!!!");

    interpret_raw( ch, "murder", mch->getNameP( ) );
}




SPELL_DECL(MentalKnife);
VOID_SPELL(MentalKnife)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  Affect af;
  int dam;

  if( ch->getModifyLevel() < 20 )
    dam = dice( level, 7 );
  else if( ch->getModifyLevel() < 40 )
    dam = dice( level, 8 );
  else if( ch->getModifyLevel() < 65)
    dam = dice( level, 11 );
  else if( ch->getModifyLevel() < 85)
    dam = dice( level, 15 );
  else if( ch->getModifyLevel() < 95)
    dam = dice( level, 18 );
  else dam = dice( level, 20 );

  if (saves_spell(level,victim, DAM_MENTAL, ch, DAMF_SPELL))
              dam /= 2;
  if( victim->is_npc() ) dam /= 4;

  ch->setWait(gsn_mental_attack->getBeats( ) );

  try {
      damage_nocatch( ch, victim, ch->applyCurse( dam ), sn, DAM_MENTAL, true, DAMF_SPELL );
        
      if(!victim->isAffected(sn) && !saves_spell(level, victim, DAM_MENTAL, ch, DAMF_SPELL))
        {
          af.where                    = TO_AFFECTS;
          af.type               = sn;
          af.level              = level;
          af.duration           = level;
          af.location           = APPLY_INT;
          af.modifier           = ch->applyCurse( -7 );
          af.bitvector          = 0;
          affect_to_char(victim,&af);

          af.location = APPLY_WIS;
          affect_to_char(victim,&af);

          if (ch != victim) {
            act("Твой ментальный удар повреждает разум $C2!", ch,0,victim,TO_CHAR);
            act("Ментальный удар $c2 повреждает твой разум!", ch,0,victim,TO_VICT);
            act("Ментальный удар $c2 повреждает разум $C2!", ch,0,victim,TO_NOTVICT);
          }
          else {
            act("Ментальный удар повреждает твой разум!", ch,0,0,TO_CHAR);
            act("Ментальный удар $c2 повреждает $s разум!", ch,0,0,TO_ROOM);
          }
        }
  } catch (const VictimDeathException &) {
  }

}

TYPE_SPELL(bool, MentalKnife)::spellbane( Character *, Character * ) const
{
    return false;
}



SPELL_DECL(Scourge);
VOID_SPELL(Scourge)::run( Character *ch, Room *room, int sn, int level ) 
{ 
  Character *tmp_vict;
  Character *tmp_next;
  int dam;

  if( ch->getModifyLevel() < 40 )
        dam = dice(level,6);
  else if( ch->getModifyLevel() < 65)
        dam = dice(level,9);
  else dam = dice(level,12);

  for (tmp_vict = room->people;tmp_vict != 0;
       tmp_vict = tmp_next)
    {
      tmp_next = tmp_vict->next_in_room;

        if ( tmp_vict->is_mirror()
            && ( number_percent() < 50 ) ) continue;
                        

      if ( !is_safe_spell(ch,tmp_vict,true))
        {
            if (ch->fighting != tmp_vict && tmp_vict->fighting != ch)
                yell_panic( ch, tmp_vict );
        
          if (!tmp_vict->isAffected(sn)) {
        

            if (number_percent() < level)
              spell(gsn_poison, level, ch, tmp_vict);

            if (number_percent() < level)
              spell(gsn_blindness,level,ch,tmp_vict);

            if (number_percent() < level)
              spell(gsn_weaken, level, ch, tmp_vict);

            if (saves_spell(level,tmp_vict, DAM_FIRE, ch, DAMF_SPELL))
              dam /= 2;
            damage( ch, tmp_vict, ch->applyCurse( dam ), sn, DAM_FIRE, true, DAMF_SPELL );
          }

        }
    }

}


SPELL_DECL(Transform);
VOID_SPELL(Transform)::run( Character *ch, Character *, int sn, int level ) 
{ 
  Affect af;
  int hp_modif;

  if (ch->isAffected(sn) || ch->hit > ch->max_hit)
    {
      act("Ты уже переполне$gно|н|на жизненной энергией.", ch, 0, 0, TO_CHAR);
      return;
    }

  hp_modif = ch->applyCurse( min(30000 - ch->max_hit, ( int )( ch->max_hit / 1.6 ) ) );
  ch->hit += hp_modif;

  if( ch->isAffected(gsn_haste ) ) affect_strip( ch, gsn_haste );

  af.where                = TO_AFFECTS;
  af.type               = sn;
  af.level              = level;
  af.duration           = 24;
  af.location           = APPLY_HIT;
  af.modifier           = hp_modif;
  af.bitvector          = 0;
  affect_to_char(ch,&af);

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_DEX;
    af.modifier  = - (4 + level / 10);
    af.bitvector = 0;
    affect_to_char( ch, &af );

  ch->send_to("Прилив жизненной силы затмевает твой разум.\n\r");

}



