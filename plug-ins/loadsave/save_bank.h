/* Object bank -- persistent per-owner object storage cells.
 *
 * A bank ENTRY is one on-disk file in the same record format as the
 * dropped-object saves in save_drop.cpp (which this re-keys): a flat list of
 * #O..End blocks with Nest depth preserved. One file = one top-level entry
 * (a bag deposits as its whole subtree in a single file), named by the
 * top-level object's Id, under bank/<kind>/<key>/. Per-entry files make
 * selective withdrawal a read-one-file-and-unlink (the phase-1 whole-file
 * reader, unchanged) instead of a fragile mid-cell splice.
 */
#ifndef SAVE_BANK_H
#define SAVE_BANK_H

#include <vector>

#include "dlstring.h"
#include "xmlmultistring.h"

class Character;
class Object;

/* One browse row: everything the vault listing needs WITHOUT materializing the
 * object, read by a cheap header-peek of the entry file. Any parse slip here is
 * cosmetic (a mislabeled row) -- withdrawal never trusts these fields, it reads
 * the file with the real object loader. */
struct BankEntry {
    long long      id;         // top-level object Id; also the entry filename
    int            vnum;
    XMLMultiString shortDescr;  // instance override; empty -> resolve from prototype
    int            itemType;    // instance override, or -1 (resolve from prototype)
    int            level;       // instance override, or -1 (resolve from prototype)
    int            contents;    // nested sub-record count (bag contents); 0 = none

    BankEntry( ) : id( 0 ), vnum( 0 ), itemType( -1 ), level( -1 ), contents( 0 ) { }
};

/* Strip temporary affects (duration >= 0) from obj and its whole nest subtree,
 * using the engine's own affect_remove_obj path so stat modifiers are reversed.
 * Permanent affects (duration < 0, e.g. enchant weapon at -1) are kept -- they
 * are the item's identity and must survive banking. This closes the exploit of
 * parking a buffed item so its temp affect never decays while frozen. */
void bank_strip_temp_affects( Object *obj );

/* Deposit obj (with its full contained subtree) as ONE file
 * bank/<kind>/<key>/<Id>: refuse if anything in the subtree is NOSAVEDROP (the
 * serializer would silently drop it), else strip temp affects, write the
 * serialized subtree, and extract obj via the nocount (logout) path -- the
 * Fenia wrapper is kept and relinked by Id on withdrawal, and the proto count
 * is left intact so banked limited items can't repop under it. Returns false
 * (no mutation, no extract) on refusal or a write failure. After this returns
 * true obj is out of the world -- callers MUST NOT touch it again. */
bool bank_deposit( Object *obj, const DLString &kind, const DLString &key );

/* List the <kind>/<key> owner's stored entries WITHOUT loading any object: one
 * header-peek per entry file. Result sorted by Id ascending (roughly deposit
 * order) so the displayed [n] numbering is stable between calls. */
void bank_browse( const DLString &kind, const DLString &key, std::vector<BankEntry> &out );

/* Materialize the single entry file bank/<kind>/<key>/<id> into ch's inventory
 * and unlink it. Reuses the phase-1 whole-file reader (login semantics: no
 * create_obj_dropped, so the proto count is not bumped -- the nocount deposit
 * never dropped it). Returns false and keeps the file if it is missing or a
 * mid-file read fails, so a corrupt entry is preserved for inspection rather
 * than destroyed. */
bool bank_withdraw_entry( Character *ch, const DLString &kind, const DLString &key, long long id );

#endif
