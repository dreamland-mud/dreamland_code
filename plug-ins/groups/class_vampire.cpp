/* $Id: class_vampire.cpp,v 1.1.2.25.6.15 2010-09-01 21:20:44 rufina Exp $
 * 
 * ruffina, 2004
 * some commands remained from DreamLand 2.0
 */
#include "class_vampire.h"

#include "skill.h"
#include "spelltemplate.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"
#include "commandtemplate.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "roomutils.h"
#include "npcharacter.h"
#include "desire.h"
#include "object.h"

#include "gsn_plugin.h"
#include "act_move.h"
#include "mercdb.h"
#include "save.h"
#include "magic.h"
#include "fight.h"
#include "chance.h"
#include "occupations.h"
#include "onehit.h"
#include "onehit_weapon.h"
#include "damage_impl.h"
#include "vnum.h"
#include "merc.h"
#include "handler.h"
#include "act.h"
#include "interp.h"
#include "def.h"
#include "skill_utils.h"

PROF(vampire);
DESIRE(bloodlust);
RELIG(karmina);
GSN(grave);

#define ROOM_VNUM_GRAVE         5

#define IS_MOB_VAMPIRE(ch) ((ch)->is_npc( ) && IS_SET((ch)->act, ACT_VAMPIRE))

/*----------------------------------------------------------------------------
 * Vampiric Bite 
 *---------------------------------------------------------------------------*/
class VampiricBiteOneHit: public OneHit, public SkillDamage {
public:
    VampiricBiteOneHit( Character *ch, Character *victim );
    
    virtual void init( );
    virtual void calcTHAC0( );
    virtual void calcDamage( );
    void damBase( );
    virtual void postDamageEffects( );

protected:
    virtual bool mprog_immune();    
};

VampiricBiteOneHit::VampiricBiteOneHit( Character *ch, Character *victim )
            : Damage( ch, victim, 0, 0 ), OneHit( ch, victim ),
              SkillDamage( ch, victim, gsn_vampiric_bite, 0, 0, DAMF_WEAPON )
{
}

bool VampiricBiteOneHit::mprog_immune()
{
    return SkillDamage::mprog_immune();
}

void VampiricBiteOneHit::init( )
{
    dam_type = IS_BLOODLESS( victim ) ? DAM_PIERCE : DAM_NEGATIVE;
    skill = 20 + gsn_vampiric_bite->getEffective( ch );
}

void VampiricBiteOneHit::damBase( )
{
    int ave, level = skill_level(*gsn_vampiric_bite, ch);
    
         if (level >= 100) ave = level - 27; // as Tier 1 weapons, sigma distribution TODO: refactor
    else if (level >= 90)  ave = level - 18;	
    else if (level >= 80)  ave = level - 12;
    else if (level >= 70)  ave = level - 5;	
    else if (level >= 60)  ave = level - 1;	
    else if (level >= 50)  ave = level + 3;
    else if (level >= 45)  ave = level + 4;	
    else if (level >= 40)  ave = level + 9;
    else if (level >= 30)  ave = level + 10;
    else if (level >= 20)  ave = level + 11;
    else                   ave = level + 9;;
    
    dam = ave * skill / 100;                   // as weapon with skill bonus

    if (number_percent( ) <= skill / 8)        // as sharpness
        dam = 2 * dam + (dam * 2 * number_percent( ) / 100); 

    if (IS_BLOODLESS(victim)) {
        dam = dam/2;
    }
}


void VampiricBiteOneHit::calcDamage( )
{
    damBase( ); 
    damApplyEnhancedDamage( );
    damApplyPosition( );

    int slevel = skill_level(*gsn_vampiric_bite, ch);    
    dam = ( slevel / 15 + 1 ) * dam + slevel;

    damApplyDamroll( );
    
    OneHit::calcDamage( );
}

void VampiricBiteOneHit::calcTHAC0( )
{
    thacBase( );
    thacApplyHitroll( );
    thacApplySkill( );
    thac0 -= 10 * (100 - gsn_vampiric_bite->getEffective( ch ));
}

void VampiricBiteOneHit::postDamageEffects( )
{	

    if(!IS_BLOODLESS (victim)){

    // vampiric bite gives hp/mana to ch from victim
    int hit_ga, mana_ga;
	
    if ( !IS_SET( victim->form, FORM_COLD_BLOOD ) ) {
    	hit_ga = min( (orig_dam / 2 ), (int)victim->max_hit );
    	mana_ga = min( (orig_dam / 2 ), (int)victim->max_mana );
	ch->pecho("Твое здоровье и энергия восполняются, когда ты высасываешь кровь из противника.");
    }
    else {
    	hit_ga = min( (orig_dam / 10 ), (int)victim->max_hit );
    	mana_ga = min( (orig_dam / 10 ), (int)victim->max_mana );
	ch->pecho("Ты с отвращением высасываешь кровь, {cхолодную{x как сердца разработчиков.");	    
    }
	
    ch->hit   += hit_ga;
    ch->hit   =  min( ch->hit , ch->max_hit);
    ch->mana  += mana_ga;
    ch->mana  =  min( ch->mana , ch->max_mana);	
    update_pos( ch );

    // corrupt victim	
    Affect af;
    int level = skill_level(*gsn_vampiric_bite, ch);
    if ( (level > number_percent()) && (!IS_AFFECTED(victim,AFF_CORRUPTION)) ) {	
    	af.bitvector.setTable(&affect_flags);
    	af.type      = gsn_corruption;
   	    af.level     = level;
    	af.duration  = level / 10;
    	af.location = APPLY_HITROLL;
    	af.modifier  = - (level / 10);
    	af.bitvector.setValue(AFF_CORRUPTION);
        affect_join( victim, &af );	
	    
    	oldact_p("Ты вскрикиваешь от боли, когда рана от клыков $c2 начинает гнить!", ch, 0, victim, TO_VICT, POS_DEAD);
    	oldact("Рана от твоих клыков на шее $C2 начинает гноиться.", ch, 0, victim, TO_CHAR);	    
    }
    }

    else{
       ch->pecho("Ты не ощущаешь ни капли крови в этом существе. Брррр..."); 
    }
}

