/* $Id: onehit_undef.cpp,v 1.1.2.18 2010-09-01 21:20:44 rufina Exp $
 * 
 * ruffina, 2004
 */
#include "logstream.h"

#include "onehit_undef.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "skill.h"
#include "skillcommand.h"
#include "affect.h"
#include "affecthandler.h"
#include "race.h"
#include "religion.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "core/object.h"
#include "room.h"
#include "roomutils.h"
#include "clanreference.h"
#include "areabehaviorplugin.h"

#include "dreamland.h"
#include "debug_utils.h"
#include "fight.h"
#include "weapongenerator.h"
#include "material.h"
#include "immunity.h"
#include "../anatolia/handler.h"
#include "skill_utils.h"
#include "move_utils.h"
#include "gsn_plugin.h"
#include "profflags.h"
#include "act.h"
#include "mercdb.h"
#include "merc.h"
#include "vnum.h"
#include "def.h"

CLAN(shalafi);

PROF(none);
PROF(warrior);
PROF(paladin);
PROF(anti_paladin);
PROF(ninja);
PROF(ranger);
PROF(samurai);
PROF(thief);

WEARLOC(tat_wrist_l);
WEARLOC(tat_wrist_r);

RELIG(cradya);
RELIG(phobos);
RELIG(deimos);

/*----------------------------------------------------------------------------
 * Hit by weapon or bare hands
 *---------------------------------------------------------------------------*/
UndefinedOneHit::UndefinedOneHit( Character *ch, Character *victim, bool secondary )
            : Damage( ch, victim, 0, 0, DAMF_WEAPON ), 
              WeaponOneHit( ch, victim, secondary )
{
}

bool UndefinedOneHit::canDamage( )
{
    if (!OneHit::canDamage( ))
        return false;

    if (!dam) 
        return true;

    if (ch != victim) {
        if (victim->is_mirror( )) {
            oldact("$c1 разбивается на мелкие осколки.",victim,0,0,TO_ROOM);
            extract_char(victim);
            return false;
        }
                
        if (defenseParry( ) 
            || defenseHandBlock( ) 
            || defenseBatSwarm( ) 
            || defenseBlink( ) 
            || defenseShieldBlock( ) 
            || defenseCrossBlock( ) 
            || defenseDodge( ))
        {
            return false;
        }
    }

    return true;
}

void UndefinedOneHit::protectPrayer( )
{
    if (victim->isAffected( gsn_prayer )) 
        dam -= dam * (3 + victim->getModifyLevel( ) / 10) / 100;
}

bool UndefinedOneHit::checkHands( )
{
    if (!ch->getWearloc( ).isSet( wear_hands ))
        return false;

    if (!ch->getWearloc( ).isSet( secondary ? wear_wrist_l : wear_wrist_r ))
        return false;

    return true;
}

bool UndefinedOneHit::canHit()
{
    if (!OneHit::canHit( ))
        return false;

    if (!checkHands( ))
        return false;
    
    return true;
}

void UndefinedOneHit::calcDamage( )
{
    damBase( );
    damApplyMasterHand( );
    damApplyEnhancedDamage( );
    damApplyMasterSword( );
    damApplyPosition( );
    damApplyDamroll( );
    damApplyAttitude( );
    damApplyDeathblow( );
    damApplyCounter( );
    damApplyReligion();

    damNormalize( );

    protectSanctuary( );
    protectAlign( );
    protectTemperature( );
    protectPrayer( ); 
    protectImmune( );
    protectRazer( ); 

    if (wield)
        protectMaterial( wield );
}

void UndefinedOneHit::priorDamageEffects( )
{
    damEffectMasterHand( );
    damEffectMasterSword( );
    damEffectCriticalStrike( );
}

bool UndefinedOneHit::mprog_hit()
{
    DLString damType = damage_table.name( dam_type );
    
    FENIA_CALL( victim, "Hit", "CisO", ch, dam, damType.c_str( ), wield );
    FENIA_NDX_CALL( victim->getNPC( ), "Hit", "CCisO", victim, ch, dam, damType.c_str( ), wield );

    for (auto &paf: victim->affected.findAllWithHandler())
        if (paf->type->getAffect()->onHit(SpellTarget::Pointer(NEW, victim), paf, ch, dam, damType.c_str(), wield))
            return true;

    return false;
}

void UndefinedOneHit::postDamageEffects( )
{
    damEffectDestroyEquipment( );
    damEffectFeeble( );
    damEffectFunkyWeapon( );
    damEffectSlice( );
    damEffectVorpal();
}

/**
 * One, two! One, two! and through and through
 * The vorpal blade went snicker-snack!
 * He left it dead, and with its head
 * He went galumphing back.
 * 
 * Раз-два, раз-два! Горит трава,
 * Взы-взы — стрижает меч,
 * Ува! Ува! И голова
 * Барабардает с плеч!
 */ 
void UndefinedOneHit::damEffectVorpal()
{
    // Will work for both first and second wield, if worn.
    if (!wield || ch->fighting != victim)
        return;

    if (!IS_WEAPON_STAT(wield, WEAPON_VORPAL))
        return;

    // No vorpal from mobs: repeats sun sword logic, can be easily changed if needed.
    if (ch->is_npc())
        return;

    if (victim->is_immortal())
        return;

    // Chances are 1% for goods and 0.5% for others. Generic 'vorpal' warning is shown more often.
    if (!chance(5)) 
        return;

    const char *msgAll;
    
    if (wield->value0() == WEAPON_SWORD)
        msgAll = "{mРаз-два, раз-два! Горит трава, взы-взы -- стрижает меч!";
    else if (wield->value0() == WEAPON_AXE)
        msgAll = "{mРаз-два! Раз-два! Горит трава, взы-взы свирчит топор!{x";
    else if (wield->value0() == WEAPON_POLEARM)
        msgAll = "{mРаз-два! Раз-два! Горит трава, взы-взы свирчит бердыш!{x";
    else if (wield->value0() == WEAPON_WHIP)
        msgAll = "{mРаз-два! Раз-два! Горит трава, взы-взы стрижает плеть!{x";
    else if (wield->value0() == WEAPON_DAGGER)
        msgAll = "{mРаз-два! Раз-два! Горит трава, взы-взы стрижает нож!{x";
    else
        msgAll = "{mРаз-два! Раз-два! Горит трава, взы-взы свирчит клинок!{x";

    ch->in_room->echo(POS_RESTING, msgAll);

    if (!chance(IS_GOOD(ch) ? 20 : 10))
        return;

    const char *msgVict, *msgOther;
    if (wield->value0() == WEAPON_SWORD) {
        msgOther = "{mУва! Ува! И %1$C2 голова барабардает с плеч!{x";
        msgVict = "{mУва! И твоя голова барабардает с плеч!{x";
    } else if (wield->value0() == WEAPON_AXE) {
        msgOther = "{mУва! %1$^C1 без головы остал%1$Gось|ся|ась с этих пор!{x";
        msgVict = "{mУва! И ты без головы остал%1$Gось|ся|ась с этих пор!{x";    
    } else if (wield->value0() == WEAPON_POLEARM) {
        msgOther = "{mУва! %1$C2 котелок скосило как камыш!{x";
        msgVict = "{mУва! Ува! Твой котелок скосило как камыш!{x";
    } else if (wield->value0() == WEAPON_WHIP) {
        msgOther = "{mУва! %1$C3 с головой не подружиться впредь!{x";
        msgVict = "{mУва! И тебе с головой не подружиться впредь!{x";
    } else if (wield->value0() == WEAPON_DAGGER) {
        msgOther = "{mУва! Ты голову %1$C2 у ног своих найдешь!{x";
        msgVict = "{mУва! Ты голову свою у ног своих найдешь!{x";
    } else {
        msgOther = "{mУва! %1$C2 голова лежит у твоих ног!{x";
        msgVict = "{mУва! И твоя голова лежит у твоих ног!{x";
    }

    victim->pecho(msgVict, victim);
    victim->recho(msgOther, victim);
    victim->recho("%^C1 уже ТРУП!", victim);

    group_gain( ch, victim );
    raw_kill( victim, 3, ch, FKILL_CRY|FKILL_GHOST|FKILL_CORPSE );
    pk_gain( ch, victim );
    victim->pecho("Тебя УБИЛИ!");
    throw VictimDeathException( );
}

