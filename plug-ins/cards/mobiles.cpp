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
#include "loadsave.h"

#include "def.h"
#include "l10n.h"

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

    oldact(_("{cТы уби$gло|л|ла шестерку $n2 из Колоды.{x"), 
            killer, XMLAttributeCards::getSuitNoun( suit, viewerLang( killer ) ), 0, TO_CHAR );

    if (card->getLevel( ) < 0) {
        card->setSuit( suit );
        card->setLevel( 0 );
        oldact(_("{cТеперь ТЫ займешь её место.{x"), killer, 0, 0, TO_CHAR );
    }
    
    return false;
}

