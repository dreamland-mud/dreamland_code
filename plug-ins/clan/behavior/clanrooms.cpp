/* $Id$
 *
 * ruffina, 2004
 */
#include "clanrooms.h"
#include "clanreference.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "dreamland.h"
#include "merc.h"

#include "def.h"


/*--------------------------------------------------------------------------
 * ClanRoom base class 
 *-------------------------------------------------------------------------*/
ClanArea::Pointer ClanRoom::getClanArea( )
{
    ClanArea::Pointer clanArea;
    AreaIndexData *area;

    area = getRoom( )->areaIndex();

    if (area->behavior) 
        clanArea = area->behavior.getDynamicPointer<ClanArea>( );

    return clanArea;
}


/*--------------------------------------------------------------------------
 * ClanPetShopStorage 
 *-------------------------------------------------------------------------*/
bool ClanPetShopStorage::canServeClient( Character *client )
{
    ClanArea::Pointer clanArea;
    
    if (!PetShopStorage::canServeClient( client ))
        return false;

    if (client->is_npc( ))
        return false;

    if (client->getClan( ) == room->pIndexData->clan) 
        return true;

    clanArea = getClanArea( );

    if (!clanArea)
        return true;
    
    if (clanArea->findInvitation( client->getPC( ) )) 
        return true;        
    
    client->pecho( "Тебя здесь обслуживать не будут." );
    return false;
}