/*
 * 'dominate' (former 'control animal') skill command
 */
SKILL_RUNP( dominate )
{
  char arg[MAX_INPUT_LENGTH];
  Character *victim;
  int clevel, vlevel; 
  float chance, skill_mod, stat_mod, level_mod;
        

  //////////////// ELIGIBILITY CHECKS ////////////////

  ///// Standard checks: TODO: turn this into a function 
	
  argument = one_argument( argument, arg );

  if (ch->is_npc() || !gsn_dominate->usable( ch ) )
  {
  	ch->pecho("Это умение тебе недоступно.");
	return;
  }

  if ( arg[0] == '\0' )
  {
	ch->pecho("Доминировать над кем?");
	return;
  }

  if ( ( victim = get_char_room( ch, arg ) ) == 0 )
  {
	ch->pecho("Тут таких нет.");
	return;
  }

  if ( victim == ch )
  {
	ch->pecho("Ты ДОМИНИРУЕШЬ над собой!");
	return;
  }
	
  if (!victim->is_npc( ))
  {
	ch->pecho("Доминировать над игроками нельзя -- используй вместо этого очарование.");
	return;
  }

  if ( IS_BLOODLESS(victim) ) {
	ch->pecho("Это существо не поддается доминированию.");
	return;        
  }	
    
  if (is_safe(ch,victim) || overcharmed(ch))
	return;

  if ( IS_CHARMED(victim) ) {
	ch->pecho("Это существо уже под чьим-то контролем.");
	return;
  }
  
  if ( IS_CHARMED(ch) )
  {
  	ch->pecho("Спроси разрешения у сво%1$Gего|его|й хозя%1$Gина|ина|йки.", ch->master);
        return;
  }

  if ( IS_SET(victim->imm_flags,IMM_CHARM ) )
  {
	ch->pecho("У этого существа иммунитет к очарованию.");
        return;
  }
  
  if ( victim->fighting != 0 )
  {
	ch->pecho("Подожди, пока закончится сражение.");
	return;
  }
  
  if ( !IS_AWAKE(victim) || !victim->can_see(ch) )
  {
	ch->pecho("Твоя жертва не видит тебя.");
	return;                
  }  

  //////////////// BASE MODIFIERS //////////////// TODO: add this to XML
  skill_mod   = 0.5;
  stat_mod    = 0.02;
  level_mod   = 0.05;
  clevel      = skill_level(*gsn_dominate, ch);
  vlevel      = victim->getModifyLevel(); 

  //////////////// PROBABILITY CHECKS //////////////// 
      
  ch->setWaitViolence( 1 );

  chance = gsn_dominate->getEffective( ch ) * skill_mod;
  chance += ( ch->getCurrStat(STAT_CHA) - 20 ) * stat_mod * 100;
  chance += ( ch->getCurrStat(STAT_INT) - victim->getCurrStat(STAT_INT) ) * stat_mod * 100;
  chance += ( clevel - vlevel ) * level_mod * 100;
  chance = URANGE(1, (int)chance, 100) ;   
  
  // can't dominate shoppers or +5 level mobs    
  if ( (vlevel - clevel) > 5 )
	  chance = 0;	  
  
  if ( (victim->is_npc( )) && (victim->getNPC( )->behavior) &&
       (IS_SET( victim->getNPC( )->behavior->getOccupation( ), (1 << OCC_SHOPPER) )) )
	  chance = 0;

  //////////////// THE ROLL ////////////////
	
  if ( number_percent() > (int)chance )
  {
	gsn_dominate->improve( ch, false, victim );
	do_say(victim,"Я не собираюсь следовать за тобой!");
	multi_hit( victim , ch , "murder" );
	return;
  }

  gsn_dominate->improve( ch, true, victim );

  if ( victim->master )
        victim->stop_follower( );
	
  SET_BIT(victim->affected_by,AFF_CHARM);

  if ( victim->is_npc( ) && victim->in_room )
  {
	save_mobs( victim->in_room );
  }
                
  victim->master = victim->leader = ch;

  if ( ch != victim ) {
        oldact("$C1 смотрит на тебя с покорностью.",ch,0,victim,TO_CHAR);
  	oldact("$c1 подчиняет тебя своей воле.", ch, 0, victim, TO_VICT);
  }
	
  return;
}

/*
    * 'earthquake' open graves
    * get_char_room: digged are invis for each other
    * act_p: digged cannot see each other's actions
    * do_say, do_yell, do_shout: nobody hears
    * do_wake==do_rest for digged
    * char_from_room, damage, commands with SKILL_NODIG: remove DIGGED flags 
    * digged are 'nogate', but can be summoned
    * light in room doesnt hurt
    * 'eyes of intrigue/tiger' doesnt work
*/

/*
 * 'grave' skill command
 */

