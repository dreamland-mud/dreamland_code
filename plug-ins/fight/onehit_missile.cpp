/* $Id$
 *
 * ruffina, 2004
 */
#include "onehit_missile.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

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
#include "stats_apply.h"
#include "magic.h"
#include "act.h"
#include "mercdb.h"
#include "merc.h"
#include "vnum.h"
#include "def.h"

GSN(accuracy);
CLAN(battlerager);
PROF(warrior);
PROF(samurai);
PROF(paladin);
PROF(thief);
PROF(ninja);

/*----------------------------------------------------------------------------
 * one hit with missile weapon (send by hand or thrower) 
 *----------------------------------------------------------------------------*/
MissileOneHit::MissileOneHit( 
        Character *ch, Character *victim, Object *missile, int range )
            : Damage( ch, victim, 0, 0, DAMF_WEAPON ), 
              OneHit( ch, victim )
{
    this->range = range;
    this->missile = missile;
    missileSkill = NULL;
    missile_sn = -1;
}

ThrowerOneHit::ThrowerOneHit( Character *ch, Character *victim, Object *thrower, Object *missile, int range )
            : Damage( ch, victim, 0, 0 ), MissileOneHit( ch, victim, missile, range )
{
    this->thrower = thrower;
    thrower_sn = -1;
    skill_thrower = 0;
    throwerSkill = NULL;
}

/*----------------------------------------------------------------------------
 * Avoid damages 
 *----------------------------------------------------------------------------*/
bool MissileOneHit::canDamage( )
{
    if (!OneHit::canDamage( ))
        return false;

    if (defenseDodge( )
        || defenseParry( )
        || defenseShieldBlock( ))
    {
        return false;
    }

    return true;
}

bool MissileOneHit::defenseParry( )
{
    int chance = 0;
    
    if (missile_sn != gsn_arrow)
        return false;

    if (victim->position != POS_STANDING)
        return false;
    
    if (MOUNTED(victim))
        return false;

    if (IS_AFFECTED(victim, AFF_STUN))
        return false;

    if (HAS_SHADOW( victim ))
        return false;

    if (get_weapon_sn( victim, true ) == gsn_sword) 
        chance = gsn_sword->getEffective( victim ) / 4;

    if (get_weapon_sn( victim, false ) == gsn_sword)
        chance += chance * gsn_second_weapon->getEffective( victim ) / 200;
    
    chance = chance * gsn_mastering_sword->getEffective( victim ) / 100;
    chance = chance * gsn_parry->getEffective( victim ) / 100;

    if (IS_AFFECTED(victim, AFF_WEAK_STUN))
        chance /= 2;

    if (!victim->can_see( missile )) {
        if (number_percent( ) < gsn_blind_fighting->getEffective( victim ))
            chance /= 10;
        else
            return false;
    }

    if (number_percent( ) > chance)
        return false;
    
    if (number_bits( 1 )) {
        msgFightVict( "Ты разрубаешь %3$O4 своим мечом." );
        msgFightRoom( "%2$^C1 разрубает %3$O4 своим мечом." );
        msgFightChar( "%2$^C1 разрубает %3$O4 своим мечом." );
        extract_obj( missile );
        missile = NULL;
    }
    else {
        msgFightVict( "Ты отбиваешь %3$O4 своим оружием." );
        msgFightRoom( "%2$^C1 отбивает %3$O4 своим оружием." );
        msgFightChar( "%2$^C1 отбивает %3$O4 своим оружием." );
        obj_to_room( missile, victim->in_room );
    }

    return true;
}

bool MissileOneHit::defenseShieldBlock( )
{
    return false;
}

