/* $Id: flowers.cpp,v 1.1.2.2.6.2 2007/09/11 00:33:58 rufina Exp $
 *
 * ruffina, 2005
 */

#include "flowers.h"

#include "room.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "act_move.h"

/*--------------------------------------------------------------------------
 * ClanGuardFlowers 
 *-------------------------------------------------------------------------*/
void ClanGuardFlowers::actGreet( PCharacter *wch )
{
}
void ClanGuardFlowers::actPush( PCharacter *wch )
{
}

bool ClanGuardFlowers::checkPush( PCharacter *wch ) 
{
    actPush( wch );
    transfer_char( wch, ch, get_random_room( wch ),
                   NULL, NULL, "%1$^C1 внезапно появляется." );
    return true;
}