void UndefinedOneHit::message( )
{
    RussianString noun(attack_table[attack].noun,
                       attack_table[attack].gender);

    if (immune) {
        if (ch == victim) {
            msgRoom("%2$^O1 %3$C2 бессил%2$Gьно|ен|ьна|ьны против %3$P4 сам%3$Gого|ого|ой|их", dam, &noun, ch);
            msgChar("Тебе повезло, у тебя иммунитет к этому", dam);
        }
        else {
            msgRoom("%2$^O1 %3$C2 бессил%2$Gьно|ен|ьна|ьны против %4$C2", dam, &noun, ch, victim);
            msgChar("%2$^T1 %2$O1 бессил%2$Gьно|ен|ьна|ьны против %3$C2", dam, &noun, victim);
            msgVict("Против тебя %3$O1 %2$C2 бессил%3$Gьно|ен|ьна", dam, ch, &noun);
        }
    }
    else {
        if (ch == victim) {
            msgRoom( "%2$^O1 %3$C2\6себя", dam, &noun, ch );
            msgChar( "%2$^T1 %2$O1\6тебя", dam, &noun );
        }
        else {
            if ( dam == 0 )
            {
                msgRoom( "%2$^O1 %3$C2\6%4$C2", dam, &noun, ch, victim );
                msgChar( "%2$^T1 %2$O1\6%3$C2", dam, &noun, victim );
            }
            else {
                msgRoom( "%2$^O1 %3$C2\6%4$C4", dam, &noun, ch, victim );
                msgChar( "%2$^T1 %2$O1\6%3$C4", dam, &noun, victim );
            }
            msgVict( "%2$^O1 %3$C2\6тебя", dam, &noun, ch );
        }
    }
}


/*----------------------------------------------------------------------------
 * Check victim ability to avoid the attack
 *---------------------------------------------------------------------------*/
bool UndefinedOneHit::defenseBatSwarm( )
{
    int chance;

    if (!victim->isAffected(gsn_bat_swarm))
        return false;

    chance = 50 + (victim->getModifyLevel( ) - ch->getModifyLevel( ));

    if (number_percent( ) > chance)
        return false;

    if (SHADOW(victim)) {
        msgFightVict( "Стая летучих мышей вокруг тебя сбита с толку твоей тенью." );
        msgFightChar( "Стая летучих мышей вокруг %2$C2 сбита с толку тенью." );
        msgFightRoom( "Стая летучих мышей вокруг %2$C2 сбита с толку тенью." );
        return false;
    }
    
    msgFightChar( "Ты не смо%1$Gгло|г|гла пробиться сквозь стаю летучих мышей, кружащихся вокруг %2$C2.");
    msgFightVict( "Стая летучих мышей не позволяет %1$C3 повредить тебе." );
    msgFightRoom( "%1$^C1 пытается разогнать стаю летучих мышей вокруг %2$C2." );

    return true;
}

bool UndefinedOneHit::defenseParry( )
{
    int chance, prof;
    Object *defending_weapon;

    if (!IS_AWAKE(victim))
        return false;

    if (IS_AFFECTED(victim,AFF_STUN))
        return false;

    defending_weapon = get_eq_char( victim, wear_wield );

    if (!victim->is_npc( ) && defending_weapon == 0)
        return false;

    chance    = gsn_parry->getEffective( victim ) / 2;
    prof = victim->getProfession( );

    if (prof == prof_warrior || prof == prof_samurai || prof == prof_paladin)
        chance += chance / 5;
    else if (prof == prof_anti_paladin && victim->getClan( ) == clan_shalafi) 
            chance /= 2;

    if (wield && (wield->value0() == WEAPON_FLAIL || wield->value0() == WEAPON_WHIP ))
        return false;

    if ( !victim->can_see( ch ) )
    {
        if (number_percent( ) < gsn_blind_fighting->getEffective( victim ) )
        {
            gsn_blind_fighting->improve( victim, true, ch );
            chance = ( int )( chance / 1.5 );
        }
        else
            chance = ( int )( chance / 4 );
    }

    if ( IS_AFFECTED(victim,AFF_WEAK_STUN) )
    {
        chance = ( int )( chance * 0.5 );
    }
    
    if (gsn_forest_fighting->getCommand( )->run(victim, FOREST_DEFENCE)
        &&  (number_percent( ) < gsn_forest_fighting->getEffective( victim ))) 
    {
        chance = ( int )( chance * 1.2 );
        gsn_forest_fighting->improve( victim, true, ch );
    }


    if (number_percent( ) >= chance + skill_level(*gsn_parry, victim) - ch->getModifyLevel())
        return false;

    if(SHADOW(victim))
    {
        msgFightVict( "Ты пытаешься парировать атаку, но путаешься в своей тени." );
        msgFightChar( "%2$^C1 постоянно путается в своей тени." );
        msgFightRoom( "%2$^C1 постоянно путается в своей тени." );
        return false;
    }
    
    if (wield 
        && IS_WEAPON_STAT(wield, WEAPON_FADING)
        && (!defending_weapon 
            || !IS_WEAPON_STAT(defending_weapon, WEAPON_HOLY))) 
    {
        msgFightVict( "%3$^O1 проходит насквозь через твою попытку спарировать!" );
        
        if (defending_weapon) {
            msgFightChar( "%3$^O1 проходит сквозь оружие %2$C2!" );
            msgFightRoom( "%3$^O1 %1$C2 проходит сквозь оружие %2$C2!" );
        }
        else {
            msgFightChar( "%3$^O1 проходит сквозь руки %2$C2!" );
            msgFightRoom( "%3$^O1 %1$C2 проходит сквозь руки %2$C2!" );
        }

        return false;
    }

    msgFightVict( "Ты парируешь атаку %1$C2." );
    msgFightChar( "%2$^C1 парирует твою атаку." );
    msgFightRoom( "%2$^C1 парирует атаку %1$C2." );

    destroyWeapon( );

    if ( number_percent() >  gsn_parry->getEffective( victim ) )
    {
        /* size  and weight */
        chance += min(ch->canCarryWeight( ), ch->carry_weight) / 25;
        chance -= min(victim->canCarryWeight( ), victim->carry_weight) / 20;

        if (ch->size < victim->size)
            chance += (ch->size - victim->size) * 25;
        else
            chance += (ch->size - victim->size) * 10;

        /* stats */
        chance += ch->getCurrStat(STAT_STR);
        chance -= victim->getCurrStat(STAT_DEX) * 4/3;

        if ( is_flying( ch ) )
            chance -= 10;

        /* speed */
        if (IS_QUICK(ch))
            chance += 10;

        if (IS_QUICK(victim))
            chance -= 20;

        /* level */
        chance += ( ch->getModifyLevel() - victim->getModifyLevel() ) * 2;

        /* now the attack */
        if (number_percent() < ( chance / 20  ))
        {
            oldact("Ты не можешь устоять на ногах!",ch,0,victim,TO_VICT);
            oldact("Ты падаешь вниз!",ch,0,victim,TO_VICT);
            oldact("$C1 не может устоять на ногах и падает вниз!", ch,0,victim,TO_CHAR);
            oldact("$C1 пытается парировать мощный удар $c1, но не может устоять на ногах.", ch,0,victim,TO_NOTVICT);

            victim->setWait(gsn_bash->getBeats( ));
            victim->position = POS_RESTING;
        }
    }
    gsn_parry->improve( victim, true, ch );
    return true;
}

