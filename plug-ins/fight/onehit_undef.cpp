/* $Id: onehit_undef.cpp,v 1.1.2.18 2010-09-01 21:20:44 rufina Exp $
 * 
 * ruffina, 2004
 */
#include <string.h>

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
#include "fight_extract.h"
#include "dreamland.h"
#include "debug_utils.h"
#include "fight.h"
#include "weapongenerator.h"
#include "material.h"
#include "immunity.h"
#include "loadsave.h"
#include "skill_utils.h"
#include "move_utils.h"
#include "string_utils.h"
#include "profflags.h"
#include "act.h"

#include "configurable.h"
#include "multiinflectedstring.h"
#include "merc.h"
#include "vnum.h"
#include "def.h"
#include "l10n.h"

CLAN(shalafi);

PROF(none);
PROF(warrior);
PROF(paladin);
PROF(anti_paladin);
PROF(ninja);
PROF(ranger);
PROF(samurai);
PROF(thief);
PROF(druid);

GSN(axe);
GSN(bash);
GSN(bat_swarm);
GSN(blind_fighting);
GSN(blink);
GSN(critical_strike);
GSN(cross_block);
GSN(deathblow);
GSN(dodge);
GSN(forest_fighting);
GSN(hand_block);
GSN(katana);
GSN(liturgy);
GSN(mastering_pound);
GSN(mastering_sword);
GSN(nerve);
GSN(parry);
GSN(shapeshift);
GSN(shield_block);
GSN(slice);
GSN(sword);
GSN(trip);

WEARLOC(tat_wrist_l);
WEARLOC(tat_wrist_r);

RELIG(cradya);
RELIG(phobos);
RELIG(deimos);

/*----------------------------------------------------------------------------
 * Hit by weapon or bare hands
 *---------------------------------------------------------------------------*/
UndefinedOneHit::UndefinedOneHit( Character *ch, Character *victim, bool secondary, string command )
            : Damage( ch, victim, 0, 0, DAMF_WEAPON ), 
              WeaponOneHit( ch, victim, secondary, command )
{
    deathReason = "melee";
}

bool UndefinedOneHit::canDamage( )
{
    if (!OneHit::canDamage( ))
        return false;

    if (!dam) 
        return true;

    if (ch != victim) {
        if (victim->is_mirror( )) {
            oldact(_("$c1 разбивается на мелкие осколки."),victim,0,0,TO_ROOM);
            extract_char(victim);
            return false;
        }
                
        if (gsn_parry->getCommand()->apply(ch, victim, secondary)
            || gsn_hand_block->getCommand()->apply(ch, victim, secondary)
            || gsn_bat_swarm->getCommand()->apply(ch, victim, secondary) 
            || gsn_blink->getCommand()->apply(ch, victim, secondary) 
            || gsn_shield_block->getCommand()->apply(ch, victim, secondary)
            || gsn_cross_block->getCommand()->apply(ch, victim, secondary)
            || gsn_dodge->getCommand()->apply(ch, victim, secondary))
        {
            return false;
        }
    }

    return true;
}

void UndefinedOneHit::protectPrayer( )
{
    if (victim->isAffected( gsn_liturgy )) 
        dam -= dam * (3 + victim->getModifyLevel( ) / 10) / 100;
}

/** Prevent attacks with chopped upper limbs. */
bool UndefinedOneHit::checkHands( )
{
    //TO-DO: provide better logic later, allow blobs, beasts etc. to hit without hands
    if (Char::lostWearloc(ch, wear_hands))
        return false;

    WearlocationReference &attackingHand = secondary ? wear_wrist_l : wear_wrist_r;
    if (Char::lostWearloc(ch, attackingHand))
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
    damapply_class(ch, dam);
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
        if (paf->type->getAffect() && paf->type->getAffect()->onHit(SpellTarget::Pointer(NEW, victim), paf, ch, dam, damType.c_str(), wield))
            return true;

    return false;
}

