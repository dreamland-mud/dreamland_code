
/* $Id: group_weaponsmaster.cpp,v 1.1.2.21.6.12 2009/09/11 11:24:54 rufina Exp $
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

#include "objthrow.h"
#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "affect.h"
#include "pcharacter.h"
#include "race.h"
#include "npcharacter.h"
#include "object.h"
#include "gsn_plugin.h"
#include "act_move.h"
#include "mercdb.h"

#include "magic.h"
#include "damage.h"
#include "material.h"
#include "fight.h"
#include "vnum.h"
#include "stats_apply.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "interp.h"
#include "def.h"


/*
 * 'second wield' skill command
 */

SKILL_RUNP( second )
{
    Object *obj;

    if (argument[0] == '\0') {
	ch->println( "Вооружиться чем?" );
	return;
    }

    if (( obj = get_obj_carry (ch, argument) ) == 0) {
	ch->println( "У тебя нет этого." );
	return;
    }

    if (!obj->can_wear( ITEM_WIELD )) {
	ch->println( "Ты не можешь вооружиться этим как вторичным оружием." );
	return;
    }
    
    wear_second_wield->wear( obj, F_WEAR_REPLACE | F_WEAR_VERBOSE );
}

/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( Character *ch, Character *victim ,int disarm_second)
{
	Object *obj;
	Object *obj2;
	int skill;

	if (disarm_second)
	{
		if ((obj=get_eq_char(victim,wear_second_wield)) == 0)
		{
			bug("Disarm second with 0 wear_second_wield",0);
			return;
		}
	}
	else
	{
		if ((obj=get_eq_char(victim,wear_wield)) == 0)
		{
			bug("Disarm first with 0 wear_wield",0);
			return;
		}
	}

	if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	{
		act_p("$S оружие не двигается с места!",ch,0,victim,TO_CHAR,POS_RESTING);
		act_p("$c1 пытается обезоружить тебя, но оружие не двигается с места!",
			ch,0,victim,TO_VICT,POS_RESTING);
		act_p("$c1 пытается обезоружить $C4, но оружие не двигается с места.",
			ch,0,victim,TO_NOTVICT,POS_RESTING);
		return;
	}

	if (( skill = gsn_grip->getEffective( victim ) ) > 1)
	{
		skill += (victim->getCurrStat(STAT_STR) - ch->getCurrStat(STAT_STR)) * 5;

		if ( number_percent() < skill )
		{
			act_p("$C1 хватает тебя за руку и ускользает!",
				ch,0,victim,TO_CHAR,POS_RESTING);
			act_p("$c1 пытается обезоружить тебя, но ты хватаешь $s за руку и ускользаешь!",
				ch,0,victim,TO_VICT,POS_RESTING);
			act_p("$c1 пытается обезоружить $C4, но безуспешно.",
				ch,0,victim,TO_NOTVICT,POS_RESTING);
			gsn_grip->improve( victim, true, ch );
			return;
		}
		else 	
			gsn_grip->improve( victim, false, ch );
	}

	act_p("Ты обезоруживаешь $C4!", ch,0, victim, TO_CHAR,POS_FIGHTING);
	act_p( "$c1 обезоруживает $C4!",ch, 0, victim,TO_NOTVICT,POS_FIGHTING);

	obj_from_char( obj );

	if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) ) 
	{
	   	act_p("$c1 {rВЫБИ$gЛО{x|Л{x|ЛА{x у тебя оружие!", ch, 0, victim, TO_VICT ,POS_FIGHTING);
		obj_to_char( obj, victim );
	}
	else
	{
	   	act_p("$c1 {rВЫБИ$gЛО{x|Л{x|ЛА{x у тебя оружие, и оно упало на землю!", ch, 0, victim, TO_VICT ,POS_FIGHTING);
		obj_to_room( obj, victim->in_room );
		if (victim->is_npc() && victim->wait == 0 && victim->can_see(obj))
			do_get_raw(victim, obj);
	}

	if ( (obj2 = get_eq_char(victim, wear_second_wield)) != 0)
	{
		act_p( "Ты вооружаешься вторичным оружием как основным!",
			ch, 0, victim,TO_VICT,POS_FIGHTING);
		act_p( "$C1 вооружается вторичным оружием как основным!",
			ch, 0,victim,TO_CHAR ,POS_FIGHTING);
		act_p( "$C1 вооружается вторичным оружием как основным!",
			ch, 0, victim,TO_NOTVICT ,POS_FIGHTING);
		unequip_char( victim, obj2);
		equip_char( victim, obj2 , wear_wield);
	}

	return;
}

