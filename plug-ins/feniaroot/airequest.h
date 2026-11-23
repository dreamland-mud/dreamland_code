#ifndef AIREQUEST_H
#define AIREQUEST_H

#include <jsoncpp/json/json.h>
#include "dlstring.h"

/** What ai_request did with a request. */
enum AiRequestResult {
    AI_REQUEST_OK,        // the file is in the spool
    AI_REQUEST_INVALID,   // bad kind or a document over the size cap: a caller bug
    AI_REQUEST_IO         // the spool could not be written: logged, try later
};

/**
 * Game -> AI worker (dreamland_tools ai-worker, docs/plans/AI_FEATURES.md 3.1):
 * one JSON file {kind, time, payload} per request in var/misc/ai, the messenger
 * spool pattern. The game never waits on the model. Used by the Fenia Root
 * method .aiRequest and by the OLC editors (AI phase 1, olc_gen).
 * kind: lowercase latin letters, digits or _, up to 32 characters. Strings in
 * payload are KOI8; the whole document is written as UTF-8. error names the
 * reason for anything but AI_REQUEST_OK.
 */
AiRequestResult ai_request(const DLString &kind, const Json::Value &payload, DLString &error);

/** True when the AI feature may run now: .tmp.ai.enabled(feature) in Fenia
 *  (kill switch, 'ai on', the feature's flag, a fresh worker heartbeat). False
 *  when Fenia is not up or the helper throws. */
bool ai_enabled(const DLString &feature);

#endif
