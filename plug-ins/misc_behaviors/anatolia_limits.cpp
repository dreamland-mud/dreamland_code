/* $Id$
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
/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT                           *        
 *     ANATOLIA has been brought to you by ANATOLIA consortium                   *
 *         Serdar BULUT {Chronos}                bulut@rorqual.cc.metu.edu.tr       *
 *         Ibrahim Canpunar  {Mandrake}        canpunar@rorqual.cc.metu.edu.tr    *        
 *         Murat BICER  {KIO}                mbicer@rorqual.cc.metu.edu.tr           *        
 *         D.Baris ACAR {Powerman}        dbacar@rorqual.cc.metu.edu.tr           *        
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *        
 ***************************************************************************/

#include "anatolia_limits.h"

#include "affect.h"
#include "room.h"
#include "roomutils.h"
#include "pcharacter.h"
#include "character.h"
#include "object.h"

#include "dreamland.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "magic.h"
#include "gsn_plugin.h"
#include "effects.h"
#include "handler.h"
#include "fight.h"
#include "damage_impl.h"
#include "def.h"

/*
 * Excalibur, Dwarven Catacombs
 */
void Excalibur::wear( Character *ch )
{
  oldact("$o1 загорается ослепительно-белым светом.", ch,obj,0,TO_CHAR);
  oldact("$o1 загорается ослепительно-белым светом.", ch,obj,0,TO_ROOM);
  
}

void Excalibur::equip( Character *ch )
{
  short level = ch->getModifyLevel();

  if ( level > 20 && level <= 30)        obj->value2(4);
  else if ( level > 30 && level <= 40)   obj->value2(5);
  else if ( level > 40 && level <= 50)   obj->value2(6);
  else if ( level > 50 && level <= 60)   obj->value2(8);
  else if ( level > 60 && level <= 70)   obj->value2(10);
  else if ( level > 70 && level <= 80)   obj->value2(11);
  else obj->value2(12);
}

void Excalibur::remove( Character *ch )
{
  oldact("Пылающая аура вокруг $o2 исчезает.",ch,obj,0,TO_CHAR);
  oldact("Пылающая аура вокруг $o2 исчезает.",ch,obj,0,TO_ROOM);
}

bool Excalibur::death( Character *ch )
{
    if (obj->wear_loc != wear_wield && obj->wear_loc != wear_second_wield)
        return false;

    oldact_p("$o1 начинает светиться голубым пламенем.", ch,obj,0,TO_CHAR,POS_DEAD);
    oldact("$o1 начинает светиться голубым пламенем.", ch,obj,0,TO_ROOM);
    ch->hit = ch->max_hit;
    ch->pecho("Ты чувствуешь себя намного лучше.");
    oldact("$c1 выглядит намного лучше.",ch,0,0,TO_ROOM);
    return true;
}

void Excalibur::speech( Character *ch, const char *speech )
{
    if (ch != obj->getCarrier( ))
        return;

  if ((!str_cmp(speech, "sword of acid") || !str_cmp(speech, "меч кислоты"))
      && (ch->fighting) && ((get_eq_char(ch,wear_wield) == obj) ||
                        (get_eq_char(ch,wear_second_wield) == obj)  ) )
    {
      ch->pecho("Лезвие Экскалибра брызжет кислотой.");
      oldact_p("Кислота брызжет с лезвия Экскалибра.",
             ch,0,0,TO_ROOM,POS_RESTING);
      spell(gsn_acid_blast, ch->getModifyLevel(),ch,ch->fighting, FSPELL_BANE );
      ch->setWaitViolence( 2 );
    }
}

bool Excalibur::sac( Character *ch )
{
  oldact("{RБОГИ В ГНЕВЕ!{x",ch,0,0,TO_ALL);
  rawdamage( ch, ch, DAM_HOLY, (ch->hit - 1) > 1000 ? 1000 : (ch->hit - 1), true, "gods" );
  ch->gold = 0;
  return true;
}

/*
 * Musical bracers, bracers of energy, glinting silver bracelet
 */
void HasteBracers::wear( Character *ch ) 
{
  if( !( ch->isAffected(gsn_haste ) ) ) {
//      ch->pecho("Предмет принимает форму твоих рук, плотно прилегая к коже.");
      ch->pecho("Твое тело наполняется удивительной легкостью.");
    }
}
void HasteBracers::equip( Character *ch ) 
{
  Affect af;
  short level = ch->getModifyLevel();

  if( !( ch->isAffected(gsn_haste ) ) ) {
      af.bitvector.setTable(&affect_flags);
      af.type = gsn_haste;
      af.duration = -2;
      af.level = level;
      af.bitvector.setValue(AFF_HASTE);
      af.location = APPLY_DEX;
      af.modifier = 1 + ( level >= 18 ) + ( level >= 30) + ( level >= 45 );
      affect_to_char(ch, &af);
    }
}

