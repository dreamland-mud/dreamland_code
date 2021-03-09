/* $Id: flowers.cpp,v 1.1.2.2.6.2 2007/09/11 00:33:58 rufina Exp $
 *
 * ruffina, 2005
 */

#include "flowers.h"

#include "room.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "act_move.h"
#include "handler.h"
#include "interp.h"
#include "act.h"

/*--------------------------------------------------------------------------
 * ClanGuardFlowers 
 *-------------------------------------------------------------------------*/
void ClanGuardFlowers::actGreet( PCharacter *wch )
{
    do_say(ch, "Мир, дружба, жвачка!");    
}
void ClanGuardFlowers::actPush( PCharacter *wch )
{
    act("%2$^C1 вежливо выпроваживает тебя восвояси.", wch, 0, ch, TO_CHAR );
    act("%2$^C1 вежливо выпроваживает %1$C2 восвояси.", wch, 0, ch, TO_ROOM );    
}

bool ClanGuardFlowers::checkPush( PCharacter *wch ) 
{
    actPush( wch );
    transfer_char( wch, ch, get_random_room( wch ),
                   NULL, NULL, "%1$^C1 внезапно появляется." );
    return true;
}
