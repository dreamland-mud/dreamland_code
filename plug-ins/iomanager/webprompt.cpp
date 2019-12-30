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

static const DLString NONE = "none";

WebPromptAttribute::~WebPromptAttribute( )
{
}    


void WebPromptAttribute::init( )
{
    clear( );
}

void WebPromptAttribute::clear( )
{
    prompt.clear( );
}   


void WebPromptAttribute::updateIfNew( const DLString &field, const Json::Value &newValue, Json::Value &prompt )
{
    // First time a value disappears, we need to send "none" to front-end to hide corresponding row.
    // Subsequent calls can just omit the value from prompt, until it's back again.
    if (newValue.empty( )) {
        if (!this->prompt.isAvailable( field ) || !this->prompt[field].empty( )) {
            prompt[field] = NONE;
        }
        this->prompt[field] = DLString::emptyString;
        return;
    }

    // Serialize new value to string and see if it differs from the stored value.
    Json::FastWriter writer;
    DLString value = writer.write( newValue );
    if (!this->prompt.isAvailable( field ) || this->prompt[field] != value) {
        // First occurence or changed value, update it in the attribute and send to front-end.
        this->prompt[field] = value;
        prompt[field] = newValue;
        return;
    }

    // Nothing changed, send nothing in the prompt.
}