/*
 * 'disarm' skill command
 */

SKILL_RUNP( disarm )
{
	Character *victim;
	Object *obj;
	int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon,disarm_second=0;
	char arg[MAX_INPUT_LENGTH];

	if ( MOUNTED(ch) )
	{
		ch->send_to("Ты не сможешь обезоружить, если ты верхом!\n\r");
		return;
	}

	hth = 0;

	if ( (chance = gsn_disarm->getEffective( ch )) <= 1)
	{
		ch->send_to("Ты не знаешь как обезоружить противника.\n\r");
		return;
	}

	if ( !ch->is_npc() && !ch->move )
	{
		act("Ты слишком уста$gло|л|ла для этого.", ch, 0, 0, TO_CHAR);
		return;
	}
	else
		ch->move -= move_dec( ch );

	if (SHADOW(ch))
	{
		ch->send_to("Да... Как глупо пытаться разоружить свою тень.\n\r");
		act_p("$c1 пытается выбить у своей тени оружие.\n...как глупо это выглядит.",
			ch, 0, 0, TO_ROOM,POS_RESTING);
		return;
	}

	if ( get_eq_char( ch, wear_wield ) == 0
		&& ( (hth = gsn_hand_to_hand->getEffective( ch )) == 0
			|| ( ch->is_npc() && !IS_SET(ch->getNPC()->off_flags,OFF_DISARM))))
	{
		ch->send_to("Для этого нужно сначала вооружиться.\n\r");
		return;
	}

	if ( ( victim = ch->fighting ) == 0 )
	{
		ch->send_to("Сейчас ты не сражаешься.\n\r");
		return;
	}

	if ( ( obj = get_eq_char( victim, wear_wield ) ) == 0 )
	{
		ch->send_to("Твой противник не вооружен.\n\r");
		return;
	}

	argument = one_argument(argument,arg);
	if ( !ch->is_npc() && arg[0] != '\0' )
	{
		if (is_name(arg,"second") )
			disarm_second = 1;
		else
			disarm_second = 0;
	}

	/* find weapon skills */
	ch_weapon = ch->getSkill(get_weapon_sn(ch, false));

	vict_weapon = victim->getSkill(get_weapon_sn(victim, disarm_second));
	ch_vict_weapon = ch->getSkill(get_weapon_sn(victim, disarm_second));

	/* modifiers */

	/* skill */
	if ( get_eq_char(ch,wear_wield) == 0)
		chance = chance * hth/150;
	else
		chance = chance * ch_weapon/100;

	chance += (ch_vict_weapon/2 - vict_weapon) / 2;

	/* dex vs. strength */
	chance += ch->getCurrStat(STAT_DEX);
	chance -= 2 * victim->getCurrStat(STAT_STR);

	/* level */
	chance += ( ch->getModifyLevel() - victim->getModifyLevel() ) * 2;

	/* and now the attack */
	if (number_percent() < chance)
	{
		ch->setWait( gsn_disarm->getBeats( )  );
		disarm( ch, victim ,disarm_second);
		gsn_disarm->improve( ch, true, victim );
	}
	else
	{
		ch->setWait( gsn_disarm->getBeats( ) );
		act_p("Тебе не удалось обезоружить $C4.",
			ch,0,victim,TO_CHAR,POS_RESTING);
		act_p("$c1 пытается обезоружить тебя, но не может.",
			ch,0,victim,TO_VICT,POS_RESTING);
		act_p("$c1 пытается обезоружить $C4, но не может.",
			ch,0,victim,TO_NOTVICT,POS_RESTING);
		gsn_disarm->improve( ch, false, victim );
	}

	return;
}


