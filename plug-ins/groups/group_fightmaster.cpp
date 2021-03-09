
/* $Id: group_fightmaster.cpp,v 1.1.2.24.6.20 2010-09-01 21:20:45 rufina Exp $
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

#include "logstream.h"
#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "commonattributes.h"
#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "race.h"
#include "object.h"
#include "gsn_plugin.h"
#include "act_move.h"
#include "mercdb.h"

#include "onehit.h"
#include "onehit_weapon.h"
#include "magic.h"
#include "fight.h"
#include "damage.h"
#include "skill_utils.h"
#include "clanreference.h"
#include "vnum.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "material.h"
#include "interp.h"
#include "def.h"
#include "morphology.h"

GSN(riding);
CLAN(shalafi);
PROF(anti_paladin);
PROF(samurai);

/*
 * 'bash door' skill command
 */

SKILL_RUNP( bashdoor )
{
        char arg[MAX_INPUT_LENGTH];
        Character *gch;
        int chance=0;
        EXTRA_EXIT_DATA *peexit = 0;
        int damage_bash,door=0;
        Room *room = ch->in_room;

        one_argument( argument, arg );

        if (!gsn_bash_door->getEffective( ch ))
        {
                ch->pecho("Выбить дверь? Как это?");
                return;
        }

        if (MOUNTED(ch))
        {
                ch->pecho("Только не верхом!");
                return;
        }

        if (RIDDEN(ch))
        {
                ch->pecho("Ты не можешь выбить дверь, когда оседлан{Sfа{Sx.");
                return;
        }

        if (arg[0] == '\0')
        {
                ch->pecho("Выбить дверь в каком направлении?");
                return;
        }

        if (ch->fighting)
        {        
                ch->pecho("Сначала закончи сражение.");
                return;
        }

        peexit = room->extra_exits.find(arg);
        if ((!peexit || !ch->can_see( peexit ))
             && ( door = find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY ) ) < 0)
        {
                ch->pecho("Но тут нечего выбивать!");
                return;
        }

        int slevel = skill_level(*gsn_bash_door, ch);
                
        /* look for guards */
        for ( gch = room->people; gch; gch = gch->next_in_room )
        {
                if ( gch->is_npc() && IS_AWAKE(gch) && slevel + 5 < gch->getModifyLevel() )
                {
                        act("%2$^C1 стоит слишком близко к двери.", ch, gch, 0,TO_CHAR);
                        return;
                }
        }

        // 'bash door'
        EXIT_DATA *pexit = 0;
        EXIT_DATA *pexit_rev = 0;
        int        exit_info;

        if ( peexit != 0 )
        {
                door = DIR_SOMEWHERE;
                exit_info = peexit->exit_info;
        }
        else
        {
                pexit = room->exit[door];
                exit_info = pexit->exit_info;
        }

        if ( !IS_SET(exit_info, EX_CLOSED) )
        {
                ch->pecho("Здесь уже открыто.");
                return;
        }

        if ( !IS_SET(exit_info, EX_LOCKED) )
        {
                ch->pecho("Просто попробуй открыть.");
                return;
        }

        if ( IS_SET(exit_info, EX_NOPASS) && !IS_SET(exit_info, EX_BASH_ONLY))
        {
                ch->pecho("Эту дверь невозможно вышибить.");
                return;
        }

        /* modifiers */

        /* size  and weight */
        chance += ch->getCarryWeight( ) / 100;

        chance += (ch->size - 2) * 20;

        /* stats */
        chance += ch->getCurrStat(STAT_STR);

        if (is_flying( ch ))
                chance -= 10;

        /* level
        chance += ch->getModifyLevel() / 10;
        */

        chance += ( gsn_bash_door->getEffective( ch ) - 90 + skill_level_bonus(*gsn_bash_door, ch) );
        const char *doorname = peexit ? peexit->short_desc_from : direction_doorname(pexit);
        oldact("Ты бьешь в $N4, пытаясь выбить!", ch,0, doorname,TO_CHAR);
        oldact("$c1 бьет в $N4, пытаясь выбить!", ch,0, doorname,TO_ROOM);

        if (room->isDark( ) && !IS_AFFECTED(ch, AFF_INFRARED ))
                chance /= 2;

        chance = URANGE( 3, chance, 98 );

        /* now the attack */
        if (number_percent() < chance )
        {
                gsn_bash_door->improve( ch, true );

                if ( peexit != 0 )
                {
                        REMOVE_BIT(peexit->exit_info, EX_LOCKED);
                        REMOVE_BIT(peexit->exit_info, EX_CLOSED);
                        act("%^C1 выбивает дверь.", ch, 0, 0,TO_ROOM);
                        act("Ты выбиваешь дверь!", ch, 0, 0,TO_CHAR);
                }
                else
                {
                        REMOVE_BIT(pexit->exit_info, EX_LOCKED);
                        REMOVE_BIT(pexit->exit_info, EX_CLOSED);
                        act("%^C1 выбивает дверь.", ch, 0, 0,TO_ROOM);
                        act("Ты выбиваешь дверь!", ch, 0, 0,TO_CHAR);

                        /* open the other side */
                        if ((pexit_rev = direction_reverse(room, door)))
                        {
                                REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
                                REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
                                direction_target(room, door)->echo(POS_RESTING, "%^N1 с грохотом вылетает.", doorname);
                        }
                }


                ch->setWait( gsn_bash_door->getBeats( )  );
        }
        else
        {
                act("Обессилев, ты падаешь лицом вниз!", ch,0,0,TO_CHAR);
                oldact("Обессилев, $c1 упа$gло|л|ла лицом вниз.", ch,0,0,TO_ROOM);
                gsn_bash_door->improve( ch, false );
                ch->position = POS_RESTING;
                ch->setWait( gsn_bash_door->getBeats( ) * 3 / 2  );
                damage_bash = ch->damroll + number_range(4,4 + 4* ch->size + chance/5);
                damage(ch,ch,damage_bash,gsn_bash_door, DAM_BASH, true, DAMF_WEAPON);
                if(IS_CHARMED(ch) && ch->master->getPC()){
                DLString petName = Syntax::noun(ch->getNameP('1'));
                ch->master->pecho(fmt(0,"%1$^C1 упа%1$Gло|л|ла и не может ходить и выполнять некоторые команды. Напиши {y{hc{lRприказать %2$s встать{lEorder %2$s stand{x, если хочешь продолжить выбивать %1$Gим|им|ей двери.",ch, petName.c_str()));
                }
        }

        return;
}


