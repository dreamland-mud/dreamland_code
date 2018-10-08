/* $Id: objquestbehavior.cpp,v 1.1.4.2.10.1 2007/09/29 19:33:59 rufina Exp $
 *
 * ruffina, 2004
 */

#include "objquestbehavior.h"
#include "quest.h"
#include "loadsave.h"

/*--------------------------------------------------------------------
 * MandatoryItem
 *--------------------------------------------------------------------*/
bool MandatoryItem::extract( bool count )
{
    if (count)
        mandatoryExtract( );
        
    return ObjQuestBehavior::extract( count );
}

/*--------------------------------------------------------------------
 * PersonalItem 
 *--------------------------------------------------------------------*/
void PersonalItem::get( Character *ch ) 
{
    if (ch->is_immortal( ))
        return;

    if (ourHero( ch )) 
        getByHero( ch->getPC( ) );
    else if (!ourMobile( ch->getNPC( ) )) {
        obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
        getByOther( ch );
    }
}