bool UndefinedOneHit::defenseBlink( )
{
    int chance;
    
    if ( victim->is_npc() )
        return false;

    if (!IS_SET(victim->act, PLR_BLINK_ON) || !gsn_blink->usable( victim ))
        return false;

    chance  = gsn_blink->getEffective( victim ) / 2;

    if ( ( number_percent( ) >= chance + victim->getModifyLevel() - ch->getModifyLevel() )
        || ( number_percent( ) < 50 )
        || ( victim->mana < max( victim->getModifyLevel() / 5, 1 ) ) )
        return false;

    victim->mana -= max( victim->getModifyLevel() / 5, 1 );

    if(SHADOW(victim))
    {
        msgFightVict("Ты мерцаешь и попадаешь под удар.");
        msgFightChar("%2$^C1 мерцает... но тень выдает %2$P4.");
        msgFightRoom("%2$^C1 мерцает... но тень выдает %2$P4.");
        return false;
    }

    msgFightVict( "Ты мерцаешь и уклоняешься от атаки %1$C2.");
    msgFightChar( "%2$^C1 мерцает и уклоняется от твоей атаки.");
    msgFightRoom( "%2$^C1 мерцает и уклоняется от атаки %1$C2.");

    gsn_blink->improve( victim, true, ch );
    return true;
}

bool UndefinedOneHit::defenseShieldBlock( )
{
    int chance, prof;

    if ( !IS_AWAKE(victim) )
        return false;
    
    if ( get_eq_char( victim, wear_shield ) == 0 )
        return false;

    if ( IS_AFFECTED(victim,AFF_STUN) )
        return false;
        
    chance = gsn_shield_block->getEffective( victim );
    
    if (chance <= 1)
        return false;

    chance = chance / 2 - 10;
    prof = victim->getProfession( );

    if (prof == prof_warrior || prof == prof_samurai || prof == prof_paladin)
        chance += 10;

    if (wield) { 
        if (wield->value0() == WEAPON_FLAIL)
            chance /= 2;
        if (wield->value0() == WEAPON_WHIP)
            return false;
    }

    if ( !victim->can_see( ch ) )
    {
        if (number_percent( ) < gsn_blind_fighting->getEffective( victim ))
        {
            gsn_blind_fighting->improve( victim, true, ch );
            chance = ( int )( chance / 1.5 );
        }
        else
            chance = ( int )( chance / 4 );
    }

    if ( IS_AFFECTED(victim,AFF_WEAK_STUN) )
    {
        chance = ( int )( chance * 0.5 );
    }
    
    if (gsn_forest_fighting->getCommand( )->run(victim, FOREST_DEFENCE)
        && (number_percent() < gsn_forest_fighting->getEffective( victim ))) 
    {
        chance = ( int )( chance * 1.2 );
        gsn_forest_fighting->improve( victim, true, ch );
    }
    
    if ( number_percent( ) >= chance + skill_level(*gsn_shield_block, victim) - ch->getModifyLevel()
        || ( !victim->is_npc() && !victim->move ) )
        return false;

    victim->move -= move_dec( victim );

    if (SHADOW(victim))
    {
        msgFightVict( "Ты впустую машешь щитом перед твоей тенью." );
        msgFightChar( "%2$^C1 впустую размахивает щитом перед своей тенью.");
        msgFightRoom( "%2$^C1 впустую размахивает щитом перед своей тенью.");
        return false;
    }

    msgFightVict( "Ты отражаешь щитом атаку %1$C2." );
    msgFightChar( "%2$^C1 отражает твою атаку %2$P2 щитом." );
    msgFightRoom( "%2$^C1 отражает атаку %1$C2 своим щитом." );

    destroyShield( );
    gsn_shield_block->improve( victim, true, ch );
    return true;
}

bool UndefinedOneHit::defenseDodge( )
{
    int chance, prof;

    if ( !IS_AWAKE(victim) )
        return false;

    if ( MOUNTED(victim) )
        return false;
    
    if ( IS_AFFECTED(victim,AFF_STUN) )
        return false;

    chance  = gsn_dodge->getEffective( victim ) / 2;

    /* chance for high dex. */
    chance += 2 * (victim->getCurrStat(STAT_DEX) - 20);
    prof = victim->getProfession( );

    if (prof == prof_warrior || prof == prof_samurai || prof == prof_paladin)
        chance += chance / 5;
    else if (prof == prof_thief || prof == prof_ninja)
        chance += chance / 10;
    else if (prof == prof_anti_paladin)
        chance -= chance / 10;

    if ( !victim->can_see( ch ) )
    {
        if (number_percent( ) < gsn_blind_fighting->getEffective( victim ))
        {
            gsn_blind_fighting->improve( victim, true, ch );
            chance = ( int )( chance / 1.5 );
        }
        else
            chance = ( int )( chance / 4 );
    }

    if ( IS_AFFECTED(victim,AFF_WEAK_STUN) )
    {
        chance = ( int )( chance * 0.5 );
    }

    if (gsn_forest_fighting->getCommand( )->run(victim, FOREST_DEFENCE)
        && (gsn_forest_fighting->getEffective( victim ) > number_percent( ))) 
    {
        chance = ( int )( chance * 1.2 );
        gsn_forest_fighting->improve( victim, true, ch );
    }

    if (wield && (wield->value0() == WEAPON_FLAIL || wield->value0() == WEAPON_WHIP))
        chance = ( int )( chance * 1.2 );
        
    if (number_percent( ) >= chance + ( skill_level(*gsn_dodge, victim) - ch->getModifyLevel() ) / 2
        || ( !victim->is_npc() && !victim->move ) )
        return false;

    victim->move -= move_dec( victim );

    if (SHADOW(victim))
    {
        msgFightVict( "Ты скачешь вокруг своей тени, пытаясь от нее увернуться." );
        msgFightChar( "%2$^C1 забавно прыгает вокруг своей тени." );
        msgFightRoom( "%2$^C1 забавно прыгает вокруг своей тени." );
        return false;
    }

    msgFightVict( "Ты уворачиваешься от атаки %1$C2." );
    msgFightChar( "%2$^C1 уворачивается от твоей атаки." );
    msgFightRoom( "%2$^C1 уворачивается от атаки %1$C2." );

    if ( number_percent() < (gsn_dodge->getEffective( victim ) / 20 )
        && !(is_flying( ch ) || ch->position < POS_FIGHTING) )
    {
        /* size */
        if (victim->size < ch->size)
            chance += (victim->size - ch->size) * 10;  /* bigger = harder to trip */

        /* dex */
        chance += victim->getCurrStat(STAT_DEX);
        chance -= ch->getCurrStat(STAT_DEX) * 3 / 2;

        if (is_flying( victim ) )
            chance -= 10;

        /* speed */
        if (IS_QUICK(victim))
            chance += 10;

        if (IS_QUICK(ch))
            chance -= 20;

        /* level */
            chance += ( victim->getModifyLevel() - ch->getModifyLevel() ) * 2;

        /* now the attack */
        if ( number_percent() < (chance / 20) )
        {
            oldact("$c1 теряет равновесие и падает вниз!", ch,0,victim,TO_VICT);
            oldact("$C1 уворачивается от твоей атаки, ты теряешь равновесие, и падаешь вниз!", ch,0,victim,TO_CHAR);
            oldact("$C1 уворачивается от атаки $c2, $c1 теряет равновесие и падает вниз.", ch,0,victim,TO_NOTVICT);

            ch->setWait(gsn_trip->getBeats( ));
            ch->position = POS_RESTING;
        }
    }

    gsn_dodge->improve( victim, true, ch );
    return true;
}

