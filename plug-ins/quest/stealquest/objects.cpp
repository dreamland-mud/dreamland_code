/* $Id: objects.cpp,v 1.1.2.9.6.3 2010/01/01 15:48:15 rufina Exp $
 *
 * ruffina, 2004
 */

#include "objects.h"
#include "stealquest.h"
#include "mobiles.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "act.h"

/* 
 * HiddenChest 
 */
bool HiddenChest::canLock( Character *ch ) 
{ 
    StealQuest::Pointer quest;

    if (!ourHero( ch ))
        return false;
    
    if (!( quest = getMyQuest<StealQuest>( ch->getPC( ) ) ))
        return false;

    quest->wiznet( "", "%s tries to unlock the chest", ch->getNameP( '1' ).c_str( ) );
    return quest->getItemList<LockPick>( ch->carrying );
}



/* 
 * LockPick 
 */
void LockPick::getByHero( PCharacter *ch ) 
{
    QuestPointer quest = getQuest(ch);
    if (!quest)
        return;

    quest->wiznet( "", "%s gets key", ch->getNameP( '1' ).c_str( ) );
    ch->pecho("%1$^O1 тускло поблескива%1$nет|ют.", obj );
}

void LockPick::getByOther( Character *ch ) 
{ 
    ch->pecho("Ты роняешь %1$O4.", obj);    
    ch->recho("%1$^C1 роня%1$nет|ют %2$O4.", ch, obj);
}

bool LockPick::ourMobile( NPCharacter *mob ) 
{
    return ObjQuestBehavior::ourMobile( mob )
           && mob->behavior.getDynamicPointer<Robber>( );
}


/* 
 * RobbedItem 
 */        
void RobbedItem::getByHero( PCharacter *ch ) 
{
    QuestPointer quest = getQuest(ch);
    if (!quest)
        return;

    quest->wiznet( "", "%s gets item", ch->getNameP( '1' ).c_str( ) );
    ch->pecho("%1$^O1 жажд%1$nет|ут вернуться к хозяину.", obj);
}

void RobbedItem::getByOther( Character *ch ) 
{
    ch->pecho("%1$^O1 выпада%1$nет|ют у тебя из рук.", obj);
    ch->recho("%1$^C1 роня%1$nет|ют %2$O4.", ch, obj);
}

