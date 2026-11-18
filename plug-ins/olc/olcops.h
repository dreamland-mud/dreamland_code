#ifndef __OLCOPS_H__
#define __OLCOPS_H__

#include "jsoncpp/json/json.h"
#include "dlstring.h"

class PCharacter;

/** Who an op batch runs for. Phase 0 only names the profiles: validators
 *  (phase 2) decide what a finding means for each of them. */
enum OlcPolicy {
    OLC_POLICY_IMMORTAL,
    OLC_POLICY_AGENT,
    OLC_POLICY_PLAYER
};

/**
 * Typed, non-interactive OLC operations over the existing editors.
 *
 * An op is a JSON object (strings UTF-8):
 *   kind    "room" | "mob" | "obj"
 *   action  "get"  room XML (rooms only)
 *           "cmd"  one editor subcommand line in "cmd", e.g. "sector forest"
 *           "set"  "field" + "value" [+ "lang" en|ru|ua]: a text field
 *                  (name, short, long, desc, sound, smell) goes through paste,
 *                  any other field runs as "<field> <value>"
 *   vnum    prototype vnum
 *   text    optional editor buffer for a "cmd" paste subcommand
 *   continueOnError  optional, keep going when this op fails
 *
 * Each op attaches a fresh editor state, runs one subcommand, commits and
 * detaches, exactly as the /redit, /medit, /oedit servlets do.
 *
 * Returns {ok, dryRun, changed[{kind,vnum}], errors[{op,error}], warnings[],
 * results[{op, ok, changed, xml?, output?}], skipped}. Strings in the result
 * are KOI8, as everywhere inside the engine.
 *
 * dryRun applies nothing: room subcommands edit the prototype in place, so
 * there is no copy to try them on. A dry run parses every op, resolves the
 * vnum and the subcommand and runs the refusals, then stops.
 */
class OlcOps {
public:
    static Json::Value apply(PCharacter *actor, const Json::Value &ops,
                             OlcPolicy policy, bool dryRun, bool verbose);

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
