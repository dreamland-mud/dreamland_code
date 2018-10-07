/* $Id: ccast.cpp,v 1.1.2.35.6.17 2010-09-01 21:20:45 rufina Exp $
 *
 * ruffina, 2004
 * logic based on 'do_cast' from DreamLand 2.0
 */
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "profflags.h"
#include "behavior_utils.h"
#include "commandtemplate.h"
#include "skillreference.h"
#include "spellmanager.h"
#include "skill.h"
#include "spell.h"
#include "spelltarget.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "dreamland.h"
#include "clanreference.h"
#include "fight.h"
#include "magic.h"
#include "act.h"
#include "interp.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "def.h"

#include "logstream.h"
#define log(x) LogStream::sendNotice() << x << endl


CLAN(battlerager);
GSN(shielding);
GSN(garble);
GSN(deafen);


static bool mprog_spell( Character *victim, Character *caster, int sn, bool before )
{
    FENIA_CALL( victim, "Spell", "Cii", caster, sn, before );
    FENIA_NDX_CALL( victim->getNPC( ), "Spell", "CCii", victim, caster, sn, before );
    BEHAVIOR_CALL( victim->getNPC( ), spell, caster, sn, before );
    return false;
}

static bool mprog_cast( Character *caster, SpellTarget::Pointer target, int sn, bool before )
{
    /* XXX will it work at all? */
    switch (target->type) {
    case SpellTarget::NONE:
	FENIA_CALL( caster, "Cast", "sii", target->arg, sn, before );
	FENIA_NDX_CALL( caster->getNPC( ), "Cast", "Csii", caster, target->arg, sn, before );
	break;
    case SpellTarget::OBJECT:
	FENIA_CALL( caster, "Cast", "Oii", target->obj, sn, before );
	FENIA_NDX_CALL( caster->getNPC( ), "Cast", "COii", caster, target->obj, sn, before );
	break;
    case SpellTarget::CHAR:
	FENIA_CALL( caster, "Cast", "Cii", target->victim, sn, before );
	FENIA_NDX_CALL( caster->getNPC( ), "Cast", "CCii", caster, target->victim, sn, before );
	break;
    case SpellTarget::ROOM:
	FENIA_CALL( caster, "Cast", "Rii", target->room, sn, before );
	FENIA_NDX_CALL( caster->getNPC( ), "Cast", "CRii", caster, target->room, sn, before );
	break;
    default:
	break;
    }
    
    BEHAVIOR_VOID_CALL( caster->getNPC( ), cast, target, sn, before );

    return false;
}

