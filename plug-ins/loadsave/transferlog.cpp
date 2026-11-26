/* Dream Land, transfer ledger, 2026.
 *
 * See docs/plans/MULTING_TOOLS.md, section 4 and phase P2.
 */
#include "transferlog.h"
#include "accountaudit.h"
#include "accountmanager.h"
#include "character.h"
#include "core/object.h"
#include "room.h"
#include "logstream.h"
#include "merc.h"
#include "def.h"

using namespace std;

// What a container holds, nested bags included: how many objects, and the
// vnums of the limited ones (a limited item handed over inside a bag is the
// case that matters).
static void count_contents(Object *obj, int &inside, Json::Value &limited)
{
    for (Object *o = obj->contains; o; o = o->next_content) {
        inside++;
        if (o->pIndexData && o->pIndexData->limit > 0)
            limited.append(o->pIndexData->vnum);
        count_contents(o, inside, limited);
    }
}

// Keys only the engine writes: dropped from a caller's extra fields first.
static const char *engine_keys[] = {
    "from", "to", "from_acct", "to_acct", "room",
    "vnum", "oid", "lvl", "cost", "limit", "inside", "inside_limited",
    0
};

void TransferLog::record(const DLString &kind, const DLString &from, const DLString &to,
                         int room, Object *obj, const Json::Value &extra)
{
    try {
        Json::Value row(Json::objectValue);
        if (extra.isObject())
            row = extra;
        for (int i = 0; engine_keys[i]; i++)
            row.removeMember(engine_keys[i]);

        row["from"] = from;
        row["to"] = to;

        DLString fromAcct = from.empty() ? DLString::emptyString : AccountManager::accountOf(from);
        DLString toAcct = to.empty() ? DLString::emptyString : AccountManager::accountOf(to);
        if (!fromAcct.empty())
            row["from_acct"] = fromAcct;
        if (!toAcct.empty())
            row["to_acct"] = toAcct;

        row["room"] = room;

        if (obj) {
            row["vnum"] = obj->pIndexData ? obj->pIndexData->vnum : 0;
            row["oid"] = Json::Int64(obj->getID());
            row["lvl"] = obj->level;
            row["cost"] = obj->cost;

            if (obj->pIndexData && obj->pIndexData->limit > 0)
                row["limit"] = obj->pIndexData->limit;

            // A coin pile: its own value, whatever the caller said.
            if (obj->item_type == ITEM_MONEY) {
                row["gold"] = obj->value1();
                row["silver"] = obj->value0();
            }

            int inside = 0;
            Json::Value limited(Json::arrayValue);
            count_contents(obj, inside, limited);
            if (inside > 0) {
                row["inside"] = inside;
                row["inside_limited"] = limited;
            }
        }

        AccountAudit::append("transfer.jsonl", kind, row);
    } catch (const std::exception &e) {
        LogStream::sendError() << "TransferLog: " << e.what() << endl;
    }
}

void TransferLog::record(const DLString &kind, Character *from, Character *to,
                         Object *obj, const Json::Value &extra)
{
    if (!from || !to || from == to || from->is_npc() || to->is_npc())
        return;

    int room = 0;
    if (to->in_room)
        room = to->in_room->vnum;
    else if (from->in_room)
        room = from->in_room->vnum;

    record(kind, from->getNameC(), to->getNameC(), room, obj, extra);
}