void HasteBracers::remove( Character *ch ) 
{
  if (ch->isAffected(gsn_haste))
    {
      affect_strip(ch, gsn_haste);
      ch->pecho("Твое тело теряет легкость, и твои движения замедляются.");
    }
}

/*
 * Mudschool weapon
 */
void SubissueWeapon::fight( Character *ch )
{
    int hp = HEALTH(ch);

    if (obj->wear_loc != wear_wield)
        return;

    if (hp <= 40 && chance(50)) {
        ch->pecho("Твое оружие свистит, {Y'Тебе лучше {y{hcубежать{hx {Yпрочь, подальше отсюда!'{x");
        return;
    }
}

/*
 * Two snake headed whip, Drow City
 */
void TwoSnakeWhip::wear( Character *ch )
{
  oldact_p("{GЗмеи на хлысте выдыхают пары яда.{x",
                ch,obj,0,TO_CHAR,POS_DEAD);
  oldact_p("{GЗмеи на хлысте выдыхают пары яда.{x",
                ch,obj,0,TO_ROOM,POS_DEAD);
}

void TwoSnakeWhip::equip( Character *ch )
{
  short level = ch->getModifyLevel();

  if (  level <= 10)                        obj->value2(3);
  else if ( level > 10 && level <= 20)   obj->value2(4);
  else if ( level > 20 && level <= 30)   obj->value2(5);
  else if ( level > 30 && level <= 40)   obj->value2(6);
  else if ( level > 40 && level <= 50)   obj->value2(7);
  else if ( level > 50 && level <= 60)   obj->value2(8);
  else if ( level > 60 && level <= 70)   obj->value2(9);
  else if ( level > 70 && level <= 80)   obj->value2(10);
  else obj->value2(11);
  return;
}

void TwoSnakeWhip::remove( Character *ch )
{
  oldact_p("{rЗмеи на хлысте безжизненно обвисают.{x",
                ch,obj,0,TO_CHAR,POS_DEAD);
  oldact_p("{rЗмеи на хлысте безжизненно обвисают.{x",
                ch,obj,0,TO_ROOM,POS_DEAD);
}

void TwoSnakeWhip::get( Character *ch )
{
  oldact("Тебе кажется, будто змеи на хлысте пошевелились.",ch,obj,0,TO_CHAR);
}

void TwoSnakeWhip::fight( Character *ch )
{
  if ( (get_eq_char(ch, wear_wield) == obj) ||
        ( get_eq_char(ch,wear_second_wield) == obj) )
    {
      switch(number_bits(7)) {
      case 0:
          oldact_p("Одна из змей на твоем хлысте жалит $C4!", ch, 0,
                ch->fighting, TO_CHAR,POS_RESTING);
        oldact_p("Змея с хлыста $c2 внезапно жалит тебя!", ch, 0,
                ch->fighting, TO_VICT,POS_RESTING);
        oldact_p("Змея с хлыста $c2 жалит $C4!", ch, 0,
                ch->fighting, TO_NOTVICT,POS_RESTING);
        spell(gsn_poison, ch->getModifyLevel(), ch, ch->fighting, FSPELL_BANE );
        break;
      case 1:
          oldact_p("Одна из змей на твоем хлысте жалит $C4!", ch, 0,
                ch->fighting, TO_CHAR,POS_RESTING);
        oldact_p("Змея с хлыста $c2 внезапно жалит тебя!", ch, 0,
                ch->fighting, TO_VICT,POS_RESTING);
        oldact_p("Змея с хлыста $c2 жалит $C4!", ch, 0,
                ch->fighting, TO_NOTVICT,POS_RESTING);
        spell(gsn_weaken, ch->getModifyLevel(), ch, ch->fighting, FSPELL_BANE );
        break;
      }
    }
}

/*
 * Hammer thunderbolt, Olympus
 */
void Thunderbolt::fight( Character *ch )
{
    if ( (get_eq_char(ch, wear_wield) == obj) ||
        (get_eq_char(ch,wear_second_wield) == obj) )
    {
        int dam;
        Character *victim = ch->fighting;
        int level = ch->getModifyLevel( );

        switch(number_bits(6)) {
        case 0:
            oldact("Разряд молнии выстреливает из твоего оружия и поражает $C4!", ch, 0, victim, TO_CHAR);
            oldact("Разряд молнии конденсируется на оружии $c2 и выстреливает в твою сторону!", ch, 0, victim, TO_VICT);
            oldact("Разряд молнии конденсируется на оружии $c2, и выстреливает в сторону $C2!", ch, 0, victim, TO_NOTVICT);

            dam = dice(level,4) + 12;
            if ( saves_spell( level, victim,DAM_LIGHTNING,ch, DAMF_PRAYER) )
                dam /= 2;
            damage_nocatch( ch, victim, dam, gsn_lightning_bolt, DAM_LIGHTNING , false, DAMF_PRAYER);
            break;
        }
    }
}


