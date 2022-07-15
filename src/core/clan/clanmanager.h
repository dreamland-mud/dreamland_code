/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __CLANMANAGER_H__
#define __CLANMANAGER_H__

#include "oneallocate.h"
#include "globalregistry.h"

class Clan;
struct area_file;

class ClanManager : public GlobalRegistry<Clan>, 
                    public OneAllocate
{
public:
    
    ClanManager( );
    virtual ~ClanManager( );

    inline static ClanManager *getThis( );
    void addClanHall(const DLString &clanName, area_file *areaFile);
    area_file * findClanHall(const DLString &clanName);
    
private:
    virtual GlobalRegistryElement::Pointer getDumbElement( const DLString & ) const;

    // Map from clan name to corresponding clan area, populated on clan area behavior load.
    map<DLString, area_file *> clanHalls; 
};

extern ClanManager *clanManager;

inline ClanManager * ClanManager::getThis( )
{
    return clanManager;
}

#endif
