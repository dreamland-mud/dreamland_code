/* $Id: descriptorstatemanager.cpp,v 1.1.4.2 2005/09/16 13:10:12 rufina Exp $
 *
 * ruffina, 2003
 */
#include "descriptorstatemanager.h"
#include "descriptorstatelistener.h"
#include "descriptor.h"
#include "fenia_utils.h"
#include "pcharacter.h"
#include "def.h"

DescriptorStateManager * DescriptorStateManager::thisClass = 0;

DescriptorStateManager::DescriptorStateManager( )
{
    checkDuplicate( thisClass );
    thisClass = this;
}

DescriptorStateManager::~DescriptorStateManager( )
{
    thisClass = 0;
}

void DescriptorStateManager::initialization( )
{
}

void DescriptorStateManager::destruction( )
{
}

void DescriptorStateManager::registrate( DescriptorStateListener::Pointer dsl ) 
{
    listeners.push_back( dsl );
}

void DescriptorStateManager::unregistrate( DescriptorStateListener::Pointer dsl ) 
{
    for (Listeners::iterator i = listeners.begin( ); i != listeners.end( ); i++)
        if (*i == dsl) {
            listeners.erase( i );
            break;
        }
}

void DescriptorStateManager::handle( int oldState, int newState, Descriptor *d ) 
{
    for (Listeners::iterator i = listeners.begin( ); i != listeners.end( ); i++)
        (*i)->run( oldState, newState, d );

    if (newState == CON_PLAYING && d->character && d->character->getPC())
        gprog("onConnect", "C", d->character->getPC());
}