/*----------------------------------------------------------------------------
 * bash
 *---------------------------------------------------------------------------*/
class BashOneHit: public SkillDamage {
public:
    BashOneHit( Character *ch, Character *victim );

    virtual void calcDamage( );
    virtual int calcChance( );
    int chance = 0;
};

BashOneHit::BashOneHit( Character *ch, Character *victim )
            : Damage( ch, victim, DAM_BASH, 0 ), 
              SkillDamage( ch, victim, gsn_bash, DAM_BASH, 0, DAMF_WEAPON )
{
}



void BashOneHit::calcDamage( )
{
        dam = (ch->damroll / 2) + number_range(4,4 + 4* ch->size + chance/10);
        if ( is_flying( victim ) ) dam += dam/3;
        damApplyEnhancedDamage();
        Damage::calcDamage( );
}

int BashOneHit::calcChance( )
{

        chance = gsn_bash->getEffective( ch );

    /* modifiers */
        chance = chance * 4 / 5;

   /* size  and weight */
        chance += min(ch->canCarryWeight( ), ch->carry_weight) / 25;
        chance -= min(victim->canCarryWeight( ), victim->carry_weight) / 20;

        if ( ch->size < victim->size )
                chance += (ch->size - victim->size) * 25;
        else
                chance += (ch->size - victim->size) * 10;

        /* stats */
        chance += ch->getCurrStat(STAT_STR);
        chance -= victim->getCurrStat(STAT_DEX) * 4/3;

        /* speed */
        if (IS_QUICK(ch))
                chance += 10;
        if (IS_QUICK(victim))
                chance -= 20;

        /* level */
        chance += (skill_level(*gsn_bash, ch) - victim->getModifyLevel()) * 2;

        if ( is_flying( victim ) )
                chance -= 10;
        
        if (ch->getProfession( ) == prof_anti_paladin && ch->getClan( ) == clan_shalafi)
            chance /= 2;
        return chance;
}

/*
 * 'bash' skill command
 */

SKILL_RUNP( bash )
{
        char arg[MAX_INPUT_LENGTH];
        Character *victim;
        int wait;
        bool FightingCheck;

        if ( (MOUNTED(ch)) && (!gsn_riding->available(ch)) )
        {
                ch->pecho("Ты не знаешь, как применять такие навыки верхом.");
                return;
        }

        if ( ch->fighting != 0 )
                FightingCheck = true;
        else
                FightingCheck = false;

        argument = one_argument(argument,arg);

        if ( arg[0] != '\0' && !str_cmp(arg,"door") )
        {
                interpret_fmt( ch, "bashdoor %s", argument );
                return;
        }

        if ( gsn_bash->getEffective( ch ) <= 1)
        {        
                ch->pecho("Ударить щитом? Но как это сделать?");
                return;
        }

        if ( arg[0] == '\0' )
        {
                victim = ch->fighting;
                if ( victim == 0 )
                {
                        ch->pecho("Сейчас ты не сражаешься!");
                        return;
                }
        }
        else if ( (victim = get_char_room(ch,arg)) == 0 )
        {
                ch->pecho("Этого нет здесь.");
                return;
        }

        if ( ( victim->position < POS_FIGHTING ) && ( FightingCheck ) )
        {
                oldact("Подожди пока $E встанет.",ch,0,victim,TO_CHAR);
                return;
        }

        if ( victim == ch )
        {
                ch->pecho("Ударить щитом себя?");
                return;
        }

        if ( get_eq_char( ch, wear_shield ) == 0 )
        {
                ch->pecho("Тебе нужен щит чтобы сделать это!");
                if(IS_CHARMED(ch) && ch->master->getPC() && ch->canCarryNumber( ) > 0)
                ch->master->pecho("Для этого умения твоему последователю потребуется надеть щит.");
                return;
        }
        
        /*
        if ( is_flying( ch ) )
        {
                ch->pecho("В полете? И как ты себе это представляешь?");
                return;
        }*/

        if ( is_safe(ch,victim) )
                return;

        if ( IS_CHARMED(ch) && ch->master == victim )
        {
                act("Но %2$C1 твой друг!!!",ch,victim,0,TO_CHAR);
                return;
        }

        if (SHADOW(ch))
        {
                ch->pecho("Ты безуспешно пытаешься сбить с ног свою тень.");
                act("%^C1 бьет щитом свою тень.",ch,0,0,TO_ROOM);
                return;
        }

        if ( !ch->is_npc() && !ch->move )
        {
                ch->pecho("Ты слишком уста%Gло|л|ла для этого.", ch);
                return;
        }

        if ( MOUNTED(victim) )
        {
            if (victim->mount->is_npc( ) && gsn_riding->available( victim )) {
                ch->pecho("Ты не можешь сбить с ног того, кто верхом!");
                return;
            }
            
            interpret_raw( victim, "dismount" );
        }
        
        ch->move -= move_dec( ch );

        if ( victim->isAffected(gsn_protective_shield) )
        {
                oldact_p("{YТы пытаешься сбить с ног $C4, но что-то тебе мешает сделать это.{x", ch, 0, victim,TO_CHAR,POS_FIGHTING);
                oldact_p("{Y$c1 пытается сбить тебя с ног, но твоя защита мешает сделать это.{x", ch, 0,victim,TO_VICT,POS_FIGHTING);
                oldact_p("{Y$c1 пытается сбить с ног $C4, но что-то мешает сделать это.{x", ch,0,victim,TO_NOTVICT,POS_FIGHTING);

                ch->setWait( gsn_bash->getBeats( ) );
                return;
        }

       
        BashOneHit bash ( ch , victim);
        
        /* now the attack */

        if ( number_percent() < bash.calcChance() )
        {
                if ( number_percent() < 50 )
                {
                        oldact("Сильнейшим ударом щита $c1 сбивает тебя с ног и ты падаешь!", ch,0,victim,TO_VICT);
                        act("Ты сбиваешь %2$C4 с ног ударом щита!",ch,victim,0,TO_CHAR);
                        oldact("$c1 сильнейшим ударом щита сбивает $C4 с ног.", ch,0,victim,TO_NOTVICT);

                        wait = 3;

                        switch(number_bits(2))
                        {
                        case 0:
                                wait = 1;
                                break;
                        case 1:
                                wait = 2;
                                break;
                        case 2:
                                wait = 4;
                                break;
                        case 3:
                                wait = 3;
                                break;
                        }

                        victim->setWaitViolence( wait );
                        victim->position = POS_RESTING;
                }
                else
                {
                        act("%^C1 наносит тебе удар щитом!",ch,victim,0,TO_VICT);
                        act("Ты наносишь удар щитом %2$C3!",ch,victim,0,TO_CHAR);
                        act("%1$^C1 наносит удар щитом %2$C3.",ch,victim,0,TO_NOTVICT);
                }
                      
                gsn_bash->improve( ch, true, victim );
                ch->setWait( gsn_bash->getBeats( ) );
                try{
                bash.hit(true);
                }
                catch (const VictimDeathException &e){                        
                }
        }
        else
        {
                if (number_percent() < 50)
                {
                        act("Ты промахиваешься и падаешь!",ch,victim,0,TO_CHAR);
                        act("%^C1 промахивается и падает.",ch,victim,0,TO_NOTVICT);
                        oldact("$c1 пытается сбить тебя с ног, но промахивается и падает.", ch,0,victim,TO_VICT);
                        ch->position = POS_RESTING;
                        ch->setWait( gsn_bash->getBeats( ) * 3/2 );
                }
                else
                {
                        oldact("Ты промахиваешься и едва не падаешь!", ch,0,victim,TO_CHAR);
                        oldact("$c1 промахивается и едва не падает.", ch,0,victim,TO_NOTVICT);
                        oldact("$c1 пытается сбить тебя с ног, но промахивается и едва не падает.", ch,0,victim,TO_VICT);
                }        
                damage(ch,victim,0,gsn_bash,DAM_BASH, true, DAMF_WEAPON);
                gsn_bash->improve( ch, false, victim );
                ch->setWait( gsn_bash->getBeats( ) * 1/2 );
        }
        
    if (!FightingCheck)
        yell_panic( ch, victim,
                    "Помогите! Кто-то бьет меня щитом!",
                    "Помогите! %1$^C1 бьет меня щитом!" );
}


