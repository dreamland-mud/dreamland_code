/* $Id: class_ninja.cpp,v 1.1.2.20.6.11 2010-09-01 21:20:44 rufina Exp $
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
 *    и все остальные, кто советовал и играл в этот MUD                    *п
 ***************************************************************************/
#include <sstream>
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "object.h"
#include "gsn_plugin.h"
#include "act_move.h"
#include "mercdb.h"
#include "skill_utils.h"

#include "magic.h"
#include "occupations.h"
#include "fight.h"
#include "onehit_weapon.h"
#include "chance.h"
#include "merc.h"
#include "handler.h"
#include "act.h"
#include "def.h"
#include "stats_apply.h"
#include "debug_utils.h"

/*
 * 'vanish' skill command
 */

SKILL_RUNP( vanish )
{
    Debug d(ch, "debug_ninja", "vanish");
    Character *victim;
    float chance, kidnap_chance = 0, skill_mod, stat_mod, level_mod, quick_mod, size_mod, sleep_mod, vis_mod;
    bool FightingCheck;    
    Room *pRoomIndex;
    Affect af;
    char arg[MAX_INPUT_LENGTH];
    //////////////// BASE MODIFIERS //////////////// TODO: add this to XML
    skill_mod   = 0.3;
    stat_mod    = 0.05;
    level_mod   = 0.01;
    quick_mod   = 0.1;
    size_mod    = 0.1;
    sleep_mod   = 0.1;
    vis_mod     = 0.1; 

    //////////////// ELIGIBILITY CHECKS ////////////////

    ///// Specific transportation checks
    
    if ( ch->death_ground_delay > 0
            && ch->trap.isSet( TF_NO_MOVE ) )
    {
            ch->pecho("Ты не можешь покинуть это место без посторонней помощи!");
            return;
    }

    if (ch->mount) {
        ch->pecho( "Ты не можешь исчезнуть, пока ты верхом или оседла%Gно|н|на!", ch );
        return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL))
    {
        ch->pecho("Из этого места нельзя исчезать.");
        return;
    }

    pRoomIndex = get_random_room_vanish( ch );

    if (!pRoomIndex) {
        ch->pecho("В этой зоне тебе некуда исчезать.");
        return;
    }
    
    ///// Standard checks: TODO: turn this into a function
    
    if ( !ch->is_npc() && !gsn_vanish->usable( ch ) )
    {
            ch->pecho("Ты не владеешь этим навыком.");
            return;
    }
    
    if (ch->mana < gsn_vanish->getMana( ))
    {
            ch->pecho("У тебя недостаточно энергии для этого.");
            return;
    }
    
    /* TODO: Not sure if it needs the effect: wait_state is enough
    if (ch->isAffected(gsn_vanish)) {
        ch->pecho("Тебе пока нечего бросить.");
        return;
    } */

    // Needs at least one hand
    const GlobalBitvector &loc = ch->getWearloc( );
    if (!loc.isSet( wear_hands )
    || (!loc.isSet( wear_wrist_l ) && (!loc.isSet( wear_wrist_r )) ))
    {
            ch->pecho("Тебе нужна хотя бы одна рука для этой техники.");
            return;
    }
    
    if (ch->fighting != 0) {
            FightingCheck = true;
    }
    else
            FightingCheck = false;

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {               
            victim = 0;
    }
    else {
        if ((victim = get_char_room(ch,arg)) == 0)
        {
                ch->pecho("Этого нет здесь.");
                return;
        }

        if ( ( victim->fighting != 0 ) && ( victim != ch ) )
        {
            ch->pecho("Подожди, пока закончится сражение.");
            return;
        }

        if ( IS_SET(victim->imm_flags,IMM_SUMMON) ) {
            ch->pecho("%^C4 нельзя переместить.", victim);
            return;
        }

        if (is_safe(ch,victim))
        {            
                return;
        }

        // strangled centaurs can't rearkick
        if ( IS_AWAKE(victim) && (gsn_rear_kick->getCommand( )->run( ch, victim )) )
                return;
        
        if(SHADOW(ch))
        {
                ch->pecho("Ты пытаешься схватить в охапку свою тень!");
                oldact_p("$c1 пытается схватить в охапку свою тень.",
                                        ch, 0, 0, TO_ROOM,POS_RESTING);
                return;
        }
        
        if ( victim == ch )
        {
            victim = 0; // can use your own name as an excessive arg to simply vanish
        }
        
        // CAN vanish charmed targets (e.g. your own pets)        
        // CAN vanish immortals :)
    }

    if( !ch->is_npc() && !ch->move )
    {
            ch->pecho("Ты слишком уста%Gло|л|ла для этого.", ch);
            return;
    }
    else
            ch->move -= move_dec( ch );
   
    ch->mana -= gsn_vanish->getMana( );
    ch->setWait( gsn_vanish->getBeats(ch)  );
    UNSET_DEATH_TIME(ch);
        
    if (number_percent() > gsn_vanish->getEffective( ch ) )
    {
            ch->pecho("Тебе не удается активировать световую гранату.");
            gsn_vanish->improve( ch, false );
            return;
    }    
    
    oldact("$c1 бросает на землю небольшой шар. {WЯркая вспышка{x на мгновение ослепляет тебя!", ch, 0, 0, TO_ROOM);
    ch->pecho("Ты бросаешь на землю световую гранату. {WЯркая вспышка{x на мгновение ослепляет всех вокруг!");
    gsn_vanish->improve( ch, true );

    //////////////// PROBABILITY CHECKS: SELF ////////////////

    chance = 0;
    chance += gsn_vanish->getEffective( ch );
    chance += skill_level_bonus(*gsn_vanish, ch);
    d.log(chance, "skill");

    if ( IS_AFFECTED(ch,AFF_WEAK_STUN) ) {
        chance = chance / 2;
        d.log(chance, "stun");
    }
    
    if (FightingCheck) {
        
        chance = chance / 2; 
        d.log(chance, "fight");
        chance += ch->fighting->can_see(ch) ? 0 : (vis_mod * 100);
        d.log(chance, "vis");

        if ( IS_SET(ch->fighting->imm_flags,IMM_LIGHT) ) {
            ch->pecho("{W%^C1 не поддается воздействию вспышки!{x", ch->fighting);
            chance = 0;
            d.log(chance, "imm");
        }
        if ( IS_SET(ch->fighting->res_flags,RES_LIGHT) ) {
            ch->pecho("{W%^C1 сопротивляется воздействию вспышки!{x", ch->fighting);
            chance = chance / 2; 
            d.log(chance, "res");
        }         
    }    

    d.log(chance, "chance final");

    //////////////// PROBABILITY CHECKS: KIDNAP ////////////////
    
    if (victim != 0) {
        kidnap_chance += gsn_vanish->getEffective( ch ) * skill_mod;
        d.log(kidnap_chance, "kidnap skill");
        kidnap_chance += ( ch->getCurrStat(STAT_DEX) - victim->getCurrStat(STAT_STR) ) * stat_mod * 100;
        d.log(kidnap_chance, "stats");
        kidnap_chance += ( skill_level(*gsn_vanish, ch) - victim->getModifyLevel() ) * level_mod * 100;
        d.log(kidnap_chance, "lvl");
        kidnap_chance += (ch->size - victim->size) * size_mod * 100;
        d.log(kidnap_chance, "size");
        kidnap_chance += victim->can_see(ch) ? 0 : (vis_mod * 100);
        kidnap_chance += IS_AWAKE( victim ) ? 0 : (sleep_mod * 100);    
        d.log(kidnap_chance, "vis");

        if (IS_QUICK(ch)) {
                kidnap_chance += quick_mod * 100;
                d.log(kidnap_chance, "quick");
        }
        if (IS_QUICK(victim)) {
                kidnap_chance -= quick_mod * 100;            
                d.log(kidnap_chance, "quick");
        }

        if (IS_SET(victim->res_flags, RES_LIGHT)) {
                kidnap_chance = kidnap_chance / 2;
                d.log(kidnap_chance, "res");
        }
            
        if ( IS_AFFECTED(ch,AFF_WEAK_STUN) ) {
                kidnap_chance = kidnap_chance / 2;
                d.log(kidnap_chance, "stun");
        }
        
        // neckguard can't protect if you're asleep
        if ( (victim->isAffected(gsn_backguard)) && IS_AWAKE( victim ) ) {
                kidnap_chance = kidnap_chance / 2;
                d.log(kidnap_chance, "backguard");
        }
        
        kidnap_chance = max( (float)1, kidnap_chance ); // there's always a chance        
        d.log(kidnap_chance, "kidnap final");
    }

    //////////////// THE ROLL ////////////////
    
    if ( number_percent() > chance ) {
            if (FightingCheck) {
                oldact("$c1 пытается скрыться, но противник бдит, и бой продолжается!", ch, 0, 0, TO_ROOM);
                ch->pecho("Ты пытаешься скрыться, но противник бдит, и бой продолжается!");
                return;
            }
            else {
                // weak stun is a bitch
                oldact("$c1 пытается скрыться, но спотыкается и падает!", ch, 0, 0, TO_ROOM);                
                ch->pecho("Ты пытаешься скрыться, но спотыкаешься и падаешь!");
                return;                
            }    
    }
    
    if (victim == 0 || victim == ch) {
        transfer_char( ch, ch, pRoomIndex,
            "%1$^C1 внезапно исчезает!",
            "Пользуясь всеобщим замешательством, ты исчезаешь!",
            "%1$^C1 внезапно появляется у тебя за спиной." );
        return;
    }    
    else {   
            // trying to kidnap
            oldact("$c1 пытается взять $C4 в охапку!", ch, 0, victim, TO_NOTVICT);
            oldact("Ты пытаешься взять $C4 в охапку.",   ch, 0, victim, TO_CHAR);
            oldact("$c1 пытается взять тебя в охапку!", ch, 0, victim, TO_VICT);

            if ( number_percent() < kidnap_chance ) {
                    // kidnapping success
                    transfer_char( ch, ch, pRoomIndex,
                        0,
                        "Пользуясь всеобщим замешательством, ты исчезаешь!",
                        "%1$^C1 внезапно появляется!");

                    transfer_char( victim, ch, pRoomIndex,
                        "%1$^C1 исчезает вместе с %2^C5!",
                        "%2$^C1 хватает тебя и исчезает вместе с тобой!",
                        "%1$^C1 внезапно появляется, похищенн%1$Gое|ый|ая %2$C5." );
                    
                    if (!FightingCheck) {
                        yell_panic( ch, victim,
                            "Помогите! Меня кто-то похищает!",
                            "Убери свои лапы, %1$C1!" );
        
                        multi_hit(victim,ch);
                    }
            }
            else {
                    // kidnap failed, victim escaped               
                    oldact("$C1 успевает вырваться из объятий $c2!", ch, 0, victim, TO_NOTVICT);
                    oldact("$C1 успевает вырваться из твоих объятий!",   ch, 0, victim, TO_CHAR);
                    oldact("Ты умудряешься вырваться из объятий $c2", ch, 0, victim, TO_VICT);

                    transfer_char( ch, ch, pRoomIndex,
                        "%1$^C1 внезапно исчезает!",
                        "Пользуясь всеобщим замешательством, ты исчезаешь!",
                        "%1$^C1 внезапно появляется у тебя за спиной." );
            }
      
     }
}

