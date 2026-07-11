/* $Id: ownercoupon.cpp,v 1.1.2.2 2008/02/24 17:25:40 rufina Exp $
 *
 * ruffina, 2007
 */

#include "logstream.h"
#include "ownercoupon.h"
#include "personalquestreward.h"

#include "pcharacter.h"
#include "object.h"

#include "loadsave.h"
#include "wiznet.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

bool OwnerCoupon::use( Character *ch, const char *arg ) 
{ 
    Object *item;
    
    if (ch->is_npc( ) || IS_CHARMED(ch))
        return false;
    
    if (!arg[0]) {
        ch->pecho( _("Какую именно вещь ты хочешь сделать своей собственностью?") );
        return true;
    }

    if (!( item = get_obj_carry( ch, arg ) )) {
        ch->pecho( _("У тебя нет этого.") );
        return true;
    }

    if (item->pIndexData->limit >= 0) {
        ch->pecho( _("Лимиты нельзя приватизировать!") );
        return true;
    }

    if (item->timer > 0) {
        ch->pecho( _("%^O1 исчезнет через некоторое время, не жалко купон тратить?"), item );
        return true;
    }

    if (item->pIndexData == obj->pIndexData) {
        ch->pecho( _("Это неразумно.") );
        return true;
    }

    if (item->behavior) {
        if (item->behavior.getDynamicPointer<PersonalQuestReward>( )) {
            ch->pecho( _("У %O2 уже есть постоянный владелец."), item );
            return true;
        }

        item->behavior->unsetObj( );
    }
        
    item->setOwner( ch->getNameC() );
    SET_BIT(item->extra_flags, ITEM_NOPURGE|ITEM_NOSAC|ITEM_BURN_PROOF);
    // item->setMaterial( "platinum" );
    item->behavior.setPointer( new PersonalQuestReward );
    item->behavior->setObj( item );
        
    LogStream::sendNotice( ) 
        << ch->getPC()->getName( ) << " personalizes " << item->getShortDescr( '1', LANG_DEFAULT ) 
        << " [vnum " << item->pIndexData->vnum << " ID " << item->getID( ) << "] "
        << " using " << (obj->getOwner().empty() ? obj->getOwner().c_str() : "nobody") 
        << "'s coupon [ID " << obj->getID( ) << "]" << endl;

    ch->recho( POS_RESTING, _("%^C1 проделывает манипуляции с %O5 и %O5."), ch, item, obj );
    ch->pecho( _("Ты превращаешь %O4 в свою личную вещь."), item );
    ch->pecho( _("(во избежание недоразумений внимательно прочти help owner!)") );
    extract_obj( obj );
    return true; 
}

bool OwnerCoupon::hasTrigger( const DLString &t )
{
    return (t == "use");
}

