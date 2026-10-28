#ifndef MAGICITEMS_H
#define MAGICITEMS_H

#include "dlstring.h"

class Character;
class Object;

/*
 * Potions, scrolls, wands and staves have no commands of their own:
 * 'use' resolves the item and hands it to one of these.
 */
void use_potion( Character *ch, Object *potion );
void use_scroll( Character *ch, Object *scroll, const DLString &args );
void use_wand( Character *ch, Object *wand, const DLString &args );
void use_staff( Character *ch, Object *staff );

#endif
