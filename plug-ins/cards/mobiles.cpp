/* $Id: mobiles.cpp,v 1.1.2.10.6.1 2007/09/11 00:33:54 rufina Exp $
 *
 * ruffina, 2005
 */

#include "xmlattributecards.h"
#include "mobiles.h"
#include "objects.h"
#include "occupations.h"

#include "class.h"
#include "regexp.h"

#include "clanreference.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "act.h"
#include "merc.h"
#include "handler.h"
#include "mercdb.h"
#include "def.h"

CLAN(none);
CLAN(flowers);

CardStarterBehavior::CardStarterBehavior( )
{
}

bool CardStarterBehavior::death( Character *killer ) 
{
    XMLAttributeCards::Pointer card;
    int suit;
    
    if (!killer || killer->is_npc( ))
        return false;
    if (killer->getClan( ) == clan_flowers)
        return false;

    card = killer->getPC( )->getAttributes( ).getAttr<XMLAttributeCards>( "cards" );
    suit = card->getRandomSuit( );

    oldact("{cТы уби$gло|л|ла шестерку $n2 из Колоды.{x", 
            killer, XMLAttributeCards::suitFaces[suit].mlt, 0, TO_CHAR );

    if (card->getLevel( ) < 0) {
        card->setSuit( suit );
        card->setLevel( 0 );
        oldact("{cТеперь ТЫ займешь её место.{x", killer, 0, 0, TO_CHAR );
    }
    
    return false;
}

CardSellerBehavior::CardSellerBehavior( ) 
{
}

void CardSellerBehavior::greet( Character *victim )
{
    if (!victim->is_npc( ))
        oldact("$c1 перетасовывает карты, хитро поглядывая на тебя.", ch, 0, victim, TO_VICT);
}

void CardSellerBehavior::speech( Character *victim, const char *speech ) 
{
    OBJ_INDEX_DATA *pPackIndex;
    Object *pack;
    static RegExp hello( "колод|pack" );
    
    if (victim->is_npc( ))
        return;
        
    if (!hello.match( speech )) 
        return;
    
    if (victim->getPC( )->getQuestPoints() < 50) {
        oldact("$c1 ухмыляется.", ch, 0, 0, TO_ROOM);
        oldact("$c1 произносит '{gУ тебя недостаточно дурной славы (qp), чтобы пользоваться моими картами.{x'", ch, 0, victim, TO_ROOM);
        return;
    }
    
    if (!( pPackIndex = get_obj_index( OBJ_VNUM_CARDPACK ) )) {
        oldact("$c1 произносит '{gИзвини, у меня закончились карты.{x'", ch, 0, 0, TO_ROOM);
        return;
    }
    
    CardPackBehavior::Pointer bhv( NEW );
    pack = create_object( pPackIndex, 0 );
    bhv->setObj( pack );
    pack->behavior.setPointer( *bhv );
    obj_to_char( pack, victim );

    victim->getPC( )->addQuestPoints(-50);

    oldact("$c1 вручает тебе $o4.", ch, pack, victim, TO_VICT);
    oldact("$c1 вручает $C3 $o4.", ch, pack, victim, TO_NOTVICT);
}

int CardSellerBehavior::getOccupation( )
{
    return BasicMobileDestiny::getOccupation( ) | (1 << OCC_PRACTICER);
}