bool UndefinedOneHit::defenseCrossBlock( )
{
    int chance;
    Object *def1, *def2;

    if ( !IS_AWAKE(victim) )
        return false;
    
    def1 = get_eq_char( victim, wear_wield );
    def2 = get_eq_char( victim, wear_second_wield );

    if ( def1 == 0 || def2 == 0 )
        return false;

    if ( IS_AFFECTED(victim,AFF_STUN) )
        return false;

    if ( victim->is_npc() )
    {
        chance    = min( static_cast<short>( 35 ), victim->getRealLevel( ) );
    }
    else
    {
        int prof;

        if ( gsn_cross_block->getEffective( victim ) <= 1 )
            return false;
        chance    = gsn_cross_block->getEffective( victim ) / 3;
        prof = victim->getProfession( );

        if (prof == prof_warrior || prof == prof_samurai || prof == prof_paladin)
            chance += chance / 2;
        
    }

    if ( !victim->can_see( ch ) )
    {
        if (number_percent( ) < gsn_blind_fighting->getEffective( victim ))
        {
            gsn_blind_fighting->improve( victim, true, ch );
            chance = ( int )( chance / 1.5 );
        }
        else
            chance = ( int )( chance / 4 );
    }

    if ( victim->is_npc() && victim->move <=0 )
        return false;

    if ( IS_AFFECTED(victim,AFF_WEAK_STUN) )
    {
        chance = ( int )( chance * 0.5 );
    }

    if ( number_percent( ) >= chance + ( skill_level(*gsn_cross_block, victim) - ch->getModifyLevel() ) )
        return false;

    victim->move -= move_dec( victim );

    if (SHADOW(victim))
    {
        msgFightVict( "Тень запутывает тебя." );
        msgFightChar( "Тень %2$C2 лезет со своими советам." );
        msgFightRoom( "Тень %2$C2 лезет со своими советам." );
        return false;
    }

    if (wield 
        && IS_WEAPON_STAT(wield, WEAPON_FADING)
        && !IS_WEAPON_STAT(def1, WEAPON_HOLY) 
        && !IS_WEAPON_STAT(def2, WEAPON_HOLY)) 
    {
        msgFightVict( "%3$^O1 проходит насквозь через твою попытку кросс-блокировать!" );
        msgFightChar( "%3$^O1 проходит сквозь оружие %2$C2!" );
        msgFightRoom( "%3$^O1 %1$C2 проходит сквозь оружие %2$C2!" );

        return false;
    }
    
    msgFightVict( "Ты кросс-блокируешь атаку %1$C2." );
    msgFightChar( "%2$^C1 кросс-блокирует твою атаку." );
    msgFightRoom( "%2$^C1 кросс-блокирует атаку %1$C2." );

    destroyWeapon( );

    if ( number_percent() >  gsn_cross_block->getEffective( victim ) )
    {
        /* size  and weight */
        chance += min(ch->canCarryWeight( ), ch->carry_weight) / 25;
        chance -= min(victim->canCarryWeight( ), victim->carry_weight) / 10;

        if (ch->size < victim->size)
            chance += (ch->size - victim->size) * 25;
        else
            chance += (ch->size - victim->size) * 10;

        /* stats */
        chance += ch->getCurrStat(STAT_STR);
        chance -= victim->getCurrStat(STAT_DEX) * 5/3;

        if (is_flying( ch ))
            chance -= 20;

        /* speed */
        if (IS_QUICK(ch))
            chance += 10;

        if (IS_QUICK(victim))
            chance -= 20;

        /* level */
        chance += (ch->getRealLevel( ) - victim->getRealLevel( )) * 2;

        /* now the attack */
        if (number_percent() < ( chance / 20  ))
        {
            oldact("Тебе не удается удержать равновесие!\nТы падаешь!", ch, 0, victim, TO_VICT);
            oldact("$C1 не может сдержать твою атаку и падает!", ch, 0, victim, TO_CHAR);
            oldact("$C1 не может сдержать ошеломляющую атаку $c2 и падает.", ch, 0, victim, TO_NOTVICT);

            victim->setWait(gsn_bash->getBeats( ));
            victim->position = POS_RESTING;
        }
    }
    gsn_cross_block->improve( victim, true, ch );
    return true;
}


bool UndefinedOneHit::defenseHandBlock( )
{
    int chance;

    if ( !IS_AWAKE(victim) )
        return false;
    
    if (!check_bare_hands( victim ))
        return false;

    if ( IS_AFFECTED(victim,AFF_STUN) )
        return false;
    
    if (!victim->getWearloc( ).isSet( wear_hands ))
        return false;

    if ( victim->is_npc() || !gsn_hand_block->usable( victim ))
        return false;
        
    if ((chance = gsn_hand_block->getEffective( victim )) <= 1)
        return false;

    if ( victim->getProfession( ) == prof_ninja) {
        chance /= 2;
    }
    else
        chance /= 3;

    if ( !victim->can_see( ch ) )
    {
        if (number_percent( ) < gsn_blind_fighting->getEffective( victim ))
        {
            gsn_blind_fighting->improve( victim, true, ch );
            chance = ( int )( chance / 1.5 );
        }
        else
            chance = ( int )( chance / 4 );
    }

    if ( IS_AFFECTED(victim,AFF_WEAK_STUN) )
    {
        chance = ( int )( chance * 0.5 );
    }

    if ( number_percent( ) >= chance +( skill_level(*gsn_hand_block, victim) - ch->getModifyLevel() ) ) 
        return false;
    
    victim->move -= move_dec( victim );

    if(SHADOW(victim))
    {
        msgFightVict( "Тень пинает тебя." );
        msgFightChar( "Тень и %2$C1 играют в кунг-фу." );
        msgFightRoom( "Тень и %2$C1 играют в кунг-фу." );
        return false;
    }

    msgFightVict( "Ты отражаешь руками атаку %1$C2." );
    msgFightChar( "%2$^C1 отражает руками твою атаку." );
    msgFightRoom( "%2$^C1 отражает руками атаку %1$C2." );

    gsn_hand_block->improve( victim, true, ch );
    return true;
}

