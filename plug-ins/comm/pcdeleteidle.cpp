/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          pcdeleteidle.cpp  -  description
                             -------------------
    begin                : Fri Oct 19 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include "pcdeleteidle.h"

#include "logstream.h"
#include "date.h"

#include "pcharactermanager.h"
#include "dlscheduler.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "dreamland.h"

PCDeleteIdle::PCDeleteIdle( )
{
}

PCDeleteIdle::~PCDeleteIdle( )
{
}

int PCDeleteIdle::getDiff( PCMemoryInterface *pcm )
{
    int level = pcm->get_trust( );
    int remorts = pcm->getRemorts( ).size( );

    if (remorts == 0 && level < 2)
        return Date::SECOND_IN_MONTH;
    
    if (remorts == 0 && level <= 20)
        return Date::SECOND_IN_MONTH * 6;
    
    if (level <= LEVEL_MORTAL)
        return Date::SECOND_IN_YEAR;
    
    return -1;
}

void PCDeleteIdle::run( int oldState, int newState, Descriptor *d ) 
{
    PCharacter *ch;
    int diff;
    ostringstream buf;

    if (oldState != CON_PLAYING || newState != CON_QUIT)
        return;

    if (!d || !d->character)
        return;
    
    ch = d->character->getPC( );

    diff = getDiff( ch );

    if (diff < 0)
        return;

    buf << "{xПерсонаж будет автоматически {RУДАЛЕН{x {Y" 
        << Date::getTimeAsString( dreamland->getCurrentTime( ) + diff ) 
        << "{x." << endl;

    ch->send_to( buf );
}

void PCDeleteIdle::run( PCMemoryInterface* pcm )
{
    const int diff = getDiff( pcm );
    const int lastAccessDate = pcm->getLastAccessTime( ).getTime( );
    const int currentDate = dreamland->getCurrentTime( );
    
    if (diff < 0)
        return;

    if (currentDate <= lastAccessDate + diff)
        return;
    
    eraseList.push_back( pcm->getName( ) );
}

void PCDeleteIdle::after( )
{
    while (!eraseList.empty( )) {
        DLString name = eraseList.front( );

        eraseList.pop_front( );

        if (PCharacterManager::pfDelete( name ))
            LogStream::sendWarning( ) << name << "'s deleted" << endl;
    }
        
    DLScheduler::getThis( )->putTaskInSecond( Date::SECOND_IN_DAY, Pointer( this ) );
}


void PCDeleteIdle::initialization( )
{
    SchedulerTaskRoundPlugin::initialization( );
    DescriptorStateListener::initialization( );
}

void PCDeleteIdle::destruction( )
{
    SchedulerTaskRoundPlugin::destruction( );
    DescriptorStateListener::destruction( );
}

