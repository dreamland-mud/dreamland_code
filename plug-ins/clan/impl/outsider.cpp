/* $Id: outsider.cpp,v 1.1.6.1.10.3 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2005
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/

#include "outsider.h"

#include "pcharacter.h"
#include "npcharacter.h"

#include "handler.h"
#include "act.h"
#include "def.h"


/*--------------------------------------------------------------------------
 * Outsider's ClanGuard 
 *-------------------------------------------------------------------------*/
void ClanGuardOutsider::actGreet( PCharacter *wch )
{
    do_say(ch, "Приветствую тебя, идущий собственной дорогой!");
}
void ClanGuardOutsider::actPush( PCharacter *wch )
{
    act( "$C1 холодным взглядом смотрит на тебя...\n\rТы пятишься, оступаешься на краю пропасти и с криком летишь вниз...", wch, 0, ch, TO_CHAR );
    act( "$C1 смотрит на $c4 и $c1, не выдержав этого взгляда, оступается и падает в пропасть.", wch, 0, ch, TO_ROOM );
}
void ClanGuardOutsider::actIntruder( PCharacter *wch )
{
}

