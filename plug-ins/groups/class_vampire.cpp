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

PROF(vampire);
DESIRE(bloodlust);
RELIG(karmina);


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
    dam_type = DAM_NEGATIVE;
    skill = 20 + ch->getSkill( sn );
}

void VampiricBiteOneHit::damBase( )
{
    int ave, level = ch->getModifyLevel( );
    
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
}


void VampiricBiteOneHit::calcDamage( )
{
    damBase( ); 
    damApplyEnhancedDamage( );
    damApplyPosition( );
    dam = ( ch->getModifyLevel( ) / 15 + 1) * dam + ch->getModifyLevel( );
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
    // vampiric bite gives hp/mana to ch from victim
    int hit_ga, mana_ga;
	
    if ( !IS_SET( victim->form, FORM_COLD_BLOOD ) ) {
    	hit_ga = min( (orig_dam / 2 ), (int)victim->max_hit );
    	mana_ga = min( (orig_dam / 2 ), (int)victim->max_mana );
	ch->send_to("Твое здоровье и энергия восполняются, когда ты высасываешь кровь из противника.\n\r");
    }
    else {
    	hit_ga = min( (orig_dam / 10 ), (int)victim->max_hit );
    	mana_ga = min( (orig_dam / 10 ), (int)victim->max_mana );
	ch->send_to("Ты с отвращением высасываешь кровь, {cхолодную{x как сердца разработчиков.\n\r");	    
    }
	
    ch->hit   += hit_ga;
    ch->hit   =  min( ch->hit , ch->max_hit);
    ch->mana  += mana_ga;
    ch->mana  =  min( ch->mana , ch->max_mana);	
    update_pos( ch );

    // corrupt victim	
    Affect af;
    int level = ch->getModifyLevel();
    if ( (level > number_percent()) && (!IS_AFFECTED(victim,AFF_CORRUPTION)) ) {	
    	af.where     = TO_AFFECTS;
    	af.type      = gsn_corruption;
   	    af.level     = level;
    	af.duration  = level / 10;
    	af.location  = APPLY_HITROLL;
    	af.modifier  = - (level / 10);
    	af.bitvector = AFF_CORRUPTION;
        affect_join( victim, &af );	
	    
    	act_p("Ты вскрикиваешь от боли, когда рана от клыков $c2 начинает гнить!", ch, 0, victim, TO_VICT, POS_DEAD);
    	act_p("Рана от твоих клыков на шее $C2 начинает гноиться.", ch, 0, victim, TO_CHAR, POS_RESTING);	    
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
  	ch->send_to( "Это умение тебе недоступно.\n\r");
	return;
  }

  if ( arg[0] == '\0' )
  {
	ch->send_to( "Доминировать над кем?\n\r");
	return;
  }

  if ( ( victim = get_char_room( ch, arg ) ) == 0 )
  {
	ch->send_to( "Тут таких нет.\n\r");
	return;
  }

  if ( victim == ch )
  {
	ch->send_to("Ты ДОМИНИРУЕШЬ над собой!\n\r");
	return;
  }
	
  if (!victim->is_npc( ))
  {
	ch->send_to("Доминировать над игроками нельзя -- используй вместо этого очарование.\n\r");
	return;
  }

  if ( IS_SET( victim->form, FORM_NONADOPTABLE ) ||
       IS_SET( victim->form, FORM_UNDEAD ) || 
       IS_SET( victim->form, FORM_CONSTRUCT ) ) {
	ch->send_to("Это существо не поддается доминированию.\n\r");
	return;        
  }	
    
  if (is_safe(ch,victim) || overcharmed(ch))
	return;

  if ( IS_CHARMED(victim) ) {
	ch->send_to("Это существо уже под чьим-то контролем.\n\r");
	return;
  }
  
  if ( IS_CHARMED(ch) )
  {
  	ch->pecho("Спроси разрешения у сво%1$Gего|его|й хозя%1$Gина|ина|йки.", ch->master);
        return;
  }

  if ( IS_SET(victim->imm_flags,IMM_CHARM ) )
  {
	ch->send_to("У этого существа иммунитет к очарованию.\n\r");
        return;
  }
  
  if ( victim->fighting != 0 )
  {
	ch->send_to("Подожди, пока закончится сражение.\n\r");
	return;
  }
  
  if ( !IS_AWAKE(victim) || !victim->can_see(ch) )
  {
	ch->send_to("Твоя жертва не видит тебя.\n\r");
	return;                
  }  

  //////////////// BASE MODIFIERS //////////////// TODO: add this to XML
  skill_mod   = 0.5;
  stat_mod    = 0.02;
  level_mod   = 0.05;
  clevel      = ch->getModifyLevel();
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
	multi_hit(victim,ch);
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
        act_p("$C1 смотрит на тебя с покорностью.",ch,0,victim,TO_CHAR,POS_RESTING);
  	act_p( "$c1 подчиняет тебя своей воле.", ch, 0, victim, TO_VICT,POS_RESTING );
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
 * 'dig' skill command
 */

SKILL_RUNP( dig )
{
    Object *grave;
    int chance;
    Room *room = ch->in_room;

    chance = gsn_digging->getEffective( ch );

    if (chance < 2) {
	ch->println("Для того, чтобы копать, найди и используй лопату.");
        return;
    }

    if (IS_WATER( room )) {
        ch->send_to("Ты же не хочешь промокнуть?\r\n");
        return;
    }

    if (room->sector_type == SECT_AIR) {
        ch->send_to("Копать в воздухе? И как ты себе это представляешь?\r\n");
        return;
    }

    if (room->sector_type == SECT_CITY) {
        ch->send_to("Здесь слишком твердая почва.\r\n");
        return;
    }
    
    if (room->sector_type == SECT_INSIDE || IS_SET(room->room_flags, ROOM_PRIVATE|ROOM_SOLITARY|ROOM_LAW|ROOM_SAFE))
    {
        ch->send_to("Здесь неподходящее место для копания могилы.\r\n");
        return;
    }
    
    if (get_obj_room_vnum( room, OBJ_VNUM_GRAVE )) {
        ch->send_to("Упс, похоже, этот участок уже занял твой коллега.\r\n");
        return;
    }

    if (IS_VIOLENT( ch )) {
        act_p("Ты слишком возбужде$gно|н|на, чтобы копать.", ch, 0, 0, TO_CHAR, POS_STANDING);
        return;
    }

    if (ch->move < 100) {
        act_p("Ты слишком уста$gло|л|ла для этого.", ch, 0, 0, TO_CHAR, POS_STANDING);
        return;
    }

    ch->move -= 100;
    
    ch->setWait( gsn_digging->getBeats( )  );

    if (number_percent( ) > chance) {
        act_p("$c1 предпринимает попытку закопать себя.", ch, 0, 0, TO_ROOM, POS_RESTING);
        act_p("Ты старательно ковыряешься в земле, но ничего не выходит.", ch, 0, 0, TO_CHAR, POS_RESTING);
        gsn_digging->improve( ch, false );
        return;
    }

    act_p("$c1 выкапывает себе могилку и устраивается в ней со всеми удобствами.", ch, 0, 0, TO_ROOM, POS_RESTING);
    act_p("Ты выкапываешь себе могилку и устраиваешься в ней со всеми удобствами.", ch, 0, 0, TO_CHAR, POS_RESTING);
    gsn_digging->improve( ch, true );

    if (number_percent( ) < 2) 
        act_p("Откуда-то сверху раздается громовой голос: \"ЛОПАТУ ВЕРНИ!\"", ch, 0, 0, TO_ALL, POS_RESTING);
    
    ch->dismount( );
    char_from_room( ch );
    char_to_room( ch, get_room_index( ROOM_VNUM_GRAVE ) );
    ch->was_in_room = room;
    SET_BIT(ch->act, PLR_DIGGED);
    ch->position = POS_RESTING;
    
    grave = create_object( get_obj_index( OBJ_VNUM_GRAVE ), ch->getRealLevel( ) );
    grave->setOwner( ch->getNameP( ) );
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
                ch->send_to("Ты ощериваешь клыки, пытаясь превратиться в упыря, но ничего не выходит.\n\r");
                return;
        }

        if (gsn_vampire->getLearned( ch ) < 100)
        {
                ch->send_to("Поклонись своему Гильдмастеру для вампирьей Инициации ({hc{yсправка инициация{x).\n\r");
                return;
        }

        if ( weather_info.sunlight == SUN_LIGHT
                || weather_info.sunlight == SUN_RISE )
        {
		ch->pecho( "Помни, тебе нужно остерегаться солнечных лучей!", ch );
        }

        level = ch->getModifyLevel();
        duration = level / 10 ;
        duration += 5;

        /* haste + dex, infrared */
        af.where     = TO_AFFECTS;
        af.type      = gsn_vampire;
        af.level     = level;
        af.duration  = duration;
        af.location  = APPLY_DEX;
        af.modifier  = 1 + (level /20);
        af.bitvector = AFF_HASTE | AFF_INFRARED;
        affect_to_char( ch, &af );

        /* giant strength + berserk */
        af.where     = TO_AFFECTS;
        af.location  = APPLY_STR;
        af.modifier  = 1 + (level / 20);
        af.bitvector = AFF_BERSERK;
        affect_to_char( ch, &af );

        /* sneak */
        af.where     = TO_AFFECTS;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF_SNEAK;
        affect_to_char( ch, &af );

        /* size + vuln light, holy */
        af.where     = TO_VULN;
        af.location  = APPLY_SIZE;
        af.modifier  = 1 + (level / 50 );
        af.bitvector = VULN_LIGHT | VULN_HOLY;
        affect_to_char( ch, &af );

        /* damroll + resist cold, lighting */
        af.where     = TO_RESIST;
        af.location  = APPLY_DAMROLL;
        af.modifier  = ch->damroll * 4 / 3;
        af.bitvector = RES_COLD | RES_LIGHTNING;
        affect_to_char( ch, &af );

        /* negative, charm immunity */
        af.where = TO_IMMUNE;
        af.location = APPLY_NONE;
        af.modifier = 0;
        af.bitvector = IMM_NEGATIVE | IMM_CHARM;
        affect_to_char(ch, &af);

        /* vampire flag */
        af.where     = TO_ACT_FLAG;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = PLR_VAMPIRE;
        affect_to_char( ch, &af );

	ch->pecho( "Превращаясь в кровожадн%1$Gого|ого|ую вампир%1$Gа|а|шу, ты чувствуешь прилив силы.", ch );
        act_p("$c1 неуловимо меняется, превращаясь в нечто ужасное!",ch,0,0,TO_ROOM,POS_RESTING);
}

