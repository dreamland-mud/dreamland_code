#ifndef ITEM_PROGS_H
#define ITEM_PROGS_H

class Object;
class Character;

// Item triggers needed from different places in the code.

bool oprog_get( Object *obj, Character *ch );

bool oprog_drop( Object *obj, Character *ch );

#endif