/*
 * 'trip' skill command
 */

SKILL_RUNP( trip )
{
        char arg[MAX_INPUT_LENGTH];
        Character *victim;
        int chance;
        bool FightingCheck;

        if ( MOUNTED(ch) )
        {
                ch->pecho("Ты не можешь подсечь противника, находясь в седле!");
                return;
        }

        if (ch->fighting != 0)
                FightingCheck = true;
        else
                FightingCheck = false;

        one_argument(argument,arg);

        if ( (chance = gsn_trip->getEffective( ch )) <= 1)
        {
                ch->pecho("Подсечь? Как это?");
                return;
        }

        chance = chance * 4 / 5;
    
        if ( arg[0] == '\0' )
        {
                victim = ch->fighting;
                if ( victim == 0 )
                {
                        ch->pecho("Сейчас ты не сражаешься!");
                        return;
                }
        }
        else if ( (victim = get_char_room(ch,arg)) == 0 )
        {
                ch->pecho("Этого нет здесь.");
                return;
        }

        if ( is_safe(ch,victim) )
                return;

        if ( is_flying( victim ) )
        {
                oldact("Но $S ноги не на земле.",ch,0,victim,TO_CHAR);
                return;
        }

        if ( is_flying( ch ) )
        {
                ch->pecho("В полете? И как ты себе это представляешь?");
                return;
        }

        if ( victim->position < POS_FIGHTING )
        {
                act("Но %2$C1 уже лежит...",ch,victim,0,TO_CHAR);
                return;
        }

        if ( !ch->is_npc() && !ch->move )
        {
                ch->pecho("Ты слишком уста%Gло|л|ла для этого.", ch);
                return;
        }

        if (SHADOW(ch))
        {
                ch->pecho("Твоя нога вязнет в твоей тени...");
                oldact("$c1 выделывает балетные па перед своей тенью.", ch, 0, 0, TO_ROOM);
                return;
        }

        if ( victim == ch )
        {
                ch->pecho("Ты запинаешься и падаешь!");
                ch->setWait( 2 * gsn_trip->getBeats( ) );
                act("%^C1 спотыкается о свои собственные ноги!",ch,0,0,TO_ROOM);
                return;
        }

        if ( IS_CHARMED(ch) && ch->master == victim )
        {
                oldact("Но ведь $C1 -- тво$Gй|й|я хозя$Gин|ин|йка.",ch,0,victim,TO_CHAR);
                return;
        }

        if ( MOUNTED(victim) )
        {
            if (victim->mount->is_npc( ) && gsn_riding->available( victim )) {
                ch->pecho("Ты не можешь подсечь того, кто верхом!");
                return;
            }

            interpret_raw( victim, "dismount" );
        }

        ch->move -= move_dec( ch );

        /* modifiers */

        /* size */
        if ( ch->size < victim->size )
                chance += (ch->size - victim->size) * 10; // bigger = harder to trip

        /* dex */
        chance += ch->getCurrStat(STAT_DEX);
        chance -= victim->getCurrStat(STAT_DEX) * 3 / 2;

        /* speed */
        if (IS_QUICK(ch))
                chance += 10;
        if (IS_QUICK(victim))
                chance -= 20;

        /* level */
        chance += (skill_level(*gsn_trip, ch) - victim->getModifyLevel()) * 2;

        if (ch->getProfession( ) == prof_anti_paladin && ch->getClan( ) == clan_shalafi)
            chance /= 2;

        /* now the attack */
        if ( number_percent() < chance )
        {
                if ( number_percent() < 70 )
                {
                        act("%^C1 подсекает тебя и ты падаешь!",ch,victim,0,TO_VICT);
                        oldact("Ты подсекаешь $C4 и $C1 падает!",ch,0,victim,TO_CHAR);
                        oldact("$c1 подсекает $C4, и $C1 падает!",ch,0,victim,TO_NOTVICT);
                        victim->setWaitViolence( 2 );
                        victim->position = POS_RESTING;
                }
                else
                {
                        act("%^C1 пытается подсечь тебя, но ты ухитряешься не упасть!",ch,victim,0,TO_VICT);
                        act("Ты пытаешься подсечь %2$C4!",ch,victim,0,TO_CHAR);
                        act("%1$^C1 пытается подсечь %2$C4!",ch,victim,0,TO_NOTVICT);
                }
                ch->setWait( gsn_trip->getBeats( ) );
                gsn_trip->improve( ch, true, victim );
                damage(ch,victim,number_range(2,2+2*victim->size),gsn_trip,DAM_BASH,true, DAMF_WEAPON);
        }
        else
        {
                damage(ch,victim,0,gsn_trip,DAM_BASH,true, DAMF_WEAPON);
                ch->setWait( gsn_trip->getBeats( )*2/3 );
                gsn_trip->improve( ch, false, victim );
        }

    if (!FightingCheck)
        yell_panic( ch, victim,
                    "Помогите! Кто-то только что подсек меня!",
                    "Помогите! %1$^C1 только что подсе%1$Gкло|к|кла меня!" );
}


