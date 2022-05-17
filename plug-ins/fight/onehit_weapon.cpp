/* $Id$
 *
 * ruffina, 2004
 */
#include "onehit_weapon.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "skill.h"
#include "spelltarget.h"
#include "affect.h"
#include "affecthandler.h"
#include "race.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "core/object.h"
#include "room.h"
#include "clanreference.h"

#include "fight.h"
#include "../anatolia/handler.h"
#include "math_utils.h"

#include "effects.h"
#include "damageflags.h"
#include "magic.h"
#include "occupations.h"
#include "act.h"
#include "mercdb.h"
#include "merc.h"
#include "vnum.h"
#include "def.h"

GSN(black_feeble);
GSN(counter);
GSN(holy_attack);
GSN(poison);

WeaponOneHit::WeaponOneHit( Character *ch, Character *victiim, bool secondary, string command )
                : Damage( ch, victim, 0, 0, DAMF_WEAPON ), 
                  OneHit( ch, victim )
{
    this->secondary = secondary;
    this->command = command;
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
        attack = wield->value3();
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
    int weaponDamage = 0;
    if (wield) {
        weaponDamage = dice(wield->value1(), wield->value2()) * skill / 100;
    }

    if (ch->is_npc()) {
        // Calculate and compare mob ave damage from a wielded weapon dices or from bare hands.
        // NPCs get the best damage of the two.

        NPCharacter *nch = ch->getNPC();
        int handsAve = dice_ave(nch->damage[DICE_NUMBER], nch->damage[DICE_TYPE]);
        int handsDamage = dice(nch->damage[DICE_NUMBER], nch->damage[DICE_TYPE]);
        int weaponAve = wield ? weapon_ave(wield) : 0;

        if (handsAve > weaponAve)
            dam = handsDamage;
        else
            dam = weaponDamage;

    } else if (wield) {
        // PC weapon
        dam = weaponDamage;

    } else {
        // PC hand to hand
        dam = number_range( 1 + 4 * skill/100, 
                            2 * ch->getModifyLevel( ) / 3 * skill/100 );
    }

    if (weaponSkill)
        weaponSkill->improve( ch, true, victim, dam_type, dam_flag );

    if (wield) {
        damApplyShield( );
        damApplySharp( );
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
        msgWeaponRoom( "{YСвященная аура %3$O2 поражает %2$C4.{x" );
        msgWeaponChar( "{YСвященная аура %3$O2 поражает %2$C4.{x" );
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
    Object *ch_wield, *victim_wield;

    if (command != "murder")
        return;

    if (victim->fighting)
        return;
    
    if (victim->is_npc( ) 
        && victim->getNPC( )->behavior 
        && IS_SET(victim->getNPC( )->behavior->getOccupation( ), (1 << OCC_CLANGUARD)))
        return;

    if (is_safe_nomessage(victim, ch) || is_safe_nomessage(ch,victim))
        return;
        
    if (victim->position != POS_SITTING && victim->position != POS_STANDING)
        return;

    ch_wield = get_eq_char(ch,wear_wield);
    victim_wield = get_eq_char(victim,wear_wield);

    if (!victim_wield)
        return;

    if (ch_wield && IS_WEAPON_STAT(ch_wield, WEAPON_TWO_HANDS) != IS_WEAPON_STAT(victim_wield, WEAPON_TWO_HANDS))
        return;

    if (!ch_wield && ((ch->size - victim->size)>1 || (attack_table[ch->dam_type].damage != DAM_SLASH && attack_table[ch->dam_type].damage != DAM_BASH && attack_table[ch->dam_type].damage != DAM_PIERCE)))
       return;

    chance = number_percent();

    if (ch->is_adrenalined())
        chance += 25;
    
    chance *= 2;

    if (chance <= gsn_counter->getEffective( victim ))
    {
        gsn_counter->improve( victim, true, ch );
        oldact("$C1 направляет твой удар против тебя само$gго|го|й!",ch,0,victim,TO_CHAR);
        oldact("Ты направляешь удар $c2 против $x!",ch,0,victim,TO_VICT);
        oldact("$C1 возвращает удар $c2 обратно!",ch,0,victim,TO_NOTVICT);

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

        poison = wield->affected.find(gsn_poison);
        if (!poison)
            level = wield->level;
        else
            level = poison->level;
        if ( !saves_spell(level / 2,victim,DAM_POISON) )
        {
            msgWeaponVict("Ты чувствуешь, как яд распространяется по твоим венам.");
            msgWeaponRoom("%2$^C1 отравле%2$Gно|н|на|ны ядом от %3$O2 %1$C2.");
            msgWeaponChar("%2$^C1 отравле%2$Gно|н|на|ны ядом от %3$O2.");

            af.bitvector.setTable(&affect_flags);
            af.type      = gsn_poison;
            af.level     = level * 3/4;
            af.duration  = level / 2;
            af.location = APPLY_STR;
            af.modifier  = -1;
            af.bitvector.setValue(AFF_POISON);
            af.sources.add(ch);
            af.sources.add(wield);
            affect_join( victim, &af );
        }

        // weaken the poison if it's temporary
        if ( poison != 0 )
        {
            poison->level = max(0,poison->level - 2);
            poison->duration = max(0,poison->duration - 1);
            if ( poison->level == 0 || poison->duration == 0 )
                oldact("Яд с $o2 скоро исчезнет.",ch,wield,0,TO_CHAR);
        }
    }

    if (IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC))
    {
        msgWeaponVict("Ты чувствуешь как %3$O1 вытягива%3$nет|ют из тебя жизнь.");
        msgWeaponRoom("%3$^O1 %1$C2 вытягива%3$nет|ют жизнь из %2$C2.");
        msgWeaponChar("%3$^O1 вытягива%3$nет|ют жизнь из %2$C2.");

        dam = number_range(1, wield->level / 5 + 1);
        damage_nocatch(ch,victim,dam, weapon_sn,DAM_NEGATIVE,false);
        ch->hit += dam*2;
    }
    if (IS_WEAPON_STAT(wield,WEAPON_FLAMING) )
    {
        msgWeaponVict("%3$^O1 обжига%3$nет|ют тебя.");
        msgWeaponRoom("%3$^O1 %1$C2 обжига%3$nет|ют %2$C4.");
        msgWeaponChar("%3$^O1 обжига%3$nет|ют %2$C4.");

        dam = number_range(1,wield->level / 4 + 1);
        fire_effect( (void *) victim, ch, wield->level/2,dam,TARGET_CHAR);
        damage_nocatch(ch,victim,dam, weapon_sn,DAM_FIRE,false);
    }
    if (IS_WEAPON_STAT(wield,WEAPON_FROST) )
    {
        msgWeaponVict("Ледяное прикосновение %3$O2 обмораживает тебя, покрывая льдом.");
        msgWeaponRoom("%3$^O1 %1$C2 обморажива%3$nет|ют %2$C4.");
        msgWeaponChar("%3$^O1 обморажива%3$nет|ют %2$C4.");

        dam = number_range(1,wield->level / 6 + 2);
        cold_effect(victim, ch, wield->level/2,dam,TARGET_CHAR);
        damage_nocatch(ch,victim,dam, weapon_sn,DAM_COLD,false);
    }
    if (IS_WEAPON_STAT(wield,WEAPON_SHOCKING))
    {
        msgWeaponVict("Ты пораже%2$Gно|н|на разрядом %3$O2.");
        msgWeaponRoom("%2$^C1 пораже%2$Gно|н|на|ны разрядом %3$O2 %1$C2.");
        msgWeaponChar("%2$^C1 пораже%2$Gно|н|на|ны разрядом %3$O2.");

        dam = number_range(1,wield->level/5 + 2);
        shock_effect(victim, ch, wield->level/2,dam,TARGET_CHAR);
        damage_nocatch(ch,victim,dam, weapon_sn,DAM_LIGHTNING,false);
    }
    
    if (IS_WEAPON_STAT(wield, WEAPON_SPELL)) {
        int lvl;

        for (auto &waf: wield->affected.findAllWithBits(&weapon_type2, WEAPON_SPELL)) 
            if (number_range( 1, waf->modifier ) == 1)
            {
                oldact("$o1 ярко вспыхивает!", ch, wield, 0, TO_ALL);
                lvl = std::min((int)ch->getModifyLevel(), (int)waf->level);
                spell_nocatch( waf->type, lvl, ch, victim );
            }
    }
}

