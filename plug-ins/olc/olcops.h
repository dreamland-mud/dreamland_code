#ifndef __OLCOPS_H__
#define __OLCOPS_H__

#include "jsoncpp/json/json.h"
#include "dlstring.h"

class PCharacter;

/** Who an op batch runs for; it decides what a validator finding means
 *  (olcvalidate.h): immortal = warning, agent = warning or error with strict,
 *  player = error, plus no numeric/power fields and no put, area, reset or
 *  quest writes (phase 5 replaces that with a whitelist). */
enum OlcPolicy {
    OLC_POLICY_IMMORTAL,
    OLC_POLICY_AGENT,
    OLC_POLICY_PLAYER
};

/**
 * Typed, non-interactive OLC operations over the existing editors.
 *
 * An op is a JSON object (strings UTF-8). Kinds and actions:
 *   room | mob | obj, by "vnum":
 *     get     compact JSON in "data" (the area save serializer's fields),
 *             "fields": [...] picks top-level keys; "format": "xml" gives "xml".
 *             A room answers "xml" unless "fields" or "format": "json" is given
 *     create  "vnum": <n> or "next" with "area": <area vnum>; "ref": "name"
 *             lets later ops say "$name" for the new vnum
 *     set     "field" + "value" [+ "lang" en|ru|ua]: a text field (name,
 *             short, long, desc, sound, smell, message, speedwalk, preposition,
 *             title, also as rudesc, uaname...) goes through paste, "clear": true
 *             empties it; any other field runs as "<field> <value>"
 *     cmd     one editor subcommand line in "cmd" (+ "text" for a paste form)
 *     delete  the prototype (mob/obj: tombstone until the next save; room:
 *             refused while anybody or anything is in it, it has resets, an
 *             exit or reset elsewhere leads to it, or a room editor is open on
 *             it or came from it), or one part of it:
 *             "exit": <dir> [+ "unlink": true], "eexit": <keyword>, "ed": <keyword>
 *     put     "xml": a whole prototype as the area file writes it; mob and obj
 *             are replaced or created, a room only created under a free vnum
 *   reset, by room "vnum":
 *     get, add ("mob" + "max", "maxRoom" | "obj" + "to": room | inside (+
 *     "container", "limit" (0 = no limit), "count") | <wearloc>; "after": <n> places it),
 *     delete ("number"), set ("number", "field": flag|rand|tier|wearloc,
 *     "value"), randomize (shuffle exits)
 *   area, by area "vnum": get, set, cmd (names, levels, vnums, flags, authors,
 *     speedwalk, 'reset'...); a new vnum range that leaves out a live
 *     prototype, or a file another area saves into, is refused
 *   quest, by quest "vnum": get, create ("vnum" | "next" + "area"), set, cmd,
 *     delete, step ("step": <n> | "add", "begin"/"end": {type, vnum, trigger},
 *     "info": {en, ru, ua}, "qp", "gold", "exp", "item", "delete": true)
 *   validate  room | mob | obj by "vnum", area by area "vnum" (every prototype
 *     in it): runs the validator chain at the end of the batch, applies nothing,
 *     writes no journal line
 *   continueOnError  optional, keep going when this op fails
 *
 * Editing ops attach a fresh editor state, run one subcommand, commit and
 * detach, exactly as the /redit, /medit, /oedit servlets do. Every applied op
 * goes to the journal.
 *
 * Returns {ok, dryRun, profile, strict, changed[{kind,vnum}], errors[{op,error}],
 * warnings[], findings[{severity, check, kind, vnum, field?, lang?, message}],
 * findingsSummary{"<severity> <check>": n}, findingsHidden?, results[{op, ok,
 * kind, vnum, changed, data? | xml?, output?}], skipped, saved[]?}. Strings in
 * the result are KOI8, as everywhere inside the engine. Warnings are op-level
 * notes such as quest steps that break the pairing rule.
 *
 * Validators: after the ops, every prototype the batch changed (a reset counts
 * as its room, an area op as the area header) and every one a 'validate' op
 * named goes through the chain in olcvalidate.h. Info findings are listed only
 * with verbose, the list is capped (summary counts all). Any finding the
 * profile makes an error turns ok false and skips save. There is no rollback:
 * the editors apply in place, so the edits stay in memory and the autosave
 * writes them like any other change.
 *
 * dryRun applies nothing: room subcommands edit the prototype in place, so
 * there is no copy to try them on. A dry run parses every op, resolves the
 * vnums (a create reserves its vnum for the rest of the batch) and runs the
 * refusals, then stops. save writes every changed area after a real batch.
 */
class OlcOps {
public:
    static Json::Value apply(PCharacter *actor, const Json::Value &ops,
                             OlcPolicy policy, bool dryRun, bool verbose, bool save,
                             bool strict = false);

    static const char * policyName(OlcPolicy policy);
    static bool policyLookup(const DLString &name, OlcPolicy &policy);

    /** Maps punctuation KOI8-U lacks to ASCII and converts to KOI8. False with
     *  an error when a character has no KOI8-U form: the engine's converter
     *  would silently cut the text there. */
    static bool toKoi(const std::string &utf, DLString &koi, DLString &error);
};

/** Appends one line to the op journal and echoes it to wiznet. All strings KOI8. */
void olc_journal(const DLString &actor, const DLString &profile, const DLString &source,
                 const DLString &kind, int vnum, const DLString &cmd, const DLString &args);

#endif
