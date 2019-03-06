/* $Id: templeman.cpp,v 1.1.2.4 2010-09-01 21:20:46 rufina Exp $
 *
 * ruffina, 2005
 */

#include "templeman.h"

#include "pcharacter.h"
#include "npcharacter.h"

#include "religion.h"
#include "interp.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "arg_utils.h"
#include "act.h"
#include "def.h"

RELIG(none);

/*----------------------------------------------------------------------
 * Templeman
 *---------------------------------------------------------------------*/
Templeman::Templeman( ) 
{
}

void Templeman::tell( Character *victim, const char *msg )
{
    speech(victim, msg);
}

void Templeman::speech( Character *victim, const char *speech )
{
    if (!IS_AWAKE(ch))
        return;

    if (victim->is_npc()) {
        do_say(ch, "Изыди, глупое животное.");
        return;
    }
    
    PCharacter *pvict = victim->getPC();

    if (arg_oneof(speech, "religion", "религия")) {
        do_say(ch, "Ты действительно интересуешься религией?");
        do_say(ch, "Подробности можешь почитать в 'справка религия'.");
        do_say(ch, "Не забудь, что ты религию сможешь выбрать только один раз.");
        do_say(ch, "Если ты ошибешься, я не смогу это исправить!");
        return;
    }
    
    DLString speechStr = DLString(speech).toLower();
    Religion *chosen = religionManager->findExisting(speechStr);

    if (!chosen) {
        if (religionManager->findUnstrict(speechStr)) 
            say_fmt("Назови мне целиком имя бога, которому ты хочешь служить.", ch);
        
        return;
    }
    
    if (pvict->getReligion( ) != god_none) {
        if (chosen->getName() == pvict->getReligion()->getName())
            say_fmt("Ты и так поклоняешься %2$N3.",
                    ch, pvict->getReligion( )->getRussianName( ).c_str( ) );
        else
            say_fmt("Ты уже выбра%2$Gло|л|ла свой путь! Твоя религия - %3$N1.",
                    ch, pvict, pvict->getReligion( )->getRussianName( ).c_str( ) );
        return;
    }
    
    if (!chosen->isAllowed( pvict )) {
        do_say(ch, "Эта религия не соответствует твоему характеру или расе.");
        return;
    }

    pvict->setReligion( chosen->getName( ) );
    say_fmt("С этой минуты ты навсегда избираешь своей религией %2$N4.",
            ch, pvict->getReligion( )->getRussianName( ).c_str( ) );
}

void Templeman::greet( Character *victim )
{
    if (!IS_AWAKE(ch))
        return;

    if (!ch->can_see(victim) || victim->is_npc() || victim->is_immortal())
        return;

    interpret_fmt( ch, "smile %s", victim->getNameP( ) );
}

