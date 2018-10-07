/* $Id$
 *
 * ruffina, 2004
 */
#include "onehit_weapon.h"

#include "skill.h"

#include "affect.h"
#include "race.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "object.h"
#include "room.h"
#include "clanreference.h"

#include "fight.h"
#include "handler.h"
#include "gsn_plugin.h"
#include "effects.h"
#include "magic.h"
#include "occupations.h"
#include "act.h"
#include "mercdb.h"
#include "merc.h"
#include "vnum.h"
#include "def.h"

WeaponOneHit::WeaponOneHit( Character *ch, Character *victiim, bool secondary )
		: Damage( ch, victim, 0, 0, DAMF_WEAPON ), 
		  OneHit( ch, victim )
{
    this->secondary = secondary;
    
    weapon_sn = -1;
    weaponSkill = NULL;
    wield = NULL;
    attack = 0;
}

void WeaponOneHit::init( )
{
    wield = get_wield( ch, secondary );

    if (wield && wield->item_type != ITEM_WEAPON)
	wield = NULL;

    weapon_sn = get_weapon_sn( wield );
    weaponSkill = skillManager->find( weapon_sn );
    skill = 20 + weaponSkill->getEffective( ch );
    
    if (wield) 
        attack = wield->value[3];
    else 
        attack = ch->dam_type;

    dam_type = attack_table[attack].damage;
    
    if (dam_type == -1)
	dam_type = DAM_BASH;
}

void WeaponOneHit::calcDamage( )
{
    OneHit::calcDamage( );

    if (wield)
	protectMaterial( wield );
}

/*-----------------------------------------------------------------------------
 * Calculate damage (base + bonuses)
 *----------------------------------------------------------------------------*/
void WeaponOneHit::damBase( )
{
    if (weaponSkill)
	weaponSkill->improve( ch, true, victim, dam_type, dam_flag );

    if (wield) {
	dam = dice(wield->value[1], wield->value[2]) * skill / 100;

	damApplyShield( );
	damApplySharp( );
    }
    else if (ch->is_npc( )) {
	NPCharacter *nch = ch->getNPC();

	dam = dice(nch->damage[DICE_NUMBER], nch->damage[DICE_TYPE]);
    }
    else
    {
	dam = number_range( 1 + 4 * skill/100, 
		            2 * ch->getModifyLevel( ) / 3 * skill/100 );
    }

    damApplyHoly( );
}

// no shield = more damage
void WeaponOneHit::damApplyShield( )
{
    if (get_eq_char(ch,wear_shield) == 0) 
	dam = dam * 21/20;
}

// sharpness!
void WeaponOneHit::damApplySharp( )
{
    if (wield && IS_WEAPON_STAT(wield, WEAPON_SHARP))
    {
	int percent = number_percent();
	if ( percent <= skill / 8 )
	    dam = 2 * dam + (dam * 2 * percent / 100);
    }
}

// holy weapon and 'holy attack'
void WeaponOneHit::damApplyHoly( )
{
    if (!IS_GOOD(ch) || !IS_EVIL(victim))
	return;

    if (wield && IS_WEAPON_STAT(wield,WEAPON_HOLY) && number_percent( ) < 30) 
    {
	msgWeaponVict( "{YСвященная аура %3$O2 поражает тебя.{x" );
	msgWeaponRoom( "{YСвященная аура %3$O2 поражает %2$C2.{x" );
	msgWeaponChar( "{YСвященная аура %3$O2 поражает %2$C2.{x" );
	dam += dam * 120 / 100;
	return;
    }
    
    if (number_percent( ) >= gsn_holy_attack->getEffective( ch )) 
	return;
    
    if (wield) {
	msgWeaponChar( "%3$^O4 в твоих руках на мгновение окутывает священная аура." );
	msgWeaponRoom( "%3$^O4 %1$C2 на мгновение окутывает священная аура." );
	msgWeaponVict( "%3$^O4 %1$C2 на мгновение окутывает священная аура." );
    }
    else {
	msgWeaponChar( "Твои руки наполняются священной силой." );
	msgWeaponRoom( "Руки %1$C2 наполняются священной силой." );
	msgWeaponVict( "Руки %1$C2 наполняются священной силой." );
    }
	
    gsn_holy_attack->improve( ch, true );
    dam += dam / 2;
}

