/* $Id: questmaster.cpp,v 1.1.2.8.6.6 2009/02/07 17:05:55 rufina Exp $
 *
 * ruffina, 2005
 */
#include "npcharacter.h"
#include "merc.h"
#include "interp.h"
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

    interpret_raw(ch, "say", "Хочешь получить интересное задание???");
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


