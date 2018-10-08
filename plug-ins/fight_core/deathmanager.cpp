/* $Id$
 *
 * ruffina, 2004
 */
#include "deathmanager.h"

void DeathHandler::initialization( )
{
    DeathManager::getThis( )->registrate( Pointer( this ) );
}

void DeathHandler::destruction( )
{
    DeathManager::getThis( )->unregistrate( Pointer( this ) );
}

DeathManager * DeathManager::thisClass = 0;

DeathManager::DeathManager( )
{
    checkDuplicate( thisClass );
    thisClass = this;
}

DeathManager::~DeathManager( )
{
    thisClass = 0;
}

void DeathManager::initialization( )
{
}

void DeathManager::destruction( )
{
}

void DeathManager::handleDeath( Character *killer, Character *victim )
{
    HandlersPriority::iterator hp;
    HandlersList::iterator hl;

    for (hp = handlers.begin( ); hp != handlers.end( ); hp++)
        for (hl = hp->second.begin( ); hl != hp->second.end( ); hl++)
            if ((*hl)->handleDeath( killer, victim ))
                break;
}

void DeathManager::registrate( DeathHandler::Pointer h )
{
    handlers[h->getPriority( )].push_back( h );
    handlers[h->getPriority( )].unique( );
}

void DeathManager::unregistrate( DeathHandler::Pointer h )
{
    handlers[h->getPriority( )].remove( h );
}

