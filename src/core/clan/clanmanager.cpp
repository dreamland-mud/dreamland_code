/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "clanmanager.h"
#include "clan.h"
#include "merc.h"

ClanManager* clanManager = 0;

ClanManager::ClanManager( ) 
{
    checkDuplicate( clanManager );
    clanManager = this;
    setRegistryName("clan");
    saveRegistryName();
}

ClanManager::~ClanManager( )
{
    eraseRegistryName();
    clanManager = 0;
}

GlobalRegistryElement::Pointer ClanManager::getDumbElement( const DLString &name ) const
{
    return Clan::Pointer( NEW, name );
}

void ClanManager::addClanHall(const DLString &clanName, area_file *areaFile)
{
    LogStream::sendNotice() << "Clan hall for " << clanName << " is " << areaFile->file_name << endl;
    clanHalls[clanName] = areaFile;
}

area_file * ClanManager::findClanHall(const DLString &clanName)
{
    auto c = clanHalls.find(clanName);
    if (c == clanHalls.end())
        return 0;
    return c->second;
}

