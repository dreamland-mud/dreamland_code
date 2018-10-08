/* $Id$
 *
 * ruffina, 2018
 */
#include "webprompt.h"
#include "json/json.h"
#include "descriptor.h"
#include "character.h"
#include "def.h"

WebPromptManager * WebPromptManager::thisClass = 0;

WebPromptManager::WebPromptManager( )
{
    checkDuplicate( thisClass );
    thisClass = this;
}

WebPromptManager::~WebPromptManager( )
{
    thisClass = 0;
}

void WebPromptManager::initialization( )
{
}

void WebPromptManager::destruction( )
{
}

void WebPromptManager::registrate( WebPromptListener::Pointer wpl ) 
{
    listeners.push_back( wpl );
}

void WebPromptManager::unregistrate( WebPromptListener::Pointer wpl ) 
{
    for (Listeners::iterator i = listeners.begin( ); i != listeners.end( ); i++)
        if (*i == wpl) {
            listeners.erase( i );
            break;
        }
}

void WebPromptManager::handle( Descriptor *d, Character *ch, Json::Value &json )
{
    for (Listeners::iterator i = listeners.begin( ); i != listeners.end( ); i++)
        (*i)->run( d, ch, json );
}


void WebPromptListener::initialization( ) 
{
    WebPromptManager::getThis( )->registrate( Pointer( this ) );
}

void WebPromptListener::destruction( ) 
{
    WebPromptManager::getThis( )->unregistrate( Pointer( this ) );
}