/*----------------------------------------------------------------------------
 * Damage increasing skills
 *---------------------------------------------------------------------------*/

/*
 * critical strike
 */
void UndefinedOneHit::damEffectCriticalStrike( )
{
    Debug d(ch, "debug_critical", "critical");
    int diceroll, chance, skill, stun_chance, blind_chance;
    Affect baf;

    skill = gsn_critical_strike->getEffective( ch );
    stun_chance = 75; // base thresholds
    blind_chance = 95;
    chance = 0;        
            
    //////////////// ELIGIBILITY CHECKS ////////////////
            
    if ( ch == victim )
        return;

    if ( skill <= 1)
        return;

    if ( dam == 0 )
        return;

    if ( SHADOW(ch) )
        return;
            
    // bare hands messages
    const char *msgVictBasic = "$c1 наносит тебе предательский удар в печень!"; 
    const char *msgCharBasic = "Ты наносишь $C3 предательский удар в печень!";            
    const char *msgVictStun = "{W$c1 обездвиживает тебя предательским ударом по печени!{x"; 
    const char *msgCharStun = "{WТы обездвиживаешь $C4 предательским ударом по печени!{x";
    const char *msgVictBlind = "{y$c1 внезапно ослепляет тебя, ткнув пальцем прямо в глаз!{x";
    const char *msgCharBlind = "{yТы внезапно ослепляешь $C4, ткнув пальцем прямо в глаз!{x";
    const char *msgVictHeart = "{RНеожиданно изловчившись, $c1 наносит мощнейшую серию ударов тебе ПРЯМО В СЕРДЦЕ!!!{x";
    const char *msgCharHeart = "{RНеожиданно изловчившись, ты наносишь мощнейшую серию ударов $C3 ПРЯМО В СЕРДЦЕ!!!{x";

    if (wield) {        
            switch( wield->value0() ) {
            case WEAPON_SWORD:
                        msgVictBasic = "$c1 наносит тебе внезапный удар мечом в печень!"; 
                        msgCharBasic = "Ты наносишь $C3 внезапный удар мечом в печень!";                                     
                        msgVictStun = "{W$c1 обездвиживает тебя внезапным ударом меча в печень!{x"; 
                        msgCharStun = "{WТы обездвиживаешь $C4 внезапным ударом меча в печень!{x";
                        msgVictBlind = "{y$c1 наносит тебе удар мечом в голову!{/Кровь заливает тебе глаза, ты ничего не видишь!{x";
                        msgCharBlind = "{yТы ослепляешь $C4, нанеся удар мечом в голову!{x";
                        msgVictHeart = "{RНеожиданно изловчившись, $c1 вонзает тебе меч ПРЯМО В СЕРДЦЕ!!!{x";
                        msgCharHeart = "{RНеожиданно изловчившись, ты вонзаешь $C3 меч ПРЯМО В СЕРДЦЕ!!!{x";
                        break;
            case WEAPON_DAGGER:
                        msgVictBasic = "$c1 внезапно всаживает тебе кинжал в печень!"; 
                        msgCharBasic = "Ты внезапно всаживаешь $C3 кинжал в печень!";                                    
                        msgVictStun = "{W$c1 обездвиживает тебя, внезапно всаживая кинжал в печень!{x"; 
                        msgCharStun = "{WТы обездвиживаешь $C4, внезапно всаживая кинжал в печень!{x{x";
                        msgVictBlind = "{y$c1 внезапно ослепляет тебя, ткнув кинжалом прямо в глаз!{x";
                        msgCharBlind = "{yТы внезапно ослепляешь $C4, ткнув кинжалом прямо в глаз!{x"; 
                        msgVictHeart = "{RНеожиданно изловчившись, $c1 вонзает тебе кинжал ПРЯМО В СЕРДЦЕ!!!{x";
                        msgCharHeart = "{RНеожиданно изловчившись, ты вонзаешь $C3 кинжал ПРЯМО В СЕРДЦЕ!!!{x";
                        break;
            default:
                        msgVictBlind = "{y$c1 наносит тебе удар в голову!{/Кровь заливает тебе глаза, ты ничего не видишь!{x";
                        msgCharBlind = "{yТы ослепляешь $C4 быстрым ударом в голову!{x"; 
                        msgVictHeart = "{RНеожиданно изловчившись, $c1 наносит тебе удар ПРЯМО В СЕРДЦЕ!!!{x";
                        msgCharHeart = "{RНеожиданно изловчившись, ты наносишь $C3 удар ПРЯМО В СЕРДЦЕ!!!{x";
                        break;
            }
    }            
    // thieves have +10% to blind:              65 / 95 / 100  
    // ninjas and rangers have +10% to stun:    85 / 95 / 100
    // samurai have +10% to strike heart:       75 / 85 / 100
    // everyone else:                           75 / 95 / 100 
    
    if ( ch->getProfession( ) == prof_ranger ) {                    
            if (!RoomUtils::isNature(ch->in_room))
                return;
            msgVictBasic = "$c1 внезапно сотрясает землю мощным ударом!";
            msgCharBasic = "Ты сотрясаешь землю мощным ударом, заставая врасплох $C4!";               
            msgVictStun = "{W$c1 сотрясает землю мощным ударом, обездвиживая тебя!{x";
            msgCharStun = "{WТы сотрясаешь землю мощным ударом, обездвиживая $C4!{x";
            msgVictBlind = "{y$c1 внезапной серией ударов поднимает вихрь листьев, ослепляя тебя!{x";
            msgCharBlind = "{yТы внезапной серией ударов поднимаешь вихрь листьев, ослепляя $C4!{x";
            msgVictHeart = "{R$c1 призывает силу Природы, нанося тебе мощнейший удар прямо в сердце!{x";
            msgCharHeart = "{RТы призываешь силу Природы, нанося $C3 мощнейший удар прямо в сердце!{x";                            
            stun_chance = 85;
    }       
    if ( ch->getProfession( ) == prof_thief ) {
            if ( (!wield) || (wield->value0() != WEAPON_DAGGER) )
                        return;                
            stun_chance = 65;        
    }
    if ( ch->getProfession( ) == prof_ninja ) {
            stun_chance = 85;
    }   
    if ( ch->getProfession( ) == prof_samurai ) {
            if ( (wield) && (wield->value0() == WEAPON_SWORD) ) {
                    msgVictBlind = "{yИспользуя технику кирикаэси, $c1 наносит серию ударов в голову!{/Кровь заливает тебе глаза, ты ничего не видишь!{x";
                    msgCharBlind = "{yИспользуя технику кирикаэси, ты ослепляешь $C4. Мэн!{x";                        
                    msgVictHeart = "{RИспользуя технику кацуги-вадза, $c1 внезапно наносит удар особой силы!!!{x";
                    msgCharHeart = "{RИспользуя технику кацуги-вадза, ты внезапно наносишь $C3 удар особой силы!!!{x";
            }
            if (!wield) {
                    msgVictBasic = "$c1 наносит тебе внезапный удар пяткой в печень!"; 
                    msgCharBasic = "Ты наносишь $C3 внезапный удар пяткой в печень!";                          
                    msgVictStun = "{W$c1 обездвиживает тебя внезапным ударом пяткой в печень!{x"; 
                    msgCharStun = "{WТы обездвиживаешь $C4 внезапным ударом пяткой в печень!{x";                        
            }            
            blind_chance = 85;
    }
                            
    d.log(stun_chance, "stun_chance");
    d.log(blind_chance, "blind_chance");
    d.log(chance, "chance");

    //////////////// PROBABILITY CHECKS ////////////////
        
    chance += skill / 10;
    d.log(chance, "skill");
    chance += skill_level_bonus(*gsn_critical_strike, ch);    
    d.log(chance, "bonus");

    if ( victim->getModifyLevel() > ch->getModifyLevel() ) {
        chance -= ( victim->getModifyLevel() - ch->getModifyLevel() );
        d.log(chance, "lvl");
    }

    if ( victim->getModifyLevel() < ch->getModifyLevel() ) {
        chance += ( ch->getModifyLevel() - victim->getModifyLevel() );
        d.log(chance, "lvl");
    }

    if ( IS_AFFECTED(ch,AFF_WEAK_STUN) ) {
        chance = chance / 2;
        d.log(chance, "stun");
    }

    if (IS_QUICK(ch)) {
        chance += 5;
        d.log(chance, "quick");
    }

    if (IS_QUICK(victim)) {
        chance -= 5;
        d.log(chance, "quick");
    }
    
    d.log(chance, "final chance");

    if ( number_percent() > chance ) {
        gsn_critical_strike->improve( ch, false, victim );        
        return;
    }
        
    //////////////// SUCCESS: CALCULATING EFFECT ////////////////
        
    gsn_critical_strike->improve( ch, true, victim );        
    diceroll = number_percent( );
    d.log(diceroll, "diceroll");

    if (diceroll < stun_chance) {

        // stun only in 15-35% chance, otherwise just damage
        if (diceroll >= 50) {
            victim->setWaitViolence( 2 );
            oldact( msgVictStun, ch, 0, victim, TO_VICT);
            oldact( msgCharStun, ch, 0, victim, TO_CHAR);                    
        } 
        else {
            oldact( msgVictBasic, ch, 0, victim, TO_VICT);
            oldact( msgCharBasic, ch, 0, victim, TO_CHAR);
        }
        dam += (dam * number_range( 2, 5 )) / 5;  // +40-100% damage          
    }
    else if (diceroll < blind_chance) {
        oldact( msgVictBlind, ch, 0, victim, TO_VICT);
        oldact( msgCharBlind, ch, 0, victim, TO_CHAR);
        if ( !IS_AFFECTED(victim,AFF_BLIND) )
        {
            baf.bitvector.setTable(&affect_flags);
            baf.type     = gsn_critical_strike;
            baf.level    = ch->getModifyLevel();
            baf.location = APPLY_HITROLL;
            baf.modifier     = -1 * ch->getModifyLevel() / 10;
            baf.duration     = number_range(1,5);
            baf.bitvector.setValue(AFF_BLIND);
            affect_to_char( victim, &baf );
        }
        dam += dam * number_range( 1, 2 );  // +100-200% damage          
    }
    else {
        oldact( msgVictHeart, ch, 0, victim, TO_VICT);
        oldact( msgCharHeart, ch, 0, victim, TO_CHAR);
        dam += dam * number_range( 2, 5 ); // +200-500% damage            
    }
}