/*
 * 'shield cleave' skill command
 */

SKILL_RUNP( shield )
{
    Character *victim;
    int chance,ch_weapon,vict_shield;
    Object *shield,*axe;

    if ( ( victim = ch->fighting ) == 0 )
    {
	ch->send_to("Сейчас ты не сражаешься.\n\r");
	return;
    }
	
    if ((axe = get_eq_char(ch,wear_wield)) == 0)
    {
	ch->send_to( "Сначала нужно вооружиться.\n\r");
	return;
    }

    if ((chance = gsn_shield_cleave->getEffective( ch )) == 0)
    {
	ch->send_to("Ты не знаешь как расколоть щит противника.\n\r");
	return;
    }

    if ( ( shield = get_eq_char( victim, wear_shield )) == 0 )
    {
	ch->send_to("Твой противник не использует щит.\n\r");
	return;
    }

    if (material_is_flagged( shield, MAT_INDESTR ) || shield->pIndexData->limit != -1)
	return;

    if (axe->value[0] == WEAPON_AXE )
	chance = ( int )( chance * 1.2 );
    else if (axe->value[0] != WEAPON_SWORD)
	{
	 ch->send_to("Для этого ты должен вооружиться топором или мечом.\n\r");
	 return;
	}

    /* find weapon skills */
    ch_weapon = ch->getSkill(get_weapon_sn(ch, false));
    vict_shield = std::max(1,gsn_shield_block->getEffective( ch ));
    /* modifiers */

    /* skill */
   chance = chance * ch_weapon / 200;
   chance = chance * 100 / vict_shield;

    /* dex vs. strength */
    chance += ch->getCurrStat(STAT_DEX);
    chance -= 2 * victim->getCurrStat(STAT_STR);

    /* level */
    chance += ( ch->getModifyLevel() - victim->getModifyLevel() ) * 2;
/*    chance += ch->getRealLevel( ) - victim->getRealLevel( );*/
    chance += axe->level - shield->level;

    /* and now the attack */
    SET_BIT(ch->affected_by,AFF_WEAK_STUN);

    if (number_percent() < chance)
    {
    	ch->setWait( gsn_shield_cleave->getBeats( )  );
	act_p("Ты раскалываешь щит $C2 надвое.",
               ch,0,victim,TO_CHAR,POS_RESTING);
	act_p("$c1 раскалывает твой щит надвое.",
               ch,0,victim,TO_VICT,POS_RESTING);
	act_p("$c1 раскалывает щит $C2 надвое.",
               ch,0,victim,TO_NOTVICT,POS_RESTING);
	gsn_shield_cleave->improve( ch, true, victim );
	extract_obj( get_eq_char(victim,wear_shield) );
    }
    else
    {
	ch->setWait( gsn_shield_cleave->getBeats( ) );
	act_p("Ты пытаешься расколоть щит $C2, но не выходит.",
               ch,0,victim,TO_CHAR,POS_RESTING);
	act_p("$c1 пытается расколоть твой щит, но не выходит.",
               ch,0,victim,TO_VICT,POS_RESTING);
	act_p("$c1 пытается расколоть щит $C2, но не выходит.",
               ch,0,victim,TO_NOTVICT,POS_RESTING);
	gsn_shield_cleave->improve( ch, false, victim );
    }
    return;
}

/*
 * 'weapon cleave' skill command
 */