/*
 * 'nerve' skill command
 */

SKILL_RUNP( nerve )
{
        Debug d(ch, "debug_ninja", "nerve");
        Character *victim;
        float chance, skill_mod, stat_mod, level_mod, quick_mod, size_mod, sleep_mod, vis_mod;
        bool FightingCheck;
        char arg[MAX_INPUT_LENGTH];
        
        //////////////// BASE MODIFIERS //////////////// TODO: add this to XML
        skill_mod   = 0.5;
        stat_mod    = 0.05;
        level_mod   = 0.01;
        quick_mod   = 0.1;
        size_mod    = -0.1; // HARDER to affect smaller victims, easier to affect larger
        sleep_mod   = 0.1;
        vis_mod     = 0.1;    
    
        //////////////// ELIGIBILITY CHECKS ////////////////

        ///// Standard checks: TODO: turn this into a function
    
        if ( MOUNTED(ch) )
        {
                ch->pecho("Только не верхом!");
                return;
        }

        if (!gsn_nerve->usable( ch ) )
        {
                ch->pecho("Ты не владеешь этим навыком.");
                return;
        }

        // Needs at least one hand
        const GlobalBitvector &loc = ch->getWearloc( );
        if (!loc.isSet( wear_hands )
        || (!loc.isSet( wear_wrist_l ) && (!loc.isSet( wear_wrist_r )) ))
        {
                ch->pecho("Тебе нужна хотя бы одна рука для этой техники.");
                return;
        }
    
        if (ch->fighting != 0) {
                FightingCheck = true;
                victim = ch->fighting;
        }
        else
                FightingCheck = false;

        argument = one_argument(argument,arg);

        if (arg[0] == '\0')
        {               
                if (!FightingCheck)
                {
                        ch->pecho("Сейчас ты не сражаешься!");
                        return;
                }
        }
        else if ((victim = get_char_room(ch,arg)) == 0)
        {
                ch->pecho("Этого нет здесь.");
                return;
        }

        if (is_safe(ch,victim))
        {            
                return;
        }

        if (IS_CHARMED(ch) && ch->master == victim)
        {
                oldact("Но $C1 твой друг!!!",ch,0,victim,TO_CHAR);
                return;
        }

        if( !ch->is_npc() && !ch->move )
        {
                ch->pecho("Ты слишком уста%Gло|л|ла для этого.", ch);
                return;
        }
        else
                ch->move -= move_dec( ch );

        ///// Custom messages: TODO: move these to XML as well
            
        if (victim == ch)
        {
                ch->pecho("Ты трогаешь себя в нескольких неожиданных местах и довольно улыбаешься.");
                return;
        }
            
        if(SHADOW(ch))
        {
                ch->pecho("Твои пальцы проходят сквозь тень!");
                oldact_p("$c1 пытается потрогать свою тень.",
                                        ch, 0, 0, TO_ROOM,POS_RESTING);
                return;
        }    

        if ( victim->isAffected(gsn_nerve) )
        {
                ch->pecho("Ты не можешь сделать противника еще слабее.");
                return;
        }

        if (IS_SET(victim->imm_flags, IMM_DISEASE))
        {
                oldact_p("$C1 обладает иммунитетом к этой технике.", ch, 0,
                    victim, TO_CHAR,POS_RESTING);
                return;
        }            
            
        //////////////// PROBABILITY CHECKS ////////////////
            
        chance = 0;
  
        chance += gsn_nerve->getEffective( ch ) * skill_mod;
        d.log(chance, "skill"); 
        chance += ( ch->getCurrStat(STAT_DEX) - victim->getCurrStat(STAT_CON) ) * stat_mod * 100;
        d.log(chance, "stats");
        chance += ( skill_level(*gsn_nerve, ch) - victim->getModifyLevel() ) * level_mod * 100;
        d.log(chance, "lvl");
        chance += (ch->size - victim->size) * size_mod * 100;
        d.log(chance, "size");
        chance += victim->can_see(ch) ? 0 : (vis_mod * 100);
        chance += IS_AWAKE( victim ) ? 0 : (sleep_mod * 100);    
        d.log(chance, "vis");
   
        if (IS_QUICK(ch)) {
                chance += quick_mod * 100;
                d.log(chance, "quick");
        }
        if (IS_QUICK(victim)) {
                chance -= quick_mod * 100;            
                d.log(chance, "quick");
        }

        if (IS_SET(victim->res_flags, RES_DISEASE)) {
                chance = chance / 2;
                d.log(chance, "res");
        }
            
        if ( IS_AFFECTED(ch,AFF_WEAK_STUN) ) {
                chance = chance / 2; 
                d.log(chance, "stun");
        }

        d.log((int)chance, "final");
        //////////////// THE ROLL ////////////////
            
        ch->setWait( gsn_nerve->getBeats(ch)  );
        UNSET_DEATH_TIME(ch);
    
        if ( ch->is_npc() || number_percent() < (int) chance )
        {
                gsn_nerve->getCommand()->run(ch, victim);
                oldact("Ты ослабляешь $C4, пережимая нервные окончания.",ch,0,victim,TO_CHAR);
                oldact("$c1 ослабляет тебя, пережимая твои нервные окончания.",ch,0,victim,TO_VICT);
                oldact("$c1 ослабляет $C4",ch,0,victim,TO_NOTVICT);
                gsn_nerve->improve( ch, true, victim );
        }
        else
        {
                ch->pecho("Ты нажимаешь не туда, куда надо.");
                oldact_p("$c1 нажимает пальцами на твои нервные окончания, но ничего не происходит.",
                        ch,0,victim,TO_VICT,POS_RESTING);
                oldact_p("$c1 нажимает пальцами на нервные окончания $C2, но ничего не происходит.",
                        ch,0,victim,TO_NOTVICT,POS_RESTING);
                gsn_nerve->improve( ch, false, victim );
        }

        if (!FightingCheck) {
            yell_panic( ch, victim,
                        "Помогите! Меня кто-то трогает!",
                        "Убери свои руки, %1$C1!" );
        
            multi_hit(victim,ch);
        }
}

