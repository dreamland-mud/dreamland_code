/* $Id: gods_impl.cpp,v 1.1.2.5 2010-09-01 21:20:46 rufina Exp $
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
#include "gods_impl.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "affect.h"
#include "skillcommand.h"

#include "handler.h"
#include "act.h"
#include "onehit_weapon.h"
#include "damage_impl.h"
#include "fight.h"
#include "magic.h"
#include "gsn_plugin.h"
#include "immunity.h"
#include "merc.h"
#include "mercdb.h"
#include "vnum.h"
#include "def.h"

/*----------------------------------------------------------------------
 *  fight programms 
 *---------------------------------------------------------------------*/
void AtumRaGod::tattooFight( Object *obj, Character *ch ) const 
{
    switch(number_bits(6)) {
    case 0:
    case 1:
      act_p("{CТатуировка на твоем плече вспыхивает голубым светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_cure_serious, ch->getModifyLevel(), ch, ch );
      break;
    case 2:
      act_p("{rТатуировка на твоем плече вспыхивает красным светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      do_yell( ch, "Потанцуй со {WСветом!{x");
      spell( gsn_wrath, ch->getModifyLevel( ), ch, ch->fighting );
      break;
    }
}


void ZeusGod::tattooFight( Object *obj, Character *ch ) const 
{
    switch(number_bits(6)) {
    case 0:
    case 1:
    case 2:
      act_p("{CТатуировка на твоем плече загорается голубым светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_cure_critical, ch->getModifyLevel(), ch, ch );
      break;
    case 3:
      act_p("{CТатуировка на твоем плече загорается голубым светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      if (IS_AFFECTED(ch,AFF_PLAGUE))        
        spell( gsn_cure_disease, 100, ch, ch );
      if (IS_AFFECTED(ch,AFF_POISON))        
        spell( gsn_cure_poison, 100, ch, ch );
      break;
    }
}

void SiebeleGod::tattooFight( Object *obj, Character *ch ) const 
{
    switch(number_bits(6)) {
    case 0:
      act_p("{CТатуировка на твоем плече загорается голубым светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_cure_serious, ch->getModifyLevel(), ch,ch );
      break;
    case 1:
      act_p("{rТатуировка на твоем плече загорается красным светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
        spell( gsn_bluefire, ch->getModifyLevel( ), ch, ch->fighting );
      break;
    }
}

void AhuramazdaGod::tattooFight( Object *obj, Character *ch ) const 
{
    switch(number_bits(6)) {
    case 0:
      act_p("{rТатуировка на твоем плече загорается красным светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_cure_light, ch->getModifyLevel(), ch, ch );
      break;
    case 1:
      act_p("{CТатуировка на твоем плече загорается голубым светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_cure_serious, ch->getModifyLevel(), ch, ch );
      break;
    case 2:
      act_p("{rТатуировка на твоем плече загорается красным светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell( gsn_dispel_evil, ch->getModifyLevel( ), ch, ch->fighting );
      break;
    }
}

void ShamashGod::tattooFight( Object *obj, Character *ch ) const 
{
    switch(number_bits(6)) {
    case 0:
    case 1:
      act_p("{CТатуировка на твоем плече загорается голубым светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_cure_serious, ch->getModifyLevel(), ch, ch );
      break;
    case 2:
      act_p("{rТатуировка на твоем плече загорается красным светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      do_yell(ch,"And justice for all!....");
      spell( gsn_scream, ch->getModifyLevel( ), ch, ch->in_room );
      break;
    }
}

void EhrumenGod::tattooFight( Object *obj, Character *ch ) const 
{
    switch(number_bits(6)) {
    case 0:
      act_p("{BТатуировка на твоем плече загорается голубым светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_cure_serious, ch->getModifyLevel(), ch, ch );
      break;
    case 1:
      act_p("{rТатуировка на твоем плече загорается красным светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_demonfire, ch->getModifyLevel(), ch, ch->fighting );
      break;
    }
}

void VenusGod::tattooFight( Object *obj, Character *ch ) const 
{
    switch(number_bits(7)) {
    case 0:
    case 1:
    case 2:
      act_p("{CТатуировка на твоем плече загорается голубым светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_cure_light, ch->getModifyLevel(), ch, ch );
      break;
    case 3:
      act_p("{rТатуировка на твоем плече загорается красным светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_plague, ch->getModifyLevel(), ch, ch->fighting );
      break;
    case 4:
      act_p("{CТатуировка на твоем плече загорается голубым светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_bless, ch->getModifyLevel(), ch, ch );
      break;
    }
}

void SethGod::tattooFight( Object *obj, Character *ch ) const 
{
    switch(number_bits(5)) {
    case 0:
      act_p("{CТатуировка на твоем плече загорается голубым светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_dragon_strength, ch->getModifyLevel(), ch, ch );
      break;
    case 1:
      act_p("{rТатуировка на твоем плече загорается красным светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell( gsn_dragon_breath, ch->getModifyLevel( ), ch, ch->fighting );
      break;
    }
}


void OdinGod::tattooFight( Object *obj, Character *ch ) const 
{
    switch(number_bits(5)) {
    case 0:
      act_p("{CТатуировка на твоем плече загорается голубым светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_cure_critical, ch->getModifyLevel(), ch, ch );
      break;
    case 1:
      act_p("{rТатуировка на твоем плече загорается красным светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_faerie_fire, ch->getModifyLevel(), ch, ch->fighting );
      break;
    }
}

void PhobosGod::tattooFight( Object *obj, Character *ch ) const 
{
    switch(number_bits(6)) {
    case 0:
      act_p("{CТатуировка на твоем плече загорается голубым светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_cure_serious, ch->getModifyLevel(), ch, ch );
      break;
    case 1:
      act_p("{rТатуировка на твоем плече загорается красным светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_colour_spray, ch->getModifyLevel(), ch, ch->fighting );
      break;
    }
}

void TeshubGod::tattooFight( Object *obj, Character *ch ) const 
{
    switch(number_bits(5)) {
    case 0:
      spell(gsn_blindness, ch->getModifyLevel(), ch, ch->fighting );
      ch->send_to("{rТы насылаешь завесу замешательства.{x\n\r");
      break;
    case 1:
      spell(gsn_poison, ch->getModifyLevel(), ch, ch->fighting );
      ch->send_to("{gНемного твоего безумия переходит на твоего противника.{x\n\r");
      break;
    case 2:
      spell(gsn_haste, ch->getModifyLevel(), ch, ch );
      ch->send_to("{WТы внезапно ощущаешь повышенную активность!{x\n\r");
      break;
    case 3:
      spell(gsn_shield, ch->getModifyLevel(), ch, ch );
      ch->send_to("{WТы чувствуешь себя еще большим параноиком!{x\n\r");
      break;
    }
}

void AresGod::tattooFight( Object *obj, Character *ch ) const 
{
  Affect af;

    if (number_percent() < 50)
      {
        switch(number_bits(4)) {
        case 0:
          if (IS_AFFECTED(ch,AFF_BERSERK) || ch->isAffected(gsn_berserk)
              ||  ch->isAffected(gsn_frenzy))
            {
                act("Ты становишься немного дик$gим|им|ой.", ch, 0, 0, TO_CHAR);
              return;
            }

          af.where = TO_AFFECTS;
          af.type = gsn_berserk;
          af.level = ch->getModifyLevel();
          af.duration = ch->getModifyLevel() / 3;
          af.modifier = ch->getModifyLevel() / 5;
          af.bitvector = AFF_BERSERK;

          af.location = APPLY_HITROLL;
          affect_to_char(ch, &af);

          af.location = APPLY_DAMROLL;
          affect_to_char(ch, &af);

          af.modifier = 10 * ( ch->getModifyLevel() / 10);
          af.location = APPLY_AC;
          affect_to_char(ch, &af);
        
          ch->hit += ch->getModifyLevel() * 4;
          ch->hit = std::min( ch->hit, ch->max_hit );
        
          ch->send_to("Твой пульс учащается, когда ярость охватывает тебя!\n\r");
          act_p("Взгляд $c2 становится диким.", ch,0,0,TO_ROOM,POS_RESTING);

          break;
        }
      }
    else
      {
        switch(number_bits(4)) {
        case 0:
          do_yell(ch, "Cry Havoc and Let Loose the Dogs of War!");
          break;
        case 1:
          do_yell(ch, "No Mercy!");
          break;
        case 2:
          do_yell(ch, "Los Valdar Cuebiyari!");
          break;
        case 3:
          do_yell(ch, "Carai an Caldazar! Carai an Ellisande! Al Ellisande!");
          break;
        case 4:
          do_yell(ch, "Siempre Vive el Riesgo!");
          break;
        }
      }
}


void HeraGod::tattooFight( Object *obj, Character *ch ) const 
{
    switch(number_bits(5)) {
    case 0:
      act_p("{rТатуировка на твоем плече загорается красным светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_plague, ch->getModifyLevel(), ch, ch->fighting );
      break;
    case 1:
      act_p("{rТатуировка на твоем плече загорается красным светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_poison, ch->getModifyLevel(), ch, ch->fighting );
      break;
    case 2:
      act_p("{rТатуировка на твоем плече загорается красным светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_weaken, ch->getModifyLevel(), ch, ch->fighting );
      break;
    case 3:
      act_p("{rТатуировка на твоем плече загорается красным светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_slow, ch->getModifyLevel(), ch, ch->fighting );
      break;
    }
}


void DeimosGod::tattooFight( Object *obj, Character *ch ) const 
{
    switch(number_bits(6)) {
    case 0:
    case 1:
      act_p("{CТатуировка на твоем плече загорается голубым светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_cure_serious, ch->getModifyLevel(), ch, ch );
      break;
    case 2:
      act_p("{rТатуировка на твоем плече загорается красным светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell( gsn_web, ch->getModifyLevel( ), ch, ch->fighting );
      break;
    }
}


void ErosGod::tattooFight( Object *obj, Character *ch ) const 
{
    switch(number_bits(5)) {
    case 0:
    case 1:
      if ( number_percent() < URANGE(10, ch->getModifyLevel()-5, 90) )
      {
        act_p("{CТатуировка на твоем плече загорается ослепительным светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
        spell( gsn_heal, ch->getModifyLevel(), ch, ch );
      }        
      else
      {
        act_p("{CТатуировка на твоем плече загорается голубым светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
        spell( gsn_cure_serious, ch->getModifyLevel(), ch, ch );
      }        
      break;
    case 2:
    case 3:
      if ( number_percent() < URANGE(10, ch->getModifyLevel()-5, 90) )
      {
        act_p("{CТатуировка на твоем плече загорается ослепительным светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
        spell( gsn_heal, ch->getModifyLevel(), ch, ch );
      }        
      else
      {
        act_p("{CТатуировка на твоем плече загорается голубым светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
        spell( gsn_cure_critical, ch->getModifyLevel(), ch, ch );
      }        
      break;
    }
}


void EnkiGod::tattooFight( Object *obj, Character *ch ) const 
{
    switch(number_bits(5)) {
    case 0:
      act_p("{CТатуировка на твоем плече загорается голубым светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      spell(gsn_cure_critical, ch->getModifyLevel(), ch, ch );
      break;
    case 1:
    case 2:
      act_p("{rТатуировка на твоем плече загорается красным светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      
      if (IS_EVIL(ch->fighting) && !IS_EVIL(ch))
        spell( gsn_dispel_evil, ( int )( 1.2* ch->getModifyLevel() ), ch, ch->fighting );
      else if (IS_GOOD(ch->fighting) && !IS_GOOD(ch))
        spell( gsn_dispel_good, ( int )( 1.2* ch->getModifyLevel() ), ch, ch->fighting );
      else
        spell( gsn_lightning_bolt, ( int )( 1.2* ch->getModifyLevel() ), ch, ch->fighting );

      break;
    }
}


void GoktengriGod::tattooFight( Object *obj, Character *ch ) const 
{
    switch(number_bits(4)) {
    case 0:
    case 1:
      act_p("{WТатуировка на твоем плече загорается белым светом.{x",
                   ch,0,0,TO_CHAR,POS_DEAD);
      do_say(ch,"My honour is my life.");
      one_hit(ch, ch->fighting);
      break;
    }
}


void BastGod::tattooFight( Object *obj, Character *ch ) const 
{
    switch(number_bits(5)) {
    case 0:
      if (ch->isAffected(gsn_protection_heat))
        ch->println( "{cТатуировка слегка светится.{x");
      else {
        ch->println("{BТатуировка на твоем плече на мгновение складывается в символ щита.{x");
        spell(gsn_protection_heat, ch->getModifyLevel(), ch, ch );
      }
      break;
    case 1:
    case 2:
      if (ch->isAffected(gsn_haste))
        ch->println( "{cТатуировка слегка светится.{x");
      else {
          spell(gsn_haste, ch->getModifyLevel(), ch, ch );
          ch->send_to("{WТы внезапно ощущаешь повышенную активность!{x\n\r");
      }
      break;
    }
}

// TODO: move weather & time into a separate plugin, add dependency for religion plugin.
static bool season_winter()
{
    int m = time_info.month;
    return m == 16 || m <= 3;
}

void RavenQueenGod::tattooFight( Object *obj, Character *ch ) const 
{
    Character *victim = ch->fighting;
    if (!victim)
        return;

    // Find all applicable spells for this victim.
    vector<int> spells;

    if (immune_check(victim, DAM_NEGATIVE, 0) != RESIST_IMMUNE 
        && !(victim->is_npc() && IS_SET(victim->act, ACT_UNDEAD)))
    {
        for (int i = 0; i < 3; i++)
            spells.push_back(gsn_hand_of_undead);    
    }
  
    if (!IS_AFFECTED(victim, AFF_SLOW)) 
        spells.push_back(gsn_slow);    
     
    if (!IS_AFFECTED(victim, AFF_WEAKEN)) 
        spells.push_back(gsn_weaken);    

    if (spells.empty())
        return;

    // See if tattoo has a chance to work, more chance in winter.
    int tattoo_chance = season_winter() ? 24 : 12;
    if (!chance(tattoo_chance))
        return;
   
    // Cast random spell out of available ones. 
    ch->println("{rТатуировка на твоем плече загорается красным светом.{x");
    int random_spell = spells[number_range(0, spells.size() - 1)];
    int level_bonus = season_winter() ? 10 : 0;
    spell(random_spell, ch->getModifyLevel() + level_bonus, ch, victim);
}

class TricksterGodOneHit: public SkillWeaponOneHit {
public:
    TricksterGodOneHit( Character *ch, Character *victim );
    
    virtual void calcDamage( );
};

TricksterGodOneHit::TricksterGodOneHit( Character *ch, Character *victim )
            : SkillWeaponOneHit( ch, victim, gsn_knife )
{
}

void TricksterGodOneHit::calcDamage( )
{
    damBase( );
    gsn_enhanced_damage->getCommand( )->run( ch, victim, dam );;
    damApplyPosition( );
    dam = (ch->getModifyLevel( ) / 30 + 1) * dam + ch->getModifyLevel( );
    damApplyDamroll( );
    damApplyCounter( );

    WeaponOneHit::calcDamage( );
}

void ErevanGod::tattooFight( Object *obj, Character *ch ) const 
{
    Character *victim = ch->fighting;
    if (!victim)
        return;
   
    // Chance 1 in 8 of getting a heal. 
    if (ch->hit < ch->max_hit && chance(12)) {
        ch->println("{CТатуировка на твоем плече загорается голубым светом.{x");
        spell(gsn_cure_critical, ch->getModifyLevel(), ch, ch);
        return; 
    }

    if (!IS_AFFECTED(ch, AFF_HASTE) && chance(50)) {
        spell(gsn_haste, ch->getModifyLevel(), ch, ch );
        ch->println("{WТы внезапно ощущаешь повышенную активность!{x");
        return;
    }

    // Chance 1 in 10-12 of casting a spell (disregard haste).
    if (chance(10)) {
        spell(gsn_colour_spray, ch->getModifyLevel(), ch, victim);
        return;
    }

    // Chance 1 in 25 of additional hit.
    if (chance(15)) {
        try {
            TricksterGodOneHit thit(ch, victim);
            act("{CЭреван Илесир внезапно вселяется в тебя.{x", ch, 0, 0, TO_CHAR);
            act("{CЭреван Илесир внезапно вселяется в $c4.{x", ch, 0, 0, TO_ROOM);
            thit.hit();
            do_yell(victim, "Помогите! Эреван Илесир пыряет меня ножом!");
        }
        catch (const VictimDeathException &e) {
        }
    }
}

