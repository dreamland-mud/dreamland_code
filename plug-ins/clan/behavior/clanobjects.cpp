/* $Id: clanobjects.cpp,v 1.1.2.5 2009/01/01 13:53:11 rufina Exp $
 *
 * ruffina, 2005
 */

#include "clanobjects.h"
#include "clantypes.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"

#include "merc.h"
#include "fight.h"
#include "descriptor.h"
#include "mercdb.h"
#include "interp.h"
#include "handler.h"
#include "infonet.h"
#include "messengers.h"
#include "act.h"
#include "act_move.h"
#include "def.h"

PROF(none);

/*--------------------------------------------------------------------------
 * Clan Object (base class)
 *-------------------------------------------------------------------------*/
ClanObject::ClanObject( )
{
}

ClanObject::~ClanObject( )
{
}

ClanArea::Pointer ClanObject::getClanArea( )
{
    ClanArea::Pointer clanArea;
    AreaIndexData *area;

    area = obj->pIndexData->area;

    if (area->behavior) 
        clanArea = area->behavior.getDynamicPointer<ClanArea>( );

    return clanArea;
}

/*--------------------------------------------------------------------------
 * Clan Item
 *-------------------------------------------------------------------------*/
ClanItem::ClanItem( ) 
{
}

bool ClanItem::isHolded( ) const
{
    Character *carrier = obj->carried_by;
    
    if (!carrier)
        return false;

    if (carrier->is_npc( ))
        return false;

    if (IS_SET(carrier->in_room->room_flags, ROOM_SAFE|ROOM_SOLITARY|ROOM_PRIVATE))
        return false;
     
    if (!carrier->in_room->pIndexData->guilds.empty( ))
        return false;

    if (carrier->getModifyLevel( ) < obj->level - 3)
        return false;

    return true;
}

bool ClanItem::area( ) 
{ 
    if (!clan->getData( ) || clan->getData( )->hasItem( ))
        return false;
    
    if (isHolded( ))
        return false;
    
    actDisappear( );
    extract_obj( obj );
    return true;
}

void ClanItem::actDisappear( )
{
    act("%3$^O1 загадочным образом исчезает.",  obj->getRoom( )->people, 0, obj,TO_ALL);
}

void ClanItem::get( Character *ch ) 
{ 
    if (ch->is_npc()) {
        ch->pecho("Ты не можешь владеть %1$O5 и бросаешь %1$P2.", obj);
        ch->recho("%2$^C1 не может владеть %1$O5 и бросает %1$P2.", obj, ch);
        obj_from_char(obj);
        obj_to_room(obj, ch->in_room);
        return;
    }
}

void ClanItem::give( Character *from, Character *mob ) 
{ 
    get( mob );
}

bool ClanItem::sac( Character *ch ) 
{ 
    act("{RБОГИ В ГНЕВЕ!{x",ch,0,0,TO_ALL);

    rawdamage( ch, ch, DAM_HOLY, ch->hit - 10, true );
    ch->gold = 0;

    return false;
}

bool ClanItem::extract( bool count ) 
{ 
    if (clan->getData( ))
        clan->getData( )->unsetItem( obj );

    return ClanObject::extract( count );
}

/*--------------------------------------------------------------------------
 * Clan Altar 
 *-------------------------------------------------------------------------*/
ClanAltar::ClanAltar( ) 
{
}

bool ClanAltar::fetch( Character *ch, Object *item ) 
{ 
    Descriptor *d;
    ClanArea::Pointer clanArea = getClanArea( ); 
    
    if (!clanArea)
        return false;
    
    if (item->pIndexData->vnum != clanArea->itemVnum)
        return false;
    
    actDisappear( );

    for (d = descriptor_list; d; d = d->next) 
        if (d->connected == CON_PLAYING 
            && d->character 
            && d->character->getClan( ) == clanArea->getClan( ))
        {
            actNotify( d->character );
        }
    
    if (clan->getData( ))
        clan->getData( )->unsetItem( item );

    DLString what = fmt(0, "{WКлан %s утратил свою святыню.{x", clanArea->getClan()->getShortName().c_str());
    infonet(0, 0, "{CЕхидный голос из $o2: ", what.c_str());
    send_discord_clan(what);
    send_telegram(what);

    extract_obj(obj);
    return true;
}

void ClanAltar::actAppear( )
{
    act("Ты видишь, как медленно появляется %3$O1.", obj->in_room->people, 0, obj,TO_ALL);
}

void ClanAltar::actDisappear( )
{
    act("%3$^O1 растворяется и исчезает!", obj->getRoom( )->people, 0, obj,TO_ALL);
}

void ClanAltar::actNotify( Character *ch )
{
    oldact_p("{gТы вздрагиваешь от осознания Силы своего Клана!{x", 
            ch, 0, 0, TO_CHAR, POS_DEAD );
}