void WeaponOneHit::damApplyCounter( )
{
    int chance;

    if (victim->fighting)
	return;
    
    if (IS_AFFECTED(victim, AFF_CHARM))
	return;
    
    if (victim->is_npc( ) 
	&& victim->getNPC( )->behavior 
	&& IS_SET(victim->getNPC( )->behavior->getOccupation( ), (1 << OCC_CLANGUARD)))
	return;

    if (is_safe_nomessage(victim, ch) || is_safe_nomessage(ch,victim))
	return;
	
    if (victim->position != POS_SITTING && victim->position != POS_STANDING)
	return;

    chance = number_percent();

    if (ch->is_adrenalined())
	chance += 25;
    
    chance *= 2;

    if (chance <= gsn_counter->getEffective( victim ))
    {
	gsn_counter->improve( victim, true, ch );
	act_p("$C1 направляет твой удар против тебя самого!",ch,0,victim,TO_CHAR,POS_RESTING);
	act_p("Ты направляешь удар $c2 против $x!",ch,0,victim,TO_VICT,POS_RESTING);
	act_p("$C1 возвращает удар $c2 обратно!",ch,0,victim,TO_NOTVICT,POS_RESTING);

	// set fighting state
	if (ch->fighting == NULL)
	    ch->fighting = victim ;
	if (victim->fighting == NULL)
	    victim->fighting = ch ;

	dam *= 2;
	killer = victim;
	victim = ch;
    }
    else 
	gsn_counter->improve( victim, false, ch );
}

/*----------------------------------------------------------------------------
 *  various damage effects
 *----------------------------------------------------------------------------*/
void WeaponOneHit::priorDamageEffects( )
{
}

void WeaponOneHit::postDamageEffects( )
{
    damEffectFunkyWeapon( );	
    damEffectFeeble( );
}