SKILL_RUNP( grave )
{
    Object *grave;
    int chance;
    Room *room = ch->in_room;

    chance = gsn_grave->getEffective( ch );

    if (chance < 2) {
	    ch->pecho("Ты не умеешь копать могилы.");
        return;
    }

    if (RoomUtils::isWater( room )) {
        ch->pecho("Ты же не хочешь промокнуть?");
        return;
    }

    if (room->getSectorType() == SECT_AIR) {
        ch->pecho("Копать в воздухе? И как ты себе это представляешь?");
        return;
    }

    if (room->getSectorType() == SECT_CITY) {
        ch->pecho("Здесь слишком твердая почва.");
        return;
    }
    
    if (room->getSectorType() == SECT_INSIDE || IS_SET(room->room_flags, ROOM_PRIVATE|ROOM_SOLITARY|ROOM_LAW|ROOM_SAFE))
    {
        ch->pecho("Здесь неподходящее место для копания могилы.");
        return;
    }
    
    if (get_obj_room_vnum( room, OBJ_VNUM_GRAVE )) {
        ch->pecho("Упс, похоже, этот участок уже занял твой коллега.");
        return;
    }

    if (IS_VIOLENT( ch )) {
        oldact_p("Ты слишком возбужде$gно|н|на, чтобы копать.", ch, 0, 0, TO_CHAR, POS_STANDING);
        return;
    }

    if (ch->move < 100) {
        oldact_p("Ты слишком уста$gло|л|ла для этого.", ch, 0, 0, TO_CHAR, POS_STANDING);
        return;
    }

    ch->move -= 100;
    
    ch->setWait( gsn_grave->getBeats(ch)  );

    if (number_percent( ) > chance) {
        oldact("$c1 предпринимает попытку закопать себя.", ch, 0, 0, TO_ROOM);
        oldact("Ты старательно ковыряешься в земле, но ничего не выходит.", ch, 0, 0, TO_CHAR);
        gsn_grave->improve( ch, false );
        return;
    }

    oldact("$c1 выкапывает себе могилку и устраивается в ней со всеми удобствами.", ch, 0, 0, TO_ROOM);
    oldact("Ты выкапываешь себе могилку и устраиваешься в ней со всеми удобствами.", ch, 0, 0, TO_CHAR);
    gsn_grave->improve( ch, true );

    if (number_percent( ) < 2) 
        oldact("Откуда-то сверху раздается громовой голос: \"ЛОПАТУ ВЕРНИ!\"", ch, 0, 0, TO_ALL);
    
    ch->dismount( );
    char_from_room( ch );
    char_to_room( ch, get_room_instance( ROOM_VNUM_GRAVE ) );
    ch->was_in_room = room;
    SET_BIT(ch->act, PLR_DIGGED);
    ch->position = POS_RESTING;
    
    grave = create_object( get_obj_index( OBJ_VNUM_GRAVE ), ch->getRealLevel( ) );
    grave->setOwner( ch->getNameC() );
    obj_to_room( grave, room );
}



/*
 * 'vampire' skill command
 */

SKILL_RUNP( vampire )
{
        Affect af;
        short level,duration;

        if ( ch->isAffected(gsn_vampire ) )
        {
		ch->pecho( "Ты не можешь стать еще более вампир%Gом|ом|шей!", ch );
                return;
        }

        if ( !ch->is_npc() && !gsn_vampire->usable( ch ) )
        {
                ch->pecho("Ты ощериваешь клыки, пытаясь превратиться в упыря, но ничего не выходит.");
                return;
        }

        if (gsn_vampire->getLearned( ch ) < 100)
        {
                ch->pecho("Поклонись своему Гильдмастеру для вампирьей Инициации ({hc{yсправка инициация{x).");
                return;
        }

        if ( weather_info.sunlight == SUN_LIGHT
                || weather_info.sunlight == SUN_RISE )
        {
		ch->pecho( "Помни, тебе нужно остерегаться солнечных лучей!", ch );
        }

        level = skill_level(*gsn_vampire, ch);
        duration = level / 10 ;
        duration += 5;

        af.type      = gsn_vampire;
        af.level     = level;
        af.duration  = duration;

        /* giant strength + negative, charm immunity */
        af.bitvector.setTable(&imm_flags);
        af.location = APPLY_STR;
        af.modifier  = 1 + (level / 20);
        af.bitvector.setValue(IMM_NEGATIVE | IMM_CHARM);
        affect_to_char( ch, &af );

        /* haste + dex, infrared, berserk, sneak */
        af.bitvector.setTable(&affect_flags);
        af.location = APPLY_DEX;
        af.modifier  = 1 + (level /20);
        af.bitvector.setValue(AFF_HASTE | AFF_INFRARED|AFF_BERSERK|AFF_SNEAK|AFF_FLYING);
        affect_to_char( ch, &af );

        /* size + vuln light, holy */
        af.bitvector.setTable(&vuln_flags);
        af.location = APPLY_SIZE;
        af.modifier  = 1 + (level / 50 );
        af.bitvector.setValue(VULN_LIGHT | VULN_HOLY);
        affect_to_char( ch, &af );

        /* damroll + resist cold, lighting */
        af.bitvector.setTable(&res_flags);
        af.location = APPLY_DAMROLL;
        af.modifier  = ch->damroll * 4 / 3;
        af.bitvector.setValue(RES_COLD | RES_LIGHTNING);
        affect_to_char( ch, &af );

        /* vampire flag */
        af.bitvector.setTable(&plr_flags);
        af.location = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector.setValue(PLR_VAMPIRE);
        affect_to_char( ch, &af );

	ch->pecho( "Превращаясь в кровожадн%1$Gого|ого|ую вампир%1$Gа|а|шу, ты чувствуешь прилив силы.", ch );
        oldact("$c1 неуловимо меняется, превращаясь в нечто ужасное!",ch,0,0,TO_ROOM);
}

