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
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/
#include "skill.h"
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

#include "magic.h"
#include "occupations.h"
#include "fight.h"
#include "onehit.h"
#include "onehit_weapon.h"
#include "damage_impl.h"
#include "chance.h"
#include "clan.h"
#include "vnum.h"
#include "merc.h"
#include "handler.h"
#include "act.h"
#include "interp.h"
#include "def.h"


/*
 * 'vanish' skill command
 */

SKILL_RUNP( vanish )
{
    Character *victim;
    float chance, kidnap_chance, skill_mod, stat_mod, level_mod, quick_mod, size_mod, sleep_mod, vis_mod;
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
            ch->send_to("Ты не можешь покинуть это место без посторонней помощи!\n\r");
            return;
    }

    if (ch->mount) {
        ch->pecho( "Ты не можешь исчезнуть, пока ты верхом или оседла%Gно|н|на!", ch );
        return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL))
    {
        ch->send_to("Из этого места нельзя исчезать.\n\r");
        return;
    }

    pRoomIndex = get_random_room_vanish( ch );
    
    if (!pRoomIndex) {
        ch->send_to("В этой зоне тебе некуда исчезать.\n\r");
        return;
    }
    
    ///// Standard checks: TODO: turn this into a function
    
    if ( !ch->is_npc() && !gsn_vanish->usable( ch ) )
    {
            ch->send_to("Ты не владеешь этим навыком.\n\r");
            return;
    }
    
    if (ch->mana < gsn_vanish->getMana( ))
    {
            ch->send_to("У тебя недостаточно энергии для этого.\n\r" );
            return;
    }
    
    /* TODO: Not sure if it needs the effect: wait_state is enough
    if (ch->isAffected(gsn_vanish)) {
        ch->send_to("Тебе пока нечего бросить.\r\n");
        return;
    } */
    
    // Needs at least one hand
    const GlobalBitvector &loc = ch->getWearloc( );
    if (!loc.isSet( wear_hands )
    || (!loc.isSet( wear_wrist_l ) && (!loc.isSet( wear_wrist_r )) ))
    {
            ch->send_to("Тебе нужна хотя бы одна рука для этой техники.\r\n");
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
                ch->send_to("Этого нет здесь.\n\r");
                return;
        }

        if ( ( victim->fighting != 0 ) && ( victim != ch ) )
        {
            ch->send_to("Подожди, пока закончится сражение.\n\r");
            return;
        }

        if ( IS_SET(victim->imm_flags,IMM_SUMMON) ) {
            ch->send_to(fmt(ch, "%^C4 нельзя переместить.\n\r", victim));
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
                ch->send_to("Ты пытаешься схватить в охапку свою тень!\n\r");
                act_p("$c1 пытается схватить в охапку свою тень.",
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
    ch->setWait( gsn_vanish->getBeats( )  );
    UNSET_DEATH_TIME(ch);
        
    if (number_percent() > gsn_vanish->getEffective( ch ) )
    {
            ch->send_to("Тебе не удается активировать световую гранату.\n\r");
            gsn_vanish->improve( ch, false );
            return;
    }    
    
    act_p( "$c1 бросает на землю небольшой шар. {WЯркая вспышка{x на мгновение ослепляет тебя!", ch, 0, 0, TO_ROOM,POS_RESTING);
    ch->send_to("Ты бросаешь на землю световую гранату. {WЯркая вспышка{x на мгновение ослепляет всех вокруг!\r\n");
    gsn_vanish->improve( ch, true );

    //////////////// PROBABILITY CHECKS: SELF ////////////////

    chance = 0;
    chance += gsn_vanish->getEffective( ch );

    if ( IS_AFFECTED(ch,AFF_WEAK_STUN) )
        chance = chance / 2;    
    
    if (FightingCheck) {
        
        chance = chance / 2; 
        
        chance += ch->fighting->can_see(ch) ? 0 : (vis_mod * 100);
        
        if ( IS_SET(ch->fighting->imm_flags,IMM_LIGHT) ) {
            ch->send_to(fmt(ch, "{W%^C1 не поддается воздействию вспышки!{x\n\r", ch->fighting));
            chance = 0;
        }
        if ( IS_SET(ch->fighting->res_flags,RES_LIGHT) ) {
            ch->send_to(fmt(ch, "{W%^C1 сопротивляется воздействию вспышки!{x\n\r", ch->fighting));
            chance = chance / 2; 
        }         
    }    

    //////////////// PROBABILITY CHECKS: KIDNAP ////////////////
    
    if (victim != 0) {
        kidnap_chance = 0;
        kidnap_chance += gsn_vanish->getEffective( ch ) * skill_mod;
        kidnap_chance += ( get_curr_stat_extra(ch, STAT_DEX) - get_curr_stat_extra(victim, STAT_STR) ) * stat_mod * 100;
        kidnap_chance += ( ch->getModifyLevel() - victim->getModifyLevel() ) * level_mod * 100;
        kidnap_chance += (ch->size - victim->size) * size_mod * 100;
        kidnap_chance += victim->can_see(ch) ? 0 : (vis_mod * 100);
        kidnap_chance += IS_AWAKE( victim ) ? 0 : (sleep_mod * 100);    
   
        if (IS_QUICK(ch))
                kidnap_chance += quick_mod * 100;
        if (IS_QUICK(victim))
                kidnap_chance -= quick_mod * 100;            

        if (IS_SET(victim->res_flags, RES_LIGHT))
                kidnap_chance = kidnap_chance / 2;
            
        if ( IS_AFFECTED(ch,AFF_WEAK_STUN) )
                kidnap_chance = kidnap_chance / 2;
        
        // neckguard can't protect if you're asleep
        if ( (victim->isAffected(gsn_backguard)) && IS_AWAKE( victim ) ) 
                kidnap_chance = kidnap_chance / 2;
        
        kidnap_chance = max( 1, kidnap_chance ); // there's always a chance        
    }

    //////////////// THE ROLL ////////////////
    
    if ( number_percent() > chance ) {
            if (FightingCheck) {
                act_p( "$c1 пытается скрыться, но противник бдит, и бой продолжается!", ch, 0, 0, TO_ROOM,POS_RESTING);
                ch->send_to("Ты пытаешься скрыться, но противник бдит, и бой продолжается!\r\n");
                return;
            }
            else {
                // weak stun is a bitch
                act_p( "$c1 пытается скрыться, но спотыкается и падает!", ch, 0, 0, TO_ROOM,POS_RESTING);                
                ch->send_to("Ты пытаешься скрыться, но спотыкаешься и падаешь!\r\n");
                return;                
            }    
    }
    
    if (victim = 0) {
        transfer_char( ch, ch, pRoomIndex,
            "%1^C1 внезапно исчезает!",
            "Пользуясь всеобщим замешательством, ты исчезаешь!",
            "%1^C1 внезапно появляется у тебя за спиной." );
        return;
    }    
    else {   
            // trying to kidnap
            act_p( "$c1 пытается взять $C4 в охапку!", ch, 0, victim, TO_NOTVICT,POS_RESTING );
            act_p( "Ты пытаешься взять $C4 в охапку.",   ch, 0, victim, TO_CHAR,POS_RESTING    );
            act_p( "$c1 пытается взять $C4 в охапку!", ch, 0, victim, TO_VICT,POS_RESTING    );

            if ( number_percent() < kidnap_chance ) {
                    // kidnapping success
                    transfer_char( ch, ch, pRoomIndex,
                        "%1^C1 внезапно исчезает!",
                        "Пользуясь всеобщим замешательством, ты исчезаешь!",
                        "%1^C1 внезапно появляется у тебя за спиной." );
            
                    transfer_char( victim, ch, pRoomIndex,
                        "%1^C1 исчезает вместе с %2^C5!",
                        "Ты хватаешь и утаскиваешь с собой %1^C4!",
                        "%2^C1 внезапно появляется, волоча за собой %1^C4." );
                    
                    if (!FightingCheck) {
                        yell_panic( ch, victim,
                            "Помогите! Меня кто-то похищает!",
                            "Убери свои лапы, %1$C1!" );
        
                        multi_hit(victim,ch);
                    }
            }
            else {
                    // kidnap failed, victim escaped               
                    act_p( "$C1 успевает вырваться из объятий $c2!", ch, 0, victim, TO_NOTVICT,POS_RESTING );
                    act_p( "$C1 успевает вырваться из твоих объятий!",   ch, 0, victim, TO_CHAR,POS_RESTING    );
                    act_p( "Ты умудряешься вырваться из объятий $c2", ch, 0, victim, TO_VICT,POS_RESTING    );

                    transfer_char( ch, ch, pRoomIndex,
                        "%1^C1 внезапно исчезает!",
                        "Пользуясь всеобщим замешательством, ты исчезаешь!",
                        "%1^C1 внезапно появляется у тебя за спиной." );
            }
      
     }
}

/*
 * 'nerve' skill command
 */

SKILL_RUNP( nerve )
{
        Character *victim;
        float chance, skill_mod, stat_mod, level_mod, quick_mod, size_mod, sleep_mod, vis_mod;
        bool FightingCheck;
        char arg[MAX_INPUT_LENGTH];
        
        //////////////// BASE MODIFIERS //////////////// TODO: add this to XML
        skill_mod   = 0.3;
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
                ch->send_to("Только не верхом!\n\r");
                return;
        }

        if (!gsn_nerve->usable( ch ) )
        {
                ch->send_to("Ты не владеешь этим навыком.\n\r");
                return;
        }

        // Needs at least one hand
        const GlobalBitvector &loc = ch->getWearloc( );
        if (!loc.isSet( wear_hands )
        || (!loc.isSet( wear_wrist_l ) && (!loc.isSet( wear_wrist_r )) ))
        {
                ch->send_to("Тебе нужна хотя бы одна рука для этой техники.\r\n");
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
                        ch->send_to("Сейчас ты не сражаешься!\n\r");
                        return;
                }
        }
        else if ((victim = get_char_room(ch,arg)) == 0)
        {
                ch->send_to("Этого нет здесь.\n\r");
                return;
        }

        if (is_safe(ch,victim))
        {            
                return;
        }

        if (IS_CHARMED(ch) && ch->master == victim)
        {
                act_p("Но $C1 твой друг!!!",ch,0,victim,TO_CHAR,POS_RESTING);
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
                ch->send_to("Ты трогаешь себя в нескольких неожиданных местах и довольно улыбаешься.\n\r");
                return;
        }
            
        if(SHADOW(ch))
        {
                ch->send_to("Твои пальцы проходят сквозь тень!\n\r");
                act_p("$c1 пытается потрогать свою тень.",
                                        ch, 0, 0, TO_ROOM,POS_RESTING);
                return;
        }    

        if ( victim->isAffected(gsn_nerve) )
        {
                ch->send_to("Ты не можешь сделать противника еще слабее.\n\r");
                return;
        }

        if (IS_SET(victim->imm_flags, IMM_DISEASE))
        {
                act_p("$C1 обладает иммунитетом к этой технике.", ch, 0,
                    victim, TO_CHAR,POS_RESTING);
                return;
        }            
            
        //////////////// PROBABILITY CHECKS ////////////////
            
        chance = 0;
  
        chance += gsn_nerve->getEffective( ch ) * skill_mod;
        chance += ( get_curr_stat_extra(ch, STAT_DEX) - get_curr_stat_extra(victim, STAT_CON) ) * stat_mod * 100;
        chance += ( ch->getModifyLevel() - victim->getModifyLevel() ) * level_mod * 100;
        chance += (ch->size - victim->size) * size_mod * 100;
        chance += victim->can_see(ch) ? 0 : (vis_mod * 100);
        chance += IS_AWAKE( victim ) ? 0 : (sleep_mod * 100);    
   
        if (IS_QUICK(ch))
                chance += quick_mod * 100;
        if (IS_QUICK(victim))
                chance -= quick_mod * 100;            

        if (IS_SET(victim->res_flags, RES_DISEASE))
                chance = chance / 2;
            
        if ( IS_AFFECTED(ch,AFF_WEAK_STUN) )
                chance = chance / 2;    

        //////////////// THE ROLL ////////////////
            
        ch->setWait( gsn_nerve->getBeats( )  );
        UNSET_DEATH_TIME(ch);
    
        if ( ch->is_npc() || number_percent() < (int) chance )
        {
                gsn_nerve->getCommand()->run(ch, victim);
                act_p("Ты ослабляешь $C4, пережимая нервные окончания.",ch,0,victim,TO_CHAR,POS_RESTING);
                act_p("$c1 ослабляет тебя, пережимая твои нервные окончания.",ch,0,victim,TO_VICT,POS_RESTING);
                act_p("$c1 ослабляет $C4",ch,0,victim,TO_NOTVICT,POS_RESTING);
                gsn_nerve->improve( ch, true, victim );
        }
        else
        {
                ch->send_to("Ты нажимаешь не туда, куда надо.\n\r");
                act_p("$c1 нажимает пальцами на твои нервные окончания, но ничего не происходит.",
                        ch,0,victim,TO_VICT,POS_RESTING);
                act_p("$c1 нажимает пальцами на нервные окончания $C2, но ничего не происходит.",
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
    level = ch->getModifyLevel();
    
    if (gsn_nerve->usable( ch ) )
        skill = gsn_nerve->getEffective( ch );
    else
        skill = 0;
    
    mod = -1 * (level/20 + skill/20 + 1);
    
    Affect af;
    af.where    = TO_AFFECTS;
    af.type     = gsn_nerve;
    af.level    = level;
    af.duration = level / 20;
    af.location = APPLY_STR;
    af.modifier = mod;
    af.bitvector = 0;

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
        ch->send_to("Только не верхом!\n\r");
        return;
  }
    
  if (ch->is_npc())
  {
        ch->send_to("Выносливость -- не твой удел.\n\r");
        return;
  }

  if ( gsn_endure->getEffective( ch ) <= 1 )
  {
        ch->send_to("Тебе недоступна техника выносливости.\n\r");
        return;
  }

  if (ch->isAffected(gsn_endure))
    {
      ch->send_to("Ты не можешь стать еще выносливее.\n\r");
      return;
    }
    
    level = ch->getModifyLevel();
    
    if (gsn_endure->usable( ch ) )
        skill = gsn_endure->getEffective( ch );
    else
        skill = 0;
    
    mod = -1 * (level/20 + skill/20 + 1); 
    
    ch->setWait( gsn_endure->getBeats( )  );
    gsn_endure->getCommand()->run(ch, mod);
    gsn_endure->improve( ch, true );
}


