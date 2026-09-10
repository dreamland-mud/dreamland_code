/* Dream Land, passwordless account layer, 2026.
 *
 * See ACCOUNTS_NANNY_ROADMAP.md / Trello 2zFpQBoW.
 */
#ifndef ACCOUNTAUDIT_H
#define ACCOUNTAUDIT_H

#include <jsoncpp/json/json.h>
#include "dlstring.h"

/**
 * Append-only account audit trail.
 *
 * One JSON line per event in var/log/account.jsonl. Never rotated by us -- the
 * per-boot logs mix everything and roll away, but an abuse investigation and the
 * human recovery backstop need one greppable file that outlives a reboot.
 *
 * 🛑 A password (temporary or otherwise) is NEVER passed in and must never be.
 *
 * Best-effort and side-effect-only: a write failure is logged and swallowed, it
 * never throws back into the caller. A login path or a servlet must not die
 * because the audit file is unwritable.
 */
namespace AccountAudit {
    // Append {ts, event, <fields>}. `fields` is merged in when it is an object.
    void record(const DLString &event, const Json::Value &fields);
    void record(const DLString &event);   // event with no extra fields
}

#endif
