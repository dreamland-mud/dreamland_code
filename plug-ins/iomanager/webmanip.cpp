/* $Id$
 *
 * ruffina, 2018
 */
#include <stdarg.h>
#include "webmanip.h"


WebManip::~WebManip( )
{
}

void WebManip::initialization( ) 
{
    webManipManager->registrate( Pointer( this ) );
}

void WebManip::destruction( ) 
{
    webManipManager->unregistrate( Pointer( this ) );
}

bool WebManip::decorateItem( ostringstream &buf, const DLString &descr, Object *item, Character *, const DLString &pocket, int combined ) const
{
    return false;
}

bool WebManip::decorateShopItem( ostringstream &buf, const DLString &descr, Object *item, Character * ) const
{
    return false;
}

bool WebManip::decoratePocket( ostringstream &buf, const DLString &pocket, Object *container, Character *ch ) const
{
    return false;
}

WebManipManager* webManipManager = NULL;

WebManipManager::WebManipManager( ) 
{
    checkDuplicate( webManipManager );
    webManipManager = this;
}

WebManipManager::~WebManipManager( )
{
    webManipManager = NULL;
}

void WebManipManager::registrate( WebManip::Pointer m )
{
    manips.push_back( m );
}

void WebManipManager::unregistrate( WebManip::Pointer m )
{
    manips.remove( m );
}


void WebManipManager::decorateItem( ostringstream &buf, const DLString &descr, Object *item, Character *ch, const DLString &pocket, int combined ) const
{
    bool needDefaultAction = true;

    // Find a registered 'manipulator' to handle this item.
    for (WebManipList::const_iterator m = manips.begin( ); m != manips.end( ); m++)
        if ((*m)->decorateItem( buf, descr, item, ch, pocket, combined )) {
            needDefaultAction = false;
            break;
        }

    // No one was succesful, simply output the description as is without decorations.
    if (needDefaultAction)
        buf << descr;
}

void WebManipManager::decorateShopItem( ostringstream &buf, const DLString &descr, Object *item, Character *ch ) const
{
    bool needDefaultAction = true;

    for (WebManipList::const_iterator m = manips.begin( ); m != manips.end( ); m++)
        if ((*m)->decorateShopItem( buf, descr, item, ch )) {
            needDefaultAction = false;
            break;
        }

    if (needDefaultAction)
        buf << descr;
}

void WebManipManager::decoratePocket( ostringstream &buf, const DLString &pocket, Object *container, Character *ch ) const
{
    bool needDefaultAction = true;

    for (WebManipList::const_iterator m = manips.begin( ); m != manips.end( ); m++)
        if ((*m)->decoratePocket( buf, pocket, container, ch )) {
            needDefaultAction = false;
            break;
        }

    if (needDefaultAction)
        buf << pocket;
}

void WebManipManager::initialization( )
{
}

void WebManipManager::destruction( )
{
}

