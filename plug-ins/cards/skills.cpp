/* $Id: skills.cpp,v 1.1.2.12.6.7 2008/05/27 21:30:02 rufina Exp $
 *
 * ruffina, 2005
 */
#include "objects.h"

#include "skillcommandtemplate.h"
#include "spelltemplate.h"                                                 
#include "skill.h"
#include "skillmanager.h"
#include "skillreference.h"

#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"

#include "dreamland.h"
#include "magic.h"
#include "handler.h"
#include "fight.h"
#include "damage_impl.h"
#include "onehit_weapon.h"
#include "occupations.h"
#include "act.h"
#include "interp.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

GSN(sconce);
GSN(joker);

/*
 * 'ace in sleeves' spell 
 */
SPELL_DECL(AceInSleeves);
VOID_SPELL(AceInSleeves)::run( Character *ch, char *, int sn, int level ) 
{
    OBJ_INDEX_DATA *pSleevesIndex;
    Object *sleeves;

    if (!( pSleevesIndex = get_obj_index( OBJ_VNUM_CARD_SLEEVES ) )) 
        return;

    sleeves = create_object( pSleevesIndex, 0 );
    sleeves->timer = 12;
    
    CardSleevesBehavior::Pointer bhv( NEW );
    bhv->setObj( sleeves );
    sleeves->behavior.setPointer( *bhv );
    
    obj_to_char( sleeves, ch );

    act("Ты создаешь %3$O4!", ch, sleeves, 0, TO_CHAR);
    act("%1$^C1 создает %3$C4!", ch, sleeves, 0, TO_ROOM);
}

/*
 * 'sconce' skill command
 */

SKILL_RUNP( sconce )
{
    Character *victim;
    Affect af;
    int chance;

    if (!gsn_sconce->usable( ch )) {
        ch->pecho("Тебе незнаком карточный стиль ведения боя.");
        return;
    }

    if (MOUNTED(ch)) {
        ch->pecho("Только не верхом!");
        return;
    }

    if ((victim = get_char_room(ch,argument)) == 0) {
        ch->pecho("Здесь таких нет.");
        return;
    }

    if (ch == victim) {
        ch->pecho("Не надо.. можешь потерять сознание.");
        return;
    }

    if (victim->fighting) {
        ch->pecho("Подожди пока закончится сражение.");
        return;
    }

    if (IS_CHARMED(ch)) {
        ch->pecho( "Ты же не хочешь ударить по голове сво%1$Gего|его|ю любим%1$Gого|ого|ую хозя%1$Gина|ина|йку?", ch->master);
        return;
    }

    if (IS_AFFECTED(victim,AFF_SLEEP)) {
        oldact("$E уже спит.",ch,0,victim,TO_CHAR);
        return;
    }

    if (is_safe(ch,victim))
        return;

    int k = victim->getLastFightDelay( );

    if ( k >= 0 && k < FIGHT_DELAY_TIME )
        k = k * 100 / FIGHT_DELAY_TIME;
    else
        k = 100;
    
    victim->setLastFightTime( );
    ch->setLastFightTime( );

    ch->setWait( gsn_sconce->getBeats( ) );

    chance = ( int ) ( 0.5 * gsn_sconce->getEffective( ch ) );
    chance += URANGE( 0, ( ch->getCurrStat(STAT_DEX) - BASE_STAT) * 2, (MAX_STAT-BASE_STAT) * 2);
    chance += victim->can_see(ch) ? 0 : 5;
    if (victim->is_npc( ) 
        && victim->getNPC( )->behavior
        && IS_SET(victim->getNPC( )->behavior->getOccupation( ), (1 << OCC_SHOPPER)))
        chance -= 40;

    if (number_percent( ) < chance * k / 100) {
        act("Ты со всей силы бьешь %2$C4 канделябром по голове!", ch, 0, victim, TO_CHAR);
        act("%^C1 ударяет тебя канделябром по голове! Ты отключаешься.", ch, 0, victim, TO_VICT);
        act("%1$^C1 лупит %2$C4 по голове канделябром.", ch, 0, victim, TO_NOTVICT);
        gsn_sconce->improve( ch, true, victim );

        af.type = gsn_sconce;
        af.bitvector.setTable(&affect_flags);
        af.level = ch->getModifyLevel();
        af.duration = 0;
        af.bitvector.setValue(AFF_SLEEP);
        affect_join ( victim,&af );
        
        set_violent( ch, victim, true );

        if (IS_AWAKE(victim))
            victim->position = POS_SLEEPING;
    }
    else
    {
        damage(ch,victim, ch->getModifyLevel() / 2,gsn_sconce,DAM_NONE,true);
        gsn_sconce->improve( ch, false, victim );
        
        yell_panic( ch, victim,
                    "Помогите! Кто-то засветил мне канделябром по голове!",
                    "Помогите! %1$^C1 засвети%1$Gло|л|ла мне канделябром по голове!" );
    }
}