void sucking( Character *ch, Character *victim ) 
{
    int cond, hp_gain, mana_gain;

    if (victim == ch) {
        ch->send_to("У тебя недостаточно гибкий позвоночник.\n\r");
        return;
    }

    if (is_safe( ch, victim ))
        return;

    if (IS_AWAKE( victim )) {
        ch->send_to("Сначала жертва должна уснуть.\r\n");
        return;
    }
    
    if (!victim->isAffected(gsn_vampiric_bite )) {
        ch->send_to("В жертве нет необходимой дырочки.\n\r");
        return;
    }

    if ( IS_SET( victim->form, FORM_NONADOPTABLE ) ||
         IS_SET( victim->form, FORM_UNDEAD ) || 
         IS_SET( victim->form, FORM_CONSTRUCT ) ) {
	ch->send_to("Ты не ощущаешь ни капли крови в этом существе. Брррр...\n\r");
	return;        
    }
	
    UNSET_DEATH_TIME(ch);
    ch->setWait( gsn_vampiric_bite->getBeats( )  );
                     
    act_p("Сквозь кошмарный сон ты чувствуешь, как $c1 высасывает твою {rкровь{x.", ch, 0, victim, TO_VICT, POS_DEAD);
    act_p("Ты высасываешь {rкровь{x из шеи $C2.", ch, 0, victim, TO_CHAR, POS_RESTING);
    act_p("$c1 высасывает {rкровь{x из шеи $C2.", ch, 0, victim, TO_NOTVICT, POS_RESTING);
    
    if (!ch->is_npc( )) {
	Object *tattoo = get_eq_char(ch, wear_tattoo);
	    
        if ( (ch->getReligion() == god_karmina) &&
	      (tattoo) && (chance(10)) ) {
                ch->pecho("{rКармина{x позволяет тебе насладиться кровью ради чистого удовольствия!");
                ch->recho("%^O1 на челе %C2 вспыхивает {Rярко-красным{x.", tattoo, ch);
		desire_bloodlust->gain( ch->getPC( ), 0 );		      
        }
	else {	    
        	desire_bloodlust->gain( ch->getPC( ), 20 );
	}	
        cond = ch->getPC( )->desires[desire_bloodlust];
    } 
    else 
        cond = number_range( -10, 60 );

    if ( !IS_SET( victim->form, FORM_COLD_BLOOD ) ) {    
	hp_gain = std::min( ch->getModifyLevel( ) * 5, (int)victim->max_hit );
	mana_gain = std::min( ch->getModifyLevel( ) * 5, (int)victim->max_hit );
    }	    
    else {
	act_p("Ты с отвращением глотаешь кровь $C2, {cхолодную{x как сердца разработчиков.", ch, 0, victim, TO_CHAR, POS_RESTING);	    
	hp_gain = std::min( ch->getModifyLevel( ) * 1, (int)victim->max_hit ); 
	mana_gain = std::min( ch->getModifyLevel( ) * 1, (int)victim->max_hit );	    
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
    	int level = ch->getModifyLevel();
    	if ( (level > number_percent()) && (!IS_AFFECTED(victim,AFF_CORRUPTION)) ) {	
    		af.where     = TO_AFFECTS;
    		af.type      = gsn_corruption;
   		af.level     = level;
    		af.duration  = level / 10;
    		af.location  = APPLY_HITROLL;
    		af.modifier  = - (level / 10);
    		af.bitvector = AFF_CORRUPTION;
        	affect_join( victim, &af );	
	    	
		if (!IS_AWAKE( victim )) {
    			act_p("Ты вскрикиваешь от боли, когда рана от клыков $c2 начинает гнить!", ch, 0, victim, TO_VICT, POS_DEAD);
		}
		else {
    			act_p("Ты стонешь во сне, когда рана от клыков $c2 начинает гнить!", ch, 0, victim, TO_VICT, POS_DEAD);			
		}
		act_p("Рана от твоих клыков на шее $C2 начинает гноиться.", ch, 0, victim, TO_CHAR, POS_RESTING);	    
    	}	    
        victim->position = POS_SLEEPING;
                               
        if (number_percent( ) < cond) {
            set_fighting( victim, ch );
            act_p("$c1 очнул$gось|ся|ась от терзавшего $s кошмара.", victim, 0, ch, TO_ROOM, POS_RESTING);
            act_p("Ты просыпаешься от невыносимой боли в шее!", victim, 0, ch, TO_CHAR, POS_DEAD);
            multi_hit( victim, ch );
        }
    } 
    catch (const VictimDeathException &) {
    }
}

