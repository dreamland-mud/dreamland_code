/* $Id: clanarea.cpp,v 1.1.2.2 2007/09/15 09:24:10 rufina Exp $
 *
 * ruffina, 2005
 */

#include "clanarea.h"
#include "clantypes.h"
#include "clanobjects.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"

#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "def.h"

/*--------------------------------------------------------------------------
 * Clan Area 
 *-------------------------------------------------------------------------*/
ClanArea::ClanArea( ) 
{
}

void ClanArea::update( ) 
{
    if (roomVnum <= 0 || altarVnum <= 0 || itemVnum <= 0)
        return;

    if (get_obj_index( itemVnum )->count > 0)
        return;

    if (get_obj_index( altarVnum )->count > 0)
        return;

    createAltar( );
}

void ClanArea::createAltar( )
{
    Object *container, *item;
    Room *room;

    container = create_object( get_obj_index( altarVnum ), 100 );
    item = create_object( get_obj_index( itemVnum ), 100 );
    room = get_room_instance( roomVnum );

    obj_to_obj( item, container );
    obj_to_room( container, room );
    container->behavior.getDynamicPointer<ClanAltar>( )->actAppear( );

    if (clan->getData( ))
        clan->getData( )->setItem( item );
}

ClanReference & ClanArea::getClan( )
{
    return clan;
}

Object * ClanArea::findInvitation( PCharacter *wch )
{
    Object *obj;

    if (invitationVnum <= 0)
        return 0;

    for (obj = wch->carrying; obj; obj = obj->next_content)
        if (obj->pIndexData->vnum == invitationVnum)
            break;

    return obj;
}

