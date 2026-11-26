#ifndef ITEM_PROGS_H
#define ITEM_PROGS_H

class Object;
class Character;

// Item triggers needed from different places in the code.

bool oprog_get( Object *obj, Character *ch );

bool oprog_drop( Object *obj, Character *ch );

// Food or a pill went down; true keeps the object.
bool oprog_eat( Object *obj, Character *ch );

// Asks the item's affects whether it may change hands at all. Runs before the
// item moves, so a veto leaves both characters exactly as they were.
bool oprog_give_blocked( Object *obj, Character *ch, Character *victim );

// Runs the full hand-over chain once the item is already in victim's hands, and
// returns true if anything along it claimed or took back the item.
bool omprog_give( Object *obj, Character *ch, Character *victim );

// A player let go of obj (dropped it, put it in a container): remember who and
// from which host, for obj and everything inside it. The next get compares
// that holder with the taker and hands the pair to the global onHandoff
// trigger (mule detection: drop, quit, pick up on an unregistered alt).
// In memory only: a reboot or plugin reload forgets every stamp.
void oprog_handoff_stamp( Object *obj, Character *ch );

// Forget obj's stamp and its contents': it reached someone by a path that is
// not a get (a thrown missile caught by or stuck in its target).
void oprog_handoff_clear( Object *obj );

// A coin pile merged with coins already in the container: stamped for the
// transfer ledger only (the mule alert skips it), with what ch added.
void oprog_handoff_stamp_merged( Object *money, Character *ch, int gold, int silver );

#endif
