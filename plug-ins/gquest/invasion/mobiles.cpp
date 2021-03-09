/* $Id: mobiles.cpp,v 1.1.2.2 2005/09/16 13:10:10 rufina Exp $
 *
 * ruffina, 2004
 */

#include "mobiles.h"
#include "invasion.h"
#include "invasioninfo.h"
#include "xmlattributeinvasion.h"

#include "regexp.h"

#include "object.h"
#include "pcharacter.h"
#include "npcharacter.h"

#include "act.h"
#include "handler.h"

/*--------------------------------------------------------------------------
 * Invasion Mobile 
 *-------------------------------------------------------------------------*/
InvasionMob::InvasionMob( ) 
{
}

bool InvasionMob::death( Character *killer ) 
{
    InvasionGQuest *gquest = InvasionGQuest::getThis( );
   
    if (!killer) 
        return true;

    killer = gquest->getActor( killer );

    log("InvasionMob: killed by " << killer->getNameP( ));
    
    gquest->rewardKiller( killer->getPC( ) );
    actDeath( killer );
    return true;
}

void InvasionMob::actDeath( Character *killer ) 
{
}

/*--------------------------------------------------------------------------
 * Invasion Hellper 
 *-------------------------------------------------------------------------*/
InvasionHelper::InvasionHelper( ) 
{
}

bool InvasionHelper::death( Character *killer ) 
{
    InvasionGQuest *gq = InvasionGQuest::getThis( );

    if (killer) {
        Character *wch;

        wch = gq->getActor( killer );
        log("InvasionHelper killed by " << wch->getName( ));
        
        if (!wch->is_npc( )) {
            XMLAttributeInvasion::Pointer attr;

            attr = wch->getPC( )->getAttributes( ).getAttr<XMLAttributeInvasion>( gq->getQuestID( ) );
            attr->punish( );
            wch->pecho("{YВ твоих услугах более никто не нуждается.{x");
        }
    } 

    char_to_room( gq->createHelper( ), ch->in_room );
    act("{YБожественные силы возвращают %C4 к жизни!{x", ch, 0, 0, TO_ROOM);
    return true;
}

void InvasionHelper::tell( Character *victim, const char *speech ) 
{
    Object *obj;
    unsigned int size;
    PCharacter *pch;
    static RegExp help( "помогу|помочь|помощь" );
    InvasionGQuest *gquest = InvasionGQuest::getThis( );
    
    if (victim->is_npc( ))
        return;

    pch = victim->getPC( );
    
    act("%2$^C1 что-то говорит %1$C3.", ch, 0, victim, TO_NOTVICT);

    if (!help.match( speech )) {
        actWrongSpeech( pch );
        return;
    }

    obj = gquest->createInstrument( );
    size = gquest->countInstruments( pch );
    
    if (size >= 3) {
        actTooMuch( pch, obj );
        extract_obj( obj );
    }
    else {
        obj_to_char( obj, pch );
        obj->setOwner( pch->getNameP( ) );
        actGiveInstrument( pch, obj );
    }
}

void InvasionHelper::actWrongSpeech( PCharacter *pch )
{
}

void InvasionHelper::actTooMuch( PCharacter *pch, Object *obj )
{    
}

void InvasionHelper::actGiveInstrument( PCharacter *pch, Object *obj )
{
    act("%1$^C1 вручает тебе %3$C4.", ch, obj, pch, TO_VICT);
    oldact("$c1 вручает $C3 $o4.", ch, obj, pch, TO_NOTVICT);
}


