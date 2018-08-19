/* $Id$
 *
 * ruffina, 2018
 */
#include "webmanip.h"

WebManipCommand::~WebManipCommand( )
{
}

void WebManipCommand::initialization( )
{
    webManipManager->registrate( Pointer( this ) );
}

void WebManipCommand::destruction( ) 
{
    webManipManager->unregistrate( Pointer( this ) );
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

void WebManipManager::registrate( WebManipCommand::Pointer m )
{
    manips[m->getName( )] = m;
}

void WebManipManager::unregistrate( WebManipCommand::Pointer m )
{
    WebManipMap::iterator i = manips.find(m->getName( ));
    if (i != manips.end( ))
    	manips.erase(m->getName( ));
}


void WebManipManager::decorateItem( ostringstream &buf, const DLString &descr, Object *item, Character *ch, const DLString &pocket, int combined ) const
{
	static const DLString COMMAND_NAME = "decorateItem";
	ItemManipArgs args( ch, item, descr, pocket, combined );
	if (!run( buf, COMMAND_NAME, args ))
		buf << descr;
}

void WebManipManager::decorateShopItem( ostringstream &buf, const DLString &descr, Object *item, Character *ch ) const
{
	static const DLString COMMAND_NAME = "decorateShopItem";
	ShopItemManipArgs args( ch, item, descr );
	if (!run( buf, COMMAND_NAME, args ))
		buf << descr;
}

void WebManipManager::decoratePocket( ostringstream &buf, const DLString &pocket, Object *container, Character *ch ) const
{
	static const DLString COMMAND_NAME = "decoratePocket";
	PocketManipArgs args( ch, pocket, container );
	if (!run( buf, COMMAND_NAME, args ))
		buf << pocket;
}

bool WebManipManager::run( ostringstream &buf, const DLString &command, const ManipCommandArgs &args ) const
{
	bool success = false;

	WebManipMap::const_iterator i = manips.find( command );
	if (i != manips.end( )) {
		WebManipCommand::Pointer cmd = i->second;
		if (cmd->run( buf, args )) {
			success = true;
		}
	}

	return success;
}

void WebManipManager::initialization( )
{
}

void WebManipManager::destruction( )
{
}

