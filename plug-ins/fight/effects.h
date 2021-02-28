/* $Id: effects.h,v 1.1.2.2 2008/04/14 20:12:35 rufina Exp $
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
#ifndef EFFECTS_H
#define EFFECTS_H

#include "bitstring.h"

typedef void effect_fun_t(void *vo, short level, int dam, int target, bitstring_t dam_flag );

void    acid_effect     (void *vo, short level, int dam, int target, bitstring_t dam_flag = 0 );
void    cold_effect     (void *vo, short level, int dam, int target, bitstring_t dam_flag = 0 );
void    fire_effect     (void *vo, short level, int dam, int target, bitstring_t dam_flag = 0 );
void    poison_effect   (void *vo, short level, int dam, int target, bitstring_t dam_flag = 0 );
void    shock_effect    (void *vo, short level, int dam, int target, bitstring_t dam_flag = 0 );
void    sand_effect     (void *vo, short level, int dam, int target, bitstring_t dam_flag = 0 );
void    scream_effect   (void *vo, short level, int dam, int target, bitstring_t dam_flag = 0 );

#endif