BOOL_SKILL(endure)::run(Character *ch, int modifier)
{      
    Affect af;

    af.where         = TO_AFFECTS;
    af.type         = gsn_endure;
    af.level         = ch->getModifyLevel();
    af.duration   = ch->getModifyLevel() / 4;
    af.location = APPLY_SAVING_SPELL;
    af.modifier = modifier;    
    af.bitvector = 0;

    affect_to_char(ch,&af);

    act("Ты мгновенно концентрируешься, готовясь к столкновению с магией.", ch, 0, 0, TO_CHAR);
    act("$c1 мгновенно концентрируется, готовясь к столкновению с магией.", ch,0,0,TO_ROOM);
    return true;
}


/*----------------------------------------------------------------------------
 * Assassinate 
 *---------------------------------------------------------------------------*/
class AssassinateOneHit: public SkillWeaponOneHit {
public:
    AssassinateOneHit( Character *ch, Character *victim );

    virtual void calcDamage( );
};

AssassinateOneHit::AssassinateOneHit( Character *ch, Character *victim )
            : Damage(ch, victim, 0, 0, DAMF_WEAPON), SkillWeaponOneHit( ch, victim, gsn_assassinate )
{
}

void AssassinateOneHit::calcDamage( )
{
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
    chance += ( get_curr_stat_extra(ch, STAT_STR) - get_curr_stat_extra(victim, STAT_CON) ) * stat_mod * 100;
    chance += ( ch->getModifyLevel() - victim->getModifyLevel() ) * level_mod * 100;
    chance += (ch->size - victim->size) * size_mod * 100;
    chance += victim->can_see(ch) ? 0 : (vis_mod * 100);
    chance += IS_AWAKE( victim ) ? 0 : (sleep_mod * 100);            
    if (IS_QUICK(ch))
        chance += quick_mod * 100;
    if (IS_QUICK(victim))
        chance -= quick_mod * 100;            

    if (IS_SET(victim->res_flags, RES_WEAPON))
        chance = chance / 2;
            
    if ( IS_AFFECTED(ch,AFF_WEAK_STUN) )
        chance = chance / 2; 
    
    // neckguard can't protect if you're asleep
    if ( (victim->isAffected(gsn_backguard)) && IS_AWAKE( victim ) ) 
        chance = chance / 2;    

    // only check for assassinate spam without strangle
    int k = ch->getLastFightDelay( );
    if (k >= 0 && k < FIGHT_DELAY_TIME && IS_AWAKE( victim ))
        chance -= (FIGHT_DELAY_TIME - k) * time_mod * 100;
        
    UNSET_DEATH_TIME(ch);
    victim->setLastFightTime( );
    ch->setLastFightTime( );    
    
    chance = max( 1, chance ); // there's always a chance

    //////////////// THE ROLL ////////////////
    
    Chance mychance(ch, (int) chance, 100);

    if (mychance.reroll()) {
        act_p("Ты {R+++ ЛОМАЕШЬ ШЕЮ +++{x $C3!",ch,0,victim,TO_CHAR,POS_RESTING);
        act_p("$c1 {R+++ ЛОМАЕТ ШЕЮ +++{x $C3!",ch,0,victim,TO_NOTVICT,POS_RESTING);
        act_p("$c1 {R+++ ЛОМАЕТ ТЕБЕ ШЕЮ +++{x!",ch,0,victim,TO_VICT,POS_DEAD);

        gsn_assassinate->improve( ch, true, victim );

        handleDeath( );
        throw VictimDeathException( );
    }
    else
    {
        gsn_assassinate->improve( ch, false, victim );
        damBase( );
        gsn_enhanced_damage->getCommand( )->run( ch, victim, dam );
        damApplyPosition( );  
        dam *= 2;
        damApplyDamroll( );
        WeaponOneHit::calcDamage( );        
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
            ch->send_to("Только не верхом!\n\r");
            return;
    }
    one_argument( argument, arg );

    if ( ch->master != 0 && ch->is_npc() )
            return;

    if ( !ch->is_npc() && !gsn_assassinate->usable( ch ) )
    {
            ch->send_to("Ты не имеешь понятия, как ломать шеи.\n\r");
            return;
    }

    if ( IS_CHARMED(ch) )
    {
            ch->pecho( "Ты же не хочешь убить сво%1$Gего|его|ю любим%1$Gого|ого|ю хозя%1$Gина|ина|йку.", ch->master);
            return;
    }

    if ( arg[0] == '\0' )
    {
            ch->send_to("Кого убиваем СЕГОДНЯ?\n\r");
            return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
            ch->send_to("Этого нет здесь.\n\r");
            return;
    }

    if ( victim == ch )
    {
            ch->send_to("Твой путь не включает в себя суицид.\n\r");
            return;
    }

    if ( is_safe( ch, victim ) )
            return;

    if ( victim->is_immortal() && !victim->is_npc() )
    {
            ch->send_to("На Бессмертных это не подействует.\n\r");
            return;
    }

    if ( victim->fighting != 0 )
    {
            ch->send_to("Подожди, пока закончится сражение.\n\r");
            return;
    }
    
    const GlobalBitvector &loc = ch->getWearloc( );
    
    if (!loc.isSet( wear_hands )
        || !loc.isSet( wear_wrist_l )
        || !loc.isSet( wear_wrist_r ))
    {
        ch->send_to("У тебя нет рук.\r\n");
        return;
    }

    if (!check_bare_hands(ch))        
    {
            ch->send_to("Освободи обе руки для этого.\n\r");
            return;
    }

    if ( victim->hit < victim->max_hit
            && victim->can_see(ch)
            && IS_AWAKE(victim) )
    {
            act_p( "$C1 ран$Gено|ен|ена и настороженно оглядывается... ты не можешь подкрасться.",
                    ch, 0, victim, TO_CHAR,POS_RESTING);
            return;
    }

    if (IS_SET(victim->imm_flags, IMM_WEAPON))
    {
            act_p("$C1 имеет слишком крепкую шею, чтобы ее можно было сломать.", ch, 0,
                    victim, TO_CHAR,POS_RESTING);
            return;
    }

    // strangled centaurs can't rearkick
    if ( IS_AWAKE(victim) && (gsn_rear_kick->getCommand( )->run( ch, victim )) )
        return;

    if(SHADOW(ch))
    {
            ch->send_to("Твои пальцы проходят сквозь тень!\n\r");
            act_p("$c1 пытается сломать шею своей тени.",
                    ch, 0, 0, TO_ROOM,POS_RESTING);
            return;
    }   
    
    ch->setWait( gsn_assassinate->getBeats( )  );
    AssassinateOneHit ass( ch, victim );    
    
    // assassination attempt can't "miss"
    try {
        ass.hit( );       
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
  Character *victim;
  float chance, skill_mod, stat_mod, quick_mod, size_mod, sleep_mod, vis_mod;
  bool FightingCheck;
  char arg[MAX_INPUT_LENGTH];
    
  //////////////// BASE MODIFIERS //////////////// TODO: add this to XML
  skill_mod   = 0.3;
  stat_mod    = 0.05;
  quick_mod   = 0.1;
  size_mod    = -0.1; // HARDER to affect smaller victims, easier to affect larger
  sleep_mod   = 0.1;
  vis_mod     = 0.1;    
    
  //////////////// ELIGIBILITY CHECKS ////////////////

  ///// Standard checks: TODO: turn this into a function
    
  if (ch->is_npc() || !gsn_caltraps->usable( ch ))
  {
        ch->send_to("Ты не владеешь этим навыком.\n\r");
        return;
  }

  // Needs at least one hand
  const GlobalBitvector &loc = ch->getWearloc( );
  if (!loc.isSet( wear_hands )
  || (!loc.isSet( wear_wrist_l ) && (!loc.isSet( wear_wrist_r )) ))
  {
        ch->send_to("Тебе нужна хотя бы одна рука для этой техники.\r\n");
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
                ch->send_to("Сейчас ты не сражаешься!\n\r");
                return;
        }
   }
   else if ((victim = get_char_room(ch,arg)) == 0)
   {
        ch->send_to("Этого нет здесь.\n\r");
        return;
   }

   if (is_safe(ch,victim))
   {            
        return;
   }

   if (IS_CHARMED(ch) && ch->master == victim)
   {
        act_p("Но $C1 твой друг!!!",ch,0,victim,TO_CHAR,POS_RESTING);
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
        ch->send_to("Твои шипы не смогут навредить летучему противнику.\n\r");
        return;
   }      

   if (victim == ch)
   {
        ch->send_to("Ты задумчиво колешь себя острым шипом в пятку. Ай!\n\r");
        return;
   }
            
   if(SHADOW(ch))
   {
        ch->send_to("Твои шипы проходят сквозь тень!\n\r");
        act_p("$c1 бросает шипы под ноги собственной тени.",
                ch, 0, 0, TO_ROOM,POS_RESTING);
        return;
   }    

   if ( victim->isAffected(gsn_caltraps) )
   {
        ch->send_to("Противник уже хромает.\n\r");
        return;
   }

   if (IS_SET(victim->imm_flags, IMM_PIERCE))
   {
        act_p("$C1 обладает иммунитетом к острым шипам.", ch, 0,
                victim, TO_CHAR,POS_RESTING);
        return;
   }       

   //////////////// PROBABILITY CHECKS ////////////////
            
   chance = 0;
        
   chance += gsn_caltraps->getEffective( ch ) * skill_mod;
   chance += ( get_curr_stat_extra(ch, STAT_DEX) - get_curr_stat_extra(victim, STAT_DEX) ) * stat_mod * 100;
   // chance += ( ch->getModifyLevel() - victim->getModifyLevel() ) * level_mod * 100; // no level check for caltraps
   chance += (ch->size - victim->size) * size_mod * 100;
   chance += victim->can_see(ch) ? 0 : (vis_mod * 100);
   chance += IS_AWAKE( victim ) ? 0 : (sleep_mod * 100);            
   if (IS_QUICK(ch))
        chance += quick_mod * 100;
   if (IS_QUICK(victim))
        chance -= quick_mod * 100;            

   if (IS_SET(victim->res_flags, RES_PIERCE))
        chance = chance / 2;
          

  //////////////// THE ROLL ////////////////      
      
  act_p("Ты кидаешь пригоршню острых шипов под ноги $C3.",
         ch,0,victim,TO_CHAR,POS_RESTING);
  act_p("$c1 кидает пригоршню острых шипов тебе под ноги!",
         ch,0,victim,TO_VICT,POS_RESTING);

  ch->setWait( gsn_caltraps->getBeats( ) );
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
    level = ch->getModifyLevel();
    
    if (gsn_caltraps->usable( ch ) )
        skill = gsn_caltraps->getEffective( ch );
    else
        skill = 0;
    
    mod = -1 * (level/10 + skill/10 + 1);
    
    try {
        damage_nocatch(ch,victim, ch->getModifyLevel(),gsn_caltraps,DAM_PIERCE, true, DAMF_WEAPON);

        if (!victim->isAffected(gsn_caltraps)) {
            Affect tohit,todam,todex;

            tohit.where     = TO_AFFECTS;
            tohit.type      = gsn_caltraps;
            tohit.level     = level;
            tohit.duration  = -1;
            tohit.location  = APPLY_HITROLL;
            tohit.modifier  = mod;
            tohit.bitvector = 0;
            affect_to_char( victim, &tohit );

            todam.where = TO_AFFECTS;
            todam.type = gsn_caltraps;
            todam.level = level;
            todam.duration = -1;
            todam.location = APPLY_DAMROLL;
            todam.modifier = mod;
            todam.bitvector = 0;
            affect_to_char( victim, &todam);

            todex.type = gsn_caltraps;
            todex.level = ch->getModifyLevel();
            todex.duration = -1;
            todex.location = APPLY_DEX;
            todex.modifier = mod/2;
            todex.bitvector = 0;
            affect_to_char( victim, &todex);

            act_p("Острые шипы вонзаются в ступни $C2, стесняя движения и вызывая хромоту.",ch,0,victim,TO_CHAR,POS_RESTING);
            act_p("Острые шипы вонзаются в твои ступни, стесняя движения и вызывая хромоту.",ch,0,victim,TO_VICT,POS_RESTING);
        }
    } catch (const VictimDeathException &) {
    }

    return true;
}


