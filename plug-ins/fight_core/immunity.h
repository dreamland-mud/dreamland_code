/* $Id$
 *
 * ruffina, 2004
 */
#ifndef IMMUNITY_H
#define IMMUNITY_H

#include "bitstring.h"

class Character;

#define RESIST_NORMAL      0
#define RESIST_VULNERABLE (A)
#define RESIST_RESISTANT  (B)
#define RESIST_IMMUNE     (C)

int  immune_check(Character *ch, int dam_type, bitstring_t dam_flag = 0);
void immune_from_flags( Character *ch, bitstring_t bit, int &res );
int  immune_resolve( int res );

#endif