void UndefinedOneHit::damApplyMasterSword( ) 
{
    if (weapon_sn != gsn_sword)
        return;
        
    if (number_percent( ) > gsn_mastering_sword->getEffective( ch ))
        return;

    gsn_mastering_sword->improve( ch, true, victim );
    dam = dam * 150 /100;
}

void UndefinedOneHit::damEffectMasterSword( ) 
{
    Object *katana = wield;

    if (weapon_sn != gsn_sword)
        return;
        
    if (number_percent( ) > gsn_mastering_sword->getEffective( ch ))
        return;

    if (!IS_WEAPON_STAT(katana, WEAPON_KATANA))
        return;
    
    if (!katana->extra_descr || !katana->extra_descr->description)
        return;
    
    if (strstr(katana->extra_descr->description, ch->getNameP( )) == 0)
        return;

    if (immune_check( victim, dam_type, dam_flag ) == RESIST_IMMUNE)
        return;
     
    if (ch->getModifyLevel( ) - victim->getModifyLevel( ) > 10)
        return;
        
    katana->cost++;
    
    if (katana->cost <= 249)
        return;
            
    katana->cost = 0;

    WeaponGenerator()
        .item(katana)
        .skill(gsn_katana)
        .hitrollTier(1)
        .damrollTier(1)
        .incrementHitroll()
        .incrementDamroll();

    oldact("$o1 $c2 загорается {Cголубым светом{x.", ch, katana, 0, TO_ROOM);
    oldact("$o1 в твоей $T руке загорается {Cголубым светом{x.", 
            ch, katana, (secondary ? "левой" : "правой"), TO_CHAR);
}

void UndefinedOneHit::damApplyDeathblow( ) 
{
    int chance;
    
    if (ch->is_npc( ) || !gsn_deathblow->usable( ch, false ))
        return;
    
    chance = gsn_deathblow->getEffective( ch );

    if (victim->is_npc( ) && victim->getNPC( )->behavior && !victim->getNPC( )->behavior->isAfterCharm( )) {
        if (victim->getProfession( )->getFlags( victim ).isSet(PROF_MAGIC))
            chance /= 8;
        else
            chance /= 10;
    }
    else
        chance /= 6;
    
    if (number_percent( ) < chance) {
        int clevel = max( (short)2, ch->getPC( )->getClanLevel( ) );
        int mlevel = ch->getModifyLevel( );
        int min_dam = dam + dam * 2 * clevel * mlevel / (600);
        int max_dam = dam + dam * 4 * clevel * mlevel / (600);

        dam = number_range( min_dam, max_dam );

        oldact("Твои руки наполняются смертоносной силой!",ch,0,0,TO_CHAR);
        oldact("Руки $c2 наполняются смертоносной силой!",ch,0,0,TO_ROOM);
        gsn_deathblow->improve( ch, true, victim );
    }
    else
        gsn_deathblow->improve( ch, false, victim );
}

