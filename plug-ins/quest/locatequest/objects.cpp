/* $Id: objects.cpp,v 1.1.2.7.6.2 2008/03/01 20:37:15 rufina Exp $
 *
 * ruffina, 2004
 */

#include "objects.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "act.h"

void LocateItem::getByHero( PCharacter *ch ) 
{
}

void LocateItem::getByOther( Character *ch ) 
{
    ch->pecho( "%1$O1 не принадлежит тебе, и ты бросаешь %1$P2.", obj );
    ch->recho( "%2$C1 бросает %1$O4.", obj, ch );
}