/*----------------------------------------------------------------------------
 * kick
 *---------------------------------------------------------------------------*/
class KickOneHit: public SkillDamage {
public:
    KickOneHit( Character *ch, Character *victim );

    virtual void calcDamage( );

};

KickOneHit::KickOneHit( Character *ch, Character *victim )
            : Damage( ch, victim, DAM_BASH, 0 ),
              SkillDamage( ch, victim, gsn_kick, DAM_BASH, 0, DAMF_WEAPON )
{
}


void KickOneHit::calcDamage( )
{
        Object *on_feet;
        dam = number_range( 1, ch->getModifyLevel() );
                
                if ( (ch->getProfession( ) == prof_samurai)
                        && IS_SET ( ch->parts, PART_FEET)
                        && ((on_feet=get_eq_char(ch,wear_feet)) == 0
                        || (on_feet!=0 && !material_is_typed( on_feet, MAT_METAL ) ) ) )
                {
                        dam *= 2;
                }
                
                dam += ch->damroll / 2;
                damApplyEnhancedDamage( );

                //10% extra damage for every skill level
                dam+= dam * skill_level_bonus(*gsn_kick, ch) / 10;

                if (IS_SET( ch->parts, PART_TWO_HOOVES ))
                    dam = 3 * dam / 2;
                else if (IS_SET( ch->parts, PART_FOUR_HOOVES ))
                    dam *= 2;

               
        Damage::calcDamage( );
}

/*----------------------------------------------------------------------------
 * doublekick
 *---------------------------------------------------------------------------*/
class DoubleKickOneHit: public SkillDamage {
public:
    DoubleKickOneHit( Character *ch, Character *victim );

    virtual void calcDamage( );
};

DoubleKickOneHit::DoubleKickOneHit( Character *ch, Character *victim )
            : Damage( ch, victim, DAM_BASH, 0 ),
              SkillDamage( ch, victim, gsn_double_kick, DAM_BASH, 0, DAMF_WEAPON )
{
}

void DoubleKickOneHit::calcDamage( )
{
        Object *on_feet;
        dam = number_range( 1, ch->getModifyLevel() );
                
                if ( (ch->getProfession( ) == prof_samurai)
                        && IS_SET ( ch->parts, PART_FEET)
                        && ((on_feet=get_eq_char(ch,wear_feet)) == 0
                        || (on_feet!=0 && !material_is_typed( on_feet, MAT_METAL ) ) ) )
                {
                        dam *= 2;
                }
                
                dam += ch->damroll / 2;
                damApplyEnhancedDamage( );

                //10% extra damage for every skill level
                dam+= dam * skill_level_bonus(*gsn_double_kick, ch) / 10;

                if (IS_SET( ch->parts, PART_TWO_HOOVES ))
                    dam = 3 * dam / 2;
                else if (IS_SET( ch->parts, PART_FOUR_HOOVES ))
                    dam *= 2;

               
        Damage::calcDamage( );
}

/*
 * 'kick' skill command
 */