BOOL_SKILL(nerve)::run(Character *ch, Character *victim)
{
    int level, skill, mod;
    level = skill_level(*gsn_nerve, ch);
    
    if (gsn_nerve->usable( ch ) )
        skill = gsn_nerve->getEffective( ch );
    else
        skill = 0;
    
    mod = -1 * (level/20 + skill/20 + 1);
    
    Affect af;
    af.type     = gsn_nerve;
    af.level    = level;
    af.duration = level / 20;
    af.location = APPLY_STR;
    af.modifier = mod;

    affect_to_char(victim,&af);
    return true;
}

/*
 * 'endure' skill command
 */

SKILL_RUNP( endure )
{
    int level, skill, mod;    
    
  ///// Standard checks: TODO: turn this into a function
    
  if ( MOUNTED(ch) )
  {
        ch->pecho("Только не верхом!");
        return;
  }
    
  if (ch->is_npc())
  {
        ch->pecho("Выносливость -- не твой удел.");
        return;
  }

  if ( gsn_endure->getEffective( ch ) <= 1 )
  {
        ch->pecho("Тебе недоступна техника выносливости.");
        return;
  }

  if (ch->isAffected(gsn_endure))
    {
      ch->pecho("Ты не можешь стать еще выносливее.");
      return;
    }
    
    level = skill_level(*gsn_endure, ch);
    
    if (gsn_endure->usable( ch ) )
        skill = gsn_endure->getEffective( ch );
    else
        skill = 0;
    
    mod = -1 * (level/20 + skill/20 + 1); 
    
    ch->setWait( gsn_endure->getBeats(ch)  );
    gsn_endure->getCommand()->run(ch, mod);
    gsn_endure->improve( ch, true );
}


BOOL_SKILL(endure)::run(Character *ch, int modifier)
{      
    Affect af;

    af.type     = gsn_endure;
    af.level    = skill_level(*gsn_endure, ch);
    af.duration = skill_level(*gsn_endure, ch) / 4;
    af.location = APPLY_SAVING_SPELL;
    af.modifier = modifier;    

    affect_to_char(ch,&af);

    oldact("Ты мгновенно концентрируешься, готовясь к столкновению с магией.", ch, 0, 0, TO_CHAR);
    oldact("$c1 мгновенно концентрируется, готовясь к столкновению с магией.", ch,0,0,TO_ROOM);
    return true;
}