bool MissileOneHit::defenseDodge( )
{
    int chance; 

    if (!IS_AWAKE(victim) || MOUNTED(victim))
        return false;

    if (IS_AFFECTED(victim, AFF_STUN))
        return false;

    if (HAS_SHADOW( victim ))
        return false;

    if (victim->is_npc( ))
        chance  = min( 30, (int)victim->getModifyLevel( ) );
    else
    {
        int prof = victim->getTrueProfession( );

        chance  = gsn_dodge->getEffective( victim ) / 4;
        chance += victim->getCurrStat(STAT_DEX) - 20;

        if (prof == prof_warrior || prof == prof_samurai || prof == prof_paladin)
            chance += chance / 5;
        else if (prof == prof_thief || prof == prof_ninja)
            chance += chance / 10;
    }
    
    chance -= (11 - range) * 10; /* XXX */

    if (!victim->is_npc( ) && victim->getClan( ) != clan_battlerager)
        chance /= 2;
        
    if (IS_AFFECTED(victim, AFF_WEAK_STUN))
        chance /= 2;

    if (!victim->can_see( missile )) {
        if (number_percent( ) < gsn_blind_fighting->getEffective( victim ))
            chance /= 10;
        else
            return false;
    }

    if (number_percent( ) >= chance)
        return false;

    if (!victim->is_npc()
        && victim->getClan( ) == clan_battlerager 
        && victim->getClan( )->isRecruiter( victim->getPC( ) ))
    {
        msgFightVict( "Ты ловишь руками %3$O4." );
        msgFightRoom( "%2$^C1 ловит руками %3$O4." );
        msgFightChar( "%2$^C1 ловит руками %3$O4." );
        obj_to_char( missile, victim );
        return true;
    }

    msgFightVict( "Ты уклоняешься от %3$O2." );
    msgFightRoom( "%2$^C1 уклоняется от %3$O2" );
    msgFightChar( "%2$^C1 уклоняется от %3$O2." );

    obj_to_room( missile, victim->in_room );
    gsn_dodge->improve( victim, true, ch );
    return true;
}

/*----------------------------------------------------------------------------
 * init weapon skills 
 *----------------------------------------------------------------------------*/
void MissileOneHit::init( )
{
    missile_sn = get_weapon_sn( missile );
    missileSkill = skillManager->find( missile_sn );
    skill = missileSkill->getEffective( ch ) + 20;
    dam_type = attack_table[missile->value[3]].damage;
}

void ThrowerOneHit::init( )
{
    MissileOneHit::init( );
    
    thrower_sn = get_weapon_sn( thrower );
    throwerSkill = skillManager->find( thrower_sn );
    skill_thrower = throwerSkill->getEffective( ch ) + 20;
}

/*----------------------------------------------------------------------------
 * Damage calculations with modifiers 
 *----------------------------------------------------------------------------*/
void MissileOneHit::calcDamage( )
{
    damBase( );
    damApplySharp( );
    damApplyHoly( );
    damApplyAccuracy( );
    dam = number_range( dam, 2 * dam );
    damApplyStrength( );
    damApplyDamroll( );
    damApplyMissile( );

    OneHit::calcDamage( );
    protectMaterial( missile );
}

void MissileOneHit::damBase( )
{
    missileSkill->improve( ch, true, victim, dam_type, dam_flag );
    dam = dice( missile->value[1], missile->value[2] ) * skill / 100;
}

void ThrowerOneHit::damBase( )
{
    throwerSkill->improve( ch, true, victim, dam_type, dam_flag );
    MissileOneHit::damBase( );
    dam += dice( thrower->value[1], thrower->value[2] ) * skill_thrower / 100;
}

void MissileOneHit::damApplySharp( )
{
    if (IS_WEAPON_STAT(missile, WEAPON_SHARP)) {
        int percent = number_percent( );

        if (percent <= skill / 8)
            dam = 2 * dam + (dam * 2 * percent / 100);
    }
}

void MissileOneHit::damApplyHoly( )
{
    if (IS_GOOD(ch) 
        && IS_EVIL(victim)
        && IS_WEAPON_STAT(missile, WEAPON_HOLY) 
        && number_percent( ) < 30) 
    {
        dam += dam * 120 / 100;
    }
}

void MissileOneHit::damApplyAccuracy( )
{
    if (ch->isAffected( gsn_accuracy ))
        dam *= 2;
}

void MissileOneHit::damApplyStrength( )
{
    dam += get_str_app(ch).missile;
}

void MissileOneHit::damApplyDamroll( )
{
    dam += ch->damroll / 2;
}

void MissileOneHit::damApplyMissile( )
{
    Affect *paf;
    int bonus = 0;

    for (paf = missile->affected; paf != 0; paf = paf->next)
        if (paf->location == APPLY_DAMROLL)
            bonus += paf->modifier;

    bonus *= 10;
    dam += bonus;
}

/*----------------------------------------------------------------------------
 * calc THAC 
 *----------------------------------------------------------------------------*/
void MissileOneHit::calcTHAC0( )
{
    OneHit::calcTHAC0( );
    thacApplyMissile( );
}