SKILL_RUNP( weapon )
{
    Character *victim;
    Object *wield,*axe;
    int chance,ch_weapon,vict_weapon;

    if ( ( victim = ch->fighting ) == 0 )
    {
	ch->send_to("Сейчас ты не сражаешься.\n\r");
	return;
    }

    if ( (axe = get_eq_char(ch,wear_wield)) == 0)
    {
	ch->send_to( "Сначала тебе нужно вооружитья.\n\r");
	return;
    }

    if ((chance = gsn_weapon_cleave->getEffective( ch )) == 0)
    {
	ch->send_to("Ты не знаешь как раскалывают оружие противника.\n\r");
	return;
    }

    if ( (wield = get_eq_char( victim, wear_wield )) == 0 )
    {
	ch->send_to("Твой противник должен быть вооружен.\n\r");
	return;
    }

    if (material_is_flagged( wield, MAT_INDESTR ) || wield->pIndexData->limit != -1 )
	return;


    if (axe->value[0] == WEAPON_AXE )
	chance = ( int )( chance * 1.2 );
    else if (axe->value[0] != WEAPON_SWORD)
	{
	 ch->send_to("Для этого тебе нужно вооружиться топором или мечом.\n\r");
	 return;
	}

    /* find weapon skills */
    ch_weapon = ch->getSkill(get_weapon_sn(ch, false));
    vict_weapon = std::max(1,victim->getSkill(get_weapon_sn(victim, false)));
    /* modifiers */

    /* skill */
    chance = chance * ch_weapon / 200;
    chance = chance * 100 / vict_weapon;

    /* dex vs. strength */
    chance += ch->getCurrStat(STAT_DEX) + ch->getCurrStat(STAT_STR);
    chance -= victim->getCurrStat(STAT_STR) +
			2 * victim->getCurrStat(STAT_DEX);

    chance += ( ch->getModifyLevel() - victim->getModifyLevel() ) * 2;
    chance += axe->level - wield->level;

    /* and now the attack */
    SET_BIT(ch->affected_by,AFF_WEAK_STUN);
    if (number_percent() < chance)
    {
    	ch->setWait( gsn_weapon_cleave->getBeats( )  );
	act_p("Ты раскалываешь оружие $C2 надвое.",
               ch,0,victim,TO_CHAR,POS_RESTING);
	act_p("$c1 раскалывает твое оружие надвое.",
               ch,0,victim,TO_VICT,POS_RESTING);
	act_p("$c1 раскалывает оружие $C2 надвое.",
               ch,0,victim,TO_NOTVICT,POS_RESTING);
	gsn_weapon_cleave->improve( ch, true, victim );
	extract_obj( get_eq_char(victim,wear_wield) );
    }
    else
    {
	ch->setWait( gsn_weapon_cleave->getBeats( ) );
	act_p("Ты пытаешься расколоть оружие $C2, но не выходит.",
               ch,0,victim,TO_CHAR,POS_RESTING);
	act_p("$c1 пытается расколоть твое оружие, но у н$s не выходит.",
               ch,0,victim,TO_VICT,POS_RESTING);
	act_p("$c1 пытается расколоть оружие $C2, но у н$s не выходит.",
               ch,0,victim,TO_NOTVICT,POS_RESTING);
	gsn_weapon_cleave->improve( ch, false, victim );
    }
    return;
}



/*
 * 'lash' skill command
 */

