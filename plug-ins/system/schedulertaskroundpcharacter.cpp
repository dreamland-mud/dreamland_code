/* $Id: schedulertaskroundpcharacter.cpp,v 1.1.4.2.6.1 2009/09/24 14:09:13 rufina Exp $
 *
 * ruffina, 2003
 *
 * переписано заново, добавлены приоритеты
 */
/***************************************************************************
                          schedulertask.cpp  -  description
                             -------------------
    begin                : Fri Oct 19 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#pragma implementation

#include "dlscheduler.h"
#include "schedulertaskroundpcharacter.h"
#include "schedulerlist.h"
#include "pcharactermanager.h"
#include "pcharactermemorylist.h"
#include "pcmemoryinterface.h"
#include "pcharacter.h"
#include "descriptor.h"

int SchedulerTaskRoundPCMemory::getPriority( ) const
{
    return SCDP_ROUND + 10;
}

int SchedulerTaskRoundPCharacter::getPriority( ) const
{
    return SCDP_ROUND + 20;
}

void SchedulerTaskRoundPCMemory::run( )
{
    const PCharacterMemoryList& list = PCharacterManager::getPCM( );
    for( PCharacterMemoryList::const_iterator pos = list.begin( );
					      pos != list.end( );
					      pos++ )
	run( pos->second );
}

void SchedulerTaskRoundPCharacter::run( )
{
    for( Descriptor* d = descriptor_list; d != 0; d = d->next )
	if( d->connected == CON_PLAYING && d->character )
	    run( d->character->getPC( ) );
}
