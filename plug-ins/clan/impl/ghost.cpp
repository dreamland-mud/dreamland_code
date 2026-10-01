/* $Id: ghost.cpp,v 1.1.6.3.6.5 2010-09-01 21:20:44 rufina Exp $
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

#include "clanmobiles.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "skillreference.h"
#include "act.h"

#include "loadsave.h"
#include "interp.h"
#include "magic.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

GSN(acid_arrow);
GSN(acid_blast);
GSN(blindness);
GSN(dispel_affects);
GSN(energy_drain);
GSN(fireball);
GSN(lightning_breath);
GSN(plague);
GSN(spellbane);
GSN(weaken);

/*--------------------------------------------------------------------------
 * Sharu-Gorul 
 *-------------------------------------------------------------------------*/