void sucking( Character *ch, Character *victim ) 
{
    int cond, hp_gain, mana_gain;
    bool karminaBonus = false;

    if (victim == ch) {
        ch->pecho("У тебя недостаточно гибкий позвоночник.");
        return;
    }

    if (is_safe( ch, victim ))
        return;

    if (IS_AWAKE( victim )) {
        ch->pecho("Сначала жертва должна уснуть.");
        return;
    }
    
    if (!victim->isAffected(gsn_vampiric_bite )) {
        ch->pecho("В жертве нет необходимой дырочки.");
        return;
    }

    if ( IS_BLOODLESS( victim ) ) {
	ch->pecho("Ты не ощущаешь ни капли крови в этом существе. Брррр...");
	return;        
    }
	
    UNSET_DEATH_TIME(ch);
    ch->setWait( gsn_vampiric_bite->getBeats(ch)  );
                     
    oldact_p("Сквозь кошмарный сон ты чувствуешь, как $c1 высасывает твою {rкровь{x.", ch, 0, victim, TO_VICT, POS_DEAD);
    oldact("Ты высасываешь {rкровь{x из шеи $C2.", ch, 0, victim, TO_CHAR);
    oldact("$c1 высасывает {rкровь{x из шеи $C2.", ch, 0, victim, TO_NOTVICT);
    
    if (!ch->is_npc( )) {
	Object *tattoo = get_eq_char(ch, wear_tattoo);
	    
        if ( (ch->getReligion() == god_karmina) &&
	      (tattoo) && (chance(10)) ) {
                ch->pecho("{rКармина{x позволяет тебе насладиться кровью ради чистого удовольствия!");
                ch->recho("%^O1 на челе %C2 вспыхивает {Rярко-красным{x.", tattoo, ch);
		desire_bloodlust->gain( ch->getPC( ), 0 );	
        karminaBonus = true;	      
        }
	else {	    
        	desire_bloodlust->gain( ch->getPC( ), 20 );
	}	
        cond = ch->getPC( )->desires[desire_bloodlust];
    } 
    else 
        cond = number_range( -10, 60 );

    int slevel = skill_level(*gsn_vampiric_bite, ch);
	
    if ( !IS_SET( victim->form, FORM_COLD_BLOOD ) ) {    
	hp_gain = std::min( slevel * 5, (int)victim->max_hit );
	mana_gain = std::min( slevel * 5, (int)victim->max_hit );
    }	    
    else {
	oldact("Ты с отвращением глотаешь кровь $C2, {cхолодную{x как сердца разработчиков.", ch, 0, victim, TO_CHAR);	    
	hp_gain = std::min( slevel * 1, (int)victim->max_hit ); 
	mana_gain = std::min( slevel * 1, (int)victim->max_hit );	    
    }
	    
    ch->hit += hp_gain;
    ch->hit = std::min( ch->hit , ch->max_hit );
    ch->mana += mana_gain;
    ch->mana = std::min( ch->mana , ch->max_mana );	    
    update_pos( ch );
    
    victim->position = POS_STANDING;
    
    try {
        RawDamage( ch, victim, DAM_OTHER, hp_gain ).hit( true );

    	// corrupt victim	
   	Affect af;

    	if ( (slevel > number_percent()) && (!IS_AFFECTED(victim,AFF_CORRUPTION)) ) {	
    		af.bitvector.setTable(&affect_flags);
    		af.type      = gsn_corruption;
   		    af.level     = slevel;
    		af.duration  = slevel / 10;
    		af.location = APPLY_HITROLL;
    		af.modifier  = - (slevel / 10);
    		af.bitvector.setValue(AFF_CORRUPTION);
        	affect_join( victim, &af );	
	    	
		if (!IS_AWAKE( victim )) {
    			oldact_p("Ты вскрикиваешь от боли, когда рана от клыков $c2 начинает гнить!", ch, 0, victim, TO_VICT, POS_DEAD);
		}
		else {
    			oldact_p("Ты стонешь во сне, когда рана от клыков $c2 начинает гнить!", ch, 0, victim, TO_VICT, POS_DEAD);			
		}
		oldact("Рана от твоих клыков на шее $C2 начинает гноиться.", ch, 0, victim, TO_CHAR);	    
    	}	    
        victim->position = POS_SLEEPING;
                               
        if (number_percent( ) < cond && !karminaBonus) {
            set_fighting( victim, ch );
            oldact("$c1 очнул$gось|ся|ась от терзавшего $s кошмара.", victim, 0, ch, TO_ROOM);
            oldact_p("Ты просыпаешься от невыносимой боли в шее!", victim, 0, ch, TO_CHAR, POS_DEAD);
            multi_hit( victim, ch , "murder" );
        }
    } 
    catch (const VictimDeathException &) {
    }
}

/*
 * 'suck' command
 */

CMDRUNP( suck )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    one_argument( argument, arg );

    if (!gsn_vampiric_bite->usable( ch )) 
    {
        ch->pecho("Ты с надеждой ощупываешь свои клыки, но они не оправдывают ожиданий.");
        return;
    }

    if (!IS_VAMPIRE(ch) && !IS_MOB_VAMPIRE(ch)) {
	 ch->pecho( "Высасывать кровь можно, только превратившись в вампир%Gа|а|шу!", ch );
         return;
    }

    if (arg[0] == '\0') {
        ch->pecho("Пить кровь из кого?");
        return;
    }

    if ((victim = get_char_room( ch, arg )) == 0) {
        ch->pecho("Таких здесь нет.");
        return;
    }

    sucking( ch, victim );
}

/*
 * 'vampiric bite' skill command
 */