SKILL_RUNP( lash )
{
    char arg[MAX_INPUT_LENGTH];
    Object *whip;
    Character *victim;
    bool wasFighting;
    int chance;
    
    if (!gsn_lash->usable( ch )) {
	act("$c1 угрощающе щелкает хлыстом.", ch, 0, 0, TO_ROOM);
	ch->send_to( "Что?\r\n" );
	return;
    }
    
    one_argument(argument, arg);
    
    whip = get_eq_char(ch, wear_wield);
    if (!whip || whip->item_type != ITEM_WEAPON || whip->value[0] != WEAPON_WHIP)
	whip = get_eq_char(ch, wear_second_wield);
    if (!whip || whip->item_type != ITEM_WEAPON || whip->value[0] != WEAPON_WHIP) 
    {
	ch->send_to( "Возьми в руки хлыст.\n\r" );
	return;
    }

    if (arg[0] == '\0') {
	victim = ch->fighting;
	if (victim == NULL) {
	    ch->send_to( "Но ты ни с кем не сражаешься!\n\r" );
	    return;
	}
    }
    else if ((victim = get_char_room(ch, arg)) == NULL) {
	ch->send_to("Tут таких нет.\n\r");
	return;
    }
    
    chance = gsn_lash->getEffective( ch );

    if (victim == ch || chance < 50) {
	ch->send_to("Ты запутываешься в хлысте и падаешь!\n\r");
	ch->setWaitViolence( 5 );
	act("$c1 старательно опутывает свои ноги хлыстом и падает на землю.", ch, 0, 0, TO_ROOM);
	return;
    }

    if (is_safe(ch, victim))
	return;

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
	act("Но $C1 твой друг!", ch, NULL, victim, TO_CHAR);
	return;
    }

    if (SHADOW(ch)) {
	ch->send_to("Ты пытаешься огреть хлыстом собственную тень.\n\r");
	act_p("$c1 бьет свою тень хлыстом.",ch,0,0,TO_ROOM,POS_RESTING);
	return;
    }

    wasFighting = (ch->fighting != NULL);
    
    chance += ch->getCurrStat(STAT_DEX) * 2;
    chance -= victim->getCurrStat(STAT_DEX) * 2;
    chance += ch->getCurrStat(STAT_STR);
    chance -= victim->getCurrStat(STAT_STR);

    if (IS_AFFECTED(ch, AFF_HASTE))
	chance += 20;
    if (IS_AFFECTED(victim, AFF_HASTE))
	chance -= 20;
    if (IS_AFFECTED(ch, AFF_SLOW))
	chance -= 10;
    if (IS_AFFECTED(victim, AFF_SLOW))
	chance += 10;

    chance += (ch->getModifyLevel( ) - victim->getModifyLevel( )) * 2;
    chance += gsn_whip->getEffective( ch ) - 100;
    chance = chance * 4 / 5;

    if (number_percent( ) < chance) {
	int dam;

	gsn_lash->improve( ch, true, victim );
	ch->setWaitViolence( 1 );
	victim->setWait( gsn_lash->getBeats( ) );

	dam = ch->damroll;
	dam += number_range(4, 4 + 4 * ch->getCurrStat(STAT_STR) + chance / 10);
	chance /= 2;

	if (ch->size < victim->size)
	    chance += (ch->size - victim->size) * 10; 
	
	chance += (ch->getCurrStat(STAT_STR) - victim->getCurrStat(STAT_STR)) * 4;
	
	if (is_flying( victim )) 
	    chance = 0;
	
	try {
	    if (damage_nocatch(ch,victim,dam,gsn_lash, DAM_BASH, true, DAMF_WEAPON)
		&& number_percent( ) < chance) 
	    {
		act("$c1 подсекает тебя своим хлыстом!!", ch, NULL, victim, TO_VICT);
		act("Ты подсекаешь $C4 своим хлыстом!", ch, NULL, victim, TO_CHAR);
		act("$c1 подсекает $C4 своим хлыстом.", ch, NULL, victim, TO_NOTVICT);
		
		victim->setWaitViolence( number_range( 0, 2 ) );
		victim->position = POS_RESTING;
	    }
	} catch (const VictimDeathException &) {
	}
    }
    else {
	damage(ch,victim,2,gsn_lash,DAM_BASH, false, DAMF_WEAPON);
	act("Ты лишь оцарапа$gло|л|ла $C4.", ch, NULL, victim, TO_CHAR);
	act("$c1 взмахом хлыста поцарапал $C4!", ch, NULL, victim, TO_NOTVICT);
	act("Ты уклоняешься от хлыста $c2.", ch, NULL, victim, TO_VICT);
	gsn_lash->improve( ch, false, victim );
	ch->setWaitViolence( 1 );
    }
    
    if (!wasFighting)
	yell_panic( ch, victim,
	            "Помогите! Кто-то пытается огреть меня хлыстом!",
		    "Помогите! %1$^C1 бичует меня!" );
}

