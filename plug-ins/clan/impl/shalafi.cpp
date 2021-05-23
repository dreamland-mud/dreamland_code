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
    oldact("$C1 бросает на тебя мимолетный взгляд.\n\rИ тут же ты чувствуешь, как некая магическая сила вышвыривает тебя вон.", wch, 0, ch, TO_CHAR );
    oldact("$C1 бросает на $c4 мимолетный взгляд и $c1 мгновенно исчезает.", wch, 0, ch, TO_ROOM );
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
        ch->pecho("Эта вещь не может быть превращена в зелье.");
        return;
      }

    if (obj->wear_loc != wear_none)
      {
        ch->pecho("Вещь должна находиться в списке инвентаря.");
        return;
      }

    for( vial=ch->carrying; vial != 0; vial=vial->next_content )
      if ( vial->pIndexData->vnum == OBJ_VNUM_POTION_VIAL )
        break;
    if (  vial == 0 )  {
        ch->pecho("У тебя нет сосуда, необходимого для изготовления зелья.");
        return;
    }


    if (number_percent() < 50)
      {
        ch->pecho("Твоя попытка закончилась неудачей, уничтожив ингредиент.");
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

    potion->value0(level);

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
          spell = gsn_heal;
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
          spell = gsn_heal;
          break;
        case 7:
          spell = gsn_heal;
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

    potion->value1(spell);
    extract_obj(obj);
    oldact_p("Используя магические силы, ты изготавливаешь $o4!",
           ch, potion, 0, TO_CHAR,POS_RESTING);
    oldact_p("Используя магические силы $c1 изготовляет $o4!",
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

    interpret_raw( ch, "murder", mch->getNameC() );
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

  if (saves_spell(level,victim, DAM_MENTAL, ch, DAMF_MAGIC))
              dam /= 2;
  if( victim->is_npc() ) dam /= 4;

  ch->setWait(gsn_mental_attack->getBeats(ch) );

  try {
      damage_nocatch( ch, victim, ( dam ), sn, DAM_MENTAL, true, DAMF_MAGIC );
        
      if(!victim->isAffected(sn) && !saves_spell(level, victim, DAM_MENTAL, ch, DAMF_MAGIC))
        {
          af.type               = sn;
          af.level              = level;
          af.duration           = level;

          af.modifier           = ( -7 );
          af.location = APPLY_INT;
          affect_to_char(victim,&af);

          af.location = APPLY_WIS;
          affect_to_char(victim,&af);

          if (ch != victim) {
            oldact("Твой ментальный удар повреждает разум $C2!", ch,0,victim,TO_CHAR);
            oldact("Ментальный удар $c2 повреждает твой разум!", ch,0,victim,TO_VICT);
            oldact("Ментальный удар $c2 повреждает разум $C2!", ch,0,victim,TO_NOTVICT);
          }
          else {
            oldact("Ментальный удар повреждает твой разум!", ch,0,0,TO_CHAR);
            oldact("Ментальный удар $c2 повреждает $s разум!", ch,0,0,TO_ROOM);
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

  int dam;

  if( ch->getModifyLevel() < 40 )
        dam = dice(level,6);
  else if( ch->getModifyLevel() < 65)
        dam = dice(level,9);
  else dam = dice(level,12);


        for(auto &tmp_vict : ch->in_room->getPeople())
        {
            if(!tmp_vict->isDead() && tmp_vict->in_room == ch->in_room){

        if ( tmp_vict->is_mirror()
            && ( number_percent() < 50 ) ) continue;
                        

      if ( !is_safe_spell(ch,tmp_vict,true))
        {
            if (ch->fighting != tmp_vict && tmp_vict->fighting != ch)
                yell_panic( ch, tmp_vict );
        
          if (!tmp_vict->isAffected(sn)) {
        
          try{
            if (number_percent() < level)
              spell(gsn_poison, level, ch, tmp_vict);

            if (number_percent() < level)
              spell(gsn_blindness,level,ch,tmp_vict);

            if (number_percent() < level)
              spell(gsn_weaken, level, ch, tmp_vict);

            if (saves_spell(level,tmp_vict, DAM_FIRE, ch, DAMF_MAGIC))
              dam /= 2;
            damage_nocatch( ch, tmp_vict, ( dam ), sn, DAM_FIRE, true, DAMF_MAGIC );
          }
            catch (const VictimDeathException &) {
                   continue;
            }
          }
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
      oldact("Ты уже переполне$gно|н|на жизненной энергией.", ch, 0, 0, TO_CHAR);
      return;
    }

  hp_modif = ( min(30000 - ch->max_hit, ( int )( ch->max_hit / 1.6 ) ) );
  ch->hit += hp_modif;

  af.type               = sn;
  af.level              = level;
  af.duration           = 24;

  af.location = APPLY_HIT;
  af.modifier           = hp_modif;
  affect_to_char(ch,&af);

  af.location = APPLY_DEX;
  af.modifier  = - (4 + level / 10);
  af.bitvector.setTable(&affect_flags);
  af.bitvector.setValue(AFF_SLOW);    
  affect_to_char( ch, &af );

  ch->pecho("Прилив жизненной силы затмевает твой разум.");

}