/*
 * 'suck' command
 */

SKILL_RUNP( suck )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    one_argument( argument, arg );

    if (!gsn_vampiric_bite->usable( ch )) 
    {
        ch->send_to("Ты с надеждой ощупываешь свои клыки, но они не оправдывают ожиданий.\n\r");
        return;
    }

    if (!IS_VAMPIRE(ch) && !IS_MOB_VAMPIRE(ch)) {
	 ch->pecho( "Высасывать кровь можно, только превратившись в вампир%Gа|а|шу!", ch );
         return;
    }

    if (arg[0] == '\0') {
        ch->send_to("Пить кровь из кого?\n\r");
        return;
    }

    if ((victim = get_char_room( ch, arg )) == 0) {
        ch->send_to("Таких здесь нет.\n\r");
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
            ch->send_to("Только не верхом!\n\r");
            return;
    }
	
    one_argument( argument, arg );

    if ( ch->master != 0 && ch->is_npc() )
            return;

    if ( !ch->is_npc() && !gsn_vampiric_bite->usable( ch ) )
    {
            ch->send_to("Ты не умеешь кусаться.\n\r");
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
        ch->send_to("Укусить кого?\n\r");
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
        ch->send_to("Таких здесь нет.\n\r");
        return;
    }

    if ( victim == ch )
    {
        ch->send_to("От безделья ты пытаешься грызануть себя за локоть, но ничего не выходит.\n\r");
        return;
    }

    if ( IS_AWAKE(victim) )
    {
        ch->send_to("Сначала усыпи жертву.\n\r");
        return;
    }

    if ( IS_SET( victim->form, FORM_NONADOPTABLE ) ||
         IS_SET( victim->form, FORM_UNDEAD ) || 
         IS_SET( victim->form, FORM_CONSTRUCT ) ) {
	ch->send_to("Ты не ощущаешь ни капли крови в этом существе. Брррр...\n\r");
	return;        
    }	

    if (victim->isAffected(gsn_vampiric_bite )) {
        ch->send_to("Из шеи жертвы уже можно пить.\r\n");
        return;
    }
    
    if ( is_safe( ch, victim ) )
      return;
	
    if (IS_SET(victim->imm_flags, IMM_WEAPON))
    {
            act_p("$C1 имеет слишком крепкую шею, чтобы ее можно было прокусить.", ch, 0,
                    victim, TO_CHAR,POS_RESTING);
            return;
    }
	
    if(SHADOW(ch))
    {
            ch->send_to("Твои клыки проходят сквозь тень!\n\r");
            act_p("$c1 пытается прогрызть шею своей тени.",
                    ch, 0, 0, TO_ROOM,POS_RESTING);
            return;
    }
    

    UNSET_DEATH_TIME(ch);
    victim->setLastFightTime( );
    ch->setLastFightTime( );	
    ch->setWait( gsn_vampiric_bite->getBeats( )  );
    
    VampiricBiteOneHit vb( ch, victim );
    
    try {
        if (!IS_AWAKE(victim)
            && Chance(ch, gsn_vampiric_bite->getEffective( ch )-1, 100).reroll())
        {
            af.type     = gsn_vampiric_bite;
            af.level    = ch->getModifyLevel();
            af.duration = ch->getModifyLevel() / 40 + 1;
            af.location = APPLY_HITROLL;
            af.modifier = - ch->getModifyLevel( ) / 2;
            affect_join( victim, &af );

            gsn_vampiric_bite->improve( ch, true, victim );

            if (!ch->is_npc( ))
                cond = ch->getPC( )->desires[desire_bloodlust];
            else
                cond = number_range( -10, 80 );
                    
            if (cond < 0 && number_percent( ) > 50) {
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
        
        //////////////// BASE MODIFIERS //////////////// TODO: add this to XML
        skill_mod   = 0.2;
        stat_mod    = 0.04;
        level_mod   = 0.01;
        quick_mod   = 0.1;
        sleep_mod   = 0.1;
        vis_mod     = 0.1;
        time_mod    = 0.05;

        //////////////// ELIGIBILITY CHECKS ////////////////

        ///// Standard checks: TODO: turn this into a function 
    
        if ( MOUNTED(ch) )
        {
                ch->send_to("Только не верхом!\n\r");
                return;
        }
	
        if ( !gsn_vampiric_touch->usable( ch ) )
        {
                ch->send_to("Это умение тебе недоступно.\n\r");
                return;
        }	

    	if (!IS_VAMPIRE(ch) && !IS_MOB_VAMPIRE(ch))
    	{
        	ch->send_to("Это умение доступно только вампирам.\n\r");
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
                ch->send_to("Тебе нужна хотя бы одна рука для прикосновения.\r\n");
                return;
        }
	
        argument = one_argument(argument,arg);
	
        if ( arg[0] == '\0' )
        {
            ch->send_to("Усыпить кого?\n\r");
            return;
        }
	
    	if ( (victim = get_char_room(ch,arg)) == 0 )
    	{
        	ch->send_to("Тут таких нет.\n\r");
        	return;
    	}

    	if ( ch == victim )
    	{
        	ch->send_to("Может стоит просто заснуть?\n\r");
        	return;
    	}

    	if ( victim->isAffected(gsn_vampiric_touch) )
    	{
        	ch->send_to("Твоя жертва еще не отошла от прикосновения.\n\r");
        	return;
    	}
	
        if ( victim->fighting != 0 )
        {
                ch->send_to("Подожди, пока закончится сражение.\n\r");
                return;
        }
	
        if ( is_safe(ch,victim) )
        {
                return;
        }
	
        if (IS_SET(victim->imm_flags, IMM_NEGATIVE))
        {
                act_p("$C1 имеет иммунитет к темной магии.", ch, 0,
                        victim, TO_CHAR,POS_RESTING);
                return;
        }
	
        // sleepy centaurs can't rearkick
        if ( IS_AWAKE(victim) && (gsn_rear_kick->getCommand( )->run( ch, victim )) )
            return;
	
        if(SHADOW(ch))
        {
                ch->send_to("Твое прикосновение проходит сквозь тень!\n\r");
                act_p("$c1 пытается усыпить собственную тень.",
                    ch, 0, 0, TO_ROOM,POS_RESTING);
                return;
        }
	
        //////////////// PROBABILITY CHECKS ////////////////
            
        chance = 0;
        chance += gsn_vampiric_touch->getEffective( ch ) * skill_mod;
        chance += ( ch->getCurrStat(STAT_INT) - victim->getCurrStat(STAT_CON) ) * stat_mod * 100;
        chance += ( ch->getModifyLevel() - victim->getModifyLevel() ) * level_mod * 100;
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

        int k = ch->getLastFightDelay( );
        if (k >= 0 && k < FIGHT_DELAY_TIME) {
            chance -= (FIGHT_DELAY_TIME - k) * time_mod * 100;
        }
           
        chance = max( (float)1, chance ); // there's always a chance

        //////////////// THE ROLL ////////////////

        UNSET_DEATH_TIME(ch);
        victim->setLastFightTime( );
        ch->setLastFightTime( ); 

    	ch->setWait( gsn_vampiric_touch->getBeats( ) );

    if (Chance(ch, chance, 100).reroll())
    {
        act_p("Ты прикасаешься к шее $C2 и $E забывается в ужасном кошмаре.",
                                ch,0,victim,TO_CHAR,POS_RESTING);
        act_p("$c1 прикасается к твоей шее и ты забываешься в ужасном кошмаре.",
                                ch,0,victim,TO_VICT,POS_RESTING);
        act_p("$c1 прикасается к шее $C2 и $E забывается в ужасном кошмаре.",
                                ch,0,victim,TO_NOTVICT,POS_RESTING);
        gsn_vampiric_touch->improve( ch, true, victim );

        af.type = gsn_vampiric_touch;
        af.where = TO_AFFECTS;
        af.level = ch->getModifyLevel();
        af.duration = ch->getModifyLevel() / 20 + 1;
        af.location = APPLY_NONE;
        af.modifier = 0;
        af.bitvector = AFF_SLEEP;
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
SKILL_RUNP( unmorph )
{
    if (ch->getProfession( ) != prof_vampire)
    {
     ch->send_to("Ты не владеешь вампирьей трансформацией.\n\r");
     return;
    }

    if ( !IS_VAMPIRE(ch) )
    {
     ch->send_to("Ты уже выш{Smел{Sfла{Sx из вампирьей трансформации.\n\r");
     return;
    }

   affect_strip(ch, gsn_vampire);
   REMOVE_BIT(ch->act,PLR_VAMPIRE);
   ch->send_to("Ты выходишь из вампирьей трансформации и принимаешь свой обычный облик.\n\r");
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
        ch->send_to("Ты не владеешь этим!\n\r");
        return;
    }

    if (ch->isAffected(gsn_bloodlet )) {
        ch->send_to("У тебя еще не зажили старые раны.\r\n");
        return;
    }
    
    if (IS_VAMPIRE( ch )) {
        ch->send_to("Сначала детрансформируйся. Кровь упыря тебя не возбудит.\r\n");
        return;
    }

    ch->setWait( gsn_bloodlet->getBeats( ) );
    dam = ch->getModifyLevel( );
    
    if (number_percent( ) < chance) {
        act_p("Ты перерезаешь себе вены.\r\nВид собственной {Rкрови{x возбуждает тебя!", ch, 0, 0, TO_CHAR, POS_RESTING);
        act_p("$c1 разрезает свою руку и жадно смотрит на капающую кровь.", ch, 0, 0, TO_ROOM, POS_RESTING);
        ch->getPC( )->desires[desire_bloodlust] = 0;
        gsn_bloodlet->improve( ch, true );

    } else {
        act_p("Упс! Кажется, ты потеря$gло|л|ла СЛИШКОМ много крови!", ch, 0, 0, TO_CHAR, POS_RESTING);
        act_p("$c1 слишком сильно ранит свою руку и не может остановить кровь.", ch, 0, 0, TO_ROOM, POS_RESTING);
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
        ch->send_to("Что?\r\n");
        return;
    }

    if (!DIGGED(ch)) {
        ch->send_to("Нападать можно только из-под земли.\r\n");
        return;
    }
    
    one_argument( argument, arg );
    
    if (arg[0] == '\0') {
        if (ch->ambushing[0] == '\0') {
            ch->send_to("Чью тень ты хочешь подкараулить?\n\r");
            return;
        }
        else  {
            ch->printf("Ты ждешь, когда некто '{R%s{x' отбросит тень на твою могилу.\n\r", ch->ambushing);
            return;
        }
    }

    ch->setWait( gsn_bonedagger->getBeats( )  );
    ch->ambushing = str_dup( arg );
    run( ch, str_empty );
}

BOOL_SKILL( bonedagger )::run( Character *ch ) 
{
    Affect af;
    Character *victim;
    Room *room = ch->was_in_room;
    
    if (!DIGGED(ch) || !ch->ambushing || ch->ambushing[0] == 0)
        return false;
    
    if (room->light <= 0) {
        if (IS_SET(room->room_flags, ROOM_DARK))
            return false;
        
        if (weather_info.sunlight == SUN_DARK)
            return false;
    }

    for (victim = room->people; victim; victim = victim->next_in_room) {
        if (!ch->can_see( victim ))
            continue;
        
        if (is_safe_nomessage( victim, ch ))
            continue;
        
        if (!is_name( ch->ambushing, victim->getNameP( ) ))
            continue;

        break;
    }

    if (!victim)
        return false;
    
    act_p("Твоя тень падает на могилу...", victim, 0, 0, TO_CHAR, POS_RESTING);
    act_p("Тень $c2 падает на могилу...", victim, 0, 0, TO_ROOM, POS_RESTING);
    
    undig( ch );
    ch->position = POS_STANDING;
    
    try {
        BonedaggerOneHit bd( ch, victim );
        
        if (number_percent( ) > gsn_bonedagger->getEffective( ch )) {
            act_p("$c1 костяным ножом промахивается мимо твоей тени!", ch, 0, victim, TO_VICT, POS_RESTING);
            act_p("$c1 костяным ножом промахивается мимо тени $C2!", ch, 0, victim, TO_NOTVICT, POS_RESTING);
            act_p("Ты костяным ножом промахиваешься мимо тени $C2!", ch, 0, victim, TO_CHAR, POS_RESTING);
            
            gsn_bonedagger->improve( ch, false, victim );
            bd.miss( );
            return true;
        }
        
        gsn_bonedagger->improve( ch, true, victim );
        
        act_p("$c1 приковывает твою тень костяным ножом к земле!\r\nТы не можешь сдвинуться с места!", ch, 0, victim, TO_VICT, POS_RESTING);
        act_p("$c1 приковывает тень $C2 костяным ножом к земле!", ch, 0, victim, TO_NOTVICT, POS_RESTING);
        act_p("Ты приковываешь тень $C2 костяным ножом к земле!", ch, 0, victim, TO_CHAR, POS_RESTING);

        af.type = gsn_bonedagger;
        af.level = ch->getModifyLevel( );
        af.duration = 1;
        af.location = APPLY_NONE;
        af.modifier = 0;
        af.where = TO_DETECTS;
        af.bitvector = ADET_WEB;
        affect_to_char( victim, &af );
        
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
      ch->send_to("Ты не умеешь чуять присутствие живых организмов.\n\r");
      return;
    }

  if (ch->isAffected(gsn_sense_life))
    {
      ch->send_to("Ты уже можешь почуять присутствие живых организмов.\n\r");
      return;
    }

    mana = gsn_sense_life->getMana( );
    
  if (ch->mana < mana)
    {
      ch->send_to("У тебя не хватает энергии для этого.\n\r");
      return;
    }

  ch->setWait( gsn_sense_life->getBeats( )  );

  if (!ch->is_npc() && number_percent() < gsn_sense_life->getEffective( ch ))
    {
      Affect af;

      af.where  = TO_DETECTS;
      af.type         = gsn_sense_life;
      af.level         = ch->getModifyLevel();
      af.duration = ch->getModifyLevel();
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = DETECT_LIFE;
      affect_to_char(ch, &af);

      ch->mana -= mana;

      act_p("Ты начинаешь чувствовать присутствие живых организмов в комнате!",
             ch,0,0,TO_CHAR,POS_RESTING);
      act_p("$c1 выглядит более чувствительным к присутствию живых организмов.",
             ch,0,0,TO_ROOM,POS_RESTING);
      gsn_sense_life->improve( ch, true );
    }
  else
    {
      ch->mana -= mana / 2;

     ch->send_to("Твоя попытка закончилась неудачей.\n\r" );
      gsn_sense_life->improve( ch, false );
    }

}

SPELL_DECL(BatSwarm);
VOID_SPELL(BatSwarm)::run( Character *ch, Character *, int sn, int level ) 
{ 
    Affect af;

    if (!ch->fighting) {
        ch->send_to("Сейчас ты не сражаешься!\r\n");
        return;
    }

    if (!IS_VAMPIRE(ch) && !IS_MOB_VAMPIRE(ch)) {
	ch->pecho( "Для этого тебе необходимо превратиться в вампир%Gа|а|шу!", ch );    
        return;
    }

    if (ch->isAffected(sn)) {
        ch->send_to("Две стаи летучих мышей -- это слишком.\r\n");
        return;
    }
	
    act_p("В воздухе внезапно раздается шелест крыльев и едва различимый писк.", ch, 0, 0, TO_ALL, POS_RESTING);
    act_p("На зов $c2 слетается стая летучих мышей и окружает $s живым облаком.", ch, 0, 0, TO_ROOM, POS_RESTING);
    act_p("Стая летучих мышей прибывает по твоему зову и окружает тебя живым облаком.", ch, 0, 0, TO_CHAR, POS_RESTING);

    af.where            = TO_AFFECTS;
    af.type            = sn;
    af.level            = level;
    af.duration            = 1 + level / 10;
    af.bitvector    = 0;
    af.modifier            = 0;
    af.location            = APPLY_NONE;
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
        act( "$c1 одаривает $C4 равнодушным холодным взглядом.", ch, 0, actor, TO_NOTVICT );
        act( "$c1 одаривает тебя равнодушным холодным взглядом.", ch, 0, actor, TO_VICT );
        return false;
    }
	    
    if ( socialName != "bow" ) {
	if ( victim != ch ) {
		return false;
	}
	else  {
        	act( "$c1 с отвращением смотрит на ужимки $C2.", ch, 0, actor, TO_NOTVICT );
        	act( "$c1 с отвращением смотрит на твои ужимки.", ch, 0, actor, TO_VICT );
        	say_act( actor, ch, "Тебе нужно {hc{yпоклониться{x своему мастеру, $c1." );	    
        	return false;		
	}
    }
    else {
    	if (!victim || victim != ch) {
        	act( "$c1 смотрит на $C4 как на полн{Smого{Sfую{Sx идиот{Smа{Sfку{Sx.", ch, 0, actor, TO_NOTVICT );
        	act( "$c1 смотрит на тебя как на полн{Smого{Sfую{Sx идиот{Smа{Sfку{Sx.", ch, 0, actor, TO_VICT );
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

    act( "$C1 делится секретом бессмертия с $c5.", actor, 0, ch, TO_ROOM );
    act( "$C1 делится с тобой секретом бессмертия.", actor, 0, ch, TO_CHAR );
    act_p( "{BМолнии сверкают на небе.{x", actor, 0, ch, TO_ALL, POS_SLEEPING );
    return true;
}

