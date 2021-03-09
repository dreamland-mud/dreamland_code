/* $Id: ccast.cpp,v 1.1.2.35.6.17 2010-09-01 21:20:45 rufina Exp $
 *
 * ruffina, 2004
 * logic based on 'do_cast' from DreamLand 2.0
 */
#include "profflags.h"
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
#include "fight_exception.h"
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
BONUS(mana);


bool mprog_spell( Character *victim, Character *caster, Skill::Pointer skill, bool before );
void mprog_cast( Character *caster, SpellTarget::Pointer target, Skill::Pointer skill, bool before );

static bool string_has_separator(const DLString &s)
{
    for (DLString::size_type i = 0; i < s.size(); i++)
        if (dl_is_arg_separator(s.at(i)))
            return true;

    return false;
}

static Spell::Pointer spell_lookup(Character *ch, const DLString &fullArguments, DLString &spellName, DLString &spellArgs)
{
    Spell::Pointer spell;

    // Player knows what they're doing, quotes added. Look up spell by the first argument.
    if (string_has_separator(fullArguments))
        return SpellManager::lookup(spellName, ch);

    // May be forgot the quotes (e.g. [cast create spring]). 
    // Try to match the whole argument, then cut off the last word & try again, and so on.
    StringList args(fullArguments);
    DLString offcuts;

    while (!args.empty()) {
        spell = SpellManager::lookup(args.toString(), ch);

        if (spell) {
            // Reset spell args to the remainder of the string.
            spellArgs = offcuts.stripWhiteSpace();
            return spell;
        }

        DLString lastWord = args.back();
        offcuts = lastWord + " " + offcuts;
        args.pop_back();
    }

    // Nothing found.
    return spell;
}

CMDRUN( cast )
{
    std::basic_ostringstream<char> buf;
    Character *victim;
    int mana, slevel;
    bool offensive;
    Skill::Pointer skill;
    Spell::Pointer spell;
    SpellTarget::Pointer target;
    DLString fullArguments, spellName, spellArgs; 

    if (ch->is_npc( ) && !( ch->desc != 0 || ch->master != 0 ))
        return;

    if (ch->is_npc( ) && ch->master != 0) {
        if (!ch->getProfession( )->getFlags( ch ).isSet(PROF_CASTER)) {
            oldact("$C1 говорит тебе '{GЯ не понимаю, чего ты хочешь, хозя$gин|ин|йка.{x'", ch->master, 0, ch, TO_CHAR);
            return;
        }
    }
    
    if (!ch->is_npc( ) && !ch->move) {
        ch->pecho("У тебя нет сил даже пошевелить языком.");
        return;
    }

    if (ch->isAffected(gsn_shielding ) && number_percent( ) > 50) {
        ch->pecho("Ты пытаешься сосредоточиться на заклинании, но что-то останавливает тебя.");
        return;
    }

    if ((ch->isAffected(gsn_garble ) || ch->isAffected(gsn_deafen )) && number_percent( ) > 50) {
        ch->pecho("Ты не можешь настроиться на правильную интонацию.");
        return;
    }

    if (HALF_SHADOW(ch)) {
        ch->pecho("Твоя тень поглощает всякую попытку сотворить заклинание.");
        oldact("$c1 пытается сотворить заклинание, но тень не дает $m сосредоточится.", ch, 0, 0, TO_ROOM);
        return;
    }

    if (ch->death_ground_delay > 0 && ch->trap.isSet( TF_NO_CAST )) {
        ch->pecho("Тебя занимает более важное занятие - спасение своей жизни.");
        return;
    }

    // Split user input into spell name (potentially in quotes) and spell args (rest of the string).
    fullArguments = constArguments;
    fullArguments.stripWhiteSpace();
    spellArgs = fullArguments;
    spellName = spellArgs.getOneArgument();
    
    if (spellName.empty( )) {
        ch->pecho("Колдовать что и на кого?");
        return;
    }

    if (ch->getClan( ) == clan_battlerager && !ch->is_immortal( )) {
        ch->pecho("Ты {RBattleRager{x, а не презренный маг!");
        return;
    }

    if (ch->is_npc( ) && ch->master && ch->master->getClan( ) == clan_battlerager) {
        say_fmt("Хозя%2$Gин|ин|йка, я уважаю твои убеждения.", ch, ch->master);
        return;
    }
    
    // Find the spell and potentially adjust spell arguments.
    spell = spell_lookup(ch, fullArguments, spellName, spellArgs);

    if (!spell) {
        if (ch->is_npc( ) && ch->master) 
            do_say(ch, "Да не умею я");
        else
            ch->pecho("Ты не знаешь такого заклинания.");

        return;
    }
    
    skill = spell->getSkill( );
    
    if (!spell->checkPosition( ch ))
        return;
    
    if (!skill->usable( ch, true ))
        return;
        
    if (IS_SET(ch->in_room->room_flags,ROOM_NO_CAST)) {
        ch->pecho("Стены этой комнаты поглотили твое заклинание.");
        oldact("$c1 произне$gсло|с|сла заклинание, но стены комнаты поглотили его.", ch, 0, 0, TO_ROOM);
        return;
    }

    mana = spell->getManaCost( ch );
    if (!ch->is_npc() && bonus_mana->isActive(ch->getPC(), time_info))
        mana /= 2;

    if (ch->mana < mana) {
        if (ch->is_npc( ) && ch->master != 0) 
            say_fmt("Хозя%2$Gин|ин|йка, у меня мана кончилась!", ch, ch->master);
        else 
            ch->pecho("У тебя не хватает энергии (mana).");

        return;
    }

    target = spell->locateTargets( ch, spellArgs, buf );
    if (target->error != 0) {
        ch->send_to( buf );
        return;
    }

    victim = target->victim;
    offensive = spell->getSpellType( ) == SPELL_OFFENSIVE;

    if (!spell->properOrder(ch)) {
        if (offensive && victim && !victim->is_npc( ))
            say_fmt("Хозя%2$Gин|ин|йка, я %3$Gего|его|её боюсь!", ch, ch->master, victim);
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
        ch->pecho("Ты не можешь сконцентрироваться.");
        skill->improve( ch, false, victim );
        ch->mana -= mana / 2;
        target->castFar = false;
    }
    else {
        try {
            bool fForbidCasting = false;
            bool fForbidReaction = false;

            ch->mana -= mana;
            slevel = spell->getSpellLevel( ch, target->range );
            
            if (victim)
                fForbidCasting = mprog_spell( victim, ch, skill, true );
            
            mprog_cast( ch, target, skill, true );

            if (!fForbidCasting)
                spell->run( ch, target, slevel );

            if (victim)
                fForbidReaction = mprog_spell( victim, ch, skill, false );

            mprog_cast( ch, target, skill, false );
            skill->improve( ch, true, victim );

            if (fForbidReaction)
                return;

        } catch (const VictimDeathException &ex) {
            // In the case when victim dies from the spell, the only thing
            // left to do is to improve knowledge for the caster. 
            skill->improve( ch, true, victim );
            return; 
        }
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


