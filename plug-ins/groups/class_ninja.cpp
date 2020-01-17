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
    Room *pRoomIndex;
    Affect af;

    if ( !ch->is_npc() && !gsn_vanish->usable( ch ) )
    {
            ch->send_to("Что?\n\r");
            return;
    }
    
    if (ch->mana < gsn_vanish->getMana( ))
    {
            ch->send_to("У тебя недостаточно энергии для этого.\n\r" );
            return;
    }

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

    ch->mana -= gsn_vanish->getMana( );
    ch->setWait( gsn_vanish->getBeats( )  );
    
    if (ch->isAffected(gsn_vanish)) {
        ch->send_to("Тебе пока нечего бросить.\r\n");
        return;
    }
    
    if (number_percent() > gsn_vanish->getEffective( ch ) )
    {
            ch->send_to("Твоя попытка закончилась неудачей!\n\r");
            gsn_vanish->improve( ch, false );
            return;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL))
    {
        ch->send_to("Боги покинули тебя.\n\r");
        return;
    }

    pRoomIndex = get_random_room_vanish( ch );
    
    if (!pRoomIndex) {
        ch->send_to("Тебе некуда скрыться.\n\r");
        return;
    }
    
  act_p( "$c1 бросает на землю небольшой шар.", ch, 0, 0, TO_ROOM,POS_RESTING);
  ch->send_to("Ты бросаешь на землю небольшой шар.\r\n");

  gsn_vanish->improve( ch, true );

  if (!ch->is_npc() && ch->fighting != 0 && number_bits(1) == 1)
  {
    ch->send_to("Твоя попытка закончилась неудачей!\n\r");
    return;
  }
    
    transfer_char( ch, ch, pRoomIndex,
            "%1^C1 исчезает!",
            "Ты исчезаешь!",
            "%1^C1 появляется из ничего." );
}

/*
 * 'nerve' skill command
 */

SKILL_RUNP( nerve )
{
        Character *victim;
        char arg[MAX_INPUT_LENGTH];

        if ( MOUNTED(ch) )
        {
                ch->send_to("Только не верхом!\n\r");
                return;
        }

        one_argument(argument,arg);

        if (!gsn_nerve->usable( ch ) )
        {
                ch->send_to("Ты не владеешь этой техникой.\n\r");
                return;
        }

        if (ch->fighting == 0)
        {
                ch->send_to("Сейчас ты не сражаешься.\n\r");
                return;
        }

        victim = ch->fighting;

        if ( is_safe(ch,victim) )
                return;

        if ( ch->isAffected(gsn_nerve) )
        {
                ch->send_to("Ты не можешь сделать этого противника еще слабее.\n\r");
                return;
        }

        ch->setWait( gsn_nerve->getBeats( )  );

        if ( ch->is_npc()
                || number_percent() < (gsn_nerve->getEffective( ch ) + ch->getModifyLevel()
                                        + ch->getCurrStat(STAT_DEX))/2 )
        {
                gsn_nerve->getCommand()->run(ch, victim);
                act_p("Ты ослабляешь $C4, пережимая нервные точки.",ch,0,victim,TO_CHAR,POS_RESTING);
                act_p("$c1 ослабляет тебя, пережимая твои нервные точки.",ch,0,victim,TO_VICT,POS_RESTING);
                act_p("$c1 ослабляет $C4",ch,0,victim,TO_NOTVICT,POS_RESTING);
                gsn_nerve->improve( ch, true, victim );
        }
  else
        {
                ch->send_to("Ты нажимаешь не туда, куда надо.\n\r");
                act_p("$c1 нажимает пальцами на твое тело, но ничего не происходит.",
                        ch,0,victim,TO_VICT,POS_RESTING);
                act_p("$c1 нажимает пальцами на тело $C2, но ничего не происходит.",
                        ch,0,victim,TO_NOTVICT,POS_RESTING);
                gsn_nerve->improve( ch, false, victim );
        }

        if (!victim->fighting) {
            yell_panic( ch, victim,
                        "Помогите! Меня кто-то трогает!",
                        "Убери свои руки, %1$C1!" );
        
            multi_hit(victim,ch);
        }
}