void WeaponOneHit::damEffectFunkyWeapon( )
{
    int dam;
    
    if (!wield)
	return;
	
    if (ch->fighting != victim)
	return;

    if (IS_WEAPON_STAT(wield,WEAPON_POISON))
    {
	short level;
	Affect *poison, af;

	if ( (poison = wield->affected->affect_find(gsn_poison)) == 0 )
	    level = wield->level;
	else
	    level = poison->level;
	if ( !saves_spell(level / 2,victim,DAM_POISON) )
	{
	    msgWeaponVict("Ты чувствуешь, как яд распространяется по твоим венам.");
	    msgWeaponRoom("%2$^C1 отравле%2$Gно|н|на ядом от %3$O2 %1$C2.");
	    msgWeaponChar("%2$^C1 отравле%2$Gно|н|на ядом от %3$O2.");

	    af.where     = TO_AFFECTS;
	    af.type      = gsn_poison;
	    af.level     = level * 3/4;
	    af.duration  = level / 2;
	    af.location  = APPLY_STR;
	    af.modifier  = -1;
	    af.bitvector = AFF_POISON;
	    affect_join( victim, &af );
	}

	// weaken the poison if it's temporary
	if ( poison != 0 )
	{
	    poison->level = max(0,poison->level - 2);
	    poison->duration = max(0,poison->duration - 1);
	    if ( poison->level == 0 || poison->duration == 0 )
		act_p("Яд с $o2 проходит.",ch,wield,0,TO_CHAR,POS_RESTING);
	}
    }

    if (IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC))
    {
	msgWeaponVict("Ты чувствуешь как %3$O1 вытягивает из тебя жизнь.");
	msgWeaponRoom("%3$^O1 %1$C2 вытягивает жизнь из %2$C2.");
	msgWeaponChar("%3$^O1 вытягивает жизнь из %2$C2.");

	dam = number_range(1, wield->level / 5 + 1);
	damage_nocatch(ch,victim,dam,0,DAM_NEGATIVE,false);
	ch->hit += dam/2;
    }
    if (IS_WEAPON_STAT(wield,WEAPON_FLAMING) )
    {
	msgWeaponVict("%3$^O1 обжигает тебя.");
	msgWeaponRoom("%3$^O1 %1$C2 обжигает %2$C4.");
	msgWeaponChar("%3$^O1 обжигает %2$C4.");

	dam = number_range(1,wield->level / 4 + 1);
	fire_effect( (void *) victim,wield->level/2,dam,TARGET_CHAR);
	damage_nocatch(ch,victim,dam,0,DAM_FIRE,false);
    }
    if (IS_WEAPON_STAT(wield,WEAPON_FROST) )
    {
	msgWeaponVict("Ледяное прикосновение %3$O2 обмораживает тебя, покрывая льдом.");
	msgWeaponRoom("%3$^O1 %1$C2 обмораживает %2$C4.");
	msgWeaponChar("%3$^O1 обмораживает %2$C4.");

	dam = number_range(1,wield->level / 6 + 2);
	cold_effect(victim,wield->level/2,dam,TARGET_CHAR);
	damage_nocatch(ch,victim,dam,0,DAM_COLD,false);
    }
    if (IS_WEAPON_STAT(wield,WEAPON_SHOCKING))
    {
	msgWeaponVict("Ты пораже%2$Gно|н|на разрядом %3$O2.");
	msgWeaponRoom("%2$^C1 пораже%2$Gно|н|на разрядом %3$O2 %1$C2.");
	msgWeaponChar("%2$^C1 пораже%2$Gно|н|на разрядом %3$O2.");

	dam = number_range(1,wield->level/5 + 2);
	shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
	damage_nocatch(ch,victim,dam,0,DAM_LIGHTNING,false);
    }
    
    if (IS_WEAPON_STAT(wield, WEAPON_SPELL)) {
	Affect *waf;
	int lvl;

	for (waf = wield->affected; waf; waf = waf->next) 
	    if (IS_SET( waf->bitvector, WEAPON_SPELL ) 
		&& waf->where == TO_WEAPON
		&& number_range( 1, waf->modifier ) == 1) 
	    {
		act_p("$o1 ярко вспыхивает!", ch, wield, 0, TO_ALL, POS_RESTING);
		lvl = std::min(ch->getModifyLevel(), waf->level);
		spell( waf->type, lvl, ch, victim );
	    }
    }
}

void WeaponOneHit::damEffectFeeble( )
{
    int d;
    
    if (!victim->isAffected(gsn_black_feeble ))
	return;
    
    d = number_percent( );
    if (d >= gsn_black_feeble->getEffective( victim ))
	return;
    
    if (d >= (IS_GOOD(ch) ? 15 : 7))
	return;
   
    SET_BIT( ch->affected_by, AFF_WEAK_STUN );
    act("{DЧерная немощь{x поражает твою руку!", ch, 0, 0, TO_CHAR);
    act("{DЧерная немощь{x поражает руку $c2!", ch, 0, 0, TO_ROOM);

    if (wield && !IS_OBJ_STAT(wield, ITEM_NOREMOVE)) {
	act("Ты парализова$gно|н|на и роняешь оружие!", ch, 0, 0, TO_CHAR);
	
	obj_from_char( wield );

	if (IS_OBJ_STAT(wield, ITEM_NODROP) || IS_OBJ_STAT(wield, ITEM_INVENTORY))
	    obj_to_char( wield, ch );
	else
	    obj_to_room( wield, ch->in_room );
    }
    
    ch->setWaitViolence( 1 );
}

/*----------------------------------------------------------------------------
 * message output with spam-control
 *----------------------------------------------------------------------------*/
void WeaponOneHit::msgOutput( Character *wch, const char *msg )
{
    wch->pecho( msg, ch, victim, wield );
}

