/* $Id: mobiles.cpp,v 1.1.2.4.6.1 2009/09/17 18:08:56 rufina Exp $
 *
 * ruffina, 2004
 */

#include "mobiles.h"
#include "objects.h"
#include "rainbow.h"
#include "rainbowinfo.h"
#include "scenarios.h"

#include "commonattributes.h"

#include "class.h"
#include "so.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "object.h"

#include "act.h"
#include "act_move.h"
#include "fight.h"
#include "handler.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

RainbowMob::RainbowMob( ) 
{
}

RainbowMob::~RainbowMob( ) 
{
}

void RainbowMob::setChar( NPCharacter *mob ) 
{
    BasicMobileDestiny::setChar( mob );
    SET_BIT(ch->imm_flags, IMM_SUMMON|IMM_CHARM);
}

void RainbowMob::unsetChar( ) 
{
    REMOVE_BIT(ch->imm_flags, IMM_SUMMON|IMM_CHARM);
    BasicMobileDestiny::unsetChar( );
}

bool RainbowMob::specIdle( ) 
{
    Character *wch;
    Object *obj;
    RainbowGQuest *gq = RainbowGQuest::getThis( );

    if (!gq || gq->isHidden( ))
        return false;
    
    for (wch = ch->in_room->people; wch; wch = wch->next_in_room) {
        if (wch->is_npc( )) 
            continue;
        if (wch->getPC( )->getAttributes( ).isAvailable( gq->getQuestID( ) )) 
            continue;
        if (!ch->can_see( wch ))
            continue;
        if (!hasMyNumber( wch ))
            break;
    }
    
    if (!wch)
        return false;
     
    log("RainbowMob: [" << number << "] to " << wch->getName( ));
    gq->getScenario( )->onGivePiece( wch->getPC( ), ch );
    
    obj = gq->createPiece( number ); 
    obj_to_char( obj, wch );
    act("%1$^C1 вручает тебе %3$C4.", ch, obj, wch, TO_VICT);
    oldact("$c1 вручает $C3 $o4.", ch, obj, wch, TO_NOTVICT);
    obj->behavior->get( wch );
    return false;
}

void RainbowMob::greet( Character *mob ) 
{
}

void RainbowMob::entry( ) 
{
}

bool RainbowMob::death( Character *killer ) 
{
// create another, punish the killer    
    Character *wch;
    RainbowGQuest *gq = RainbowGQuest::getThis( );

    if (!gq)
        return false;
    
    if (killer) {
        wch = gq->getActor( killer );
        log("RainbowMob: [" << number << "] killed by " << wch->getName( ));
        
        if (!wch->is_npc( )) {
            wch->getPC( )->getAttributes( ).getAttr<XMLEmptyAttribute>( gq->getQuestID( ) );
            wch->pecho("{RТы не сможешь больше принимать участие в задании, т.к. осквернил%Gо||а свои руки убийством.{x", wch);
        }
    } 

    gq->recreateMob( this );
    act("Божественные силы возвращают %C4 к жизни!", ch, 0, 0, TO_ROOM);
    return false;
}


bool RainbowMob::hasMyNumber( Character *wch )
{
    Object *obj;
    RainbowPiece::Pointer bhv;

    for (obj = wch->carrying; obj; obj = obj->next_content) 
        if (obj->behavior) {
            bhv = obj->behavior.getDynamicPointer<RainbowPiece>( );

            if (bhv && bhv->number == number) 
                return true;
        }
    
    return false;
}

