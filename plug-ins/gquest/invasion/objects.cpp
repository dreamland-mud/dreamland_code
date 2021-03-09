/* $Id: objects.cpp,v 1.1.2.1.6.1 2007/09/11 00:34:06 rufina Exp $
 *
 * ruffina, 2004
 */

#include "objects.h"
#include "invasion.h"
#include "invasioninfo.h"

#include "object.h"
#include "npcharacter.h"
#include "pcharacter.h"

#include "handler.h"
#include "act.h"

/*--------------------------------------------------------------------------
 * Invasion Object 
 *-------------------------------------------------------------------------*/
InvasionObj::InvasionObj( ) 
{
}

void InvasionObj::greet( Character *ch ) 
{ 
}

void InvasionObj::actDestroy( Character *ch )
{
}

/*--------------------------------------------------------------------------
 * Invasion Instrument 
 *-------------------------------------------------------------------------*/
InvasionInstrument::InvasionInstrument( ) : charges( 5 )
{
}

void InvasionInstrument::wear( Character *ch ) 
{ 
    act("Ты покрепче сжимаешь %3$O4, готовясь к грядущим подвигам.", ch, obj, 0, TO_CHAR);
    act("%1$^C1 с серьезным видом сжимает %3$C4.", ch, obj, 0, TO_ROOM);
}

bool InvasionInstrument::use( Character *ch, const char *args ) 
{ 
    Object *trgt;
    InvasionGQuest *gquest = InvasionGQuest::getThis( );
    
    if (ch->is_npc( ))
        return false;
    
    if (obj->wear_loc != wear_hold) {
        act("Покрепче зажми %3$O4 в руках - глядишь, поможет..", ch, obj, 0, TO_CHAR);
        return true;
    }
    
    if (!( trgt = get_obj_room( ch, args ) )) {
        ch->pecho("Цель не найдена.");
        act("%1$^C1 угрожающе размахивает %3$C5 - берегись!", ch, obj, 0, TO_ROOM);
        return true;
    }
    
    if (!trgt->behavior || !trgt->behavior.getDynamicPointer<InvasionObj>( )) {
        ch->pecho("То, на что ты замахиваешься, не сделало тебе ничего плохого.");
        return true;
    }

    actUse( ch, trgt );
    trgt->behavior.getDynamicPointer<InvasionObj>( )->actDestroy( ch );
    extract_obj( trgt );

    log("InvasionObj: destroyed by " << ch->getNameP( ));
    gquest->rewardKiller( ch->getPC( ) );
    
    if (--charges <= 0) {
        actDestroy( ch );
        extract_obj( obj );
    }

    return true; 
}

void InvasionInstrument::actDestroy( Character *ch )
{
}

void InvasionInstrument::actUse( Character *ch, Object *trgt )
{
}


