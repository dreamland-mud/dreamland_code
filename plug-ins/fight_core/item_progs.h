#ifndef ITEM_PROGS_H
#define ITEM_PROGS_H

class Object;
class Character;

// Item triggers needed from different places in the code.

bool oprog_get( Object *obj, Character *ch );

bool oprog_drop( Object *obj, Character *ch );

// Asks the item's affects whether it may change hands at all. Runs before the
// item moves, so a veto leaves both characters exactly as they were.
bool oprog_give_blocked( Object *obj, Character *ch, Character *victim );

#endif