/*----------------------------------------------------------------------------
 * Assassinate 
 *---------------------------------------------------------------------------*/
class AssassinateOneHit: public SkillDamage {
public:
    AssassinateOneHit( Character *ch, Character *victim );

    virtual void calcDamage( );
};

AssassinateOneHit::AssassinateOneHit( Character *ch, Character *victim )
            : Damage( ch, victim, DAM_BASH, 0 ),
              SkillDamage( ch, victim, gsn_assassinate, DAM_BASH, 0, DAMF_WEAPON )
{
}

void AssassinateOneHit::calcDamage( )
{
    Debug d(ch, "debug_ninja", "assa");
    float chance, skill_mod, stat_mod, level_mod, size_mod, vis_mod, sleep_mod, quick_mod, time_mod;

    //////////////// BASE MODIFIERS //////////////// TODO: add this to XML
    skill_mod   = 0.08;
    stat_mod    = 0.01;
    level_mod   = 0.01;
    quick_mod   = 0.1;
    size_mod    = 0.03;
    sleep_mod   = 0.05;
    vis_mod     = 0.05;
    time_mod    = 0.05;
    
    //////////////// PROBABILITY CHECKS ////////////////
            
    chance = 0;
        
    chance += gsn_assassinate->getEffective( ch ) * skill_mod;
    d.log(chance, "skill");
    chance += ( ch->getCurrStat(STAT_STR) - victim->getCurrStat(STAT_CON) ) * stat_mod * 100;
    d.log(chance, "stats");
    chance += ( skill_level(*gsn_assassinate, ch) - victim->getModifyLevel() ) * level_mod * 100;
    d.log(chance, "lvl");
    chance += (ch->size - victim->size) * size_mod * 100;
    d.log(chance, "size");
    chance += victim->can_see(ch) ? 0 : (vis_mod * 100);    
    chance += IS_AWAKE( victim ) ? 0 : (sleep_mod * 100);            
    d.log(chance, "vis");

    if (IS_QUICK(ch)) {
        chance += quick_mod * 100;
        d.log(chance, "quick");
    }        

    if (IS_QUICK(victim)) {
        chance -= quick_mod * 100;            
        d.log(chance, "quick");
    }

    if (IS_SET(victim->res_flags, RES_WEAPON)) {
        chance = chance / 2;
        d.log(chance, "res");
    }
            
    if ( IS_AFFECTED(ch,AFF_WEAK_STUN) ) {
        chance = chance / 2;
        d.log(chance, "stun");
    }
    
    // neckguard can't protect if you're asleep
    if ( (victim->isAffected(gsn_backguard)) && IS_AWAKE( victim ) ) {
        chance = chance / 2;
        d.log(chance, "backguard");
    }

    // only check for assassinate spam without strangle
    int k = ch->getLastFightDelay( );
    if (k >= 0 && k < FIGHT_DELAY_TIME && IS_AWAKE( victim )) {
        chance -= (FIGHT_DELAY_TIME - k) * time_mod * 100;
        d.log(chance, "adrenaline");
    }
        
    UNSET_DEATH_TIME(ch);
    victim->setLastFightTime( );
    ch->setLastFightTime( );    
    
    chance = max( (float)1, chance ); // there's always a chance
    d.log((int)chance, "final");

    //////////////// THE ROLL ////////////////
    
    Chance mychance(ch, (int) chance, 100);

    if (mychance.reroll()) {
        oldact("Ты {R+++ ЛОМАЕШЬ ШЕЮ +++{x $C3!",ch,0,victim,TO_CHAR);
        oldact("$c1 {R+++ ЛОМАЕТ ШЕЮ +++{x $C3!",ch,0,victim,TO_NOTVICT);
        oldact_p("$c1 {R+++ ЛОМАЕТ ТЕБЕ ШЕЮ +++{x!",ch,0,victim,TO_VICT,POS_DEAD);

        gsn_assassinate->improve( ch, true, victim );

        handleDeath( );
        throw VictimDeathException( );
    }
    else
    {
        gsn_assassinate->improve( ch, false, victim );

        dam = skill_level(*gsn_assassinate, ch) + ch->damroll;
        dam += dam * get_str_app(ch).damage / 100;
        damApplyEnhancedDamage( );
        if ( !IS_AWAKE(victim) )
            dam *= 2;

        Damage::calcDamage( );

    }
}

/*
 * 'assassinate' skill command
 */
SKILL_RUNP( assassinate )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

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

    if ( !ch->is_npc() && !gsn_assassinate->usable( ch ) )
    {
            ch->pecho("Ты не имеешь понятия, как ломать шеи.");
            return;
    }

    if ( IS_CHARMED(ch) )
    {
            ch->pecho( "Ты же не хочешь убить сво%1$Gего|его|ю любим%1$Gого|ого|ю хозя%1$Gина|ина|йку.", ch->master);
            return;
    }

    if ( arg[0] == '\0' )
    {
            ch->pecho("Кого убиваем СЕГОДНЯ?");
            return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
            ch->pecho("Этого нет здесь.");
            return;
    }

    if ( victim == ch )
    {
            ch->pecho("Твой путь не включает в себя суицид.");
            return;
    }

    if ( is_safe( ch, victim ) )
            return;

    if ( victim->is_immortal() && !victim->is_npc() )
    {
            ch->pecho("На Бессмертных это не подействует.");
            return;
    }

    if ( victim->fighting != 0 )
    {
            ch->pecho("Подожди, пока закончится сражение.");
            return;
    }
    
    const GlobalBitvector &loc = ch->getWearloc( );
    
    if (!loc.isSet( wear_hands )
        || !loc.isSet( wear_wrist_l )
        || !loc.isSet( wear_wrist_r ))
    {
        ch->pecho("У тебя нет рук.");
        return;
    }

    if (!check_bare_hands(ch))        
    {
            ch->pecho("Освободи обе руки для этого.");
            return;
    }

    if ( victim->hit < victim->max_hit
            && victim->can_see(ch)
            && IS_AWAKE(victim) )
    {
            oldact_p("$C1 ран$Gено|ен|ена и настороженно оглядывается... ты не можешь подкрасться.",
                    ch, 0, victim, TO_CHAR,POS_RESTING);
            return;
    }

    if (IS_SET(victim->imm_flags, IMM_WEAPON))
    {
            oldact_p("$C1 имеет слишком крепкую шею, чтобы ее можно было сломать.", ch, 0,
                    victim, TO_CHAR,POS_RESTING);
            return;
    }

    // strangled centaurs can't rearkick
    if ( IS_AWAKE(victim) && (gsn_rear_kick->getCommand( )->run( ch, victim )) )
        return;

    if(SHADOW(ch))
    {
            ch->pecho("Твои пальцы проходят сквозь тень!");
            oldact_p("$c1 пытается сломать шею своей тени.",
                    ch, 0, 0, TO_ROOM,POS_RESTING);
            return;
    }   
    
    ch->setWait( gsn_assassinate->getBeats(ch)  );
    AssassinateOneHit ass( ch, victim );    
    
    // assassination attempt can't "miss"
    try {
        ass.hit( true );       
        yell_panic( ch, victim,
                    "Помогите! Кто-то пытается УБИТЬ меня!",
                    "Помогите! %1$^C1 пытается УБИТЬ меня!" );
    }
    catch (const VictimDeathException& e) {                                     
    }
}

