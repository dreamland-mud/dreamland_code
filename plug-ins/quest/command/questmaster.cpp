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
    // The ambient invite moved to the `questmasterchat` Fenia behavior (onSpec),
    // attached alongside this C++ behavior. Neutered here so the two do not both
    // announce; this C++ behavior stays attached only for occupation discovery
    // (getOccupation / canGiveQuest below).
    return false;
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
