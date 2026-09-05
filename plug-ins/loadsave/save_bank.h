/* Object bank -- persistent per-owner object storage cells.
 *
 * Phase 1: cell store primitives. A bank cell is the same on-disk record format
 * as the dropped-object saves in save_drop.cpp (which this re-keys), a flat list
 * of #O..End blocks with Nest depth preserved, keyed by owner instead of room.
 */
#ifndef SAVE_BANK_H
#define SAVE_BANK_H

#include "dlstring.h"

class Character;
class Object;

/* Strip temporary affects (duration >= 0) from obj and its whole nest subtree,
 * using the engine's own affect_remove_obj path so stat modifiers are reversed.
 * Permanent affects (duration < 0, e.g. enchant weapon at -1) are kept -- they
 * are the item's identity and must survive banking. This closes the exploit of
 * parking a buffed item so its temp affect never decays while frozen. */
void bank_strip_temp_affects( Object *obj );

/* Deposit obj (with its full contained subtree) into the <kind>/<key> cell:
 * refuse if anything in the subtree is NOSAVEDROP (the serializer would silently
 * drop it), else strip temp affects, append the serialized record, and extract
 * obj via the nocount (logout) path -- the Fenia wrapper is kept and relinked by
 * Id on withdrawal, and the proto count is left intact so banked limited items
 * can't repop under it. Returns false (no mutation, no extract) on refusal or a
 * write failure. After this returns true obj is out of the world -- callers MUST
 * NOT touch it again. */
bool bank_deposit( Object *obj, const DLString &kind, const DLString &key );

/* Materialize every record in the <kind>/<key> cell into ch's inventory and
 * delete the cell file. Phase-1 whole-cell withdrawal; per-Id selective
 * withdrawal + the cheap manifest come in phase 2. */
void bank_withdraw_all( Character *ch, const DLString &kind, const DLString &key );

#endif