CMDRUN( cast )
{
    std::basic_ostringstream<char> buf;
    Character *victim;
    int mana, slevel, sn;
    bool offensive;
    Skill::Pointer skill;
    Spell::Pointer spell;
    SpellTarget::Pointer target;
    DLString arguments, spellName; 

    if (ch->is_npc( ) && !( ch->desc != 0 || ch->master != 0 ))
	return;

    if (ch->is_npc( ) && ch->master != 0) {
	if (!ch->getProfession( )->getFlags( ch ).isSet(PROF_CASTER)) {
	    act_p( "$C1 говорит тебе '{GЯ не понимаю, чего ты хочешь, хозя$gин|ин|йка.{x'", ch->master, 0, ch, TO_CHAR, POS_RESTING );
	    return;
	}
    }
    
    if (!ch->is_npc( ) && !ch->move) {
	ch->send_to("У тебя нет сил даже пошевелить языком.\n\r");
	return;
    }

    if (ch->isAffected(gsn_shielding ) && number_percent( ) > 50) {
	ch->send_to("Ты пытаешься сосредоточиться на заклинании, но что-то останавливает тебя.\n\r");
	return;
    }

    if ((ch->isAffected(gsn_garble ) || ch->isAffected(gsn_deafen )) && number_percent( ) > 50) {
	ch->send_to("Ты не можешь настроиться на правильную интонацию.\n\r");
	return;
    }

    if (HALF_SHADOW(ch)) {
	ch->send_to("Твоя тень поглощает всякую попытку сотворить заклинание.\n\r");
	act_p("$c1 пытается сотворить заклинание, но тень не дает $m сосредоточится.",
		ch, 0, 0, TO_ROOM,POS_RESTING);
	return;
    }

    if (ch->death_ground_delay > 0 && ch->trap.isSet( TF_NO_CAST )) {
	ch->send_to("Тебя занимает более важное занятие - спасение своей жизни.\n\r");
	return;
    }

    arguments = constArguments;
    arguments.stripWhiteSpace( );
    spellName = arguments.getOneArgument( );
    
    if (spellName.empty( )) {
	ch->send_to("Колдовать что и на кого?\n\r");
	return;
    }

    if (ch->getClan( ) == clan_battlerager && !ch->is_immortal( )) {
	ch->send_to("Ты {RBattleRager{x, а не презренный маг!\n\r");
	return;
    }

    if (ch->is_npc( ) && ch->master && ch->master->getClan( ) == clan_battlerager) {
	do_say(ch,"Хозяин, я уважаю твои убеждения.");
	return;
    }
    
    spell = SpellManager::lookup( spellName, ch );

    if (!spell) {
	if (ch->is_npc( ) && ch->master) 
	    do_say(ch, "Да не умею я");
	else
	    ch->send_to("Ты не знаешь такого заклинания.\n\r");

	return;
    }
    
    skill = spell->getSkill( );
    sn = skill->getIndex( );
    
    if (!spell->checkPosition( ch ))
	return;
    
    if (!skill->usable( ch, true ))
	return;
	
    if (IS_SET(ch->in_room->room_flags,ROOM_NO_CAST)) {
	ch->send_to("Стены этой комнаты поглотили твое заклинание.\n\r");
	act_p("$c1 произне$gсло|с|сла заклинание, но стены комнаты поглотили его.",
		ch, 0, 0, TO_ROOM,POS_RESTING);
	return;
    }

    mana = spell->getManaCost( ch );

    if (ch->mana < mana) {
	if (ch->is_npc( ) && ch->master != 0) 
	    do_say(ch,"Хозяин. У меня манна кончилась!");
	else 
	    ch->send_to("У тебя не хватает энергии (mana).\n\r");

	return;
    }

    if (!( target = spell->locateTargets( ch, arguments, buf ) )) {
	ch->send_to( buf );
	return;
    }

    victim = target->victim;
    offensive = spell->getSpellType( ) == SPELL_OFFENSIVE;

    if (offensive && ch->is_npc( ) && ch->master && ch->master != victim) {
	if (victim && !victim->is_npc( ))
	    do_say(ch, "Я его боюсь, хозяин!");
	else
	    do_say(ch, "Я не буду делать этого.");

	return;
    }

    spell->utter( ch );
    ch->setWait(spell->getBeats( ) );
    
    if (offensive) {
	UNSET_DEATH_TIME(ch);

	if (victim && is_safe( ch, victim ))
	    return;

	if (victim)
	    set_violent( ch, victim, false );
	
	yell_panic(ch, victim);
    }
     
    if (spell->spellbane( ch, victim ))
	return;
	
    if (number_percent( ) > skill->getEffective( ch )) {
	ch->send_to("Ты не можешь сконцентрироваться.\n\r");
	skill->improve( ch, false, victim );
	ch->mana -= mana / 2;
	target->castFar = false;
    }
    else {
	bool fForbidCasting = false;

	ch->mana -= mana;
	slevel = spell->getSpellLevel( ch, target->range );
	
	if (victim)
	    fForbidCasting = mprog_spell( victim, ch, sn, true );
	
	mprog_cast( ch, target, sn, true );

	if (!fForbidCasting)
	    spell->run( ch, target, slevel );

	if (victim)
	    mprog_spell( victim, ch, sn, false );

	mprog_cast( ch, target, sn, false );
	skill->improve( ch, true, victim );
    }
    
    if (offensive && victim) {
	if (target->castFar && target->door != -1) {
	    ch->setLastFightTime( );
	    victim->setLastFightTime( );

	    if (victim->is_npc( ) && victim->getNPC( )->behavior)
		victim->getNPC( )->behavior->shooted( ch, target->door );
	}
	else
	    attack_caster( ch, victim );
    }
}