void WeaponOneHit::damEffectFeeble( )
{
    int d;
    
    if (!victim->isAffected(gsn_black_feeble ))
        return;
    
    d = number_percent( );
    if (d >= (IS_GOOD(ch) ? 15 : 7))
        return;
   
    SET_BIT( ch->affected_by, AFF_WEAK_STUN );
    oldact("{DЧерная немощь{x поражает твою руку!", ch, 0, 0, TO_CHAR);
    oldact("{DЧерная немощь{x поражает руку $c2!", ch, 0, 0, TO_ROOM);

    if (wield && !IS_OBJ_STAT(wield, ITEM_NOREMOVE)) {
        oldact("Ты парализова$gно|н|на и роняешь оружие!", ch, 0, 0, TO_CHAR);
        
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

bool WeaponOneHit::mprog_immune()
{
    DLString damType = damage_table.name(dam_type);
    DLString sname = weaponSkill->getName();
    FENIA_NUM_CALL(victim, "Immune", dam, "CisOis", ch, dam, damType.c_str(), wield, dam_flag, sname.c_str());
    FENIA_NDX_NUM_CALL(victim->getNPC(), "Immune", dam, "CCisOis", victim, ch, dam, damType.c_str(), wield, dam_flag, sname.c_str());

    for (auto &paf: victim->affected.findAllWithHandler())
        if (paf->type->getAffect()->onImmune(SpellTarget::Pointer(NEW, victim), paf, ch, dam, damType.c_str(), wield, dam_flag, sname.c_str()))
            return true;

    return false; 
}

SkillWeaponOneHit::SkillWeaponOneHit(Character *ch, Character *victim, int sn)
            : Damage(ch, victim, 0, 0, DAMF_WEAPON),
              WeaponOneHit( ch, victim, false ),
              SkillDamage( ch, victim, sn, 0, 0, DAMF_WEAPON )
              
{
}              

bool SkillWeaponOneHit::mprog_immune()
{
    DLString damType = damage_table.name(dam_type);
    DLString sname = skillManager->find(sn)->getName();
    FENIA_NUM_CALL(victim, "Immune", dam, "CisOis", ch, dam, damType.c_str(), wield, dam_flag, sname.c_str());
    FENIA_NDX_NUM_CALL(victim->getNPC(), "Immune", dam, "CCisOis", victim, ch, dam, damType.c_str(), wield, dam_flag, sname.c_str());

    for (auto &paf: victim->affected.findAllWithHandler())
        if (paf->type->getAffect()->onImmune(SpellTarget::Pointer(NEW, victim), paf, ch, dam, damType.c_str(), wield, dam_flag, sname.c_str()))
            return true;
            
    return false; 
}
