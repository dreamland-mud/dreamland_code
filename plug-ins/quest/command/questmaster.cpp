/* $Id: questmaster.cpp,v 1.1.2.8.6.6 2009/02/07 17:05:55 rufina Exp $
 *
 * ruffina, 2005
 */
#include "npcharacter.h"
#include "merc.h"
#include "interp.h"
#include "arg_utils.h"
#include "act.h"
#include "mercdb.h"

#include "questmaster.h"
#include "def.h"

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

    do_say(ch, "Хочешь получить интересное задание?");
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
    if (arg_contains_someof(msg, "задание квест quest"))
        return true;
    
    return false;
}

static void tell_hint(Character *ch, Character *victim)
{
    tell_fmt("Ты очень отваж%1$Gно|ен|на, %1$C1!", victim, ch);
    tell_fmt("Изучи справку по теме {hhквестор{hx, а когда будешь готов%1$G|о|а, набери {y{hc{lRквест попросить{lEquest request{x.", victim, ch);
}

void QuestMaster::speech( Character *victim, const char *msg )
{
    if (my_message(msg)) {
        tell_hint(ch, victim);
        oldact("$c1 что-то говорит $C3.", ch, 0, victim, TO_NOTVICT);
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
