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
};

VampiricBiteOneHit::VampiricBiteOneHit( Character *ch, Character *victim )
	    : Damage( ch, victim, 0, 0 ), OneHit( ch, victim ),
	      SkillDamage( ch, victim, gsn_vampiric_bite, 0, 0, DAMF_WEAPON )
{
}

void VampiricBiteOneHit::init( )
{
    dam_type = DAM_NEGATIVE;
    skill = 20 + ch->getSkill( sn );
}

void VampiricBiteOneHit::damBase( )
{
    int ave, level = ch->getModifyLevel( );
    
         if (level >= 100) ave = level - 12;
    else if (level >= 40)  ave = level - 10;
    else if (level >= 37)  ave = level -  9;
    else if (level >= 35)  ave = level -  8;
    else if (level >= 33)  ave = level -  7;
    else if (level >= 30)  ave = level -  6;
    else if (level >= 25)  ave = level -  3;
    else if (level >= 23)  ave = level -  2;
    else if (level >= 20)  ave = level -  1; 
    else                   ave = level;
    
    dam = ave * skill / 100;                   // as weapon with skill bonus

    if (number_percent( ) <= skill / 8)        // as sharpness
	dam = 2 * dam + (dam * 2 * number_percent( ) / 100); 
}


void VampiricBiteOneHit::calcDamage( )
{
    damBase( ); 
    gsn_enhanced_damage->getCommand( )->run( ch, victim, dam );;
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
    // vampiric bite gives hp to ch from victim
    int hit_ga = min( (orig_dam / 2 ), (int)victim->max_hit );

    ch->hit += hit_ga;
    ch->hit  = min( ch->hit , ch->max_hit);
    update_pos( ch );
    ch->send_to("Твое самочувствие улучшается, когда ты высасываешь кровь из противника.\n\r");
}

/*
 * 'control animal' skill command
 */

SKILL_RUNP( control )
{
  char arg[MAX_INPUT_LENGTH];
  Character *victim;
  int chance;

  argument = one_argument( argument, arg );

  if (ch->is_npc() || !gsn_control_animal->usable( ch ) )
    {
      ch->send_to( "Чего?\n\r");
      return;
    }

  if ( arg[0] == '\0' )
    {
      ch->send_to( "Кого очаровать?\n\r");
      return;
    }

  if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
      ch->send_to( "Нет этого тут.\n\r");
      return;
    }

  if (victim->getRace( )->isPC( ))
    {
      ch->send_to("Стоит попробовать на животных?\n\r");
      return;
    }

  if (IS_SET( victim->form, FORM_NONADOPTABLE )) {
    ch->send_to("Эта форма жизни не поддается контролю.\n\r");
    return;	
  }
    
  if (is_safe(ch,victim)) return;

  if (overcharmed(ch))  
      return;

  ch->setWaitViolence( 1 );

  chance = gsn_control_animal->getEffective( ch );

  chance += (ch->getCurrStat(STAT_CHA) - 20) * 5;
  chance += ( ch->getModifyLevel() - victim->getModifyLevel() ) * 3;
  chance +=
   (ch->getCurrStat(STAT_INT) - victim->getCurrStat(STAT_INT)) * 5;

 if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||	 number_percent() > chance
    ||   ch->getModifyLevel() < ( victim->getModifyLevel() + 2 )
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||	 (victim->is_npc( ) 
	    && victim->getNPC( )->behavior
	    && IS_SET(victim->getNPC( )->behavior->getOccupation( ), (1 << OCC_SHOPPER))))
	{
	 gsn_control_animal->improve( ch, false, victim );
	 do_say(victim,"Я не собираюсь следовать за тобой!");
	 interpret_raw(victim, "murder", ch->getNameP( ));
	 return;
	}

  gsn_control_animal->improve( ch, true, victim );

  if ( victim->master )
	victim->stop_follower( );
  SET_BIT(victim->affected_by,AFF_CHARM);

	if ( victim->is_npc( )
		&& victim->in_room )
	{
		save_mobs( victim->in_room );
	}
		
  victim->master = victim->leader = ch;

  act_p( "Ну разве $c1 не прелесть?", ch, 0, victim, TO_VICT,POS_RESTING );
  if ( ch != victim )
	act_p("$C1 смотрит на тебя влюбленными глазами.",ch,0,victim,TO_CHAR,POS_RESTING);

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
	ch->send_to("Ты не умеешь копать!\r\n");
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

    if (room->sector_type == SECT_CITY || room->sector_type == SECT_INSIDE) {
	ch->send_to("Здесь слишком твердая почва..\r\n");
	return;
    }
    
    if (IS_SET(room->room_flags, ROOM_PRIVATE|ROOM_SOLITARY|ROOM_LAW|ROOM_SAFE))
    {
	ch->send_to("Здесь неподходящее место для копания могилы.\r\n");
	return;
    }
    
    if (get_obj_room_vnum( room, OBJ_VNUM_GRAVE )) {
	ch->send_to("Опс.. похоже, этот участок уже занял твой коллега.\r\n");
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
		ch->send_to("Ты не можешь стать еще более вампиром!\n\r");
		return;
	}

	if ( !ch->is_npc() && !gsn_vampire->usable( ch ) )
	{
		ch->send_to("Ты пытаешься выглядеть более уродливо.\n\r");
		return;
	}

        if (gsn_vampire->getLearned( ch ) < 100)
	{
		ch->send_to("Попроси гильдмастера помочь тебе.\n\r");
		return;
	}

	if ( weather_info.sunlight == SUN_LIGHT
		|| weather_info.sunlight == SUN_RISE )
	{
		ch->send_to("Тебе нужно дождаться вечера, чтобы превратиться в вампира.\n\r");
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

	ch->send_to("Превращаясь в кровожадного вампира, ты чувствуешь прилив силы.\r\n");
	act_p("$c1 неуловимо меняется, превращаясь в нечто ужасное!",ch,0,0,TO_ROOM,POS_RESTING);
}