void UndefinedOneHit::damEffectMasterHand()
{
    Debug d(ch, "debug_ninja", "stun");
    int diceroll, skill, level;
    float chance, skill_mod, stat_mod, level_mod, quick_mod;
    Affect af;

    skill = gsn_mastering_pound->getEffective(ch);
    level = skill_level(*gsn_mastering_pound, ch);

    chance = 0;
    diceroll = number_percent();

    //////////////// BASE MODIFIERS //////////////// TODO: add this to XML
    skill_mod = 0.15;
    stat_mod = 0.01;
    level_mod = 0.01;
    quick_mod = 0.05;

    //////////////// ELIGIBILITY CHECKS ////////////////

    if (skill <= 1)
        return;

    if (dam == 0)
        return;

    if (SHADOW(ch))
        return;

    if (wield)
        return;

    if (diceroll > skill)
        return;

    //////////////// SUCCESS: CALCULATING EFFECT ////////////////

    chance += skill * skill_mod;
    d.log(chance, "skill");
    chance += (ch->getCurrStat(STAT_STR) - victim->getCurrStat(STAT_CON)) * stat_mod * 100;
    d.log(chance, "stats");
    chance += (level - victim->getModifyLevel()) * level_mod * 100;
    d.log(chance, "lvl");

    if ( victim->isAffected(gsn_nerve) ) {
        chance = chance + 5;        
        d.log(chance, "nerve");         
    }
            
    if (IS_AFFECTED(ch, AFF_WEAK_STUN)) {
        chance = chance / 2;
        d.log(chance, "stun");        
    }

    if (IS_QUICK(ch)) {
        chance += quick_mod * 100;
        d.log(chance, "quick");
    }
    if (IS_QUICK(victim)) {
        chance -= quick_mod * 100;
        d.log(chance, "quick");
    }

    chance = max(1, (int)chance); // there's always a chance;
    d.log(chance, "final");

    if (diceroll > chance)
        return;

    if ( (victim->isAffected(gsn_nerve)) && (number_percent() < 20) )
        ch->pecho("С ослабленными нервными окончаниями оглушить противника становится легче.");        
                
    if (!IS_AFFECTED(victim, AFF_WEAK_STUN)) {
        SET_BIT(victim->affected_by, AFF_WEAK_STUN);
        if (ch != victim) {
            oldact("{rТвой удар в голову слегка оглушает $C4!{x", ch, 0, victim, TO_CHAR);
            oldact("{r$c1 слегка оглушает тебя ударом в голову!{x", ch, 0, victim, TO_VICT);
            oldact("{r$c1 слегка оглушает $C4 ударом в голову!{x", ch, 0, victim, TO_NOTVICT);
        } else {
            oldact("{rТвой удар отклонен тебе ж в голову! Ты слегка оглушаешь СЕБЯ!{x", ch, 0, victim, TO_CHAR);
            oldact("{r$c1 слегка оглушает СЕБЯ ударом в голову!{x", ch, 0, victim, TO_NOTVICT);
        }
    } else if (diceroll < (chance / 2) && !IS_AFFECTED(victim, AFF_STUN)) {
        if (ch != victim) {

            if(IS_AFFECTED(victim, AFF_WEAK_STUN)){
                REMOVE_BIT(victim->affected_by,AFF_WEAK_STUN);
            }

            SET_BIT(victim->affected_by, AFF_STUN);

            oldact("{rМощной серией ударов в голову ты сильно оглушаешь $C4!{x", ch, 0, victim, TO_CHAR);
            oldact("{r$c1 сильно оглушает тебя мощной серией ударов в голову!{x", ch, 0, victim, TO_VICT);
            oldact("{r$c1 оглушает $C4 мощной серией ударов в голову!{x", ch, 0, victim, TO_NOTVICT);
        }
    }

    gsn_mastering_pound->improve(ch, true, victim);
}

void UndefinedOneHit::damApplyMasterHand()
{
    Debug d(ch, "debug_mpound", "mpound");
    int diceroll, skill;
    float dam_bonus, stat_mod, level_mod;
    Affect af;

    d.log(dam, "orig damage");

    skill = gsn_mastering_pound->getEffective(ch);
    dam_bonus = 0;
    diceroll = number_percent();

    //////////////// BASE MODIFIERS //////////////// TODO: add this to XML
    stat_mod = 0.01;
    level_mod = 0.01;

    if (wield)
        return;

    if (diceroll > skill)
        return;

    //////////////// SUCCESS: CALCULATING EFFECT ////////////////

    dam_bonus += (ch->getCurrStat(STAT_STR) - 20) * stat_mod * 100; // TODO: this should roll vs. victim's CON instead
    d.log(dam_bonus, "stats");
    dam_bonus += (skill_level(*gsn_mastering_pound, ch) - victim->getModifyLevel()) * level_mod * 100;
    dam_bonus += skill_level(*gsn_mastering_pound, ch) / 10;
    d.log(dam_bonus, "lvl");
    dam_bonus = (int)URANGE(1, (int)dam_bonus, 20);
    d.log(dam_bonus, "final");

    int bonus_dice = dice(dam_bonus, 10) * skill / 100;
    d.log(bonus_dice, "damage bonus");
    dam += bonus_dice;

    int old_bonus_dice = dice( 3 + ch->getModifyLevel() / 10, 10 ) * skill / 100;
    d.log(old_bonus_dice, "{yold damage bonus{x");
}

void UndefinedOneHit::damApplyReligion()
{
    // Cradya followers get more damage from their pets, clan pets excluded.
    if (ch->is_npc() 
            && ch->leader 
            && ch->leader->getReligion() == god_cradya
            && get_eq_char(ch->leader, wear_tattoo) != 0)
    {
        if (!area_is_clan(ch->getNPC()->pIndexData->area)) {         
            dam = dam * 150 / 100;
        }
        return;
    }

    // Phobos and Deimos followers work well together.
    if (get_eq_char(ch, wear_tattoo) != 0) {
        for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room) {
            if (rch == ch)
                continue;
            if (!is_same_group(ch, rch))
                continue;
            if (get_eq_char(rch, wear_tattoo) == 0)
                continue;

            if ((ch->getReligion() == god_deimos && rch->getReligion() == god_phobos)
                || (ch->getReligion() == god_phobos && rch->getReligion() == god_deimos))
            {           
                if (chance(1)) {
                    ch->recho(rch, "%^C1 и %C1 наводят {Rстрах и ужас{x на противников, нанося дополнительный урон!", ch, rch);
                    ch->pecho("Ты и %C1 наводите {Rстрах и ужас{x на противников, нанося дополнительный урон!", rch);
                    rch->pecho("Ты и %C1 наводите {Rстрах и ужас{x на противников, нанося дополнительный урон!", ch);
                }

                dam = dam * 110 / 100;
                return;
            }
        }
    }
}

/*----------------------------------------------------------------------------
 * Chop off a hand 
 *---------------------------------------------------------------------------*/