SKILL_RUNP( kick )
{
        Character *victim;
        int chance;
        char arg[MAX_INPUT_LENGTH];
        bool FightingCheck;

        if (gsn_kick->getEffective( ch ) <= 1)
        {
                ch->pecho("У тебя плохая растяжка.");
                return;
        }

        if ( MOUNTED(ch) )
        {
                ch->pecho("Ты не можешь ударить ногой, если ты верхом!");
                return;
        }

        if (ch->fighting != 0)
                FightingCheck = true;
        else
                FightingCheck = false;

        argument = one_argument(argument,arg);

        if (arg[0] == '\0')
        {
                victim = ch->fighting;
                if (victim == 0)
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

        if (victim == ch)
        {
                ch->pecho("Ударить себя ногой? Довольно тяжело...");
                return;
        }

        if (is_safe(ch,victim))
        {
                return;
        }

        if (IS_CHARMED(ch) && ch->master == victim)
        {
                act("Но %2$C1 твой друг!!!",ch,victim,0,TO_CHAR);
                return;
        }

        if( !ch->is_npc() && !ch->move )
        {
                ch->pecho("Ты слишком уста%Gло|л|ла для этого.", ch);
                return;
        }
        else
                ch->move -= move_dec( ch );

        if(SHADOW(ch))
        {
                ch->pecho("Твоя нога вязнет в твоей тени...");
                oldact("$c1 выделывает балетные па перед своей тенью.", ch, 0, 0, TO_ROOM);
                return;
        }

        chance = number_percent( );
        
        if ( is_flying( ch ) )
                chance = ( int )( chance * 1.1 );
        
        ch->setWait( gsn_kick->getBeats( )  );
        
        if (chance < gsn_kick->getEffective( ch ))
        {
                gsn_kick->improve( ch, true, victim );
                KickOneHit kick (ch , victim);
                try{
                        kick.hit(true);
                }
                catch (const VictimDeathException &) {
                }         
                                
                if (victim->in_room == ch->in_room)
                    if (number_percent( ) < (gsn_double_kick->getEffective( ch ) * 8) / 10)
                    {
                        gsn_double_kick->improve( ch, true, victim );
                        DoubleKickOneHit doublekick (ch , victim);
                        try{
                                doublekick.hit(true);
                        }
                        catch (const VictimDeathException &) {
                        } 
                    }
        }
        else
        {
                damage( ch, victim, 0, gsn_kick,DAM_BASH, true, DAMF_WEAPON );
                gsn_kick->improve( ch, false, victim );
        }
        
    if (!FightingCheck) {
        if (IS_SET(ch->parts, PART_TWO_HOOVES|PART_FOUR_HOOVES))
            yell_panic( ch, victim,
                        "Помогите! Кто-то ударил меня копытом!",
                        "Помогите! %1$^C1 удари%1$Gло|л|ла меня копытом!" );
        else
            yell_panic( ch, victim,
                        "Помогите! Кто-то ударил меня ногой!",
                        "Помогите! %1$^C1 удари%1$Gло|л|ла меня ногой!" );
    }
}


/*
 * 'concentrate' skill command
 */

SKILL_RUNP( concentrate )
{
    int chance;

    if ( MOUNTED(ch) )
    {
        ch->pecho("Ты не можешь сконцентрироваться, если ты верхом!");
        return;
    }

    if ((chance = gsn_concentrate->getEffective( ch )) == 0)
    {
        ch->pecho("Ты пытаешься сконцентрироваться, но не выходит.");
        return;
    }

    if (ch->isAffected(gsn_concentrate))
    {
        oldact("Ты уже сконцентрировал$gось|ся|ась для сражения.", ch, 0, 0, TO_CHAR );
        return;
    }
        
    if (ch->mana < gsn_concentrate->getMana( ))
    {
        ch->pecho("У тебя не хватает энергии для этого.");
        return;
    }

    /* fighting */
    if (ch->fighting)
    {
        ch->pecho("Поздно концентрироваться, бой уже в самом разгаре!");
        return;
    }


    if(SHADOW(ch)) {
      ch->pecho("Эта странная тень не дает тебе сконцентрироваться.");
      oldact("$c1 пыжится, но никак не сконцентрируется -- того и гляди, лопнет.", ch, 0, 0, TO_ROOM);
      return;
    }
    
    ch->setWaitViolence( 2 );

    if (number_percent() < chance)
    {
        ch->mana -= gsn_concentrate->getMana( );
        gsn_concentrate->getCommand()->run(ch);
        gsn_concentrate->improve( ch, true );
    }
    else
    {
        ch->pecho("Ты пытаешься сконцентрироваться для следующего сражения, но не выходит.");
        gsn_concentrate->improve( ch, false );
    }
}

BOOL_SKILL( concentrate )::run( Character *ch ) 
{
    Affect af;
    int level = skill_level(*gsn_concentrate, ch);

    af.type         = gsn_concentrate;
    af.level        = level;
    af.duration     = number_fuzzy( level / 8);
    af.modifier     = max( 1, level / 8 );

    af.location = APPLY_HITROLL;
    affect_to_char(ch,&af);

    af.location = APPLY_DAMROLL;
    affect_to_char(ch,&af);

    af.modifier        = max( 1, level / 10 );
    af.location = APPLY_AC;
    affect_to_char(ch,&af);

    if (ch->position > POS_SITTING)
        interpret_raw(ch, "sit");

    ch->pecho("Ты отдыхаешь, концентрируясь для следующего сражения!");
    ch->recho("%^C1 концентрируется для следующего сражения.", ch);
    return true;
}


/*
 * 'crush' skill command
 */

SKILL_RUNP( crush )
{
        Character *victim;
        int chance = 100, wait = 0;
        int damage_crush;

        if ( MOUNTED(ch) )
                return;
        
        if (!gsn_crush->usable( ch ))
            return;

        if ( (victim = ch->fighting) == 0 )
                return;

        if ( victim->position < POS_FIGHTING )
                return;

        if ( is_safe(ch,victim) )
                return;

        if ( IS_CHARMED(ch) && ch->master == victim )
                return;

        if (SHADOW(ch))
        {
                return;
        }

        if ( victim->isAffected(gsn_protective_shield) )
        {
                act("{YТвой мощный удар как будто соскальзывает c %2$C2, не причиняя вреда.", ch,victim,0,TO_CHAR);
                act("{YМощный удар %C2 скользит по поверхности твоего охранного щита.{x", ch,victim,0,TO_VICT);
                act("{YМощный удар %1$C2 как будто соскальзывает с %2$C2.{x", ch,victim,0,TO_NOTVICT);

                ch->setWait( gsn_crush->getBeats( ) );
                return;
        }

        /* modifiers */

        /* size  and weight */
        chance += min(ch->canCarryWeight( ), ch->carry_weight) / 25;
        chance -= min(victim->canCarryWeight( ), victim->carry_weight) / 20;

        if ( ch->size < victim->size )
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
        if ( number_percent() < chance )
        {
                oldact("$c1 сбивает тебя с ног мощнейшим ударом!", ch,0,victim,TO_VICT);
                oldact("Ты бросаешься на $C4, и сбиваешь $S с ног!",ch,0,victim,TO_CHAR);
                oldact("$c1 сбивает $C4 с ног мощнейшим ударом.", ch,0,victim,TO_NOTVICT);

                wait = 3;

                switch(number_bits(2))
                {
                case 0: wait = 1; break;
                case 1: wait = 2; break;
                case 2: wait = 4; break;
                case 3: wait = 3; break;
                }

                victim->setWaitViolence( wait );
                ch->setWait( gsn_crush->getBeats( ) );
                victim->position = POS_RESTING;
                damage_crush = (ch->damroll / 2) + number_range(4,4 + 4* ch->size + chance/10);
                damage(ch,victim,damage_crush,gsn_crush, DAM_BASH, true, DAMF_WEAPON);
        }
        else
        {
                damage(ch,victim,0,gsn_crush,DAM_BASH, true, DAMF_WEAPON);
                act("Ты промахиваешься и падаешь!",ch,victim,0,TO_CHAR);
                act("%^C1 делает резкое движение и падает.",ch,victim,0,TO_NOTVICT);
                oldact("$c1 пытается сбить тебя с ног, но ты делаешь шаг в сторону, и $e падает!", ch,0,victim,TO_VICT);
                ch->position = POS_RESTING;
                ch->setWait( gsn_crush->getBeats( ) * 3/2 );
        }
}


/*
 * 'berserk' skill command
 */

SKILL_RUNP( berserk )
{
        int chance, hp_percent, mana;

        if ( (chance = gsn_berserk->getEffective( ch )) <= 1)
        {
                ch->pecho("Ты краснеешь от напряжения, но ничего не получается.");
                return;
        }

        if ( IS_AFFECTED(ch,AFF_BERSERK) || ch->isAffected(gsn_berserk)
                || ch->isAffected(gsn_frenzy) )
        {
                oldact("Ты становишься немного дик$gим|им|ой.", ch, 0, 0, TO_CHAR);
                return;
        }

        if ( IS_AFFECTED(ch,AFF_CALM) )
        {
                oldact("Ты слишком миролюби$gво|в|ва для этого.", ch, 0, 0, TO_CHAR);
                return;
        }
        
        mana = gsn_berserk->getMana( );
        
        if ( ch->mana < mana )
        {
                ch->pecho("У тебя недостаточно энергии для этого.");
                return;
        }

        /* modifiers */

        /* fighting */
        if ( ch->position == POS_FIGHTING )
                chance += 10;

        /* damage -- below 50% of hp helps, above hurts */

        hp_percent = HEALTH(ch);
        chance += 25 - hp_percent / 2;

        if ( number_percent() < chance )
        {
                ch->setWaitViolence( 1 );
                ch->mana -= mana;

                ch->pecho("Твой пульс учащается, когда ты входишь в ярость!");
                oldact_p("Глаза $c2 загораются {rдиким огнем{x.",ch,0,0,TO_ROOM,POS_FIGHTING);
                gsn_berserk->improve( ch, true );
                gsn_berserk->getCommand()->run(ch);
        }
        else
        {
                ch->setWaitViolence( 2 );
                ch->mana -= mana / 2;

                ch->pecho("Твой пульс ускоряется, но ничего не происходит.");
                gsn_berserk->improve( ch, false );
        }

}

BOOL_SKILL( berserk )::run( Character *ch )
{
    Affect af;
    int level = skill_level(*gsn_berserk, ch);

    /* heal a little damage */
    ch->hit += level * 2;
    ch->hit = min(ch->hit, ch->max_hit);

    af.type         = gsn_berserk;
    af.level        = level;
    af.duration     = number_fuzzy( level / 8);

    af.modifier        = max( 1, level / 5 );
    af.location = APPLY_HITROLL;
    affect_to_char(ch,&af);

    af.location = APPLY_DAMROLL;
    af.bitvector.setTable(&affect_flags);
    af.bitvector.setValue(AFF_BERSERK);
    affect_to_char(ch,&af);

    af.modifier        = max( 10, 10 * ( level / 5 ) );
    af.location = APPLY_AC;
    af.bitvector.clear();
    affect_to_char(ch,&af);
    return true;
}

/*
 * 'dirt kick' skill command
 */

SKILL_RUNP( dirt )
{
        char arg[MAX_INPUT_LENGTH];
        Character *victim;
        int chance;
        bool FightingCheck;

        if ( MOUNTED(ch) )
        {
                ch->pecho("Ты не можешь ослепить пылью, если ты верхом!");
                return;
        }

        if (ch->fighting != 0)
                FightingCheck = true;
        else
                FightingCheck = false;

        one_argument(argument,arg);

        if ( ( chance = gsn_dirt_kicking->getEffective( ch )) <= 1)
        {
                ch->pecho("Ты лишь пачкаешь свои ноги в пыли.");
                return;
        }

        if (arg[0] == '\0')
        {
                victim = ch->fighting;
                if (victim == 0)
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

        if (is_flying( ch ) )
        {
                ch->pecho("Когда летаешь?");
                return;
        }

        if (IS_AFFECTED(victim,AFF_BLIND))
        {
                oldact("Но $E уже ничего не видит.",ch,0,victim,TO_CHAR);
                return;
        }

        if (victim == ch)
        {
                ch->pecho("Довольно глупая мысль.");
                return;
        }

        if (is_safe(ch,victim))
                return;

        if (IS_CHARMED(ch) && ch->master == victim)
        {
                act("Но %2$C1 твой лучший друг!",ch,victim,0,TO_CHAR);
                return;
        }

        if( !ch->is_npc() && !ch->move )
        {
                ch->pecho("Ты слишком уста%Gло|л|ла для этого.", ch);
                return;
        } 

        if(SHADOW(ch))
        {
                ch->pecho("Твоя бездонная тень поглощает пыль.");
                act("%^C1 бросает в свою тень пыль.",ch, 0, 0,TO_ROOM);
                return;
        }

        if ( MOUNTED(victim) )
        {
            if (victim->mount->is_npc( ) && gsn_riding->available( victim )) {
                ch->pecho("Ты не можешь ослепить пылью того, кто верхом!");
                return;
            }

            interpret_raw( victim, "dismount" );
        }

        ch->move -= move_dec( ch );

        /* modifiers */

        /* dexterity */
        chance += ch->getCurrStat(STAT_DEX);
        chance -= 2 * victim->getCurrStat(STAT_DEX);

        /* speed  */
        if (IS_QUICK(ch))
                chance += 10;
        if (IS_QUICK(victim))
                chance -= 25;

        /* level */
        int slevel = skill_level(*gsn_dirt_kicking, ch);
        chance += (slevel - victim->getModifyLevel()) * 2;

        if (chance % 5 == 0)
                chance += 1;

        /* terrain */

        switch(ch->in_room->getSectorType())
        {
        case(SECT_INSIDE):                chance -= 20;        break;
        case(SECT_CITY):                chance -= 10;        break;
        case(SECT_FIELD):                chance +=  5;        break;
        case(SECT_FOREST):                                break;
        case(SECT_HILLS):                                break;
        case(SECT_MOUNTAIN):                chance -= 10;        break;
        case(SECT_WATER_SWIM):                chance  =  0;        break;
        case(SECT_WATER_NOSWIM):        chance  =  0;        break;
        case(SECT_UNDERWATER):        chance  =  0;        break;                        
        case(SECT_AIR):                        chance  =  0;          break;
        case(SECT_DESERT):                chance += 10;   break;
        }

        if (chance == 0)
        {
                ch->pecho("Здесь нет пыли...");
                return;
        }

        if (ch->getProfession( ) == prof_anti_paladin && ch->getClan( ) == clan_shalafi)
            chance /= 2;

        /* now the attack */
        if (number_percent() < chance)
        {
                oldact("$c1 ослепле$gно|н|на пылью, попавшей $m в глаза!",                        victim,0,0,TO_ROOM);

                ch->setWait( gsn_dirt_kicking->getBeats( ) );

                try {
                    damage_nocatch(ch,victim,slevel/5+1,gsn_dirt_kicking,DAM_NONE, true);
                    gsn_dirt_kicking->getCommand()->run(ch, victim);
                    gsn_dirt_kicking->improve( ch, true, victim );
                } catch (const VictimDeathException &) {
                    return;
                }
        }
        else
        {
                damage(ch,victim,0,gsn_dirt_kicking,DAM_NONE, true);
                gsn_dirt_kicking->improve( ch, false, victim );
                ch->setWait( gsn_dirt_kicking->getBeats( ) );
        }

    if (!FightingCheck)
        yell_panic( ch, victim,
                    "Кто-то только что ослепил меня пылью, которая попала в глаза!",
                    "Умри, %1$C1, грязное животное!" );        
}

BOOL_SKILL( dirt )::run( Character *ch, Character *victim ) 
{
    Affect af;

    victim->pecho("Ты ничего не видишь!");
    int slevel = skill_level(*gsn_dirt_kicking, ch);
        
    af.bitvector.setTable(&affect_flags);
    af.type         = gsn_dirt_kicking;
    af.level         = slevel;
    af.duration        = 0;
    af.location = APPLY_HITROLL;
    af.modifier        = -1 * (slevel / 20 + 1);
    af.bitvector.setValue(AFF_BLIND);

    affect_to_char(victim,&af);
    return true;
}

/*
 * 'warcry' skill command
 */

SKILL_RUNP( warcry )
{
  Affect af;
  XMLStringAttribute::Pointer attr;

  if (!gsn_warcry->usable( ch ) )
    {
      ch->pecho("Что?");
      return;
    }

  if (ch->isAffected(gsn_bless) || ch->isAffected(gsn_warcry))
    {
      ch->pecho("Боевой клич не может помочь тебе еще больше.");
      return;
    }

  if (ch->mana < gsn_warcry->getMana( ))
    {
      ch->pecho("У тебя не хватает энергии для боевого клича.");
      return;
    }

  ch->setWait( gsn_warcry->getBeats( )  );

    if (number_percent() > gsn_warcry->getEffective( ch )) {
        switch (number_range( 1, 2 )) {
        case 1: 
            act("Ты тихонько хрюкаешь.", ch, 0, 0,TO_CHAR);
            act("%^C1 тихонько хрюкает.", ch, 0, 0,TO_ROOM);
            break;
        case 2:
            act("Ты сдавленно стонешь.", ch, 0, 0,TO_CHAR);
            act("%^C1 издает сдавленные стоны.", ch, 0, 0,TO_ROOM);
            break;
        }
        
        gsn_warcry->improve( ch, false );
        return;
    }

    ch->mana -= gsn_warcry->getMana( );
    
    int level = skill_level(*gsn_warcry, ch);

    af.type      = gsn_warcry;
    af.level         = level;
    af.duration  = 6 + level / 2;
    af.location = APPLY_HITROLL;
    af.modifier  = level / 8;
    affect_to_char( ch, &af );

    af.location = APPLY_SAVING_SPELL;
    af.modifier  = 0 - level / 8;
    affect_to_char( ch, &af );

    if (!ch->is_npc( )) 
        attr = ch->getPC( )->getAttributes( ).findAttr<XMLStringAttribute>( "warcry" );

    if (attr) {
        interpret_raw( ch, "yell", attr->getValue( ).c_str( ) );
    }
    else {
        act("Ты издаешь боевой клич и чувствуешь, что теперь тебе все по плечу!", ch, 0, 0,TO_CHAR);
        act("%^C1 издает боевой клич!", ch, 0, 0,TO_ROOM);
    }

    gsn_warcry->improve( ch, true );
}


/*----------------------------------------------------------------------------
 * smash
 *---------------------------------------------------------------------------*/
class SmashOneHit: public SkillDamage {
public:
    SmashOneHit( Character *ch, Character *victim );

    virtual void calcDamage( );
    virtual int calcChance( );
    int chance = 0;
};

SmashOneHit::SmashOneHit( Character *ch, Character *victim )
            : Damage( ch, victim, DAM_BASH, 0 ),
              SkillDamage( ch, victim, gsn_smash, DAM_BASH, 0, DAMF_WEAPON )
{
}

void SmashOneHit::calcDamage( )
{
        dam = (ch->damroll / 2) + number_range(4, 4 + 5 * ch->size + chance/10);
        damApplyEnhancedDamage();
        Damage::calcDamage( );
}

int SmashOneHit::calcChance( )
{

        chance = gsn_smash->getEffective( ch );
        /* modifiers */
        chance = chance * 4 / 5;

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

        if (is_flying( ch ))
          chance -= 10;

        /* speed */
        if (IS_QUICK(ch))
          chance += 10;
        if (IS_QUICK(victim))
          chance -= 20;

        /* level */
         chance += skill_level(*gsn_smash, ch) - victim->getModifyLevel();

         return chance;
}

/*
 * 'smash' skill command
 */

SKILL_RUNP( smash )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;
    int wait;
    bool FightingCheck;

    if ( MOUNTED(ch) ) 
    {
        ch->pecho("Ты не можешь сбить с ног, если ты верхом!");
        return;
    }

    if (ch->fighting != NULL)
        FightingCheck = true;
    else
        FightingCheck = false;

    argument = one_argument(argument,arg);

    if (gsn_smash->getEffective( ch ) <= 1) {        
        ch->pecho("Сбить с ног? Но как это сделать?");
        return;
    }

    if (arg[0] == '\0') {
        victim = ch->fighting;
        if (victim == NULL) {
            ch->pecho("Сейчас ты не сражаешься!");
            return;
        }
    }
    else if ((victim = get_char_room(ch,arg)) == NULL) {
        ch->pecho("Этого нет здесь.");
        return;
    }

    if (victim->position < POS_FIGHTING) {
        oldact("Подожди пока $E встанет.", ch,0,victim,TO_CHAR);
        return;
    } 

    if (victim == ch) {
        ch->pecho("Сбить с ног себя??? Не получится...");
        return;
    }

    if (is_safe(ch,victim))
        return;

    if (IS_CHARMED(ch) && ch->master == victim) {
        act("Но %2$C1 твой друг!!!",ch,victim,0,TO_CHAR);
        return;
    }

    if( !ch->is_npc() && !ch->move ) {
          oldact("Ты слишком уста$gло|л|ла для этого.", ch, 0, 0, TO_CHAR);
          return;
    } 

    if(SHADOW(ch)) {
      ch->pecho("Ты безуспешно пытаешься бороться со своей тенью.");
      act("%^C1 обнимается со своей тенью.", ch, 0, 0,TO_ROOM);
      return;
    }
   
    if ( MOUNTED(victim) ) {
        if (victim->mount->is_npc( ) && gsn_riding->available( victim )) {
            ch->pecho("Ты не можешь сбить с ног того, кто верхом!");
            return;
        }

        interpret_raw( victim, "dismount" );
    }

    ch->move -= move_dec( ch );

    if (victim->isAffected(gsn_protective_shield)) {
        oldact_p("{YТы пытаешься сбить с ног $C4, но что-то тебе мешает сделать это.{x", ch, 0, victim,TO_CHAR,POS_FIGHTING);
        oldact_p("{Y$c1 пытается сбить тебя с ног, но твоя защита мешает сделать это.{x", ch, 0,victim,TO_VICT,POS_FIGHTING);
        oldact_p("{Y$c1 пытается сбить с ног $C4, но что-то мешает сделать это.{x", ch,0,victim,TO_NOTVICT,POS_FIGHTING);

        ch->setWait( gsn_smash->getBeats( ) );
        return;
     }
   
   SmashOneHit smash (ch, victim);
   int chance = smash.calcChance();
   
    if (!ch->is_npc() && !victim->is_npc())
        LogStream::sendNotice() 
            << "smash: " 
            << ch->getName() << " " << ch->size << ":" << min(ch->canCarryWeight( ), ch->carry_weight) << ":" << ch->getCurrStat(STAT_STR) << ":" << ch->getModifyLevel() << ", "
            << victim->getName() << " " << victim->size << ":" << min(victim->canCarryWeight( ), victim->carry_weight) << ":" << victim->getCurrStat(STAT_DEX) << ":" << victim->getModifyLevel() << ", "
            << " chance " << chance << endl;

    /* now the attack */

    if (number_percent() < chance) {
        oldact("Сильнейшим ударом $c1 сбивает тебя с ног и ты падаешь на землю!", ch,0,victim,TO_VICT);
        oldact("Ты сбиваешь $C4 с ног, посылая $S на землю!", ch,0,victim,TO_CHAR);
        oldact("$c1 сильнейшим ударом сбивает $C4 с ног.", ch,0,victim,TO_NOTVICT);
        gsn_smash->improve( ch, true, victim );

        wait = 3;

        switch(number_bits(2)) {
        case 0: wait = 1; break;
        case 1: wait = 2; break;
        case 2: wait = 4; break;
        case 3: wait = 3; break;
        }
        
        victim->setWaitViolence( wait );
        ch->setWait( gsn_smash->getBeats( ) );
        

        try {
           smash.hit(true);
            
            if (number_percent() < gsn_smash->getEffective( ch ) - 40)
            {
                 if ( number_percent() > 30 ) 
                    victim->position=POS_SITTING;
                 else
                    victim->position=POS_RESTING;
            }        
        } catch (const VictimDeathException &) {
            return;
        }
    }
    else
    {
        damage(ch,victim,0,gsn_smash,DAM_BASH, true, DAMF_WEAPON);
        oldact("Ты промахиваешься и падаешь лицом на пол!", ch,0,victim,TO_CHAR);
        oldact("$c1 промахивается и падает лицом на пол.", ch,0,victim,TO_NOTVICT);
        oldact("$c1 пытается ударить тебя, но промахивается и падает на пол.", ch,0,victim,TO_VICT);
        gsn_smash->improve( ch, false, victim );

        if ( number_percent() > 5 )         
            ch->position=POS_SITTING;       
        else                                 
            ch->position=POS_RESTING;        
        ch->setWait( gsn_smash->getBeats( ) * 3/2 ); 
    }

    if (!FightingCheck)
        yell_panic( ch, victim,
                    "Помогите! Кто-то сбил меня с ног!",
                    "Помогите! %1$^C1 сбивает меня с ног!" );
}

/*
 * 'area attack' skill command
 */
SKILL_DECL( areaattack );
BOOL_SKILL( areaattack )::run( Character *ch, Character *victim ) 
{
    int count=0, max_count;
    Character *vch, *vch_next;
    
    if (number_percent() >= gsn_area_attack->getEffective( ch ))
        return false;
    
    gsn_area_attack->improve( ch, true, victim );

    int level = skill_level(*gsn_area_attack, ch);

    if (level < 70)
        max_count = 1;
    else if (level < 80)
        max_count = 2;
    else if (level < 90)
        max_count = 3;
    else
        max_count = 4;

    for ( vch = ch->in_room->people; vch != 0; vch = vch_next )
    {
        vch_next = vch->next_in_room;
        if (vch != victim && vch->fighting == ch)
        {
            one_hit_nocatch( ch, vch );
            count++;
        }
        if ( count == max_count )
            break;
    }

    return true;
}
