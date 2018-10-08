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

    act("Ты создаешь $o4!", ch, sleeves, 0, TO_CHAR);
    act("$c1 создает $o4!", ch, sleeves, 0, TO_ROOM);
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
	ch->send_to("Тебе незнаком карточный стиль ведения боя.\n\r");
	return;
    }

    if (MOUNTED(ch)) {
	ch->send_to("Только не верхом!\n\r");
	return;
    }

    if ((victim = get_char_room(ch,argument)) == 0) {
	ch->send_to("Здесь таких нет.\n\r");
	return;
    }

    if (ch == victim) {
	ch->send_to("Не надо.. можешь потерять сознание.\n\r");
	return;
    }

    if (victim->fighting) {
	ch->send_to("Подожди пока закончится сражение.\n\r");
	return;
    }

    if (IS_AFFECTED( ch, AFF_CHARM )) {
	ch->send_to( "Ты же не хочешь ударить по голове своего любимого хозяина?\n\r");
	return;
    }

    if (IS_AFFECTED(victim,AFF_SLEEP)) {
	act_p("$E уже спит.",ch,0,victim,TO_CHAR,POS_RESTING);
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
    chance += URANGE( 0, ( ch->getCurrStat(STAT_DEX) - 20) * 2, 10);
    chance += victim->can_see(ch) ? 0 : 5;
    if (victim->is_npc( ) 
	&& victim->getNPC( )->behavior
	&& IS_SET(victim->getNPC( )->behavior->getOccupation( ), (1 << OCC_SHOPPER)))
	chance -= 40;

    if (number_percent( ) < chance * k / 100) {
	act("Ты со всей силы бьешь $C4 канделябром по голове!", ch, 0, victim, TO_CHAR);
	act("$c1 ударяет тебя канделябром по голове! Ты отключаешься.", ch, 0, victim, TO_VICT);
	act("$c1 лупит $C4 по голове канделябром.", ch, 0, victim, TO_NOTVICT);
	gsn_sconce->improve( ch, true, victim );

	af.type = gsn_sconce;
	af.where = TO_AFFECTS;
	af.level = ch->getModifyLevel();
	af.duration = 0;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = AFF_SLEEP;
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
class JokerOneHit: public WeaponOneHit, public SkillDamage {
public:
    JokerOneHit( Character *ch, Character *victim )
		: Damage( ch, victim, 0, 0 ), WeaponOneHit( ch, victim, false ),
		  SkillDamage( ch, victim, gsn_joker, 0, 0, DAMF_WEAPON )
    {
    }

    virtual void calcDamage( )
    {
	if ( ( ( !IS_AWAKE( victim ) && number_percent() <= URANGE(5, 5+( ch->getModifyLevel() - victim->getModifyLevel() ) * 2, 20) )
		|| ( IS_AWAKE( victim ) && number_percent() <= 10 ) )
	    && !victim->is_immortal())
	{
	    act_p("Твоя {Rшутка{x над $C5 удалась!",ch,0,victim,TO_CHAR,POS_RESTING);
	    act_p("$c1 удачно {R+++ПОШУТИ$gЛО|Л|ЛА+++{x над $C5!",ch,0,victim,TO_NOTVICT,POS_RESTING);
	    act_p("$c1 удачно {R+++ПОШУТИ$gЛО|Л|ЛА+++{x!",ch,0,victim,TO_VICT,POS_DEAD);

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
	ch->send_to("У тебя плохое чувство юмора.\n\r");
	return;
    }

    if (IS_AFFECTED( ch, AFF_CHARM )) {
	ch->send_to( "Нехорошо шутить над своим хозяином.\n\r");
	return;
    }

    if ( arg[0] == '\0' ) {
	ch->send_to("Пошутить над кем?\n\r");
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 ) {
	ch->send_to("Этого нет здесь.\n\r");
	return;
    }

    if (victim == ch) {
	ch->send_to("Сам пошутил - сам посмеялся.\n\r");
	return;
    }

    if (is_safe( ch, victim ))
	return;

    if (victim->is_immortal( )) {
	ch->send_to("А вот эта шутка окончится для тебя плачевно.\n\r");
	return;
    }

    if (victim->fighting != 0) {
	ch->send_to("Подожди, пока закончится сражение.\n\r");
	return;
    }
    
    if (victim->hit < victim->max_hit
	    && victim->can_see(ch)
	    && IS_AWAKE(victim) )
    {
	act( "Нехорошо шутить над больными!", ch, 0, victim, TO_CHAR );
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
	    act_p("Твоя шутка не удалась..",ch,0,victim,TO_CHAR,POS_RESTING);
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
