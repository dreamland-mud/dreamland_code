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
#include "race.h"
#include "religion.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "object.h"
#include "room.h"
#include "clanreference.h"
#include "areabehaviormanager.h"

#include "dreamland.h"
#include "fight.h"
#include "material.h"
#include "immunity.h"
#include "handler.h"
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
PROF(universal);
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
            act_p("$c1 разбивается на мелкие осколки.",victim,0,0,TO_ROOM,POS_RESTING);
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
    gsn_enhanced_damage->getCommand( )->run( ch, victim, dam );;
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
    damEffectGroundStrike( );
}

bool UndefinedOneHit::mprog_hit()
{
    DLString damType = damage_table.name( dam_type );
    
    FENIA_CALL( victim, "Hit", "CisO", ch, dam, damType.c_str( ), wield );
    FENIA_NDX_CALL( victim->getNPC( ), "Hit", "CCisO", victim, ch, dam, damType.c_str( ), wield );
    return false;
}

void UndefinedOneHit::postDamageEffects( )
{
    damEffectDestroyEquipment( );
    damEffectFeeble( );
    damEffectFunkyWeapon( );
    damEffectSlice( );
}

void UndefinedOneHit::message( )
{
    RussianString noun(attack_table[attack].noun,
                       attack_table[attack].gender);

    if (immune) {
        if (ch == victim) {
            msgRoom("%1$^O1 %2$C2 бессил%1$Gьно|ен|ьна против %2$P4 сам%2$Gого|ого|ой|их", &noun, ch);
            msgChar("Тебе повезло, у тебя иммунитет к этому");
        }
        else {
            msgRoom("%1$^O1 %2$C2 бессил%1$Gьно|ен|ьна против %3$C2", &noun, ch, victim);
            msgChar("%1$^T1 %1$O1 бессил%1$Gьно|ен|ьна против %2$C2", &noun, victim);
            msgVict("Против тебя %2$O1 %1$C2 бессил%2$Gьно|ен|ьна", ch, &noun);
        }
    }
    else {
        if (ch == victim) {
            msgRoom( "%1$^O1 %2$C2\6себя", &noun, ch );
            msgChar( "%1$^T1 %1$O1\6тебя", &noun );
        }
        else {
            if ( dam == 0 )
            {
                msgRoom( "%1$^O1 %2$C2\6%3$C2", &noun, ch, victim );
                msgChar( "%1$^T1 %1$O1\6%2$C2", &noun, victim );
            }
            else {
                msgRoom( "%1$^O1 %2$C2\6%3$C4", &noun, ch, victim );
                msgChar( "%1$^T1 %1$O1\6%2$C4", &noun, victim );
            }
            msgVict( "%1$^O1 %2$C2\6тебя", &noun, ch );
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
    prof = victim->getTrueProfession( );

    if (prof == prof_warrior || prof == prof_samurai || prof == prof_paladin)
        chance += chance / 5;
    else if (prof == prof_anti_paladin && victim->getClan( ) == clan_shalafi) 
            chance /= 2;

    if (wield && (wield->value[0] == WEAPON_FLAIL || wield->value[0] == WEAPON_WHIP ))
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


    if (number_percent( ) >= chance + victim->getModifyLevel() - ch->getModifyLevel())
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
        msgFightVict( "%3$^O1 passes straight through your attempt to parry!" );
        
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
            act("Ты не можешь устоять на ногах!",ch,0,victim,TO_VICT);
            act("Ты падаешь вниз!",ch,0,victim,TO_VICT);
            act("$C1 не может устоять на ногах и падает вниз!", ch,0,victim,TO_CHAR);
            act("$C1 пытается парировать мощный удар $c1, но не может устоять на ногах.", ch,0,victim,TO_NOTVICT);

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
    prof = victim->getTrueProfession( );

    if (prof == prof_warrior || prof == prof_samurai || prof == prof_paladin)
        chance += 10;

    if (wield) { 
        if (wield->value[0] == WEAPON_FLAIL)
            chance /= 2;
        if (wield->value[0] == WEAPON_WHIP)
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
    
    if ( number_percent( ) >= chance + victim->getModifyLevel() - ch->getModifyLevel()
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
    prof = victim->getTrueProfession( );

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

    if (wield && (wield->value[0] == WEAPON_FLAIL || wield->value[0] == WEAPON_WHIP))
        chance = ( int )( chance * 1.2 );
        
    if (number_percent( ) >= chance + ( victim->getModifyLevel() - ch->getModifyLevel() ) / 2
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
            act("$c1 теряет равновесие и падает вниз!", ch,0,victim,TO_VICT);
            act("$C1 уворачивается от твоей атаки, ты теряешь равновесие, и падаешь вниз!", ch,0,victim,TO_CHAR);
            act("$C1 уворачивается от атаки $c2, $c1 теряет равновесие и падает вниз.", ch,0,victim,TO_NOTVICT);

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
        prof = victim->getTrueProfession( );

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

    if ( number_percent( ) >= chance + ( victim->getModifyLevel() - ch->getModifyLevel() ) )
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
        msgFightVict( "%3$^O1 passes straight through your attempt to cross block!" );
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
            act("Тебе не удается удержать равновесие!\nТы падаешь!", ch, 0, victim, TO_VICT);
            act("$C1 не может сдержать твою атаку и падает!", ch, 0, victim, TO_CHAR);
            act("$C1 не может сдержать ошеломляющую атаку $c2 и падает.", ch, 0, victim, TO_NOTVICT);

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

    if ( victim->getTrueProfession( ) == prof_ninja) {
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

    if ( number_percent( ) >= chance +( victim->getModifyLevel() - ch->getModifyLevel() ) ) 
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
 *  from Anatolia 3.0
 */
void UndefinedOneHit::damEffectGroundStrike( ) 
{
    int diceroll, levelDiff, chance;
    Affect baf;

    if (ch == victim)
        return;

    if (!dam)
        return;

    if (( chance = gsn_ground_strike->getEffective( ch ) ) <= 1)
        return;
    
    if ( ch->in_room->sector_type != SECT_HILLS
                && ch->in_room->sector_type != SECT_MOUNTAIN
                && ch->in_room->sector_type != SECT_FOREST
                && ch->in_room->sector_type != SECT_FIELD )
        return;

    if(SHADOW(ch))
        return;

    diceroll = number_range( 0, 100 );
    levelDiff = victim->getModifyLevel( ) - ch->getModifyLevel( );

    if (levelDiff > 0)
        diceroll += levelDiff * 2;
    else 
        diceroll += levelDiff;

    if ( diceroll <= (chance/3) ) {  
        gsn_ground_strike->improve( ch, true, victim );
        dam += dam * diceroll/200;
    }  

    if ( diceroll > (chance/15) ) 
        return;

    diceroll = number_percent( );
    gsn_ground_strike->improve( ch, true, victim );
  
    if( diceroll < 75 ) {  
        act_p( "{RЗемля трясется под твоими ногами!{x", ch, 0, victim, TO_VICT,POS_RESTING );
        act_p( "{RЗемля под ногами $C2 начинает трястись, повинуясь твоему приказу!{x", ch, NULL, victim, TO_CHAR,POS_RESTING );
        
        victim->setWaitViolence( 2 );
        dam += (dam * number_range( 2, 5 )) / 5;                        
    } 
    else if (diceroll < 95) {   
        act_p( "{yТы ослепле$Gно|н|на атакой $c2!{x", ch, NULL, victim, TO_VICT, POS_RESTING );
        act_p( "{yТы ослепляешь $C4 своей атакой!{x", ch, NULL, victim, TO_CHAR, POS_RESTING );

        if (!IS_AFFECTED(victim,AFF_BLIND)) {
              baf.where    = TO_AFFECTS;
              baf.type = gsn_ground_strike;
              baf.level = ch->getModifyLevel( ); 
              baf.location = APPLY_HITROLL; 
              baf.modifier = -4;
              baf.duration = number_range(1,5); 
              baf.bitvector = AFF_BLIND;
              affect_to_char( victim, &baf );
        }  

        dam += dam * number_range( 1, 2 );                        
    } 
    else {
        act_p( "{R$c1 вырывает твое сердце! OUCH!!{x", ch, NULL, victim, TO_VICT ,POS_RESTING ); 
        act_p( "{RТы вырываешь сердце $C2!{x", ch, NULL, victim, TO_CHAR ,POS_RESTING );

        dam += dam * number_range( 2, 5 );                        
    }
}

/*
 * critical strike
 */
void UndefinedOneHit::damEffectCriticalStrike( )
{
    int diceroll, chance;
    Affect baf;
    
    if (ch == victim)
        return;

    if (( chance = gsn_critical_strike->getEffective( ch ) ) <= 1)
        return;

    if (dam == 0)
        return;

    if ( get_eq_char(ch,wear_wield) != 0
            && get_eq_char(ch,wear_second_wield) != 0
            && number_percent() > HEALTH(ch))
        return;

    if(SHADOW(ch))
        return;

    diceroll = number_range( 0, 100 );
    if ( victim->getRealLevel( ) > ch->getRealLevel( ) )
        diceroll += ( victim->getModifyLevel() - ch->getModifyLevel() ) * 2;
    if ( victim->getRealLevel( ) < ch->getRealLevel( ) )
        diceroll -= ( ch->getModifyLevel() - victim->getModifyLevel() );

    if (diceroll <= (chance/2)) {
        gsn_critical_strike->improve( ch, true, victim );
        dam += dam * diceroll/200;
    }

    if (diceroll > (chance/13))
        return;
   
    diceroll = number_percent( );
    gsn_critical_strike->improve( ch, true, victim );

    if (diceroll < 75) {
        act_p( "{R$c1 бросает тебя умелым движением!{x", ch, 0, victim, TO_VICT,POS_RESTING);
        act_p( "{RТы бросаешь $C4 умелым движением!{x", ch, 0, victim, TO_CHAR,POS_RESTING);

        victim->setWaitViolence( 2 );
        dam += (dam * number_range( 2, 5 )) / 5;            
    }
    else if (diceroll < 95) {
        act_p( "{y$c1 ослепляет тебя своей атакой!{x", ch, 0, victim, TO_VICT ,POS_RESTING);
        act_p( "{yТы ослепляешь $C4 своей атакой!{x", ch, 0, victim, TO_CHAR,POS_RESTING);

        if ( !IS_AFFECTED(victim,AFF_BLIND) )
        {
            baf.where    = TO_AFFECTS;
            baf.type     = gsn_critical_strike;
            baf.level    = ch->getModifyLevel();
            baf.location     = APPLY_HITROLL;
            baf.modifier     = -4;
            baf.duration     = number_range(1,5);
            baf.bitvector    = AFF_BLIND;
            affect_to_char( victim, &baf );
        }
        dam += dam * number_range( 1, 2 );            
    }
    else {
        act_p( "{r$c1 подрезает тебя больно! ОЙ!!{x", ch, 0, victim, TO_VICT ,POS_RESTING);
        act_p( "{rТы подрезаешь $C4 больно!  Это действительно больно!{x", ch, 0, victim, TO_CHAR ,POS_RESTING);
        dam += dam * number_range( 2, 5 );            
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
    Affect *paf;
    int old_mod;
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

    paf = katana->affected->affect_find(gsn_katana);
    if (!paf)
        return;
    
    if (paf->level == 120)
        return;

    old_mod = paf->modifier;            
    paf->modifier = min(paf->modifier+1, ch->getModifyLevel() / 3);
    ch->hitroll += paf->modifier - old_mod;
    
    if (paf->next != 0) {
        paf->next->modifier = paf->modifier;
        ch->damroll += paf->modifier - old_mod;
    }
    
    act("$o1 $c2 загорается {Cголубым светом{x.", ch, katana,0, TO_ROOM);
    act("$o1 в твоей $T руке загорается {Cголубым светом{x.", 
            ch, katana, (secondary ? "левой" : "правой"), TO_CHAR);
}

void UndefinedOneHit::damApplyDeathblow( ) 
{
    int chance;
    
    if (ch->is_npc( ) || !gsn_deathblow->usable( ch, false ))
        return;
    
    chance = gsn_deathblow->getEffective( ch );

    if (victim->is_npc( ) && victim->getNPC( )->behavior && !victim->getNPC( )->behavior->isAfterCharm( )) {
        if (victim->getTrueProfession( )->getFlags( victim ).isSet(PROF_MAGIC))
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
        dam = ch->applyCurse( dam );

        act("Твои руки наполняются смертоносной силой!",ch,0,0,TO_CHAR);
        act("Руки $c2 наполняются смертоносной силой!",ch,0,0,TO_ROOM);
        gsn_deathblow->improve( ch, true, victim );
    }
    else
        gsn_deathblow->improve( ch, false, victim );
}

void UndefinedOneHit::damEffectMasterHand( )
{
    if (wield)
        return;
        
    if (number_percent( ) > gsn_mastering_pound->getEffective( ch ))
        return;
    
    if (!chance( number_range( 10, 30 ) ))
        return;
    
    SET_BIT(victim->affected_by,AFF_WEAK_STUN);
    
    if (ch != victim) {
        act("{rТы оглушаешь $C4 своим ударом!{x", ch,0,victim,TO_CHAR);
        act("{r$c1 оглушает тебя своим ударом!{x", ch,0,victim,TO_VICT);
        act("{r$c1 оглушает $C4 своим ударом!{x", ch,0,victim,TO_NOTVICT);
    } else {
        act("{rТы оглушаешь себя!{x", ch,0,victim,TO_CHAR);
        act("{r$c1 оглушает себя!{x", ch,0,victim,TO_NOTVICT);
    }

    gsn_mastering_pound->improve( ch, true, victim );
}

void UndefinedOneHit::damApplyMasterHand( )
{
    if (wield)
        return;
        
    if (number_percent( ) > gsn_mastering_pound->getEffective( ch ))
        return;
    
    gsn_mastering_pound->improve( ch, true, victim );
    int level = skill_level(*gsn_mastering_pound, ch);
    dam += dice( 3 + level / 10, 10 ) * skill / 100;        
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
            if (ch->getReligion() == god_deimos && rch->getReligion() != god_phobos)
                continue;
            if (ch->getReligion() == god_phobos && rch->getReligion() != god_deimos)
                continue;
            
            dam = dam * 110 / 100;
            return;
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
    
    if (axe->value[3] != DAMW_SLASH && axe->value[3] != DAMW_CHOP && axe->value[3] != DAMW_SLICE)
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
        act_p("Твое оружие скользит по запястью $C2.", ch, 0, victim, TO_CHAR, POS_RESTING);
        act_p("$o1 $c2 скользит по твоему запястью.", ch, axe, victim, TO_VICT, POS_RESTING);
        act_p("$o1 $c2 скользит по запястью $C2.", ch, axe, victim, TO_NOTVICT, POS_RESTING);
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
            arm->value[3] = 1;
        else if (!IS_SET(victim->form,FORM_EDIBLE))
            arm->item_type = ITEM_TRASH;
    }

    obj_to_room( arm, victim->in_room );
    act( "{R$c1 отруби$gло|л|ла тебе $t руку!{x", ch, sideName.c_str( ), victim, TO_VICT );
    act( "$c1 отруби$gло|л|ла $C3 $t руку!", ch, sideName.c_str( ), victim, TO_NOTVICT );
    act( "{RТы отрубаешь $t руку $C3!{x", ch, sideName.c_str( ), victim, TO_CHAR );
    gsn_slice->improve( ch, true, victim );
    act( "Отрубленная рука $c2 падает на землю.", victim, 0, 0, TO_ROOM );
    act( "Твоя отрубленная рука падает на землю.", victim, 0, 0, TO_CHAR );

    /* affect */
    af.where = TO_LOCATIONS;
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
                act("$o1 медленно исчезает.", victim, obj, 0, TO_CHAR);
                extract_obj(obj);
            }
            else if (!IS_SET( obj->extra_flags, ITEM_NODROP )) {
                obj_from_char( obj );
                obj_to_room( obj, victim->in_room );
                act_p( "$o1 падает на землю.", victim, obj, 0, TO_ALL, POS_RESTING );
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
