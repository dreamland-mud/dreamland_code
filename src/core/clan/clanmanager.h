/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __CLANMANAGER_H__
#define __CLANMANAGER_H__

#include "oneallocate.h"
#include "globalregistry.h"

class Clan;

class ClanManager : public GlobalRegistry<Clan>, 
                    public OneAllocate
{
public:
    
    ClanManager( );
    virtual ~ClanManager( );

    inline static ClanManager *getThis( );
    
private:
    virtual GlobalRegistryElement::Pointer getDumbElement( const DLString & ) const;
};

extern ClanManager *clanManager;

inline ClanManager * ClanManager::getThis( )
{
    return clanManager;
}

#endif
