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

#include "handler.h"
#include "def.h"

#include "logstream.h"
#define log(x) LogStream::sendNotice() << x << endl


CLAN(battlerager);
GSN(shielding);
GSN(garble);
GSN(deafen);

bool mprog_spell( Character *victim, Character *caster, Skill::Pointer skill, bool before );
void mprog_cast( Character *caster, SpellTarget::Pointer target, Skill::Pointer skill, bool before );

static bool string_has_separator(const DLString &s)
{
    for (DLString::size_type i = 0; i < s.size(); i++)
        if (dl_is_arg_separator(s.at(i)))
            return true;

    return false;
}

// Replace a string like ', "" (with nothing but spaces and separators in it) with an empty string.
static DLString normalize_string(const DLString &s)
{
    for (DLString::size_type i = 0; i < s.size(); i++) {
        char c = s.at(i);
        if (!dl_is_arg_separator(c) && !dl_isspace(c))
            return s;
    }

    return DLString::emptyString;
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
    bool offensive;
    Skill::Pointer skill;
    Spell::Pointer spell;
    SpellTarget::Pointer target;
    DLString fullArguments, spellName, spellArgs; 

    if (ch->is_npc( ) && !( ch->desc != 0 || ch->master != 0 ))
        return;

    if (ch->is_npc( ) && IS_CHARMED(ch)) {
        if (!ch->getProfession( )->getFlags( ch ).isSet(PROF_CASTER)) {
            ch->master->pecho("%1$^C1 хотел%1$Gо||а|и бы помочь, но, к сожалению, совершенно не способ%1$Gно|ен|на|ны колдовать.", ch);
            return;
        }
    }
    
    if (!ch->is_npc( ) && !ch->move) {
        ch->pecho("У тебя нет сил даже пошевелить языком.");
        return;
    }

    // Split user input into spell name (potentially in quotes) and spell args (rest of the string).
    // Symbol '!' can act as a spell name separator, but replace it with quotes first.
    fullArguments = constArguments;
    fullArguments.stripWhiteSpace();
    fullArguments = fullArguments.replaces("!", "'");
    spellArgs = fullArguments;
    spellName = spellArgs.getOneArgument();
    
    if (spellName.empty( )) {
        ch->pecho("Колдовать что и на кого?");
        return;
    }

    if (ch->getClan( ) == clan_battlerager && !ch->is_immortal( )) {
        ch->pecho("Ты вои{Smн{Sfтельница{Sx {RКлана Ярости{x, а не презренный маг!");
        return;
    }

    if (ch->is_npc( ) && IS_CHARMED(ch) && ch->master->getClan( ) == clan_battlerager) {
        say_fmt("Хозя%2$Gин|ин|йка, я уважаю твои убеждения.", ch, ch->master);
        return;
    }
    
    // Find the spell and potentially adjust spell arguments.
    spell = spell_lookup(ch, fullArguments, spellName, spellArgs);
    spellArgs = normalize_string(spellArgs);
    
    if (!spell) {
        if (ch->is_npc( ) && IS_CHARMED(ch)) 
            do_say(ch, "Я не знаю такого заклинания.");
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
        ch->pecho("Эта местность поглощает и блокирует твое заклинание.");
        ch->recho("%1$^C1 пыта%1$nется|ются сотворить заклинание, но что-то в этой местности блокирует %1$P2.", ch);
        return;
    }

    int mana = skill->getMana(ch);
    if (mana > 0 && ch->mana < mana) {
        if (ch->is_npc( ) && IS_CHARMED(ch)) 
            say_fmt("Хозя%2$Gин|ин|йка, у меня мана кончилась!", ch, ch->master);
        else 
            ch->pecho("У тебя не хватает энергии{lE (mana){x.");

        return;
    }

    int moves = skill->getMoves(ch);
    if (moves > 0 && ch->move < moves) {
        if (ch->is_npc( ) && IS_CHARMED(ch))
            say_fmt("Хозя%2$Gин|ин|йка, я слишком устал%1$Gо||а!", ch, ch->master);
        else
            ch->pecho("Ты слишком уста%Gло|л|ла для этого.", ch);

        return;
    }

    target = spell->locateTargets( ch, spellArgs, buf );
    if (target->error != 0) {
        ch->pecho( buf.str() );
        if (ch->is_npc() && IS_CHARMED(ch) && !buf.str().empty())
            say_fmt("Хозя%2$Gин|ин|йка, у меня ничего не получится: %3$s", ch, ch->master, buf.str().c_str());

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

    ch->setWait(spell->getBeats(ch) );

    if (ch->isAffected( gsn_shielding ) && number_percent( ) > 50) {
        ch->pecho("Ты пытаешься сотворить заклинание, но изолирующий экран блокирует тебя.");
        ch->recho("%1$^C1 пыта%1$nется|ются сотворить заклинание, но изолирующий экран блокирует %1$P2.", ch);
        return;
    }

    if (ch->isAffected( gsn_garble ) && number_percent( ) > 50) {
        ch->pecho("Твой язык заплетается, и ты не можешь как следует произнести заклинание.");
        ch->recho("Язык %1$C2 заплетается, и %1$P1 не мо%1$nжет|гут как следует произнести заклинание.", ch);        
        return;
    }
    
    if (ch->isAffected( gsn_deafen ) && number_percent( ) > 50) {
        ch->pecho("Глухота мешает тебе как следует произнести заклинание.");
        ch->recho("Глухота мешает %1$C3 как следует произнести заклинание.", ch);        
        return;
    }    

    if (HALF_SHADOW(ch)) {
        ch->pecho("Твоя тень поглощает всякую попытку сотворить заклинание.");
        ch->recho("%1$^C1 пыта%1$nется|ются сотворить заклинание, но тень не дает %1$P3 сосредоточиться.", ch);
        return;
    }

    if (ch->death_ground_delay > 0 && ch->trap.isSet( TF_NO_CAST )) {
        ch->pecho("Ловушка, в которой ты оказал%1$Gось|ся|ась, препятствует сотворению заклинаний.", ch);
        ch->recho("%1$^C1 пыта%1$nется|ются сотворить заклинание, но ловушка мешает %1$P3 сосредоточиться.", ch);
        return;
    }

    spell->utter( ch );

    if (offensive) {
        UNSET_DEATH_TIME(ch);

        if (victim && is_safe( ch, victim ))
            return;

        if (victim)
            set_violent( ch, victim, false );
    }
     
    if (spell->spellbane( ch, victim ))
        return;
        
    if (number_percent( ) > skill->getEffective( ch )) {
        ch->pecho("Ты пытаешься сотворить заклинание, но теряешь концентрацию и терпишь неудачу.");
        ch->recho("%1$^C1 пыта%1$nется|ются сотворить заклинание, но теря%1$nет|ют концентрацию и терп%1$nит|ят неудачу.", ch);

        skill->improve( ch, false, victim );
        ch->mana -= mana / 2;
        ch->move -= moves / 2;

        target->castFar = false;
    }
    else {
        int slevel = spell->getSpellLevel( ch, target->range );
        // Check if spell level penalty prevents from casting.
        if (slevel <= 0)
            return;

        try {
            bool fForbidCasting = false;
            bool fForbidReaction = false;

            ch->mana -= mana;
            ch->move -= moves;            
            
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
        yell_panic(ch, victim, "Помогите! Кто-то напал на меня!", "Помогите! %1$^C1 напа%1$Gло|л|ла на меня!");

        if (target->castFar && target->door != -1) {
            ch->setLastFightTime( );
            victim->setLastFightTime( );

            if (victim->is_npc( ) && victim->getNPC( )->behavior)
                victim->getNPC( )->behavior->shot( ch, target->door );
        }
        else
            attack_caster( ch, victim );
    }
}