SKILL_RUNP( bite )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;
    int cond;
    Affect af;

    //////////////// ELIGIBILITY CHECKS ////////////////

    ///// Standard checks: TODO: turn this into a function    
    
    if ( MOUNTED(ch) )
    {
            ch->pecho("Только не верхом!");
            return;
    }
	
    one_argument( argument, arg );

    if ( ch->master != 0 && ch->is_npc() )
            return;

    if ( !ch->is_npc() && !gsn_vampiric_bite->usable( ch ) )
    {
            ch->pecho("Ты не умеешь кусаться.");
            return;
    }
	
    if ( IS_CHARMED(ch) )
    {
            ch->pecho( "Ты же не хочешь укусить сво%1$Gего|его|ю любим%1$Gого|ого|ю хозя%1$Gина|ина|йку.", ch->master);
            return;
    }

    if (!IS_VAMPIRE(ch) && !IS_MOB_VAMPIRE(ch)) {
  	    ch->pecho( "Чтобы укусить, сначала необходимо превратиться в вампир%Gа|а|шу!", ch );    
        return;
    }

    if ( arg[0] == '\0' )
    {
        ch->pecho("Укусить кого?");
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
        ch->pecho("Таких здесь нет.");
        return;
    }

    if ( victim == ch )
    {
        ch->pecho("От безделья ты пытаешься грызануть себя за локоть, но ничего не выходит.");
        return;
    }

    if ( IS_AWAKE(victim) )
    {
        ch->pecho("Сначала усыпи жертву.");
        return;
    }

    if (victim->isAffected(gsn_vampiric_bite ) && !IS_BLOODLESS ( victim )) {
        ch->pecho("Из шеи жертвы уже можно пить.");
        return;
    }
    
    if ( is_safe( ch, victim ) )
      return;
	
    if (IS_SET(victim->imm_flags, IMM_WEAPON))
    {
            oldact_p("$C1 имеет слишком крепкую шею, чтобы ее можно было прокусить.", ch, 0,
                    victim, TO_CHAR,POS_RESTING);
            return;
    }
	
    if(SHADOW(ch))
    {
            ch->pecho("Твои клыки проходят сквозь тень!");
            oldact_p("$c1 пытается прогрызть шею своей тени.",
                    ch, 0, 0, TO_ROOM,POS_RESTING);
            return;
    }
    

    UNSET_DEATH_TIME(ch);
    victim->setLastFightTime( );
    ch->setLastFightTime( );	
    ch->setWait( gsn_vampiric_bite->getBeats(ch)  );
    
    VampiricBiteOneHit vb( ch, victim );
    
    try {
        if (!IS_AWAKE(victim)
            && Chance(ch, gsn_vampiric_bite->getEffective( ch )-1, 100).reroll())
        {
    	    int slevel = skill_level(*gsn_vampiric_bite, ch);	
		
            af.type     = gsn_vampiric_bite;
            af.level    = slevel;
            af.duration = slevel / 40 + 1;
            af.location = APPLY_HITROLL;
            af.modifier = - slevel / 2;
            affect_join( victim, &af );

            gsn_vampiric_bite->improve( ch, true, victim );

            if (!ch->is_npc( ))
                cond = ch->getPC( )->desires[desire_bloodlust];
            else
                cond = number_range( -10, 80 );
                    
            if ((cond < 0 && number_percent( ) > 50) || IS_BLOODLESS (victim)) {
                vb.hit( );
            }
            else 
                sucking( ch, victim );   
        }
        else
        {
            gsn_vampiric_bite->improve( ch, false, victim );
            vb.miss( );
        }
        
        yell_panic( ch, victim,
                    "Помогите, {Dсоздание ночи{Y кусает меня!",
                    "Помогите, {Dсоздание ночи{Y кусает меня!" );
    }
    catch (const VictimDeathException& e) {                                     
    }
}


/*
 * 'vampiric touch' skill command
 */