void UndefinedOneHit::postDamageEffects( )
{
    damEffectDestroyEquipment( );
    damEffectFeeble( );
    damEffectFunkyWeapon( );
    gsn_slice->getCommand()->apply(ch, victim, secondary);
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

    MultiMessage msgAll;

    if (wield->value0() == WEAPON_SWORD)
        msgAll = _("{mРаз-два, раз-два! Горит трава, взы-взы стрижает меч!{x");
    else if (wield->value0() == WEAPON_AXE)
        msgAll = _("{mРаз-два! Раз-два! Горит трава, взы-взы свирчит топор!{x");
    else if (wield->value0() == WEAPON_POLEARM)
        msgAll = _("{mРаз-два! Раз-два! Горит трава, взы-взы свирчит бердыш!{x");
    else if (wield->value0() == WEAPON_WHIP)
        msgAll = _("{mРаз-два! Раз-два! Горит трава, взы-взы стрижает плеть!{x");
    else if (wield->value0() == WEAPON_DAGGER)
        msgAll = _("{mРаз-два! Раз-два! Горит трава, взы-взы стрижает нож!{x");
    else
        msgAll = _("{mРаз-два! Раз-два! Горит трава, взы-взы свирчит клинок!{x");

    ch->in_room->echo(POS_RESTING, msgAll);

    if (!chance(IS_GOOD(ch) ? 20 : 10))
        return;

    MultiMessage msgVict, msgOther;
    if (wield->value0() == WEAPON_SWORD) {
        msgOther = _("{mУва! Ува! И %1$C2 голова барабардает с плеч!{x");
        msgVict = _("{mУва! И твоя голова барабардает с плеч!{x");
    } else if (wield->value0() == WEAPON_AXE) {
        msgOther = _("{mУва! %1$^C1 без головы остал%1$Gось|ся|ась|ись с этих пор!{x");
        msgVict = _("{mУва! И ты без головы остал%1$Gось|ся|ась|ись с этих пор!{x");
    } else if (wield->value0() == WEAPON_POLEARM) {
        msgOther = _("{mУва! %1$C2 котелок скосило как камыш!{x");
        msgVict = _("{mУва! Ува! Твой котелок скосило как камыш!{x");
    } else if (wield->value0() == WEAPON_WHIP) {
        msgOther = _("{mУва! %1$C3 с головой не подружиться впредь!{x");
        msgVict = _("{mУва! И тебе с головой не подружиться впредь!{x");
    } else if (wield->value0() == WEAPON_DAGGER) {
        msgOther = _("{mУва! Ты голову %1$C2 у ног своих найдешь!{x");
        msgVict = _("{mУва! Ты голову свою у ног своих найдешь!{x");
    } else {
        msgOther = _("{mУва! %1$C2 голова лежит у твоих ног!{x");
        msgVict = _("{mУва! И твоя голова лежит у твоих ног!{x");
    }

    victim->pecho(msgVict, victim);
    victim->recho(msgOther, victim);
    victim->recho(_("%^C1 уже ТРУП!!"), victim);
    victim->pecho(_("Тебя УБИЛИ!"));

    eventBus->publish(CharDeathEvent(victim, ch, 0, "vorpal", dam_type));
    throw VictimDeathException( );
}

/*-----------------------------------------------------------------------------
 * Trilingual attack nouns (Trello 2594)
 *
 * attack_table keeps the RU noun (byte-identical); en/ua forms live in
 * config/fight/attack_nouns.json, indexed 1:1 with attack_table. A RU viewer --
 * or any viewer with a missing en/ua cell -- gets the RU InflectedString exactly
 * as before, so this is zero-regression.
 *----------------------------------------------------------------------------*/
struct AttackNounRow {
    DLString en, ua;
    void fromJson(const Json::Value &v)
    {
        en = v["en"].asString();
        ua = v["ua"].asString();
    }
};
static json_vector<AttackNounRow> attackNouns;

CONFIGURABLE_LOADED(fight, attack_nouns)
{
    attackNouns.fromJson(value);
}