/*
 * 'caltraps' skill command
 */

SKILL_RUNP( caltraps )
{
  Debug d(ch, "debug_ninja", "caltraps");
  Character *victim;
  float chance, skill_mod, stat_mod, quick_mod, size_mod, sleep_mod, vis_mod;
  bool FightingCheck;
  char arg[MAX_INPUT_LENGTH];
    
  //////////////// BASE MODIFIERS //////////////// TODO: add this to XML
  skill_mod   = 0.5;
  stat_mod    = 0.05;
  quick_mod   = 0.1;
  size_mod    = -0.1; // HARDER to affect smaller victims, easier to affect larger
  sleep_mod   = 0.1;
  vis_mod     = 0.1;    
    
  //////////////// ELIGIBILITY CHECKS ////////////////

  ///// Standard checks: TODO: turn this into a function
    
  if (ch->is_npc() || !gsn_caltraps->usable( ch ))
  {
        ch->pecho("Ты не владеешь этим навыком.");
        return;
  }

  // Needs at least one hand
  const GlobalBitvector &loc = ch->getWearloc( );
  if (!loc.isSet( wear_hands )
  || (!loc.isSet( wear_wrist_l ) && (!loc.isSet( wear_wrist_r )) ))
  {
        ch->pecho("Тебе нужна хотя бы одна рука для этой техники.");
        return;
  }
    
  if (ch->fighting != 0) {
        FightingCheck = true;
        victim = ch->fighting;
   }
    else
        FightingCheck = false;

   argument = one_argument(argument,arg);

   if (arg[0] == '\0')
   {               
        if (!FightingCheck)
        {
                ch->pecho("Сейчас ты не сражаешься!");
                return;
        }
   }
   else if ((victim = get_char_room(ch,arg)) == 0)
   {
        ch->pecho("Этого нет здесь.");
        return;
   }

   if (is_safe(ch,victim))
   {            
        return;
   }

   if (IS_CHARMED(ch) && ch->master == victim)
   {
        oldact("Но $C1 твой друг!!!",ch,0,victim,TO_CHAR);
        return;
   }

   if( !ch->is_npc() && !ch->move )
   {
        ch->pecho("Ты слишком уста%Gло|л|ла для этого.", ch);
        return;
   }
   else
        ch->move -= move_dec( ch );

   ///// Custom messages: TODO: move these to XML as well
      
   if (is_flying( victim ))
   {
        ch->pecho("Твои шипы не смогут навредить летучему противнику.");
        return;
   }      

   if (victim == ch)
   {
        ch->pecho("Ты задумчиво колешь себя острым шипом в пятку. Ай!");
        return;
   }
            
   if(SHADOW(ch))
   {
        ch->pecho("Твои шипы проходят сквозь тень!");
        oldact_p("$c1 бросает шипы под ноги собственной тени.",
                ch, 0, 0, TO_ROOM,POS_RESTING);
        return;
   }    

   if ( victim->isAffected(gsn_caltraps) )
   {
        ch->pecho("Противник уже хромает.");
        return;
   }

   if (IS_SET(victim->imm_flags, IMM_PIERCE))
   {
        oldact_p("$C1 обладает иммунитетом к острым шипам.", ch, 0,
                victim, TO_CHAR,POS_RESTING);
        return;
   }       

   //////////////// PROBABILITY CHECKS ////////////////
            
   chance = 0;
        
   chance += gsn_caltraps->getEffective( ch ) * skill_mod;
   d.log(chance, "skill");
   chance += ( ch->getCurrStat(STAT_DEX) - victim->getCurrStat(STAT_DEX) ) * stat_mod * 100;
   d.log(chance, "stats");
   chance += skill_level_bonus(*gsn_caltraps, ch); // no level diff check for caltraps
   chance += (ch->size - victim->size) * size_mod * 100;
   d.log(chance, "size");
   chance += victim->can_see(ch) ? 0 : (vis_mod * 100);   
   chance += IS_AWAKE( victim ) ? 0 : (sleep_mod * 100);            
   d.log(chance, "vis");

   if (IS_QUICK(ch)) {
        chance += quick_mod * 100;
        d.log(chance, "quick");
   }
   if (IS_QUICK(victim)) {
        chance -= quick_mod * 100;            
        d.log(chance, "quick");
   }

   if (IS_SET(victim->res_flags, RES_PIERCE)) {
        chance = chance / 2;
        d.log(chance, "res");
   }        
          
   d.log((int)chance, "final");

  //////////////// THE ROLL ////////////////      
      
  oldact_p("Ты кидаешь пригоршню острых шипов под ноги $C3.",
         ch,0,victim,TO_CHAR,POS_RESTING);
  oldact_p("$c1 кидает пригоршню острых шипов тебе под ноги!",
         ch,0,victim,TO_VICT,POS_RESTING);

  ch->setWait( gsn_caltraps->getBeats(ch) );
  UNSET_DEATH_TIME(ch);  

  if ( (!FightingCheck) && (IS_AWAKE( victim )) ) {
       yell_panic( ch, victim,
            "Помогите! Ай! Колется!!!",
            "Убери свои чертовы шипы, %1$C1!" );
        
       multi_hit(victim,ch);
  }
      
  if ( ch->is_npc() || number_percent() > (int) chance )
  {
        damage(ch,victim,0,gsn_caltraps,DAM_PIERCE, true, DAMF_WEAPON);
        gsn_caltraps->improve( ch, false, victim );
        return;
  }
  gsn_caltraps->getCommand()->run(ch, victim);    
  gsn_caltraps->improve( ch, true, victim );
}