/*
 * Firegauntlets, Olympus
 */
void FireGauntlets::fight( Character *ch )
{
int dam;

  if ( !(get_eq_char( ch, wear_wield ) == 0  &&
        get_eq_char( ch, wear_second_wield) == 0) )
        return;

  if ( get_eq_char( ch, wear_hands ) != obj )
        return;
  if ( ch->is_npc() )
        return;

    if ( number_percent() < 50 )  {
        dam = number_percent()/2 + 30 + 2 * ch->getModifyLevel();
        oldact_p("Твои перчатки обжигают лицо $C3!",
                ch, 0, ch->fighting, TO_CHAR,POS_RESTING);
        oldact_p("Перчатки $c2 обжигают лицо $C3!",
                ch, 0, ch->fighting, TO_NOTVICT,POS_RESTING);
        oldact_p("Перчатки $C2 обжигают твое лицо!",
                ch->fighting, 0, ch, TO_CHAR,POS_RESTING);
        damage_nocatch( ch, ch->fighting, dam/2, gsn_burning_hands, DAM_FIRE, false);
        fire_effect( ch->fighting, ch, obj->level/2, dam/2, TARGET_CHAR );
    }
}

void FireGauntlets::wear( Character *ch )
{
    ch->pecho("Твои руки нагреваются от перчаток.");
}

void FireGauntlets::remove( Character *ch )
{
    ch->pecho("Твои руки почувствовали прохладу.");
}

/*
 * Armbands of volcanoe, Isles
 */
void VolcanoeArmbands::fight( Character *ch )
{
int dam;
  if ( get_eq_char( ch, wear_arms ) != obj )
        return;

  if ( ch->is_npc() )
        return;

  if ( number_percent() < 20 )  {
        dam = number_percent()/2 + 30 + 5 * ch->getModifyLevel();
        oldact_p("Твои нарукавники обжигают $C4!",
                ch, 0, ch->fighting, TO_CHAR,POS_RESTING);
        oldact_p("Нарукавники $c2 обжигают $C4!",
                ch, 0, ch->fighting, TO_NOTVICT,POS_RESTING);
        oldact_p("Нарукавники $C2 обжигают тебя!",
                ch->fighting, 0, ch, TO_CHAR,POS_RESTING);
        damage_nocatch( ch, ch->fighting, dam, gsn_burning_hands, DAM_FIRE, false);
        fire_effect( ch->fighting, ch, obj->level/2, dam, TARGET_CHAR );
  }
  return;
}

void VolcanoeArmbands::wear( Character *ch )
{
    ch->pecho("Твои руки чувствуют тепло.");
}

void VolcanoeArmbands::remove( Character *ch )
{
    ch->pecho("Твои руки вновь почувствовали прохладу.");
}


/*
 * Demonfire shield, UnderDark
 */
void DemonfireShield::fight( Character *ch )
{
int dam;

  if ( get_eq_char( ch, wear_shield ) != obj )
        return;
  if ( ch->is_npc() )
        return;

  if ( number_percent() < 15 )  {
        dam = number_percent()/2 + 5 * ch->getModifyLevel();

        oldact("Твой щит обжигает лицо $C3!", ch, 0, ch->fighting, TO_CHAR);
        oldact("Щит $c2 обжигает лицо $C3!", ch, 0, ch->fighting, TO_NOTVICT);
        oldact("Щит $C2 обжигает твое лицо!", ch->fighting, 0, ch, TO_CHAR);

        damage_nocatch( ch, ch->fighting, dam, gsn_demonfire, DAM_FIRE, false);
        fire_effect( ch->fighting, ch, obj->level,dam, TARGET_CHAR );
  }
  return;
}

void DemonfireShield::wear( Character *ch )
{
    ch->pecho("Твои руки чувствуют жар {RОгня Демонов{x.");
}

void DemonfireShield::remove( Character *ch )
{
    ch->pecho("Твои руки вновь почувствовали прохладу.");
}


/*
 * Wind boots, Elemental Canyon
 * Boots of Flying, UnderDark
 */