void MissileOneHit::thacApplyMissile( )
{
    Affect *paf;
    int bonus = 0;

    for (paf = missile->affected; paf != 0; paf = paf->next)
        if (paf->location == APPLY_HITROLL)
            bonus += paf->modifier;

    thac0 -= bonus * skill/100;
}

/*----------------------------------------------------------------------------
 * various damage effects 
 *----------------------------------------------------------------------------*/
void MissileOneHit::postDamageEffects( )
{
    damEffectFunkyWeapon( );
    damEffectStucking( );
}

void MissileOneHit::damEffectStucking( )
{
    if (dam_type == DAM_PIERCE
            && dam > victim->max_hit / 10
            && number_percent( ) < 50)
    {
        Affect af;

        af.where     = TO_AFFECTS;
        af.type      = missile_sn;
        af.level     = ch->getModifyLevel( );
        af.duration  = -1;
        af.location  = APPLY_HITROLL;
        af.modifier  = - (dam / 20);

        if (victim->is_npc( ))
            af.bitvector = 0;
        else
            af.bitvector = AFF_CORRUPTION;

        affect_join( victim, &af );

        obj_to_char( missile, victim );
        equip_char( victim, missile, wear_stuck_in );
    }
}

void MissileOneHit::damEffectFunkyWeapon( )
{
    if (IS_WEAPON_STAT(missile,WEAPON_POISON))
    {
        short level;
        Affect *poison, af;

        if ((poison = missile->affected->affect_find(gsn_poison)) == 0)
            level = missile->level;
        else
            level = poison->level;

        if (!saves_spell(level,victim,DAM_POISON))
        {
            msgWeaponVict("Ты чувствуешь, как яд растекается по твоим венам.");
            msgWeaponRoom("%2$^C1 отравле%2$Gно|н|на ядом от %3$O2.");

            af.where     = TO_AFFECTS;
            af.type      = gsn_poison;
            af.level     = level * 3/4;
            af.duration  = level / 2;
            af.location  = APPLY_STR;
            af.modifier  = -1;
            af.bitvector = AFF_POISON;
            affect_join( victim, &af );
        }
    }

    if (IS_WEAPON_STAT(missile,WEAPON_FLAMING))
    {
        msgWeaponVict("%3$^O1 обжигает тебя.");
        msgWeaponRoom("%3$^O1 обжигает %2$C4.");
        fire_effect( (void *) victim,missile->level,dam,TARGET_CHAR);
    }
    
    if (IS_WEAPON_STAT(missile,WEAPON_FROST))
    {
        msgWeaponVict("%3$^O1 обмораживает тебя.");
        msgWeaponRoom("%3$^O1 обмораживает %2$C4.");
        cold_effect(victim,missile->level,dam,TARGET_CHAR);
    }
    
    if (IS_WEAPON_STAT(missile,WEAPON_SHOCKING))
    {
        msgWeaponVict("%3$^O1 парализует тебя разрядом молнии.");
        msgWeaponRoom("%3$^O1 парализует %2$C4 разрядом молнии.");
        shock_effect(victim,missile->level,dam,TARGET_CHAR);
    }
}

/*----------------------------------------------------------------------------
 * message output with spam-control
 *----------------------------------------------------------------------------*/
void MissileOneHit::msgOutput( Character *wch, const char *msg )
{
    wch->pecho( msg, ch, victim, missile );
}

bool MissileOneHit::mprog_immune()
{
    DLString damType = damage_table.name(dam_type);
    DLString sname = missileSkill->getName();
    FENIA_NUM_CALL(victim, "Immune", dam, "CisOis", ch, dam, damType.c_str(), missile, dam_flag, sname.c_str());
    FENIA_NDX_NUM_CALL(victim->getNPC(), "Immune", dam, "CCisOis", victim, ch, dam, damType.c_str(), missile, dam_flag, sname.c_str());
    return false; 
}

bool ThrowerOneHit::mprog_immune()
{
    DLString damType = damage_table.name(dam_type);
    DLString sname = throwerSkill->getName();
    FENIA_NUM_CALL(victim, "Immune", dam, "CisOis", ch, dam, damType.c_str(), thrower, dam_flag, sname.c_str());
    FENIA_NDX_NUM_CALL(victim->getNPC(), "Immune", dam, "CCisOis", victim, ch, dam, damType.c_str(), thrower, dam_flag, sname.c_str());
    return false; 
}