SKILL_RUNP( touch )
{
	Character *victim;
        Affect af;    
        float chance, skill_mod, stat_mod, level_mod, quick_mod, sleep_mod, vis_mod, time_mod;
        char arg[MAX_INPUT_LENGTH];
	int slevel = skill_level(*gsn_vampiric_touch, ch);
        
        //////////////// BASE MODIFIERS //////////////// TODO: add this to XML
        skill_mod   = 0.5;
        stat_mod    = 0.04;
        level_mod   = 0.01;
        quick_mod   = 0.1;
        sleep_mod   = 0.1;
        vis_mod     = 0.1;
        time_mod    = 0.1;

        //////////////// ELIGIBILITY CHECKS ////////////////

        ///// Standard checks: TODO: turn this into a function 
    
        if ( MOUNTED(ch) )
        {
                ch->pecho("Только не верхом!");
                return;
        }
	
        if ( !gsn_vampiric_touch->usable( ch ) )
        {
                ch->pecho("Это умение тебе недоступно.");
                return;
        }	

    	if (!IS_VAMPIRE(ch) && !IS_MOB_VAMPIRE(ch))
    	{
        	ch->pecho( "Чтобы усыпить, сначала необходимо превратиться в вампир%Gа|а|шу!", ch ); 
        	return;
    	}

    	if ( IS_CHARMED(ch) )
    	{
        	ch->pecho("Ты же не хочешь усыпить сво%1$Gего|его|ю хозя%1$Gина|ина|йку?", ch->master);
        	return;
    	}
	
        // Needs at least one hand
        const GlobalBitvector &loc = ch->getWearloc( );
        if (!loc.isSet( wear_hands )
        || (!loc.isSet( wear_wrist_l ) && (!loc.isSet( wear_wrist_r )) ))
        {
                ch->pecho("Тебе нужна хотя бы одна рука для прикосновения.");
                return;
        }
	
        argument = one_argument(argument,arg);
	
        if ( arg[0] == '\0' )
        {
            ch->pecho("Усыпить кого?");
            return;
        }
	
    	if ( (victim = get_char_room(ch,arg)) == 0 )
    	{
        	ch->pecho("Тут таких нет.");
        	return;
    	}

    	if ( ch == victim )
    	{
        	ch->pecho("Может стоит просто заснуть?");
        	return;
    	}

    	if ( victim->isAffected(gsn_vampiric_touch) )
    	{
        	ch->pecho("Твоя жертва еще не отошла от прикосновения.");
        	return;
    	}
	
        if ( victim->fighting != 0 )
        {
                ch->pecho("Подожди, пока закончится сражение.");
                return;
        }
	
        if ( is_safe(ch,victim) )
        {
                return;
        }
	
        if (IS_SET(victim->imm_flags, IMM_NEGATIVE))
        {
                oldact_p("$C1 имеет иммунитет к темной магии.", ch, 0,
                        victim, TO_CHAR,POS_RESTING);
                return;
        }
	
        if (gsn_rear_kick->getCommand( )->apply( ch, victim ))
            return;
	
        if(SHADOW(ch))
        {
                ch->pecho("Твое прикосновение проходит сквозь тень!");
                oldact_p("$c1 пытается усыпить собственную тень.",
                    ch, 0, 0, TO_ROOM,POS_RESTING);
                return;
        }
	
        //////////////// PROBABILITY CHECKS ////////////////
            
        chance = 0;
	
        chance += gsn_vampiric_touch->getEffective( ch ) * skill_mod;
        chance += ( ch->getCurrStat(STAT_INT) - victim->getCurrStat(STAT_CON) ) * stat_mod * 100;
        chance += ( slevel - victim->getModifyLevel() ) * level_mod * 100;
        chance += victim->can_see(ch) ? 0 : (vis_mod * 100);
        chance += IS_AWAKE( victim ) ? 0 : (sleep_mod * 100);            

        if (IS_QUICK(ch)) {
            chance += quick_mod * 100;
        }

        if (IS_QUICK(victim)) {
            chance -= quick_mod * 100;            
        }

        if (IS_SET(victim->res_flags, RES_NEGATIVE)) {
            chance = chance / 2;
        }
            
        if ( IS_AFFECTED(ch,AFF_WEAK_STUN) ) {
            chance = chance / 2;
        }
    
        // neckguard can't protect if you're asleep
        if ( (victim->isAffected(gsn_backguard)) && IS_AWAKE( victim ) ) {
            chance = chance / 2;
        }

        int k = victim->getLastFightDelay( );
        if (k >= 0 && k < FIGHT_DELAY_TIME) {
            chance -= (FIGHT_DELAY_TIME - k) * time_mod * 100;
        }
           
        chance = max( (float)1, chance ); // there's always a chance

        //////////////// THE ROLL ////////////////

        UNSET_DEATH_TIME(ch);
        victim->setLastFightTime( );
        ch->setLastFightTime( ); 

    	ch->setWait( gsn_vampiric_touch->getBeats(ch) );

    if (Chance(ch, chance, 100).reroll())
    {
        oldact_p("Ты прикасаешься к шее $C2 и $E забывается в ужасном кошмаре.",
                                ch,0,victim,TO_CHAR,POS_RESTING);
        oldact_p("$c1 прикасается к твоей шее и ты забываешься в ужасном кошмаре.",
                                ch,0,victim,TO_VICT,POS_RESTING);
        oldact_p("$c1 прикасается к шее $C2 и $E забывается в ужасном кошмаре.",
                                ch,0,victim,TO_NOTVICT,POS_RESTING);
        gsn_vampiric_touch->improve( ch, true, victim );

        af.type = gsn_vampiric_touch;
        af.bitvector.setTable(&affect_flags);
        af.level = slevel;
        af.duration = slevel / 50 + 1;
        af.bitvector.setValue(AFF_SLEEP);
        affect_join ( victim,&af );

        if (IS_AWAKE(victim))
                victim->position = POS_SLEEPING;
        
        set_violent( ch, victim, true );
    }
    else
    {
        damage(ch,victim,0,gsn_vampiric_touch,DAM_NONE, true);
        gsn_vampiric_touch->improve( ch, false, victim );
        if ( !victim->is_npc() )
        {
                do_yell(victim, "Помогите! Вампиры!");
        }
    }
}

/*
 * 'unmorph' command
 */
CMDRUNP( unmorph )
{
    if (ch->getProfession( ) != prof_vampire)
    {
     ch->pecho("Ты не владеешь вампирьей трансформацией.");
     return;
    }

    if ( !IS_VAMPIRE(ch) )
    {
     ch->pecho("Ты уже выш{Smел{Sfла{Sx из вампирьей трансформации.");
     return;
    }

   affect_strip(ch, gsn_vampire);
   REMOVE_BIT(ch->act,PLR_VAMPIRE);
   ch->pecho("Ты выходишь из вампирьей трансформации и принимаешь свой обычный облик.");
}

/*
 * 'bloodlet' skill command
 */