void UndefinedOneHit::message( )
{
    DLString en, ua;
    if (attack >= 0 && attack < (int)attackNouns.size()) {
        en = attackNouns[attack].en;
        ua = attackNouns[attack].ua;
    }
    MultiInflectedString noun(attack_table[attack].noun, en, ua,
                              attack_table[attack].gender);

    if (immune) {
        if (ch == victim) {
            msgRoom(_("%2$^O1 %3$C2 бессил%2$Gьно|ен|ьна|ьны против %3$P4 сам%3$Gого|ого|ой|их"), dam, &noun, ch);
            msgChar(_("Тебе повезло, у тебя иммунитет к этому"), dam);
        }
        else {
            msgRoom(_("%2$^O1 %3$C2 бессил%2$Gьно|ен|ьна|ьны против %4$C2"), dam, &noun, ch, victim);
            msgChar(_("%2$^T1 %2$O1 бессил%2$Gьно|ен|ьна|ьны против %3$C2"), dam, &noun, victim);
            msgVict(_("Против тебя %3$O1 %2$C2 бессил%3$Gьно|ен|ьна"), dam, ch, &noun);
        }
    }
    else {
        if (ch == victim) {
            msgRoom( _("%2$^O1 %3$C2\6себя"), dam, &noun, ch );
            msgChar( _("%2$^T1 %2$O1\6тебя"), dam, &noun );
        }
        else {
            if ( dam == 0 )
            {
                msgRoom( _("%2$^O1 %3$C2\6%4$C2"), dam, &noun, ch, victim );
                msgChar( _("%2$^T1 %2$O1\6%3$C2"), dam, &noun, victim );
            }
            else {
                msgRoom( _("%2$^O1 %3$C2\6%4$C4"), dam, &noun, ch, victim );
                msgChar( _("%2$^T1 %2$O1\6%3$C4"), dam, &noun, victim );
            }
            msgVict( _("%2$^O1 %3$C2\6тебя"), dam, &noun, ch );
        }
    }
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
    MultiMessage msgVictBasic = _("$c1 наносит тебе предательский удар в печень!");
    MultiMessage msgCharBasic = _("Ты наносишь $C3 предательский удар в печень!");
    MultiMessage msgVictStun = _("{W$c1 обездвиживает тебя предательским ударом по печени!{x");
    MultiMessage msgCharStun = _("{WТы обездвиживаешь $C4 предательским ударом по печени!{x");
    MultiMessage msgVictBlind = _("{y$c1 внезапно ослепляет тебя, ткнув пальцем прямо в глаз!{x");
    MultiMessage msgCharBlind = _("{yТы внезапно ослепляешь $C4, ткнув пальцем прямо в глаз!{x");
    MultiMessage msgVictHeart = _("{RНеожиданно изловчившись, $c1 наносит мощнейшую серию ударов тебе ПРЯМО В СЕРДЦЕ!!!{x");
    MultiMessage msgCharHeart = _("{RНеожиданно изловчившись, ты наносишь мощнейшую серию ударов $C3 ПРЯМО В СЕРДЦЕ!!!{x");

    if (wield) {        
            switch( wield->value0() ) {
            case WEAPON_SWORD:
                        msgVictBasic = _("$c1 наносит тебе внезапный удар мечом в печень!");
                        msgCharBasic = _("Ты наносишь $C3 внезапный удар мечом в печень!");
                        msgVictStun = _("{W$c1 обездвиживает тебя внезапным ударом меча в печень!{x");
                        msgCharStun = _("{WТы обездвиживаешь $C4 внезапным ударом меча в печень!{x");
                        msgVictBlind = _("{y$c1 наносит тебе удар мечом в голову!{/Кровь заливает тебе глаза, ты ничего не видишь!{x");
                        msgCharBlind = _("{yТы ослепляешь $C4, нанеся удар мечом в голову!{x");
                        msgVictHeart = _("{RНеожиданно изловчившись, $c1 вонзает тебе меч ПРЯМО В СЕРДЦЕ!!!{x");
                        msgCharHeart = _("{RНеожиданно изловчившись, ты вонзаешь $C3 меч ПРЯМО В СЕРДЦЕ!!!{x");
                        break;
            case WEAPON_DAGGER:
                        msgVictBasic = _("$c1 внезапно всаживает тебе кинжал в печень!");
                        msgCharBasic = _("Ты внезапно всаживаешь $C3 кинжал в печень!");
                        msgVictStun = _("{W$c1 обездвиживает тебя, внезапно всаживая кинжал в печень!{x");
                        msgCharStun = _("{WТы обездвиживаешь $C4, внезапно всаживая кинжал в печень!{x{x");
                        msgVictBlind = _("{y$c1 внезапно ослепляет тебя, ткнув кинжалом прямо в глаз!{x");
                        msgCharBlind = _("{yТы внезапно ослепляешь $C4, ткнув кинжалом прямо в глаз!{x");
                        msgVictHeart = _("{RНеожиданно изловчившись, $c1 вонзает тебе кинжал ПРЯМО В СЕРДЦЕ!!!{x");
                        msgCharHeart = _("{RНеожиданно изловчившись, ты вонзаешь $C3 кинжал ПРЯМО В СЕРДЦЕ!!!{x");
                        break;
            default:
                        msgVictBlind = _("{y$c1 наносит тебе удар в голову!{/Кровь заливает тебе глаза, ты ничего не видишь!{x");
                        msgCharBlind = _("{yТы ослепляешь $C4 быстрым ударом в голову!{x");
                        msgVictHeart = _("{RНеожиданно изловчившись, $c1 наносит тебе удар ПРЯМО В СЕРДЦЕ!!!{x");
                        msgCharHeart = _("{RНеожиданно изловчившись, ты наносишь $C3 удар ПРЯМО В СЕРДЦЕ!!!{x");
                        break;
            }
    }            
    // thieves/druids have +10% to blind:       65 / 95 / 100  
    // ninjas and rangers have +10% to stun:    85 / 95 / 100
    // samurai have +10% to strike heart:       75 / 85 / 100
    // everyone else:                           75 / 95 / 100 
    
    if ( ch->getProfession( ) == prof_ranger ) {                    
            if (!RoomUtils::isNature(ch->in_room))
                return;
            msgVictBasic = _("$c1 внезапно сотрясает землю мощным ударом!");
            msgCharBasic = _("Ты сотрясаешь землю мощным ударом, заставая врасплох $C4!");
            msgVictStun = _("{W$c1 сотрясает землю мощным ударом, обездвиживая тебя!{x");
            msgCharStun = _("{WТы сотрясаешь землю мощным ударом, обездвиживая $C4!{x");
            msgVictBlind = _("{y$c1 внезапной серией ударов поднимает вихрь листьев, ослепляя тебя!{x");
            msgCharBlind = _("{yТы внезапной серией ударов поднимаешь вихрь листьев, ослепляя $C4!{x");
            msgVictHeart = _("{R$c1 призывает силу Природы, нанося тебе мощнейший удар прямо в сердце!{x");
            msgCharHeart = _("{RТы призываешь силу Природы, нанося $C3 мощнейший удар прямо в сердце!{x");

            stun_chance = 85;
    }
    if ( ch->getProfession( ) == prof_druid ) {
            if (!ch->isAffected(gsn_shapeshift))
                return;
            msgVictBasic = _("$c1 внезапно всаживает тебе когти в печень!");
            msgCharBasic = _("Ты внезапно всаживаешь $C3 когти в печень!");
            msgVictStun = _("{W$c1 разрывает когтями печень, обездвиживая тебя!{x");
            msgCharStun = _("{WТы разрываешь когтями печень, обездвиживая $C4!{x");
            msgVictBlind = _("{y$c1 царапает когтями глаза, ослепляя тебя!{x");
            msgCharBlind = _("{yТы царапаешь когтями глаза, ослепляя $C4!{x");
            msgVictHeart = _("{R$c1 всаживает острые когти тебе прямо в сердце!{x");
            msgCharHeart = _("{RТы всаживаешь острые когти прямо в сердце $C2!{x");

            stun_chance = 65;
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
                    msgVictBlind = _("{yИспользуя технику кирикаэси, $c1 наносит серию ударов в голову!{/Кровь заливает тебе глаза, ты ничего не видишь!{x");
                    msgCharBlind = _("{yИспользуя технику кирикаэси, ты ослепляешь $C4. Мэн!{x");
                    msgVictHeart = _("{RИспользуя технику кацуги-вадза, $c1 внезапно наносит удар особой силы!!!{x");
                    msgCharHeart = _("{RИспользуя технику кацуги-вадза, ты внезапно наносишь $C3 удар особой силы!!!{x");
            }
            if (!wield) {
                    msgVictBasic = _("$c1 наносит тебе внезапный удар пяткой в печень!");
                    msgCharBasic = _("Ты наносишь $C3 внезапный удар пяткой в печень!");
                    msgVictStun = _("{W$c1 обездвиживает тебя внезапным ударом пяткой в печень!{x");
                    msgCharStun = _("{WТы обездвиживаешь $C4 внезапным ударом пяткой в печень!{x");
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

    // requires according body parts: guts (liver), eye, heart
    if (diceroll < stun_chance) {
        if (IS_SET(victim->parts, PART_GUTS)) {
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
        else dam += (dam * number_range( 1, 4 )) / 10;  // +10-40% damage if no body parts
    }
    else if (diceroll < blind_chance) {
        if (!IS_AFFECTED(victim,AFF_BLIND) && IS_SET(victim->parts, PART_EYE))
        {
            oldact( msgVictBlind, ch, 0, victim, TO_VICT);
            oldact( msgCharBlind, ch, 0, victim, TO_CHAR);

            baf.bitvector.setTable(&affect_flags);
            baf.type     = gsn_critical_strike;
            baf.level    = ch->getModifyLevel();
            baf.location = APPLY_HITROLL;
            baf.modifier     = -1 * ch->getModifyLevel() / 10;
            baf.duration     = number_range(1,5);
            baf.bitvector.setValue(AFF_BLIND);
            affect_to_char( victim, &baf );

            dam += dam * number_range( 1, 2 );  // +100-200% damage 
        }
        else dam += (dam * number_range( 1, 4 )) / 10;  // +10-40% damage if no body parts
    }
    else {
        if (IS_SET(victim->parts, PART_HEART)) {
            oldact( msgVictHeart, ch, 0, victim, TO_VICT);
            oldact( msgCharHeart, ch, 0, victim, TO_CHAR);
            dam += dam * number_range( 2, 5 ); // +200-500% damage
        }
        else dam += (dam * number_range( 1, 4 )) / 10;  // +10-40% damage if no body parts
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
    
    if (katana->extraDescriptions.empty())
        return;
    
    // TODO check all descriptions or make it an owner
    DLString edText = katana->extraDescriptions.front()->description.get(LANG_DEFAULT);
    if (!String::contains(edText, ch->getNameC()) && !String::contains(edText, ch->getNameP('2')))
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

    oldact(_("$o1 $c2 загорается {Cголубым светом{x."), ch, katana, 0, TO_ROOM);
    oldact(_("$o1 в твоей $T руке загорается {Cголубым светом{x."), 
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

        oldact(_("Твои руки наполняются смертоносной силой!"),ch,0,0,TO_CHAR);
        oldact(_("Руки $c2 наполняются смертоносной силой!"),ch,0,0,TO_ROOM);
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
    skill_mod = 0.1;
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
    // normalize chance, maxed at 15%
    chance = (int)URANGE(1, (int)chance, 15);
    d.log(chance, "final");

    if (diceroll > chance)
        return;

    if ( (victim->isAffected(gsn_nerve)) && (number_percent() < 15) )
        ch->pecho(_("С ослабленными нервными окончаниями оглушить противника становится легче."));        
                
    if (!IS_AFFECTED(victim, AFF_WEAK_STUN)) {
        SET_BIT(victim->affected_by, AFF_WEAK_STUN);
        if (ch != victim) {
            oldact(_("{rТвой удар в голову слегка оглушает $C4!{x"), ch, 0, victim, TO_CHAR);
            oldact(_("{r$c1 слегка оглушает тебя ударом в голову!{x"), ch, 0, victim, TO_VICT);
            oldact(_("{r$c1 слегка оглушает $C4 ударом в голову!{x"), ch, 0, victim, TO_NOTVICT);
        } else {
            oldact(_("{rТвой удар отклонен тебе ж в голову! Ты слегка оглушаешь СЕБЯ!{x"), ch, 0, victim, TO_CHAR);
            oldact(_("{r$c1 слегка оглушает СЕБЯ ударом в голову!{x"), ch, 0, victim, TO_NOTVICT);
        }
    } else if (diceroll < (chance / 2) && !IS_AFFECTED(victim, AFF_STUN)) {
        if (ch != victim) {

            if(IS_AFFECTED(victim, AFF_WEAK_STUN)){
                REMOVE_BIT(victim->affected_by,AFF_WEAK_STUN);
            }

            SET_BIT(victim->affected_by, AFF_STUN);

            oldact(_("{rМощной серией ударов в голову ты сильно оглушаешь $C4!{x"), ch, 0, victim, TO_CHAR);
            oldact(_("{r$c1 сильно оглушает тебя мощной серией ударов в голову!{x"), ch, 0, victim, TO_VICT);
            oldact(_("{r$c1 оглушает $C4 мощной серией ударов в голову!{x"), ch, 0, victim, TO_NOTVICT);
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

    dam_bonus += (ch->getCurrStat(STAT_STR) - victim->getCurrStat(STAT_CON)) * stat_mod * 100;
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
                    ch->recho(rch, _("%^C1 и %C1 наводят {Rстрах и ужас{x на противников, нанося дополнительный урон!"), ch, rch);
                    ch->pecho(_("Ты и %C1 наводите {Rстрах и ужас{x на противников, нанося дополнительный урон!"), rch);
                    rch->pecho(_("Ты и %C1 наводите {Rстрах и ужас{x на противников, нанося дополнительный урон!"), ch);
                }

                dam = dam * 110 / 100;
                return;
            }
        }
    }
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