void UndefinedOneHit::damEffectSlice( ) 
{
    Affect af;
    Object *axe = wield, *arm;
    int chance, timer;
    int sliced_loc;
    DLString name, sideName;
    bool left;
    vector<int> sliced;
    
    if (ch == victim || !axe || victim->is_npc( ))
        return;
   
    if ((chance = gsn_slice->getEffective( ch )) <= 1)
        return;
    
    if (axe->value3() != DAMW_SLASH && axe->value3() != DAMW_CHOP && axe->value3() != DAMW_SLICE)
        return;
    
    if (number_bits(1)) {
        sliced_loc = wear_wrist_l;
        left = true;
        sideName = "левую";
    }
    else {
        sliced_loc = wear_wrist_r;
        left = false;
        sideName = "правую";
    }

    if (!victim->getWearloc( ).isSet( sliced_loc ))
        return;

    if (number_range( 1, 100 ) > 1)
        return;

    chance = (chance * ch->getSkill( weapon_sn )) / 100;
    
    chance += ch->getCurrStat(STAT_DEX ) - victim->getCurrStat(STAT_DEX );
    chance += 2 * (ch->getCurrStat(STAT_STR ) - victim->getCurrStat(STAT_STR ));

    chance += (skill_level(*gsn_slice, ch) - victim->getModifyLevel()) * 2;

    if (!IS_WEAPON_STAT( axe, WEAPON_SHARP ))
        chance -= chance / 10;

    if (!IS_WEAPON_STAT( axe, WEAPON_TWO_HANDS ))
        chance -= chance / 10;
    
    if (number_percent( ) > chance) {
        oldact("Твое оружие скользит по запястью $C2.", ch, 0, victim, TO_CHAR);
        oldact("$o1 $c2 скользит по твоему запястью.", ch, axe, victim, TO_VICT);
        oldact("$o1 $c2 скользит по запястью $C2.", ch, axe, victim, TO_NOTVICT);
        gsn_slice->improve( ch, false, victim );
        return;
    }
    
    ch->setWait(gsn_slice->getBeats( ) );
    victim->setWait(2 * gsn_slice->getBeats( ) );

    /* timer */
    timer = std::max( 2, ch->getModifyLevel( ) / 10 ); 
    if (victim->getRace( )->getAff( ).isSet( AFF_REGENERATION ))
        timer /= 2;

    /* drop sliced arm */
    arm = create_object( get_obj_index( OBJ_VNUM_SLICED_ARM ), victim->getRealLevel( ) );
    
    name = victim->getNameP( '2' );
    arm->fmtShortDescr( arm->getShortDescr( ), name.c_str( ) );
    arm->fmtDescription( arm->getDescription( ), name.c_str( ) );
    arm->from = str_dup(name.c_str());
    arm->timer = timer;

    if (arm->item_type == ITEM_FOOD) {
        if (IS_SET(victim->form,FORM_POISON))
            arm->value3(1);
        else if (!IS_SET(victim->form,FORM_EDIBLE))
            arm->item_type = ITEM_TRASH;
    }

    obj_to_room( arm, victim->in_room );
    oldact("{R$c1 отруби$gло|л|ла тебе $t руку!{x", ch, sideName.c_str( ), victim, TO_VICT );
    oldact("$c1 отруби$gло|л|ла $C3 $t руку!", ch, sideName.c_str( ), victim, TO_NOTVICT );
    oldact("{RТы отрубаешь $t руку $C3!{x", ch, sideName.c_str( ), victim, TO_CHAR );
    gsn_slice->improve( ch, true, victim );
    oldact("Отрубленная рука $c2 падает на землю.", victim, 0, 0, TO_ROOM );
    oldact("Твоя отрубленная рука падает на землю.", victim, 0, 0, TO_CHAR );

    /* affect */
    af.type  = gsn_slice;
    af.level = ch->getModifyLevel( );
    af.duration = timer;
    af.global.setRegistry( wearlocationManager );

    /* undress */
    sliced.push_back( left ? wear_wrist_l : wear_wrist_r );
    sliced.push_back( left ? wear_finger_l : wear_finger_r );
    sliced.push_back( left ? wear_tat_wrist_l : wear_tat_wrist_r );
    sliced.push_back( wear_second_wield );

    if (left) {
        sliced.push_back( wear_shield );
        sliced.push_back( wear_hold );
    }
    else {
        sliced.push_back( wear_wield );
    }

    if (!victim->getWearloc( ).isSet( left ? wear_wrist_r : wear_wrist_l )) {
        sliced.push_back( wear_hands );
        sliced.push_back( wear_arms );
    }
    
    for (unsigned int s = 0; s < sliced.size( ); s++) {
        Wearlocation *loc = wearlocationManager->find( sliced[s] );
        Object *obj = loc->find( victim );

        if (obj) {
            loc->unequip( obj );

            if (obj->item_type == ITEM_TATTOO) {
                oldact("$o1 медленно исчезает.", victim, obj, 0, TO_CHAR);
                extract_obj(obj);
            }
            else if (!IS_SET( obj->extra_flags, ITEM_NODROP )) {
                obj_from_char( obj );
                obj_to_room( obj, victim->in_room );
                oldact("$o1 падает на землю.", victim, obj, 0, TO_ALL);
            }
        }
        
        af.global.set( sliced[s] );
    }
    
    affect_to_char( victim, &af );
}


/*----------------------------------------------------------------------------
 * Destroy equipment, shield, weapon 
 *---------------------------------------------------------------------------*/
void UndefinedOneHit::damEffectDestroyEquipment( )
{
    Object *destroy = NULL, *obj;
    int count = 0;
    int chances;
    
    if (!wield || victim->is_npc( ) || ch == victim)
        return;

    if (!chance( 6 ))
        return;

    for (obj = victim->carrying; obj; obj = obj->next_content) 
        if (chance( obj->wear_loc->getDestroyChance( ) ))
            if (canDestroy( obj ))
                if (number_range( 0, count++ ) == 0) 
                    destroy = obj;

    if (!destroy)
        return;
        
    chances = getDestroyChance( destroy );

    if (number_percent( ) < chances && chances > 50)
        damage_to_obj( ch, wield, destroy, chances / 5 );
}

bool UndefinedOneHit::canDestroy( Object *obj )
{
    if (chance( 11 ))
        return false;
    if (ch->getModifyLevel( ) < victim->getModifyLevel( ) - 10)
        return false;
    if (obj->pIndexData->limit != -1)
        return false;
    if (number_percent( ) > skill)
        return false;
    if (material_is_flagged( obj, MAT_INDESTR ))
        return false;
    return true;
}


int UndefinedOneHit::getDestroyChance( Object *destroy )
{
    int chance;
    
    if (!wield)
        return 0;

    if (material_is_typed( wield, MAT_METAL )) {
        chance = 35; 

        if (material_is_flagged( wield, MAT_TOUGH ))
            chance += 15; 

        if (material_is_typed( destroy, MAT_METAL ))  
            chance -= 20;
        else                         
            chance += 35; 
    }
    else {
        chance = 25;

        if (material_is_typed( destroy, MAT_METAL ))  
            chance -= 20;
    }

    chance += (ch->getModifyLevel( ) - victim->getModifyLevel( )) / 5;
    chance += (wield->level - destroy->level) / 2;

    if (IS_WEAPON_STAT(wield,WEAPON_SHARP))
        chance += 20; 

    if (weapon_sn == gsn_axe) 
        chance += 20; 
        
    if (IS_OBJ_STAT( destroy, ITEM_BLESS)) 
        chance -= 10;
    if (IS_OBJ_STAT( destroy, ITEM_MAGIC)) 
        chance -= 20;
        
    chance += skill - 85 ;
    chance += ch->getCurrStat(STAT_STR);
    return chance;
}

void UndefinedOneHit::destroyShield( )
{
    Object *shield;
    int chances;
    
    if (!wield || victim->is_npc( ) || ch == victim)
        return;

    if (!chance( 6 ))
        return;
        
    if (!( shield = wear_shield->find( victim ) ))
        return;

    if (!canDestroy( shield ))
        return;

    chances = getDestroyChance( shield );

    if (number_percent( ) < chances && chances > 20)
        damage_to_obj( ch, wield, shield, chances / 4 );
}

void UndefinedOneHit::destroyWeapon( )
{
    Object *weapon;
    int chances;
    
    if (!wield || victim->is_npc( ) || ch == victim)
        return;

    if (!chance( 6 ))
        return;
        
    if (!( weapon = wear_wield->find( victim ) ))
        return;

    if (!canDestroy( weapon ))
        return;

    chances = getDestroyChance( weapon );

    if (number_percent( ) < chances / 2 && chances > 20)
        damage_to_obj( ch, wield, weapon, chances / 4 );
}
