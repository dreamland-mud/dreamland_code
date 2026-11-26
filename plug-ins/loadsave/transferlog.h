/* Dream Land, transfer ledger, 2026.
 *
 * See docs/plans/MULTING_TOOLS.md, section 4 and phase P2.
 */
#ifndef TRANSFERLOG_H
#define TRANSFERLOG_H

#include <jsoncpp/json/json.h>
#include "dlstring.h"

class Character;
class Object;

/**
 * Player-to-player transfer ledger: one JSON line per hand-over in
 * var/log/transfer.jsonl, written through AccountAudit::append.
 *
 * Every transfer between two players is recorded, linked or not: a relay
 * through a third player is invisible if only linked pairs are logged. A row is
 * never identity evidence on its own, the alts card reads it next to the link.
 * No addresses, no device ids.
 *
 * Row: {ts, event: kind, from, to, from_acct, to_acct, room, <item>, <extra>}.
 * Item fields: vnum, oid (object id, follows one item through a relay), lvl,
 * cost, limit (only for a limited prototype), gold and silver for a coin pile,
 * inside (how many objects it holds) and inside_limited (their limited vnums).
 * Engine fields are set after `extra`, so a caller can add fields but never
 * rewrite who, where or what.
 *
 * Best-effort like the account trail: a failure is logged, never thrown.
 */
namespace TransferLog {
    // Both sides by name ("" for a side that is not a player, such as a clan
    // treasury). room is a vnum, 0 when unknown. obj may be null.
    void record(const DLString &kind, const DLString &from, const DLString &to,
                int room, Object *obj, const Json::Value &extra);

    // Both sides online. Writes nothing unless both are players. The room is
    // the receiver's.
    void record(const DLString &kind, Character *from, Character *to,
                Object *obj, const Json::Value &extra = Json::Value());
}

#endif