SKILL_RUNP( bloodlet )
{
    Affect af;
    int dam;
    int chance = gsn_bloodlet->getEffective( ch );
    
    if ( chance < 2 ) {
        ch->pecho("Ты не владеешь этим!");
        return;
    }

    if (ch->isAffected(gsn_bloodlet )) {
        ch->pecho("У тебя еще не зажили старые раны.");
        return;
    }
    
    if (IS_VAMPIRE( ch )) {
        ch->pecho("Сначала детрансформируйся. Кровь упыря тебя не возбудит.");
        return;
    }

    ch->setWait( gsn_bloodlet->getBeats(ch) );
    dam = ch->getModifyLevel( );
    
    if (number_percent( ) < chance) {
        oldact("Ты перерезаешь себе вены.\r\nВид собственной {Rкрови{x возбуждает тебя!", ch, 0, 0, TO_CHAR);
        oldact("$c1 разрезает свою руку и жадно смотрит на капающую кровь.", ch, 0, 0, TO_ROOM);
        ch->getPC( )->desires[desire_bloodlust] = 0;
        gsn_bloodlet->improve( ch, true );

    } else {
        oldact("Упс! Кажется, ты потеря$gло|л|ла СЛИШКОМ много крови!", ch, 0, 0, TO_CHAR);
        oldact("$c1 слишком сильно ранит свою руку и не может остановить кровь.", ch, 0, 0, TO_ROOM);
        ch->getPC( )->desires[desire_bloodlust] = -6;
        dam *= 2;
        gsn_bloodlet->improve( ch, false );
    }

    rawdamage(ch, ch, DAM_OTHER, dam, true);
    
    postaffect_to_char( ch, gsn_bloodlet, 5 );
}

/*----------------------------------------------------------------------------
 * Bonedagger 
 *---------------------------------------------------------------------------*/
class BonedaggerOneHit: public SkillWeaponOneHit {
public:
    BonedaggerOneHit( Character *ch, Character *victim );

    virtual void calcTHAC0( );
    virtual void calcDamage( );
    void damApplyReligion();
};


BonedaggerOneHit::BonedaggerOneHit( Character *ch, Character *victim )
            : Damage(ch, victim, 0, 0, DAMF_WEAPON), SkillWeaponOneHit( ch, victim, gsn_bonedagger )
{
}

void BonedaggerOneHit::damApplyReligion()
{
    if (ch->getReligion() == god_karmina && chance(5)) {
        Object *tattoo = get_eq_char(ch, wear_tattoo);
        if (tattoo) {
            ch->pecho("{rКармина{x придает твоему костяному ножу особую остроту.");
            dam *= 2;
        }
    }
}

void BonedaggerOneHit::calcDamage( ) 
{
    damBase( );
    damApplyEnhancedDamage( );
    damApplyPosition( );
    damApplyDamroll( );
    damApplyReligion();

    dam *= 4;

    WeaponOneHit::calcDamage( );
}    

void BonedaggerOneHit::calcTHAC0( )
{
    thacBase( );
    thacApplyHitroll( );
    thacApplySkill( );
    thac0 -= 10 * (100 - gsn_bonedagger->getEffective( ch ));
}

/*
 * 'bonedagger' skill command
 */
SKILL_RUNP( bonedagger )
{
    char arg[MAX_INPUT_LENGTH];
    int chance = gsn_bonedagger->getEffective( ch );

    if (chance < 2) {
        ch->pecho("Что?");
        return;
    }

    if (!DIGGED(ch)) {
        ch->pecho("Нападать можно только из-под земли.");
        return;
    }
    
    one_argument( argument, arg );
    
    if (arg[0] == '\0') {
        if (ch->ambushing[0] == '\0') {
            ch->pecho("Чью тень ты хочешь подкараулить?");
            return;
        }
        else  {
            ch->printf("Ты ждешь, когда некто '{R%s{x' отбросит тень на твою могилу.\n\r", ch->ambushing);
            return;
        }
    }

    ch->setWait( gsn_bonedagger->getBeats(ch)  );
    ch->ambushing = str_dup( arg );
    run( ch, str_empty );
}

SKILL_APPLY( bonedagger )
{
    Affect af;
    Character *vch;
    Room *room = ch->was_in_room;
    
    if (!DIGGED(ch) || !ch->ambushing || ch->ambushing[0] == 0)
        return false;
    
    if (room->light <= 0) {
        if (IS_SET(room->room_flags, ROOM_DARK))
            return false;
        
        if (weather_info.sunlight == SUN_DARK)
            return false;
    }

    for (vch = room->people; vch; vch = vch->next_in_room) {
        if (!ch->can_see( vch ))
            continue;
        
        if (is_safe_nomessage( vch, ch ))
            continue;
        
        if (!is_name( ch->ambushing, vch->getNameC() ))
            continue;

        break;
    }

    if (!vch)
        return false;
    
    oldact("Твоя тень падает на могилу...", vch, 0, 0, TO_CHAR);
    oldact("Тень $c2 падает на могилу...", vch, 0, 0, TO_ROOM);
    
    undig( ch );
    ch->position = POS_STANDING;
    
    try {
        BonedaggerOneHit bd( ch, vch );
        
        if (number_percent( ) > gsn_bonedagger->getEffective( ch )) {
            oldact("$c1 костяным ножом промахивается мимо твоей тени!", ch, 0, vch, TO_VICT);
            oldact("$c1 костяным ножом промахивается мимо тени $C2!", ch, 0, vch, TO_NOTVICT);
            oldact("Ты костяным ножом промахиваешься мимо тени $C2!", ch, 0, vch, TO_CHAR);
            
            gsn_bonedagger->improve( ch, false, vch );
            bd.miss( );
            return true;
        }
        
        gsn_bonedagger->improve( ch, true, vch );
        
        oldact("$c1 приковывает твою тень костяным ножом к земле!\r\nТы не можешь сдвинуться с места!", ch, 0, vch, TO_VICT);
        oldact("$c1 приковывает тень $C2 костяным ножом к земле!", ch, 0, vch, TO_NOTVICT);
        oldact("Ты приковываешь тень $C2 костяным ножом к земле!", ch, 0, vch, TO_CHAR);

        af.type = gsn_bonedagger;
        af.level = ch->getModifyLevel( );
        af.duration = 1;
        
        af.modifier = 0;
        af.bitvector.setTable(&detect_flags);
        af.bitvector.setValue(ADET_WEB);
        affect_to_char( vch, &af );
        
        bd.hit( );
    }
    catch (const VictimDeathException &e) {
    }

    return true;
}