/*----------------------------------------------------------------------------
 * Joker OneHit implementation
 *---------------------------------------------------------------------------*/
class JokerOneHit: public SkillWeaponOneHit {
public:
    JokerOneHit( Character *ch, Character *victim )
                : Damage(ch, victim, 0, 0, DAMF_WEAPON), SkillWeaponOneHit( ch, victim, gsn_joker )
    {
    }

    virtual void calcDamage( )
    {
        if ( ( ( !IS_AWAKE( victim ) && number_percent() <= URANGE(5, 5+( ch->getModifyLevel() - victim->getModifyLevel() ) * 2, 20) )
                || ( IS_AWAKE( victim ) && number_percent() <= 10 ) )
            && !victim->is_immortal())
        {
            act("Твоя {Rшутка{x над %2$C5 удалась!",ch,0,victim,TO_CHAR);
            oldact("$c1 удачно {R+++ПОШУТИ$gЛО|Л|ЛА+++{x над $C5!",ch,0,victim,TO_NOTVICT);
            oldact_p("$c1 удачно {R+++ПОШУТИ$gЛО|Л|ЛА+++{x!",ch,0,victim,TO_VICT,POS_DEAD);

            gsn_joker->improve( ch, true, victim );

            handleDeath( );
            throw VictimDeathException( );
        }
        else
        {
            gsn_joker->improve( ch, false, victim );
            dam = 0;
        }
    }
};

/*
 * 'joker' skill command
 */

SKILL_RUNP( joker )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    one_argument( argument, arg );

    if (!gsn_joker->usable( ch )) {
        ch->pecho("У тебя плохое чувство юмора.");
        return;
    }

    if (IS_CHARMED(ch)) {
        ch->pecho( "Нехорошо шутить над сво%1$Gим|им|ей хозя%1$Gином|ином|йкой.", ch->master);
        return;
    }

    if ( arg[0] == '\0' ) {
        ch->pecho("Пошутить над кем?");
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 ) {
        ch->pecho("Этого нет здесь.");
        return;
    }

    if (victim == ch) {
        ch->pecho("Сам пошутил - сам посмеялся.");
        return;
    }

    if (is_safe( ch, victim ))
        return;

    if (victim->is_immortal( )) {
        ch->pecho("А вот эта шутка окончится для тебя плачевно.");
        return;
    }

    if (victim->fighting != 0) {
        ch->pecho("Подожди, пока закончится сражение.");
        return;
    }
    
    if (victim->hit < victim->max_hit
            && victim->can_see(ch)
            && IS_AWAKE(victim) )
    {
        act("Нехорошо шутить над больными!", ch, 0, victim, TO_CHAR );
        return;
    }

    ch->setWait( gsn_joker->getBeats( )  );
    JokerOneHit joke( ch, victim );
    
    try {
        if ( ch->is_npc()
                || number_percent( ) < (gsn_joker->getEffective( ch ) * 0.7) )
        {
            joke.hit( );
        }
        else
        {
            act("Твоя шутка не удалась..",ch,0,victim,TO_CHAR);
            gsn_joker->improve( ch, false, victim );
            joke.miss( );
        }

        yell_panic( ch, victim,
                    "Помогите! Кто-то пытается ПОШУТИТЬ меня!",
                    "Помогите! %1$^C1 пытается ПОШУТИТЬ меня!" );
    }
    catch (const VictimDeathException& e) {                                     
    }
}