BOOL_SKILL(nerve)::run(Character *ch, Character *victim)
{
    Affect af;
    af.where    = TO_AFFECTS;
    af.type     = gsn_nerve;
    af.level    = ch->getModifyLevel();
    af.duration = ch->getModifyLevel() / 20;
    af.location = APPLY_STR;
    af.modifier = -3;
    af.bitvector = 0;

    affect_to_char(victim,&af);
    return true;
}

/*
 * 'endure' skill command
 */

SKILL_RUNP( endure )
{
  if (ch->is_npc())
    {
      ch->send_to("Выносливость -- не твой удел.\n\r");
      return;
    }

  if ( gsn_endure->getEffective( ch ) <= 1 )
  {
      ch->pecho( "Похоже ты не так выносли%Gво|в|ва, как о себе думаешь.", ch );
      return;
    }

  if (ch->isAffected(gsn_endure))
    {
      ch->send_to("Ты не можешь стать еще выносливее.\n\r");
      return;
    }


    ch->setWait( gsn_endure->getBeats( )  );
    gsn_endure->getCommand()->run(ch, -1 * (gsn_endure->getEffective( ch ) / 10));
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

    act("Ты готовишься к столкновению с магией.", ch, 0, 0, TO_CHAR);
    act("$c1 мгновенно концентрируется.", ch,0,0,TO_ROOM);
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
    int chance;

    damBase( );
    gsn_enhanced_damage->getCommand( )->run( ch, victim, dam );;
    damApplyPosition( );
    
    if (victim->is_immortal( ))
        chance = 0;
    else if (IS_AWAKE( victim ))
        chance = 10;
    else { /* XXX */
        chance = 5 + (ch->getModifyLevel( ) - victim->getModifyLevel( )) * 2;
        chance = URANGE( 5, chance, 20 );
    }

    Chance mychance(ch, chance, 100);

    if (mychance.reroll()) {
        act_p("Ты {R+++ ЛОМАЕШЬ ШЕЮ +++{x $C4!",ch,0,victim,TO_CHAR,POS_RESTING);
        act_p("$c1 {R+++ ЛОМАЕТ ШЕЮ +++{x $C4!",ch,0,victim,TO_NOTVICT,POS_RESTING);
        act_p("$c1 {R+++ ЛОМАЕТ ТЕБЕ ШЕЮ +++{x!",ch,0,victim,TO_VICT,POS_DEAD);

        gsn_assassinate->improve( ch, true, victim );

        handleDeath( );
        throw VictimDeathException( );
    }
    else
    {
        gsn_assassinate->improve( ch, false, victim );
        dam *= 2;
    }

    damApplyDamroll( );

    WeaponOneHit::calcDamage( );
}

/*
 * 'assassinate' skill command
 */