/*
 * 'throw' skill command
 */

SKILL_RUNP( throwdown )
{
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
                ch->send_to("Только не верхом!\n\r");
                return;
        }

        if ( ch->is_npc() || !gsn_throw->usable( ch ) )
        {
                ch->send_to("Ты не владеешь этим навыком.\n\r");
                return;
        }

        // Needs at least one hand
        const GlobalBitvector &loc = ch->getWearloc( );
        if (!loc.isSet( wear_hands )
        || (!loc.isSet( wear_wrist_l ) && (!loc.isSet( wear_wrist_r )) ))
        {
                ch->send_to("Тебе нужна хотя бы одна рука для этой техники.\r\n");
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
                        ch->send_to("Сейчас ты не сражаешься!\n\r");
                        return;
                }
        }
        else if ((victim = get_char_room(ch,arg)) == 0)
        {
                ch->send_to("Этого нет здесь.\n\r");
                return;
        }

        if (is_safe(ch,victim))
        {            
                return;
        }

        if (IS_CHARMED(ch) && ch->master == victim)
        {
                act_p("Но $C1 твой друг!!!",ch,0,victim,TO_CHAR,POS_RESTING);
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
                ch->send_to("Твои ноги должны стоять на земле для упора.\n\r");
                return;
        }

        if (victim == ch)
        {
                ch->send_to("Ты крепко обнимаешь себя и в экстазе падаешь на землю!\n\r");
                return;
        }
            
        if(SHADOW(ch))
        {
                ch->send_to("Твой захват проходит сквозь тень!\n\r");
                act_p("$c1 пытается бросить через плечо свою тень.",
                                        ch, 0, 0, TO_ROOM,POS_RESTING);
                return;
        } 

        if (IS_SET(victim->imm_flags, IMM_BASH))
        {
                act_p("$C1 обладает иммунитетом к этой технике.", ch, 0,
                    victim, TO_CHAR,POS_RESTING);
                return;
        }

        //////////////// PROBABILITY CHECKS ////////////////
            
        chance = 0;
  
        chance += gsn_throw->getEffective( ch ) * skill_mod;
        chance += ( get_curr_stat_extra(ch, STAT_DEX) - get_curr_stat_extra(victim, STAT_DEX) ) * stat_mod * 100;
        chance += ( ch->getModifyLevel() - victim->getModifyLevel() ) * level_mod * 100;
        chance += (ch->size - victim->size) * size_mod * 100;
        chance += victim->can_see(ch) ? 0 : (vis_mod * 100);
        chance += IS_AWAKE( victim ) ? 0 : (sleep_mod * 100);    
   
        if (IS_QUICK(ch))
                chance += quick_mod * 100;
        if (IS_QUICK(victim))
                chance -= quick_mod * 100;            

        if (IS_SET(victim->res_flags, RES_BASH))
                chance = chance / 2;
            
        if ( IS_AFFECTED(ch,AFF_WEAK_STUN) )
                chance = chance / 2; 
        
        if (is_flying( victim ))
                chance -= 10;

        //////////////// THE ROLL ////////////////
    
        ch->setWait( gsn_throw->getBeats( )  );
        UNSET_DEATH_TIME(ch);

        if (victim->isAffected(gsn_protective_shield))
        {
                act_p("{YТвоя попытка броска наталкивается на защитный щит!{x",
                                        ch,0,victim, TO_CHAR,POS_FIGHTING);
                act_p("{Y$c1 не смо$gгло|г|гла бросить тебя, натолкнувшись на защитный щит.{x",
                                        ch, 0, victim, TO_VICT,POS_FIGHTING);
                act_p("{Y$c1 распластывается по защитному щиту $C2 в попытке броска.{x",
                                        ch,0,victim,TO_NOTVICT,POS_FIGHTING);
                return;
        }


        if ( ch->is_npc() || number_percent() < chance )
        {
            if ( number_percent() < 70 ) {
                act_p("Ты бросаешь $C4 с ошеломляющей силой.",
                        ch,0,victim,TO_CHAR,POS_RESTING);
                act_p("$c1 бросает тебя с ошеломляющей силой.",
                        ch,0,victim,TO_VICT,POS_RESTING);
                act_p("$c1 бросает $C4 с ошеломляющей силой.",
                        ch,0,victim,TO_NOTVICT,POS_RESTING);
                victim->setWaitViolence( 2 + max(2, ch->getCurrStat(STAT_STR) - victim->getCurrStat(STAT_STR)) );

                victim->position = POS_RESTING;
                if (is_flying( victim )) {
                    victim->posFlags.setBit( POS_FLY_DOWN );
                    victim->println( "Ты перестаешь летать." );
                    victim->recho( "%^C1 перестает летать.", victim ); 
                }    
            }
            else {
                act("Ты бросаешь $C4 через плечо.", ch,0,victim,TO_CHAR);
                act("$c1 бросает тебя через плечо.", ch,0,victim,TO_VICT);
                act("$c1 бросает $C4 через плечо.", ch,0,victim,TO_NOTVICT);
                victim->position = POS_RESTING;
            }        

            //dam is a member of Damage class. this will work without declaring dam after enhanceddamage changes are merged
            int dam = ch->getModifyLevel() + get_curr_stat_extra(ch, STAT_STR) + ch->damroll / 2;
            gsn_enhanced_damage->getCommand( )->run( ch, victim, dam );;

            damage( ch, victim, dam, gsn_throw, DAM_BASH, true, DAMF_WEAPON );
            gsn_throw->improve( ch, true, victim );
        }
        else
        {
            act( "Твой бросок не удался.", ch, 0, 0, TO_CHAR);
            act( "$C1 пытается бросить тебя, но терпит неудачу.", victim, 0, ch,TO_CHAR);
            act( "$c1 пытается ухватиться за $C4 поудобнее, но терпит неудачу.", ch, 0, victim, TO_NOTVICT);
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
        Character *victim;
        Affect af;    
        float chance, skill_mod, stat_mod, level_mod, quick_mod, size_mod, sleep_mod, vis_mod;
        char arg[MAX_INPUT_LENGTH];
        
        //////////////// BASE MODIFIERS //////////////// TODO: add this to XML
        skill_mod   = 0.2;
        stat_mod    = 0.04;
        level_mod   = 0.01;
        quick_mod   = 0.1;
        size_mod    = -0.03; // HARDER to affect smaller victims, easier to affect larger
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

        if ( ch->is_npc() || !gsn_strangle->usable( ch ) )
        {
                ch->send_to("Ты не умеешь душить.\n\r");
                return;
        }

        const GlobalBitvector &loc = ch->getWearloc( );
        
        if (!loc.isSet( wear_hands )
            || !loc.isSet( wear_wrist_l )
            || !loc.isSet( wear_wrist_r ))
        {
            ch->send_to("У тебя нет рук.\r\n");
            return;
        }

        if (!check_bare_hands(ch))        
        {
                ch->send_to("Освободи обе руки для этого.\n\r");
                return;
        }

        if ( IS_CHARMED(ch) )
        {
                ch->pecho("Ты же не хочешь придушить сво%1$Gего|его|ю хозя%1$Gина|ина|йку?", ch->master);
                return;
        }

        if ( (victim = get_char_room(ch,argument)) == 0 )
        {
                ch->send_to("Здесь таких нет.\n\r");
                return;
        }

        if ( ch == victim )
        {
                ch->send_to("Ты смыкаешь руки на собственной шее и удовлетворенно хрипишь.\n\r");
                return;
        }

        if ( victim->isAffected(gsn_strangle) )
        {
                ch->send_to("Твоя жертва уже в отключке.\n\r");                
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
        
        if (IS_SET(victim->imm_flags, IMM_WEAPON))
        {
                act_p("$C1 имеет иммунитет к физическим воздействиям.", ch, 0,
                        victim, TO_CHAR,POS_RESTING);
                return;
        }

        // sleepy centaurs can't rearkick
        if ( IS_AWAKE(victim) && (gsn_rear_kick->getCommand( )->run( ch, victim )) )
            return;

        if(SHADOW(ch))
        {
                ch->send_to("Твои пальцы проходят сквозь тень!\n\r");
                act_p("$c1 пытается придушить собственную тень.",
                    ch, 0, 0, TO_ROOM,POS_RESTING);
                return;
        }

        //////////////// PROBABILITY CHECKS ////////////////
            
        chance = 0;
    
        chance += gsn_strangle->getEffective( ch ) * skill_mod;
        chance += ( get_curr_stat_extra(ch, STAT_DEX) - get_curr_stat_extra(victim, STAT_CON) ) * stat_mod * 100;
        chance += ( ch->getModifyLevel() - victim->getModifyLevel() ) * level_mod * 100;
        chance += (ch->size - victim->size) * size_mod * 100;
        chance += victim->can_see(ch) ? 0 : (vis_mod * 100);
        chance += IS_AWAKE( victim ) ? 0 : (sleep_mod * 100);            
        if (IS_QUICK(ch))
            chance += quick_mod * 100;
        if (IS_QUICK(victim))
            chance -= quick_mod * 100;            

        if (IS_SET(victim->res_flags, RES_WEAPON))
            chance = chance / 2;
            
        if ( IS_AFFECTED(ch,AFF_WEAK_STUN) )
            chance = chance / 2; 
    
        // neckguard can't protect if you're asleep
        if ( (victim->isAffected(gsn_backguard)) && IS_AWAKE( victim ) ) 
            chance = chance / 2;   

        int k = ch->getLastFightDelay( );
        if (k >= 0 && k < FIGHT_DELAY_TIME)
            chance -= (FIGHT_DELAY_TIME - k) * time_mod * 100;
        
        UNSET_DEATH_TIME(ch);
        victim->setLastFightTime( );
        ch->setLastFightTime( );    
    
        chance = max( 1, chance ); // there's always a chance

        //////////////// THE ROLL ////////////////
    
        ch->setWait( gsn_strangle->getBeats( ) );
        UNSET_DEATH_TIME(ch);

        Chance mychance(ch, (int) chance, 100);

        if ( ch->is_npc() || mychance.reroll())
        {
                act_p("Ты смыкаешь руки на шее $C2 и $E погружается в сон.",
                        ch,0,victim,TO_CHAR,POS_RESTING);
                act_p("$c1 смыкает руки на твоей шее и ты погружаешься в сон.",
                        ch,0,victim,TO_VICT,POS_RESTING);
                act_p("$c1 смыкает руки на шее $C2 и $E погружается в сон.",
                        ch,0,victim,TO_NOTVICT,POS_RESTING);
                gsn_strangle->improve( ch, true, victim );
        
                af.type = gsn_strangle;
                af.where = TO_AFFECTS;
                af.level = ch->getModifyLevel();
                af.duration = ch->getModifyLevel() / 20 + 1;
                af.location = APPLY_NONE;
                af.modifier = 0;
                af.bitvector = AFF_SLEEP;
                affect_join ( victim,&af );
                
                set_violent( ch, victim, true );
                set_backguard( victim );

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
        Character *tmp_vict;

        if (ch->is_npc())
                return;

        if (!gsn_poison_smoke->usable( ch ))
        {
                ch->send_to("Ты не владеешь этим навыком.\n\r");
                return;
        }

        if ( ch->mana < gsn_poison_smoke->getMana( ) )
        {
                ch->send_to("У тебя не хватает энергии для этого.\n\r");
                return;
        }

        ch->mana -= gsn_poison_smoke->getMana( );
        ch->setWait( gsn_poison_smoke->getBeats( ) );
        UNSET_DEATH_TIME(ch);

        if ( number_percent() > gsn_poison_smoke->getEffective( ch ) )
        {
                ch->send_to("Твоя попытка закончилась неудачей.\n\r");
                gsn_poison_smoke->improve( ch, false );
                return;
        }

        ch->send_to("Облако отравленного дыма наполнило комнату.\n\r");
        act_p("Облако отравленного дыма наполнило комнату.",ch,0,0,TO_ROOM,POS_RESTING);

        gsn_poison_smoke->improve( ch, true );

        for ( tmp_vict=ch->in_room->people; tmp_vict!=0; tmp_vict=tmp_vict->next_in_room )
        {
                if ( !is_safe_spell(ch,tmp_vict,true) )
                {
                        if (ch->fighting != tmp_vict && tmp_vict->fighting != ch)
                            yell_panic( ch, tmp_vict,
                                        "Помогите! Меня пытаются отравить!",
                                        "Помогите! %1$^C1 травит меня дымом!",
                                        FYP_SLEEP );

                        spell( gsn_poison, ch->getModifyLevel( ), ch, tmp_vict );

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
                ch->send_to("Ты не владеешь этим навыком.\n\r");
                return;
        }

        if (ch->mana < gsn_blindness_dust->getMana( ))
        {
                ch->send_to("У тебя не хватает энергии для этого.\n\r");
                return;
        }

        ch->mana -= gsn_blindness_dust->getMana( );
        ch->setWait( gsn_blindness_dust->getBeats( ) );
        UNSET_DEATH_TIME(ch);

        if (number_percent() > gsn_blindness_dust->getEffective( ch ) )
        {
                ch->send_to("Твоя попытка закончилась неудачей.\n\r");
                gsn_blindness_dust->improve( ch, false );
                return;
        }

        ch->send_to("Облако загадочной пыли наполнило комнату.\n\r");
        act_p("Облако загадочной пыли наполнило комнату.",ch,0,0,TO_ROOM,POS_RESTING);

        gsn_blindness_dust->improve( ch, true );
    
        gsn_blindness_dust->getCommand()->run(ch);
}

BOOL_SKILL( blindness )::run( Character *ch ) 
{
    Character *tmp_vict;

    for ( tmp_vict=ch->in_room->people; tmp_vict!=0; tmp_vict=tmp_vict->next_in_room )
    {
        if (!is_safe_spell(ch,tmp_vict,true))
        {
            if (ch->fighting != tmp_vict && tmp_vict->fighting != ch)
                yell_panic( ch, tmp_vict,
                            "Помогите! Кто-то слепит меня пылью!",
                            "Помогите! %1$^C1 слепит меня пылью!",
                            FYP_SLEEP );
            
            spell( gsn_blindness, ch->getModifyLevel( ), ch, tmp_vict );

            if (tmp_vict != ch)
                    multi_hit(tmp_vict,ch);
        }
    }
    return true;
}