BOOL_SKILL(caltraps)::run(Character *ch, Character *victim)
{
    int level, skill, mod;
    level = skill_level(*gsn_caltraps, ch);
    
    if (gsn_caltraps->usable( ch ) )
        skill = gsn_caltraps->getEffective( ch );
    else
        skill = 0;
    
    mod = -1 * (level/10 + skill/10 + 1);
    
    try {
        damage_nocatch(ch,victim, level,gsn_caltraps,DAM_PIERCE, true, DAMF_WEAPON);

        if (!victim->isAffected(gsn_caltraps)) {
            Affect tohit,todam,todex;

            tohit.type      = gsn_caltraps;
            tohit.level     = level;
            tohit.duration  = -1;
            tohit.location = APPLY_HITROLL;
            tohit.modifier  = mod;
            affect_to_char( victim, &tohit );

            todam.type = gsn_caltraps;
            todam.level = level;
            todam.duration = -1;
            todam.location = APPLY_DAMROLL;
            todam.modifier = mod;
            affect_to_char( victim, &todam);

            todex.type = gsn_caltraps;
            todex.level = level;
            todex.duration = -1;
            todex.location = APPLY_DEX;
            todex.modifier = mod/2;
            affect_to_char( victim, &todex);

            oldact("Острые шипы вонзаются в ступни $C2, стесняя движения и вызывая хромоту.",ch,0,victim,TO_CHAR);
            oldact("Острые шипы вонзаются в твои ступни, стесняя движения и вызывая хромоту.",ch,0,victim,TO_VICT);
        }
    } catch (const VictimDeathException &) {
    }

    return true;
}


/*----------------------------------------------------------------------------
 * throwdown 
 *---------------------------------------------------------------------------*/
class ThrowDownOneHit: public SkillDamage {
public:
    ThrowDownOneHit( Character *ch, Character *victim );

    virtual void calcDamage( );
};

ThrowDownOneHit::ThrowDownOneHit( Character *ch, Character *victim )
            : Damage( ch, victim, DAM_BASH, 0 ),
              SkillDamage( ch, victim, gsn_throw, DAM_BASH, 0, DAMF_WEAPON )
{
}

void ThrowDownOneHit::calcDamage( )
{
        dam = skill_level(*gsn_throw, ch) + ch->damroll / 2;
        dam += dam * get_str_app(ch).damage / 100;
        damApplyEnhancedDamage( );

        Damage::calcDamage( );
}

/*
 * 'throw' skill command
 */

SKILL_RUNP( throwdown )
{
        Debug d(ch, "debug_ninja", "throw");
        Character *victim;
        float chance, skill_mod, stat_mod, level_mod, quick_mod, size_mod, sleep_mod, vis_mod;
        bool FightingCheck;
        char arg[MAX_INPUT_LENGTH];

        //////////////// BASE MODIFIERS //////////////// TODO: add this to XML
        skill_mod   = 0.6;
        stat_mod    = 0.03;
        level_mod   = 0.01;    
        quick_mod   = 0.1;
        size_mod    = 0.25; // much easier to throw smaller victims, much harder on larger
        sleep_mod   = 0.05;
        vis_mod     = 0.05; 

        //////////////// ELIGIBILITY CHECKS ////////////////

        ///// Standard checks: TODO: turn this into a function
    
        if ( MOUNTED(ch) )
        {
                ch->pecho("Только не верхом!");
                return;
        }

        if ( ch->is_npc() || !gsn_throw->usable( ch ) )
        {
                ch->pecho("Ты не владеешь этим навыком.");
                return;
        }

        // Needs at least one hand
        const GlobalBitvector &loc = ch->getWearloc( );
        if (!loc.isSet( wear_hands )
        || (!loc.isSet( wear_wrist_l ) && (!loc.isSet( wear_wrist_r )) ))
        {
                ch->pecho("Тебе нужна хотя бы одна рука для этой техники.");
                return;
        }
    
        if (ch->fighting != 0) {
                FightingCheck = true;
                victim = ch->fighting;
        }
        else
                FightingCheck = false;

        argument = one_argument(argument,arg);

        if (arg[0] == '\0')
        {               
                if (!FightingCheck)
                {
                        ch->pecho("Сейчас ты не сражаешься!");
                        return;
                }
        }
        else if ((victim = get_char_room(ch,arg)) == 0)
        {
                ch->pecho("Этого нет здесь.");
                return;
        }

        if (is_safe(ch,victim))
        {            
                return;
        }

        if (IS_CHARMED(ch) && ch->master == victim)
        {
                oldact("Но $C1 твой друг!!!",ch,0,victim,TO_CHAR);
                return;
        }

        if( !ch->is_npc() && !ch->move )
        {
                ch->pecho("Ты слишком уста%Gло|л|ла для этого.", ch);
                return;
        }
        else
                ch->move -= move_dec( ch );

        ///// Custom messages: TODO: move these to XML as well

        if (is_flying( ch ))
        {
                ch->pecho("Твои ноги должны стоять на земле для упора.");
                return;
        }

        if (victim == ch)
        {
                ch->pecho("Ты крепко обнимаешь себя и в экстазе падаешь на землю!");
                return;
        }
            
        if(SHADOW(ch))
        {
                ch->pecho("Твой захват проходит сквозь тень!");
                oldact_p("$c1 пытается бросить через плечо свою тень.",
                                        ch, 0, 0, TO_ROOM,POS_RESTING);
                return;
        } 

        if (IS_SET(victim->imm_flags, IMM_BASH))
        {
                oldact_p("$C1 обладает иммунитетом к этой технике.", ch, 0,
                    victim, TO_CHAR,POS_RESTING);
                return;
        }

        //////////////// PROBABILITY CHECKS ////////////////
            
        chance = 0;
  
        chance += gsn_throw->getEffective( ch ) * skill_mod;
        d.log(chance, "skill");
        chance += ( ch->getCurrStat(STAT_DEX) - victim->getCurrStat(STAT_DEX) ) * stat_mod * 100;
        d.log(chance, "stats");
        chance += ( skill_level(*gsn_throw, ch) - victim->getModifyLevel() ) * level_mod * 100;
        d.log(chance, "lvl");
        chance += (ch->size - victim->size) * size_mod * 100;
        d.log(chance, "size");
        chance += victim->can_see(ch) ? 0 : (vis_mod * 100);
        chance += IS_AWAKE( victim ) ? 0 : (sleep_mod * 100);    
        d.log(chance, "vis");
   
        if (IS_QUICK(ch)) {
                chance += quick_mod * 100;
                d.log(chance, "quick");
        }

        if (IS_QUICK(victim)) {
                chance -= quick_mod * 100;            
                d.log(chance, "quick");
        }

        if (IS_SET(victim->res_flags, RES_BASH)) {
                chance = chance / 2;
                d.log(chance, "res");
        }
            
        if ( IS_AFFECTED(ch,AFF_WEAK_STUN) ) {
                chance = chance / 2; 
                d.log(chance, "stun");
        }
        
        if (is_flying( victim )) {
                chance -= 10;
                d.log(chance, "fly");
        }

        d.log(chance, "final");

        //////////////// THE ROLL ////////////////
    
        ch->setWait( gsn_throw->getBeats(ch)  );
        UNSET_DEATH_TIME(ch);

        if (victim->isAffected(gsn_protective_shield))
        {
                oldact_p("{YТвоя попытка броска наталкивается на защитный щит!{x",
                                        ch,0,victim, TO_CHAR,POS_FIGHTING);
                oldact_p("{Y$c1 не смо$gгло|г|гла бросить тебя, натолкнувшись на защитный щит.{x",
                                        ch, 0, victim, TO_VICT,POS_FIGHTING);
                oldact_p("{Y$c1 распластывается по защитному щиту $C2 в попытке броска.{x",
                                        ch,0,victim,TO_NOTVICT,POS_FIGHTING);
                return;
        }


        if ( ch->is_npc() || number_percent() < chance )
        {
            if ( number_percent() < 70 ) {
                oldact_p("Ты бросаешь $C4 с {Wошеломляющей силой{x.",
                        ch,0,victim,TO_CHAR,POS_RESTING);
                oldact_p("$c1 бросает тебя с {Wошеломляющей силой{x.",
                        ch,0,victim,TO_VICT,POS_RESTING);
                oldact_p("$c1 бросает $C4 с {Wошеломляющей силой{x.",
                        ch,0,victim,TO_NOTVICT,POS_RESTING);
                victim->setWaitViolence( 2 + URANGE (0, ch->getCurrStat(STAT_STR) - victim->getCurrStat(STAT_STR), 2) );

                victim->position = POS_RESTING;
                if (is_flying( victim )) {
                    victim->posFlags.setBit( POS_FLY_DOWN );
                    victim->pecho( "Ты перестаешь летать." );
                    victim->recho( "%^C1 перестает летать.", victim ); 
                }    
            }
            else {
                oldact("Ты бросаешь $C4 через плечо.", ch,0,victim,TO_CHAR);
                oldact("$c1 бросает тебя через плечо.", ch,0,victim,TO_VICT);
                oldact("$c1 бросает $C4 через плечо.", ch,0,victim,TO_NOTVICT);
                victim->position = POS_RESTING;
            }        

            ThrowDownOneHit throwdown( ch, victim );
            try
            {
                throwdown.hit(true);
            }
            catch (const VictimDeathException &e){   
            }                     
          
            gsn_throw->improve( ch, true, victim );
        }
        else
        {
            oldact("Твой бросок не удался.", ch, 0, 0, TO_CHAR);
            oldact("$C1 пытается бросить тебя, но терпит неудачу.", victim, 0, ch,TO_CHAR);
            oldact("$c1 пытается ухватиться за $C4 поудобнее, но терпит неудачу.", ch, 0, victim, TO_NOTVICT);
            gsn_throw->improve( ch, false, victim );
        }

        if (!FightingCheck) {
            yell_panic( ch, victim,
                        "Помогите! Меня кто-то хватает!",
                        "Убери свои руки, %1$C1!" );
        
            multi_hit(victim,ch);
        }    
}

