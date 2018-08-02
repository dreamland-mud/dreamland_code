/* $Id$
 *
 * ruffina, 2004
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
#ifndef _ACT_MOVE_H_
#define _ACT_MOVE_H_

#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"

void open_door_extra ( Character *ch, int door, void *pexit );
bool open_portal( Character *, Object * );
void open_door( Character *, int );

#endif