/*
 * 'sense life' skill command
 */

SKILL_RUNP( sense )
{
    int mana;
    
  if (ch->is_npc() || !gsn_sense_life->usable( ch ) )
    {
      ch->pecho("Ты не умеешь чуять присутствие живых организмов.");
      return;
    }

  if (ch->isAffected(gsn_sense_life))
    {
      ch->pecho("Ты уже можешь почуять присутствие живых организмов.");
      return;
    }

    mana = gsn_sense_life->getMana(ch);
    
  if (ch->mana < mana)
    {
      ch->pecho("У тебя не хватает энергии для этого.");
      return;
    }

  ch->setWait( gsn_sense_life->getBeats(ch)  );

  if (!ch->is_npc() && number_percent() < gsn_sense_life->getEffective( ch ))
    {
      Affect af;

      af.bitvector.setTable(&detect_flags);
      af.type         = gsn_sense_life;
      af.level         = ch->getModifyLevel();
      af.duration = ch->getModifyLevel();
      
      af.modifier = 0;
      af.bitvector.setValue(DETECT_LIFE);
      affect_to_char(ch, &af);

      ch->mana -= mana;

      oldact_p("Ты начинаешь чувствовать присутствие живых организмов в комнате!",
             ch,0,0,TO_CHAR,POS_RESTING);
      oldact_p("$c1 выглядит более чувствительным к присутствию живых организмов.",
             ch,0,0,TO_ROOM,POS_RESTING);
      gsn_sense_life->improve( ch, true );
    }
  else
    {
      ch->mana -= mana / 2;

     ch->pecho("Твоя попытка закончилась неудачей.");
      gsn_sense_life->improve( ch, false );
    }

}

SPELL_DECL(BatSwarm);
VOID_SPELL(BatSwarm)::run( Character *ch, Character *, int sn, int level ) 
{ 
    Affect af;

    if (!IS_VAMPIRE(ch) && !IS_MOB_VAMPIRE(ch)) {
	ch->pecho( "Для этого тебе необходимо превратиться в вампир%Gа|а|шу!", ch );    
        return;
    }

    if (ch->isAffected(sn)) {
        ch->pecho("Две стаи летучих мышей -- это слишком.");
        return;
    }
	
    oldact("В воздухе внезапно раздается шелест крыльев и едва различимый писк.", ch, 0, 0, TO_ALL);
    oldact("На зов $c2 слетается стая летучих мышей и окружает $s живым облаком.", ch, 0, 0, TO_ROOM);
    oldact("Стая летучих мышей прибывает по твоему зову и окружает тебя живым облаком.", ch, 0, 0, TO_CHAR);

    af.type            = sn;
    af.level            = level;
    af.duration            = 1 + level / 10;
    
    affect_to_char(ch, &af);
}


/*---------------------------------------------------------------------------
 * VampireGuildmaster
 *--------------------------------------------------------------------------*/

bool VampireGuildmaster::social( Character *actor, Character *victim, const DLString &socialName )
{
    if (actor->is_npc( ))
	    return false;

    if ( (actor->getProfession( ) != prof_vampire) && (number_percent() < 20) ) {
        oldact("$c1 одаривает $C4 равнодушным холодным взглядом.", ch, 0, actor, TO_NOTVICT );
        oldact("$c1 одаривает тебя равнодушным холодным взглядом.", ch, 0, actor, TO_VICT );
        return false;
    }
	    
    if ( socialName != "bow" ) {
	if ( victim != ch ) {
		return false;
	}
	else  {
        	oldact("$c1 с отвращением смотрит на ужимки $C2.", ch, 0, actor, TO_NOTVICT );
        	oldact("$c1 с отвращением смотрит на твои ужимки.", ch, 0, actor, TO_VICT );
        	say_act( actor, ch, "Тебе нужно {hc{yпоклониться{x своему мастеру, $c1." );	    
        	return false;		
	}
    }
    else {
    	if (!victim || victim != ch) {
            actor->pecho("%1$^C1 смотрит на тебя как на полн%2$Gого|ого|ую идиот%2$Gа|а|ку.", ch, actor);
            actor->recho("%1$^C1 смотрит на %2$C4 как на полн%2$Gого|ого|ую идиот%2$Gа|а|ку.", ch, actor);
        	say_act( actor, ch, "Кому ты кланяешься? Стенке?" );	    
        	return false;
	    }
    }	
	
    PCharacter *pActor = actor->getPC( );
    PCSkillData &data = pActor->getSkillData( gsn_vampire );

    if (data.learned == 100) {
        say_act( actor, ch, "Ты уже ста$gло|л|ла одн$gим|им|ой из нас, $c1." );
        return false;
    }
	
    if (pActor->getQuestPoints() < 50) {
        say_act( actor, ch, "Тебе потребуется 50 квестовых очков для обряда инициации." );
        return false;
    }

    pActor->addQuestPoints(-50);
    data.learned = 100;

    oldact("$C1 делится секретом бессмертия с $c5.", actor, 0, ch, TO_ROOM );
    oldact("$C1 делится с тобой секретом бессмертия.", actor, 0, ch, TO_CHAR );
    oldact_p("{BМолнии сверкают на небе.{x", actor, 0, ch, TO_ALL, POS_SLEEPING );
    return true;
}

