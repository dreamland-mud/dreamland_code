/* $Id$
 *
 * ruffina, 2004
 */
#include "clanmanager.h"
#include "clan.h"
#include "clan.h"

ClanManager* clanManager = 0;

ClanManager::ClanManager( ) 
{
    checkDuplicate( clanManager );
    clanManager = this;
}

ClanManager::~ClanManager( )
{
    clanManager = 0;
}

GlobalRegistryElement::Pointer ClanManager::getDumbElement( const DLString &name ) const
{
    return Clan::Pointer( NEW, name );
}