SKILL_RUNP( assassinate )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

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
            ch->send_to("Ты не имеешь понятия, как это делается.\n\r");
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
            ch->send_to("С богами такие штучки не пройдут.\n\r");
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

    if (gsn_rear_kick->getCommand( )->run( ch, victim ))
        return;

    ch->setWait( gsn_assassinate->getBeats( )  );
    AssassinateOneHit ass( ch, victim );
    
    try {
        if ( ch->is_npc()
                || number_percent( ) < (gsn_assassinate->getEffective( ch ) * 0.7) )
        {
                ass.hit( );
        }
        else
        {
                gsn_assassinate->improve( ch, false, victim );
                ass.miss( );
        }
        
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
  Character *victim = ch->fighting;

  if (ch->is_npc() || !gsn_caltraps->usable( ch ))
    {
      ch->send_to("Шипами кидаться? Не выросли еще.\n\r");
      return;
    }

  if (victim == 0)
    {
      ch->pecho("Для этого ты долж%Gно|ен|на с кем-то сражаться.", ch);
      return;
    }

  if (is_safe(ch,victim))
    return;

  act_p("Ты кидаешь пригорошню острых шипов под ноги $C3.",
         ch,0,victim,TO_CHAR,POS_RESTING);
  act_p("$c1 кидает пригорошню острых шипов тебе под ноги!",
         ch,0,victim,TO_VICT,POS_RESTING);

  ch->setWait( gsn_caltraps->getBeats( ) );

  if (!ch->is_npc() && number_percent() >= gsn_caltraps->getEffective( ch ))
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
    try {
        damage_nocatch(ch,victim, ch->getModifyLevel(),gsn_caltraps,DAM_PIERCE, true, DAMF_WEAPON);

        if (!victim->isAffected(gsn_caltraps)) {
            Affect tohit,todam,todex;

            tohit.where     = TO_AFFECTS;
            tohit.type      = gsn_caltraps;
            tohit.level     = ch->getModifyLevel();
            tohit.duration  = -1;
            tohit.location  = APPLY_HITROLL;
            tohit.modifier  = -5;
            tohit.bitvector = 0;
            affect_to_char( victim, &tohit );

            todam.where = TO_AFFECTS;
            todam.type = gsn_caltraps;
            todam.level = ch->getModifyLevel();
            todam.duration = -1;
            todam.location = APPLY_DAMROLL;
            todam.modifier = -5;
            todam.bitvector = 0;
            affect_to_char( victim, &todam);

            todex.type = gsn_caltraps;
            todex.level = ch->getModifyLevel();
            todex.duration = -1;
            todex.location = APPLY_DEX;
            todex.modifier = -5;
            todex.bitvector = 0;
            affect_to_char( victim, &todex);

            act_p("$C1 начинает хромать.",ch,0,victim,TO_CHAR,POS_RESTING);
            act_p("Ты начинаешь хромать.",ch,0,victim,TO_VICT,POS_RESTING);
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
        char arg[MAX_INPUT_LENGTH];
        int chance, dam;

        if ( MOUNTED(ch) )
        {
                ch->send_to("Нельзя никого бросить, сидя в седле!\n\r");
                return;
        }

        argument = one_argument(argument,arg);

        if ( ch->is_npc() || !gsn_throw->usable( ch ) )
        {
                ch->send_to("Ты не умеешь бросать через плечо!\n\r");
                return;
        }

        if (is_flying( ch ))
        {
                ch->send_to("Твои ноги должны стоять на земле для упора.\n\r");
                return;
        }

        if ( ( victim = ch->fighting ) == 0 )
        {
                ch->send_to("Сейчас ты не сражаешься.\n\r");
                return;
        }

        if (IS_CHARMED(ch) && ch->master == victim)
        {
                act_p("Но $C1 твой друг!",ch,0,victim,TO_CHAR,POS_RESTING);
                return;
        }

        if (is_safe(ch,victim))
                return;

        if( !ch->is_npc() && !ch->move )
        {
                ch->pecho("Ты слишком уста%Gло|л|ла для этого.", ch);
                return;
        }
        else
                ch->move -= move_dec( ch );

        ch->setWait( gsn_throw->getBeats( )  );

        if (victim->isAffected(gsn_protective_shield))
        {
                act_p("{YУ тебя не получилось добраться до $X.{x",
                                        ch,0,victim, TO_CHAR,POS_FIGHTING);
                act_p("{Y$c1 не смо$gгло|г|гла бросить тебя, натолкнувшись на защитный щит.{x",
                                        ch, 0, victim, TO_VICT,POS_FIGHTING);
                act_p("{Y$c1 распластывается по защитному щиту $C2 в попытке броска.{x",
                                        ch,0,victim,TO_NOTVICT,POS_FIGHTING);
                return;
        }

        chance = gsn_throw->getEffective( ch ) * 4 / 5;

        if (ch->size < victim->size)
                chance += (ch->size - victim->size) * 25;
        else
                chance += (ch->size - victim->size) * 10;

        /* stats */
        chance += ch->getCurrStat(STAT_STR);
        chance -= victim->getCurrStat(STAT_DEX) * 4/3;

        if (is_flying( victim ) )
                chance -= 10;

        /* speed */
        if (IS_QUICK(ch))
                chance += 10;
        if (IS_QUICK(victim))
                chance -= 20;

        /* level */
        chance += ( ch->getModifyLevel() - victim->getModifyLevel() ) * 2;

        if ( ch->is_npc() || number_percent() < chance )
        {
            if ( number_percent() < 70 ) {
                act_p("Ты бросаешь $C4 с ошеломляющей силой.",
                        ch,0,victim,TO_CHAR,POS_RESTING);
                act_p("$c1 бросает тебя с ошеломляющей силой.",
                        ch,0,victim,TO_VICT,POS_RESTING);
                act_p("$c1 бросает $C4 с ошеломляющей силой.",
                        ch,0,victim,TO_NOTVICT,POS_RESTING);
                victim->setWaitViolence( 2 );

                victim->position = POS_RESTING;
            }
            else {
                act("Ты бросаешь $C4 через плечо.", ch,0,victim,TO_CHAR);
                act("$c1 бросает тебя через плечо.", ch,0,victim,TO_VICT);
                act("$c1 бросает $C4 через плечо.", ch,0,victim,TO_NOTVICT);
            }        

            dam = ch->getModifyLevel() + ch->getCurrStat(STAT_STR) + ch->damroll / 2;
            gsn_enhanced_damage->getCommand( )->run( ch, victim, dam );;

            damage( ch, victim, dam, gsn_throw, DAM_BASH, true, DAMF_WEAPON );
            gsn_throw->improve( ch, true, victim );
        }
        else
        {
            act( "Твой бросок не удался.", ch, 0, 0, TO_CHAR);
            act( "$C1 пытается бросить тебя, но терпит неудачу.", victim, 0, ch,TO_CHAR);
            act( "$c1 пытается ухватиться за $C4 поудобнее. Но безуспешно.", ch, 0, victim, TO_NOTVICT);
            gsn_throw->improve( ch, false, victim );
        }
}

/*
 * 'strangle' skill command
 */

SKILL_RUNP( strangle )
{
        Character *victim;
        Affect af;
        int chance;

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
                ch->send_to("У тебя боязнь себя?\n\r");
                return;
        }

        if ( victim->isAffected(gsn_strangle) )
                return;

        if ( is_safe(ch,victim) )
        {
                ch->send_to("Боги защищают твою жертву.\n\r");
                return;
        }
        
        if (gsn_rear_kick->getCommand( )->run( ch, victim ))
            return;

        int k = victim->getLastFightDelay( );

        if (k >= 0 && k < FIGHT_DELAY_TIME)
            k = k * 100 / FIGHT_DELAY_TIME;
        else
            k = 100;
        
        UNSET_DEATH_TIME(ch);
        victim->setLastFightTime( );
        ch->setLastFightTime( );

        ch->setWait( gsn_strangle->getBeats( ) );

        chance = ( int ) ( 0.6 * gsn_strangle->getEffective( ch ) );
        chance += URANGE(0, (ch->getCurrStat(STAT_DEX) - 20) * 2, 10);
        chance += victim->can_see(ch) ? 0 : 5;

        if (victim->isAffected(gsn_backguard)) 
            chance /= 2;

        Chance mychance(ch, chance*k/100, 100);

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
            ch->send_to("Ась?\n\r");
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

        if (SHADOW(ch))
        {
                ch->send_to("Облако поглощается твоей тенью.\n\r");
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
            ch->send_to("Ась?\n\r");
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

        if(SHADOW(ch))
        {
                ch->send_to("Облако поглощается твоей тенью.\n\r");
                return;
        }

        ch->send_to("Облако пыли наполнило комнату.\n\r");
        act_p("Облако пыли наполнило комнату.",ch,0,0,TO_ROOM,POS_RESTING);

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