void FlyingBoots::wear( Character *ch )
{
    if (!ch->isAffected(gsn_fly)) {
        oldact("Ты обуваешь $o4, и твои ноги медленно отрываются от земли.", ch, obj, 0, TO_CHAR );
        oldact("Ты поднимаешься в воздух.", ch, 0, 0, TO_CHAR );
        oldact("$c1 поднимается в воздух.", ch, 0, 0, TO_ROOM );
    }
}
void FlyingBoots::equip( Character *ch )
{
    Affect af;

    if (ch->isAffected(gsn_fly))
        return;

    af.bitvector.setTable(&affect_flags);
    af.type = gsn_fly;
    af.duration = -2;
    af.level = ch->getModifyLevel();
    af.bitvector.setValue(AFF_FLYING);
    
    af.modifier = 0;
    affect_to_char(ch, &af);
}

void FlyingBoots::remove( Character *ch )
{
    if (!ch->isAffected(gsn_fly))
        return;
    
    affect_strip(ch, gsn_fly);
    oldact("Ты падаешь на землю. \r\nДа уж!...", ch, 0, 0, TO_CHAR );
    oldact("$c1 падает на землю.", ch, 0, 0, TO_ROOM );
}


/*
 * Hercules armbands, Galaxy
 * Girdle of giant strength, Ryzen Caverns
 * Breastplate of strength, UnderDark
 */
void GiantStrengthArmor::wear( Character *ch )
{
    oldact("Ты чувствуешь, как становишься гораздо сильнее!\r\n"
         "Твои мышцы разбухают до внушительных размеров.", ch, obj, 0, TO_CHAR );
    oldact("Мышцы $c2 дрожат от избытка силы!", ch, 0, 0, TO_ROOM );
}
void GiantStrengthArmor::equip( Character *ch )
{
    Affect af;
    short level = ch->getModifyLevel();
    
    af.type = gsn_giant_strength;
    af.duration = -2;
    af.level = ch->getModifyLevel();
    
    af.location = APPLY_STR;
    af.modifier = 1 + ( level >= 18) + ( level >= 30) + ( level >= 45);
    affect_join(ch, &af);
}

void GiantStrengthArmor::remove( Character *ch )
{
    if (ch->isAffected(gsn_giant_strength))
    {
        affect_strip(ch, gsn_giant_strength);
        oldact("Твои мышцы съеживаются до обычного состояния.", ch, 0, 0, TO_CHAR );
        oldact("Мышцы $c2 съеживаются до обычного состояния.", ch, 0, 0, TO_ROOM );
    }
}


/*
 * A shield of rose, Sewer, carried by Grand Knight of Paladins
 */
void RoseShield::fight( Character *ch )
{
  if (!RoomUtils::isNature(ch->in_room))
  return;

  if (get_eq_char(ch,wear_shield) != obj )
  return;

  if ( number_percent() < 90 )  return;

  ch->pecho("Листья на твоем щите внезапно распускаются.");
  ch->fighting->pecho("Листья на щите окружают тебя!");
  oldact("Щит Розы $c2 внезапно распускается.",ch,0,0,TO_ROOM);
  spell(gsn_slow, ch->getModifyLevel(),ch,ch->fighting);
  return;
}

/*
 * Lion claw, Pyramids
 */
void LionClaw::fight( Character *ch )
{
  bool secondary = false;
  
  if ( number_percent() < 90 )  return;

  if ( ( obj == get_eq_char(ch,wear_wield)) ||
        (secondary = (obj == get_eq_char(ch,wear_second_wield))) )
   {
     ch->pecho("{WКогти на мгновение показались из Львиной Лапы.{x");
     oldact_p("{WКогти на мгновение показались из Львиной Лапы $c2.{x",
                ch,0,0,TO_ROOM,POS_DEAD);
     
     one_hit_nocatch(ch, ch->fighting, secondary);
     one_hit_nocatch(ch, ch->fighting, secondary);
     one_hit_nocatch(ch, ch->fighting, secondary);
     one_hit_nocatch(ch, ch->fighting, secondary);
    
     return;
   }
 return;
}

/*
 * Ring of Ra, Pyramids
 */
void RingOfRa::speech( Character *ch, const char *speech )
{
    if (ch != obj->getCarrier( ))
        return;
    
  if ((!str_cmp(speech, "punish") || !str_cmp(speech, "наказать") || !str_cmp(speech, "покарать"))
      && (ch->fighting) &&
((get_eq_char(ch,wear_finger_l) == obj) || (get_eq_char(ch,wear_finger_r))) )
    {
      ch->pecho("Электрический разряд выстреливает из кольца.");
      oldact_p("Электрический разряд выстреливает из кольца.",
             ch,0,0,TO_ROOM,POS_RESTING);
      spell(gsn_lightning_breath, ch->getModifyLevel(),ch,ch->fighting, FSPELL_BANE );
      ch->setWaitViolence( 2 );
    }
}


