/* $Id: questmaster.cpp,v 1.1.2.8.6.6 2009/02/07 17:05:55 rufina Exp $
 *
 * ruffina, 2005
 */
#include <vector>

#include "player_utils.h"
#include "questmaster.h"

#include "npcharacter.h"
#include "room.h"
#include "merc.h"
#include "arg_utils.h"
#include "act.h"

#include "loadsave.h"
#include "l10n.h"

/*--------------------------------------------------------------------------
 * QuestMaster
 *------------------------------------------------------------------------*/
QuestMaster::QuestMaster( ) 
{
}

bool QuestMaster::specIdle( ) 
{ 
    if (chance(99))
        return false;

    // do_say hands its text to the `say` command as one baked argument, so an
    // untargeted ambient line can only ever come out Russian. say_act renders in
    // a listener's language and still echoes TO_ALL, so the invitation is
    // addressed to somebody who is actually standing there.
    //
    // This also leaves the say CHANNEL behind: channel-off and deafness no
    // longer suppress the line, and drunk garble and translate no longer apply
    // to it. That is how every other questmaster utterance already behaves --
    // they are all say_act -- so this makes the mob consistent with itself.
    //
    // The filter is deliberately side-effect free: canGiveQuest() routes to
    // QuestTrader::canServeClient(), which say_acts a refusal at ghosts, charmed
    // and invisible players -- using it here would have the questmaster scold
    // the room on every idle tick.
    //
    // IS_AWAKE matters because the chosen listener defines the language for the
    // whole room: oldact's position floor keeps a sleeper from seeing the line
    // at all, so picking one would render it in a language nobody reading it
    // asked for.
    std::vector<Character *> listeners;

    for (Character *wch = ch->in_room->people; wch; wch = wch->next_in_room)
        if (!wch->is_npc( ) && IS_AWAKE( wch ) && ch->can_see( wch ))
            listeners.push_back( wch );

    // Nobody to invite: stay quiet rather than talk to an empty room.
    if (listeners.empty( ))
        return false;

    say_act( listeners[number_range( 0, listeners.size( ) - 1 )], ch,
             _("Хочешь получить интересное задание? Напиши {y{hcквест попросить{x.") );
    return true;
}

int QuestMaster::getOccupation( )
{
    return Questor::getOccupation( ) | QuestTrader::getOccupation( );
}

bool QuestMaster::canGiveQuest( Character *ach )
{
    return QuestTrader::canServeClient( ach );
}

static bool my_message(const char *msg)
{
    if (arg_is_yes(msg))
        return true;
    if (arg_oneof_strict(msg, "хочу"))
        return true;
    // Ukrainian "завдання" was missing, so a UA player could not ask for a quest
    // in their own language -- same gap the pet's "де ти?" had.
    if (arg_contains_someof(msg, "задание завдання квест quest"))
        return true;
    
    return false;
}

static void tell_hint(Character *ch, Character *victim)
{
    tell_fmt(_("Ты очень отваж%1$Gно|ен|на, %1$C1!"), victim, ch);
    tell_fmt(_("Изучи справку по теме {hh125квестор{hx, а когда будешь готов%1$Gо||а, набери {y{hcквест попросить{x."), victim, ch);
}

void QuestMaster::speech( Character *victim, const char *msg )
{
    if (my_message(msg)) {
        tell_hint(ch, victim);
        oldact(_("$c1 что-то говорит $C3."), ch, 0, victim, TO_NOTVICT);
    }
}

void QuestMaster::tell( Character *victim, const char *msg )
{
    if (my_message(msg)) {
        tell_hint(ch, victim);
    }
}

DefaultQuestMaster::~DefaultQuestMaster()
{

}