/*
 * 'strangle' skill command
 */

SKILL_RUNP( strangle )
{
        Debug d(ch, "debug_ninja", "strangle");
        Character *victim;
        Affect af;    
        float chance, skill_mod, stat_mod, level_mod, quick_mod, size_mod, sleep_mod, vis_mod, time_mod;
        char arg[MAX_INPUT_LENGTH];
        
        //////////////// BASE MODIFIERS //////////////// TODO: add this to XML
        skill_mod   = 0.5;
        stat_mod    = 0.04;
        level_mod   = 0.01;
        quick_mod   = 0.1;
        size_mod    = -0.03; // HARDER to affect smaller victims, easier to affect larger
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

        if ( ch->is_npc() || !gsn_strangle->usable( ch ) )
        {
                ch->pecho("Ты не умеешь душить.");
                return;
        }

        const GlobalBitvector &loc = ch->getWearloc( );
        
        if (!loc.isSet( wear_hands )
            || !loc.isSet( wear_wrist_l )
            || !loc.isSet( wear_wrist_r ))
        {
            ch->pecho("У тебя нет рук.");
            return;
        }

        if (!check_bare_hands(ch))        
        {
                ch->pecho("Освободи обе руки для этого.");
                return;
        }

        if ( IS_CHARMED(ch) )
        {
                ch->pecho("Ты же не хочешь придушить сво%1$Gего|его|ю хозя%1$Gина|ина|йку?", ch->master);
                return;
        }

        argument = one_argument(argument,arg);

        if ( arg[0] == '\0' )
        {
            ch->pecho("И кого ты хочешь придушить?");
            return;
        }
        
        if ( (victim = get_char_room(ch,arg)) == 0 )
        {
                ch->pecho("Здесь таких нет.");
                return;
        }

        if ( ch == victim )
        {
                ch->pecho("Ты смыкаешь руки на собственной шее и удовлетворенно хрипишь.");
                return;
        }

        if ( victim->isAffected(gsn_strangle) )
        {
                ch->pecho("Твоя жертва уже в отключке.");                
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
        
        if (IS_SET(victim->imm_flags, IMM_WEAPON))
        {
                oldact_p("$C1 имеет иммунитет к физическим воздействиям.", ch, 0,
                        victim, TO_CHAR,POS_RESTING);
                return;
        }

        // sleepy centaurs can't rearkick
        if ( IS_AWAKE(victim) && (gsn_rear_kick->getCommand( )->run( ch, victim )) )
            return;

        if(SHADOW(ch))
        {
                ch->pecho("Твои пальцы проходят сквозь тень!");
                oldact_p("$c1 пытается придушить собственную тень.",
                    ch, 0, 0, TO_ROOM,POS_RESTING);
                return;
        }

        //////////////// PROBABILITY CHECKS ////////////////
            
        chance = 0;
        chance += gsn_strangle->getEffective( ch ) * skill_mod;
        d.log(chance, "skill");
        chance += ( ch->getCurrStat(STAT_DEX) - victim->getCurrStat(STAT_CON) ) * stat_mod * 100;
        d.log(chance, "stats");
        chance += ( skill_level(*gsn_strangle, ch) - victim->getModifyLevel() ) * level_mod * 100;
        d.log(chance, "lvl");
        chance += (ch->size - victim->size) * size_mod * 100;
        d.log(chance, "size");
        chance += victim->can_see(ch) ? 0 : (vis_mod * 100);
        chance += IS_AWAKE( victim ) ? 0 : (sleep_mod * 100);            
        d.log(chance, "vis");

        if (IS_QUICK(ch)) {
            chance += quick_mod * 100;
            d.log(chance, "quick");
        }

        if (IS_QUICK(victim)) {
            chance -= quick_mod * 100;            
            d.log(chance, "quick");
        }

        if (IS_SET(victim->res_flags, RES_WEAPON)) {
            chance = chance / 2;
            d.log(chance, "res");
        }
            
        if ( IS_AFFECTED(ch,AFF_WEAK_STUN) ) {
            chance = chance / 2;
            d.log(chance, "stun");
        }
    
        // neckguard can't protect if you're asleep
        if ( (victim->isAffected(gsn_backguard)) && IS_AWAKE( victim ) ) {
            chance = chance / 2;
            d.log(chance, "backguard");
        }

        int k = victim->getLastFightDelay( );
        if (k >= 0 && k < FIGHT_DELAY_TIME) {
            chance -= (FIGHT_DELAY_TIME - k) * time_mod * 100;
            d.log(chance, "adrenaline");
        }
           
    
        chance = max( (float)1, chance ); // there's always a chance
        d.log(chance, "final");

        //////////////// THE ROLL ////////////////
    
        ch->setWait( gsn_strangle->getBeats(ch) );
        UNSET_DEATH_TIME(ch);
        victim->setLastFightTime( );
        ch->setLastFightTime( );  

        Chance mychance(ch, (int) chance, 100);

        if ( ch->is_npc() || mychance.reroll())
        {
                oldact_p("Ты смыкаешь руки на шее $C2 и $E погружается в сон.",
                        ch,0,victim,TO_CHAR,POS_RESTING);
                oldact_p("$c1 смыкает руки на твоей шее и ты погружаешься в сон.",
                        ch,0,victim,TO_VICT,POS_RESTING);
                oldact_p("$c1 смыкает руки на шее $C2 и $E погружается в сон.",
                        ch,0,victim,TO_NOTVICT,POS_RESTING);
                gsn_strangle->improve( ch, true, victim );
        
                af.type = gsn_strangle;
                af.bitvector.setTable(&affect_flags);
                af.level = skill_level(*gsn_strangle, ch);
                af.duration = skill_level(*gsn_strangle, ch) / 50 + 1;
                af.bitvector.setValue(AFF_SLEEP);
                affect_join ( victim,&af );
                
                set_violent( ch, victim, true );

                if (IS_AWAKE(victim))
                        victim->position = POS_SLEEPING;
        }
        else
        {
                damage(ch,victim, 0, gsn_strangle, DAM_NONE, true, DAMF_WEAPON);
                gsn_strangle->improve( ch, false, victim );
                
                yell_panic( ch, victim,
                            "Помогите! Меня кто-то душит!",
                            "Помогите! Меня душит %1$C1!" );
        }
}


/*
 * 'poison smoke' skill command
 */

SKILL_RUNP( poison )
{

        if (ch->is_npc())
                return;

        if (!gsn_poison_smoke->usable( ch ))
        {
                ch->pecho("Ты не владеешь этим навыком.");
                return;
        }

        if ( ch->mana < gsn_poison_smoke->getMana( ) )
        {
                ch->pecho("У тебя не хватает энергии для этого.");
                return;
        }

        ch->mana -= gsn_poison_smoke->getMana( );
        ch->setWait( gsn_poison_smoke->getBeats(ch) );
        UNSET_DEATH_TIME(ch);

        if ( number_percent() > gsn_poison_smoke->getEffective( ch ) )
        {
                ch->pecho("Твоя попытка закончилась неудачей.");
                gsn_poison_smoke->improve( ch, false );
                return;
        }

        ch->pecho("Облако отравленного дыма наполнило комнату.");
        oldact("Облако отравленного дыма наполнило комнату.",ch,0,0,TO_ROOM);

        gsn_poison_smoke->improve( ch, true );

        for ( auto &tmp_vict : ch->in_room->getPeople() )
        {
                if ( !is_safe_spell(ch,tmp_vict,true) )
                {
                        if (ch->fighting != tmp_vict && tmp_vict->fighting != ch)
                            yell_panic( ch, tmp_vict,
                                        "Помогите! Меня пытаются отравить!",
                                        "Помогите! %1$^C1 травит меня дымом!",
                                        FYP_SLEEP );

                        spell( gsn_poison, skill_level(*gsn_poison_smoke, ch), ch, tmp_vict );

                        if (tmp_vict != ch)
                                multi_hit(tmp_vict,ch);
        
                }
        }
}

/*
 * 'blindness dust' skill command
 */

SKILL_RUNP( blindness )
{
        if (ch->is_npc())
                return;
        
        if (!gsn_blindness_dust->usable(ch))
        {
                ch->pecho("Ты не владеешь этим навыком.");
                return;
        }

        if (ch->mana < gsn_blindness_dust->getMana( ))
        {
                ch->pecho("У тебя не хватает энергии для этого.");
                return;
        }

        ch->mana -= gsn_blindness_dust->getMana( );
        ch->setWait( gsn_blindness_dust->getBeats(ch) );
        UNSET_DEATH_TIME(ch);

        if (number_percent() > gsn_blindness_dust->getEffective( ch ) )
        {
                ch->pecho("Твоя попытка закончилась неудачей.");
                gsn_blindness_dust->improve( ch, false );
                return;
        }

        ch->pecho("Облако загадочной пыли наполнило комнату.");
        oldact("Облако загадочной пыли наполнило комнату.",ch,0,0,TO_ROOM);

        gsn_blindness_dust->improve( ch, true );
    
        gsn_blindness_dust->getCommand()->run(ch);
}

BOOL_SKILL( blindness )::run( Character *ch ) 
{
 

    for ( auto &tmp_vict : ch->in_room->getPeople() )
    {
        if (!is_safe_spell(ch,tmp_vict,true))
        {
            if (ch->fighting != tmp_vict && tmp_vict->fighting != ch)
                yell_panic( ch, tmp_vict,
                            "Помогите! Кто-то слепит меня пылью!",
                            "Помогите! %1$^C1 слепит меня пылью!",
                            FYP_SLEEP );
            
            spell( gsn_blindness, skill_level(*gsn_blindness_dust, ch), ch, tmp_vict );

            if (tmp_vict != ch)
                    multi_hit(tmp_vict,ch);
        }
    }
    return true;
}