/*
 * 'throw spear' skill command
 */

SKILL_RUNP( throwspear )
{
	Character *victim;
	Object *spear;
	char arg1[512],arg2[512];
	bool success;
	int chance,direction;
	int range = ( ch->getModifyLevel() / 10) + 1;

	if ( ch->is_npc() )
		return; /* Mobs can't shoot spears */

	if ( !gsn_spear->usable( ch ) )
	{
		ch->send_to("Ты не знаешь как метать копье.\n\r");
		return;
	}

	argument=one_argument( argument, arg1 );
	one_argument( argument, arg2 );

	if ( arg1[0] == '\0' || arg2[0] == '\0')
	{
		ch->send_to("Метнуть копье куда и в кого?\n\r");
		return;
	}

	if ( ch->fighting )
	{
		ch->send_to("Ты не можешь сконцентрироваться для метания копья.\n\r");
		return;
	}

	direction = direction_lookup( arg1 );

	if (direction < 0)
	{
		ch->send_to("Метнуть копье куда и в кого?\n\r");
		return;
	}

	if ( ( victim = find_char( ch, arg2, direction, &range) ) == 0 )
		return;
/*
	if ( !victim->is_npc() && victim->desc == 0 )
	{
		ch->send_to("Ты не можешь сделать этого.\n\r");
		return;
	}
*/
	if ( victim == ch )
	{
		ch->send_to("Это бессмысленно.\n\r");
		return;
	}

	if ( is_safe_nomessage(ch,victim) )
	{
		ch->pecho("Боги покровительствуют %C3.", victim);
		return;
	}

	if ( ch->in_room == victim->in_room )
	{
		ch->send_to("Ты не можешь метнуть копье в упор.\n\r");
		return;
	}

	spear = get_eq_char(ch, wear_wield);

	if ( !spear
		|| spear->item_type != ITEM_WEAPON
		|| spear->value[0] != WEAPON_SPEAR )
	{
		ch->send_to("Для метания тебе необходимо копье!\n\r");
		return;    	
	}

	if ( get_eq_char(ch,wear_second_wield) || get_eq_char(ch,wear_shield) )
	{
		ch->send_to("Твоя вторая рука дожна быть свободна!\n\r");
		return;    	
	}

	ch->setWait(gsn_spear->getBeats( ) );

	chance = gsn_spear->getEffective( ch );

	if ( victim->position == POS_SLEEPING )
		chance += 40;
	if ( victim->position == POS_RESTING )
		chance += 10;
	if ( victim->position == POS_FIGHTING )
		chance -= 40;

	chance += ch->hitroll - ch->getRealLevel();

	act( "Ты метаешь $o4 $T.", ch, spear, dirs[ direction ].leave, TO_CHAR  );
	act( "$c1 метает $o4 $T.", ch, spear, dirs[ direction ].leave, TO_ROOM );

	set_violent( ch, victim, false );

	obj_from_char(spear);
	int dam;
	
	dam = dice(spear->value[1],spear->value[2]);
	dam += ch->damroll + get_str_app(ch).missile;
	dam /= 2;

	try {
	    success = send_arrow(ch,victim,spear,direction,chance,dam);
	} catch (const VictimDeathException &) {
	    victim = NULL;
	    success = true;
	}

	gsn_spear->improve( ch, success, victim );
}