void sucking( Character *ch, Character *victim ) 
{
    int cond, hp_gain;

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
    
    UNSET_DEATH_TIME(ch);
    ch->setWait( gsn_vampiric_bite->getBeats( )  );
                     
    act_p("Сквозь кошмарный сон ты чувствуешь, как $c1 высасывает твою {rкровь{x.", ch, 0, victim, TO_VICT, POS_DEAD);
    act_p("Ты высасываешь {rкровь{x из шеи $C2.", ch, 0, victim, TO_CHAR, POS_RESTING);
    act_p("$c1 высасывает {rкровь{x из шеи $C2.", ch, 0, victim, TO_NOTVICT, POS_RESTING);
    
    if (!ch->is_npc( )) {
	desire_bloodlust->gain( ch->getPC( ), 20 );
	cond = ch->getPC( )->desires[desire_bloodlust];
    } 
    else 
	cond = number_range( -10, 60 );

    hp_gain = std::min( ch->getModifyLevel( ) * 5, (int)victim->max_hit );
    ch->hit += hp_gain;
    ch->hit = std::min( ch->hit , ch->max_hit );
    update_pos( ch );
    
    victim->position = POS_STANDING;
    
    try {
	RawDamage( ch, victim, DAM_OTHER, hp_gain ).hit( true );

	victim->position = POS_SLEEPING;
			       
	if (number_percent( ) < cond) {
	    set_fighting( victim, ch );
	    act_p("$c1 очнул$gось|ся|ась от терзавшего $s кошмара.", victim, 0, ch, TO_ROOM, POS_RESTING);
	    act_p("Твой кошмар проходит - ты просыпаешься.", victim, 0, ch, TO_CHAR, POS_DEAD);
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
	ch->send_to("Хрум, хрум!\n\r");
	return;
    }

    if (!IS_VAMPIRE(ch) && !IS_MOB_VAMPIRE(ch)) {
	 ch->send_to("Высасывать кровь можно, только превратившись в вампира.\n\r");
	 return;
    }

    if (arg[0] == '\0') {
	ch->send_to("Пить кровь из кого?\n\r");
	return;
    }

    if ((victim = get_char_room( ch, arg )) == 0) {
	ch->send_to("Этого нет здесь.\n\r");
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

    one_argument( argument, arg );

    if (!gsn_vampiric_bite->usable( ch )) {
	ch->send_to("Ты не умеешь кусаться.\n\r");
	return;
    }

    if (!IS_VAMPIRE(ch) && !IS_MOB_VAMPIRE(ch)) {
	ch->send_to("Чтоб укусить ты должен сначала превратиться в вампира.\n\r");
	return;
    }

    if ( arg[0] == '\0' )
    {
	ch->send_to("Укусить кого?\n\r");
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
	ch->send_to("Этого нет здесь.\n\r");
	return;
    }

    if ( victim == ch )
    {
	ch->send_to("Поцелуй себя в локоть.\n\r");
	return;
    }

    if ( victim->position != POS_SLEEPING )
    {
	ch->send_to("Сначала он должен уснуть.\n\r");
	return;
    }

    if (victim->isAffected(gsn_vampiric_bite )) {
	ch->send_to("Отсюда уже можно пить.\r\n");
	return;
    }
    
    if ( is_safe( ch, victim ) )
      return;

    if ( victim->fighting != 0 )
    {
	ch->send_to("Ты не можешь укусить того, кто сражается.\n\r");
	return;
    }

    UNSET_DEATH_TIME(ch);
    ch->setWait( gsn_vampiric_bite->getBeats( )  );

    if ( victim->hit < (0.8 * victim->max_hit) &&
	 (IS_AWAKE(victim) ) )
    {
	act_p( "$C1 ран%Gно|ен|на и подозрител%Gьно|ен|ьна... не стоит даже пытаться.", ch, 0, victim, TO_CHAR,POS_RESTING);
	return;
    }

    if (victim->getLastFightDelay( ) < 300 && IS_AWAKE(victim) )
    {
	act_p( "$C1 настороже, не стоит даже пробовать.",ch, 0, victim, TO_CHAR,POS_RESTING);
	return;
    }
    
    VampiricBiteOneHit vb( ch, victim );
    
    try {
	if ( !IS_AWAKE(victim)
	    && (number_percent( ) < gsn_vampiric_bite->getEffective( ch )))
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
		
		af.duration = ch->getModifyLevel( ) / 50;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.where = TO_AFFECTS;
		af.bitvector = AFF_CORRUPTION;
		affect_join( victim, &af );

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
    int chance;
    Character *victim;
    Affect af;

    if (!gsn_vampiric_touch->usable( ch ))
    {
	ch->send_to("Ты не владеешь этим!\n\r");
	return;
    }

    if (!IS_VAMPIRE(ch) && !IS_MOB_VAMPIRE(ch))
    {
	ch->send_to("Ок, ок.\n\r");
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	ch->send_to("Ты же не хочешь высосать своего хозяина?\n\r");
	return;
    }

    if ( (victim = get_char_room(ch,argument)) == 0 )
    {
	ch->send_to("Здесь таких нет.\n\r");
	return;
    }

    if ( ch == victim )
    {
	ch->send_to("Что-что ты хочешь сотворить с собой?\n\r");
	return;
    }

    if ( victim->isAffected(gsn_vampiric_touch) )
    {
	ch->send_to("Твоя жертва еще не отошла от прикосновения.\n\r");
	return;
    }

    if ( is_safe(ch,victim) )
    {
	ch->send_to("Боги защищают твою жертву.\n\r");
	return;
    }

    if (gsn_rear_kick->getCommand( )->run( ch, victim ))
	return;

    int k = victim->getLastFightDelay( );

    if ( k >= 0 && k < FIGHT_DELAY_TIME )
	    k = k * 100 /	FIGHT_DELAY_TIME;
    else
	    k = 100;

    UNSET_DEATH_TIME(ch);
    victim->setLastFightTime( );
    ch->setLastFightTime( );

    ch->setWait( gsn_vampiric_touch->getBeats( ) );

    chance = 17 * gsn_vampiric_touch->getEffective( ch ) / 20;
    chance = chance * k / 100;
    
    if (victim->isAffected(gsn_backguard)) 
	chance /= 2;

    if (number_percent() < chance)
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

	set_backguard( victim );

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
 * 'human' command
 */

SKILL_RUNP( human )
{
    if (ch->getProfession( ) != prof_vampire)
    {
     ch->send_to("Что?\n\r");
     return;
    }

    if ( !IS_VAMPIRE(ch) )
    {
     ch->send_to("Ты и есть человек.\n\r");
     return;
    }

   affect_strip(ch, gsn_vampire);
   REMOVE_BIT(ch->act,PLR_VAMPIRE);
   ch->send_to("Ты принимаешь человеческий облик.\n\r");
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
	ch->send_to("Тебя возбудит только вид человеческой крови.\r\n");
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
class BonedaggerOneHit: public WeaponOneHit, public SkillDamage {
public:
    BonedaggerOneHit( Character *ch, Character *victim );

    virtual void calcTHAC0( );
    virtual void calcDamage( );
};


BonedaggerOneHit::BonedaggerOneHit( Character *ch, Character *victim )
	    : Damage( ch, victim, 0, 0 ), WeaponOneHit( ch, victim, false ),
	      SkillDamage( ch, victim, gsn_bonedagger, 0, 0, DAMF_WEAPON )
{
}
void BonedaggerOneHit::calcDamage( ) 
{
    damBase( );
    gsn_enhanced_damage->getCommand( )->run( ch, victim, dam );;
    damApplyPosition( );
    damApplyDamroll( );
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
	    ch->printf("Ты ждешь, пока некто '%s' отбросит тень на твою могилу.\n\r", ch->ambushing);
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
      ch->send_to("Что?\n\r");
      return;
    }

  if (ch->isAffected(gsn_sense_life))
    {
      ch->send_to("Ты уже можешь чувствовать присутствие живых организмов.\n\r");
      return;
    }

    mana = gsn_sense_life->getMana( );
    
  if (ch->mana < mana)
    {
      ch->send_to("У тебя не хватает энергии для этого.\n\r");
      return;
    }

  ch->setWait( gsn_sense_life->getBeats( )  );

    if(SHADOW(ch)) {
      ch->send_to("Кроме своей тени тебе не удается ничего почувствовать.\n\r");
      return;
    }

  if (!ch->is_npc() && number_percent() < gsn_sense_life->getEffective( ch ))
    {
      Affect af;

      af.where  = TO_DETECTS;
      af.type 	= gsn_sense_life;
      af.level 	= ch->getModifyLevel();
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

SPELL_DECL(BatSworm);
VOID_SPELL(BatSworm)::run( Character *ch, Character *, int sn, int level ) 
{ 
    Affect af;

    if (!ch->fighting) {
	ch->send_to("Сейчас ты не сражаешься!\r\n");
	return;
    }

    if (!IS_VAMPIRE(ch) && !IS_MOB_VAMPIRE(ch)) {
	ch->send_to("Для этого тебе необходимо превратиться в вампира.\r\n");
	return;
    }

    if (ch->isAffected(sn)) {
	ch->send_to("Две стаи летучих мышей - это слишком.\r\n");
	return;
    }

    act_p("В воздухе внезапно раздается шелест крыльев и едва различимый писк.", ch, 0, 0, TO_ALL, POS_RESTING);
    act_p("На зов $c2 слетается стая летучих мышей и окружает $s живым облаком.", ch, 0, 0, TO_ROOM, POS_RESTING);
    act_p("Стая летучих мышей прибывает по твоему зову и окружает тебя живым облаком.", ch, 0, 0, TO_CHAR, POS_RESTING);

    af.where	    = TO_AFFECTS;
    af.type	    = sn;
    af.level	    = level;
    af.duration	    = 1 + level / 10;
    af.bitvector    = 0;
    af.modifier	    = 0;
    af.location	    = APPLY_NONE;
    affect_to_char(ch, &af);
}


/*---------------------------------------------------------------------------
 * VampireGuildmaster
 *--------------------------------------------------------------------------*/

bool VampireGuildmaster::social( Character *actor, Character *victim, const DLString &socialName )
{
    if (victim != ch || actor == ch)
	return false;

    if (socialName != "bow")
	return false;

    if (actor->is_npc( ) || actor->getProfession( ) != prof_vampire) {
	act( "$c1 одаривает $C4 равнодушным холодным взглядом.", ch, 0, actor, TO_NOTVICT );
	act( "$c1 одаривает тебя равнодушным холодным взглядом.", ch, 0, actor, TO_VICT );
	return true;
    }
    
    PCharacter *pActor = actor->getPC( );
    PCSkillData &data = pActor->getSkillData( gsn_vampire );

    if (data.learned == 100) {
	say_act( actor, ch, "Ты уже ста$gло|л|ла одн$gим|им|ой из нас, $c1." );
	return true;
    }
    
    if (pActor->questpoints < 50) {
	say_act( actor, ch, "Я потребую с тебя 50 qp, но я вижу, что ты не можешь заплатить такую цену." );
	return true;
    }

    pActor->questpoints -= 50;
    data.learned = 100;

    act( "$C1 делится секретом бессмертия с $c5.", actor, 0, ch, TO_ROOM );
    act( "$C1 делится с тобой секретом бессмертия.", actor, 0, ch, TO_CHAR );
    act_p( "{BМолнии сверкают на небе.{x", actor, 0, ch, TO_ALL, POS_SLEEPING );
    return true;
}

