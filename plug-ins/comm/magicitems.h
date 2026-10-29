#ifndef MAGICITEMS_H
#define MAGICITEMS_H

#include "dlstring.h"

class Character;
class Object;

/*
 * Potions, scrolls, wands and staves have no commands of their own:
 * 'use' resolves the item and hands it to one of these. Charged items
 * (props "charged") of any type go to use_charged.
 */
void use_pill( Character *ch, Object *pill );
void use_potion( Character *ch, Object *potion );
void use_scroll( Character *ch, Object *scroll, const DLString &args );
void use_wand( Character *ch, Object *wand, const DLString &args );
void use_staff( Character *ch, Object *staff, const DLString &args );
void use_charged( Character *ch, Object *obj, const DLString &args );

#endif
