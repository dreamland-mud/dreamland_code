/* $Id: descriptorstatelistener.cpp,v 1.1.4.1 2005/07/30 14:50:07 rufina Exp $
 *
 * ruffina, 2003
 */

#include "descriptorstatelistener.h"
#include "descriptorstatemanager.h"

void DescriptorStateListener::initialization( ) 
{
    DescriptorStateManager::getThis( )->registrate( Pointer( this ) );
}

void DescriptorStateListener::destruction( ) 
{
    DescriptorStateManager::getThis( )->unregistrate( Pointer( this ) );
}

