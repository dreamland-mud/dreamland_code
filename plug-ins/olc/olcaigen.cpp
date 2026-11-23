/*
 * AI phase 1 (docs/plans/AI_FEATURES.md 4.1): text for the empty fields of a
 * room, mob or object, written by the AI worker (dreamland_tools ai-worker)
 * through /api/olc after the builder leaves the editor.
 *
 * The editors only queue a spool request (airequest.h), at 'done' after the
 * commit and only while AI is on (ai_enabled). The worker builds the context,
 * calls the model and writes only the fields still empty when it lands
 * (/api/olc 'ifEmpty', checked in the same call as the write), as journaled
 * OLC ops. Nothing here waits on it.
 */
#include "olc.h"
#include "olcstate.h"
#include "airequest.h"
#include "commonattributes.h"
#include "pcharacter.h"
#include "room.h"
#include "merc.h"
#include "def.h"

// String attribute: "off" stops the queue at 'done'. A string, because an
// integer attribute does not survive a remort.
static const char *AIGEN_ATTR = "aigen";

static const lang_t langs[] = { LANG_EN, LANG_RU, LANG_UA };
static const char *langNames[] = { "en", "ru", "ua" };

/** Add "field.lang" to empty for every language the field has no text in. */
static void empty_langs(const char *field, const XMLMultiString &value, Json::Value &empty)
{
    for (int i = 0; i < 3; i++) {
        DLString text = value.get(langs[i]);
        text.stripWhiteSpace();
        if (text.empty())
            empty.append(DLString(field) + "." + langNames[i]);
    }
}

static bool material_unset(const DLString &material)
{
    return material.empty() || material == "none" || material == "unknown";
}

/**
 * Fill empty with the empty fields of the prototype, area with its area vnum.
 * False when the prototype does not exist. The core fields are the text a
 * player reads first; extra descriptions and material are listed separately
 * in extras: an old room without extra descriptions is not a reason to call
 * the model at 'done'.
 */
static bool olc_ai_empty_fields(const DLString &entity, int vnum, Json::Value &empty,
                                Json::Value &extras, int &area)
{
    empty = Json::Value(Json::arrayValue);
    extras = Json::Value(Json::arrayValue);

    if (entity == "room") {
        RoomIndexData *pRoom = get_room_index(vnum);
        if (!pRoom)
            return false;
        area = pRoom->areaIndex ? pRoom->areaIndex->vnum : 0;
        empty_langs("name", pRoom->name, empty);
        empty_langs("desc", pRoom->description, empty);
        if (pRoom->extraDescriptions.empty())
            extras.append("ed");
        return true;
    }

    if (entity == "mob") {
        MOB_INDEX_DATA *pMob = get_mob_index(vnum);
        if (!pMob)
            return false;
        area = pMob->area ? pMob->area->vnum : 0;
        empty_langs("keyword", pMob->keyword, empty);
        empty_langs("short", pMob->short_descr, empty);
        empty_langs("long", pMob->long_descr, empty);
        empty_langs("desc", pMob->description, empty);
        return true;
    }

    if (entity == "obj") {
        OBJ_INDEX_DATA *pObj = get_obj_index(vnum);
        if (!pObj)
            return false;
        area = pObj->area ? pObj->area->vnum : 0;
        empty_langs("keyword", pObj->keyword, empty);
        empty_langs("short", pObj->short_descr, empty);
        // The object's 'long' is the description field of the prototype.
        empty_langs("long", pObj->description, empty);
        if (pObj->extraDescriptions.empty())
            extras.append("ed");
        if (material_unset(pObj->material))
            extras.append("material");
        return true;
    }

    return false;
}

static DLString olc_ai_list(const Json::Value &fields)
{
    DLString list;

    for (auto &f: fields) {
        if (!list.empty())
            list += ", ";
        list += f.asString();
    }

    return list;
}

void olc_ai_gen(PCharacter *ch, const DLString &entity, int vnum, bool manual, const DLString &hint)
{
    Json::Value empty, extras;
    int area = 0;

    if (!manual && get_string_attribute(ch, AIGEN_ATTR) == "off")
        return;

    // Failure is silence at 'done': with AI off nothing is queued or said.
    if (!ai_enabled("olc_gen")) {
        if (manual)
            ch->send_to("{CAI:{x the AI is off now ({Wai status{x), nothing was asked.\r\n");
        return;
    }

    if (!olc_ai_empty_fields(entity, vnum, empty, extras, area))
        return;

    // At 'done' the core text decides; extra descriptions and material only
    // ride along with it. 'gen' asks for everything that is empty.
    if (empty.empty() && (!manual || extras.empty())) {
        if (manual)
            ptc(ch, "{CAI:{x nothing is empty on %s %d.\r\n", entity.c_str(), vnum);
        return;
    }

    for (auto &e: extras)
        empty.append(e);

    Json::Value payload;
    payload["entity"] = entity;
    payload["vnum"] = vnum;
    payload["area"] = area;
    payload["builder"] = ch->getName();
    payload["hint"] = hint;
    payload["manual"] = manual;
    payload["empty"] = empty;

    DLString error;
    if (ai_request("olc_gen", payload, error) != AI_REQUEST_OK) {
        if (manual)
            ptc(ch, "{CAI:{x the request could not be queued: %s\r\n", error.c_str());
        return;
    }

    if (manual)
        ptc(ch, "{CAI:{x asked for %s %d: %s.\r\n", entity.c_str(), vnum, olc_ai_list(empty).c_str());
    else
        ptc(ch, "{CAI:{x asked for the empty text of %s %d ({Wgen auto off{x stops this).\r\n",
            entity.c_str(), vnum);
}

void olc_ai_done(OLCState *state, PCharacter *ch, const DLString &entity, int vnum)
{
    bool manual = state->aiAsked;
    DLString hint = state->aiHint;

    state->aiAsked = false;
    state->aiHint.clear();
    olc_ai_gen(ch, entity, vnum, manual, hint);
}

bool olc_ai_gen_command(OLCState *state, PCharacter *ch, const DLString &entity, int vnum, const char *argument)
{
    DLString args = argument;
    args.stripWhiteSpace();
    DLString rest = args;
    DLString word = rest.getOneArgument();

    if (word == "auto") {
        DLString value = rest.getOneArgument();

        if (value == "off") {
            ch->getAttributes().getAttr<XMLStringAttribute>(AIGEN_ATTR)->setValue("off");
            ch->send_to("{CAI:{x no text is asked for at 'done' any more. 'gen' still asks by hand.\r\n");
        }
        else if (value == "on") {
            ch->getAttributes().eraseAttribute(AIGEN_ATTR);
            ch->send_to("{CAI:{x the empty text fields are asked for at 'done'.\r\n");
        }
        else
            ptc(ch, "{CAI:{x at 'done': %s. Usage: {Wgen auto on{x|{Woff{x\r\n",
                get_string_attribute(ch, AIGEN_ATTR) == "off" ? "off" : "on");
        return false;
    }

    if (word == "revert") {
        DLString which = rest.getOneArgument();
        int target = vnum;

        // A length bound first: toInt throws on overflow.
        if (!which.empty()) {
            if (which.size() > 7 || which.find_first_not_of("0123456789") != DLString::npos) {
                ch->send_to("Usage: {Wgen revert{x [vnum]\r\n");
                return false;
            }
            target = which.toInt();
        }

        if (!OLCState::can_edit(ch, target)) {
            ptc(ch, "{CAI:{x you cannot build in %s %d.\r\n", entity.c_str(), target);
            return false;
        }

        Json::Value payload;
        payload["entity"] = entity;
        payload["vnum"] = target;
        payload["builder"] = ch->getName();
        DLString error;
        if (ai_request("olc_revert", payload, error) != AI_REQUEST_OK)
            ptc(ch, "{CAI:{x the request could not be queued: %s\r\n", error.c_str());
        else
            ptc(ch, "{CAI:{x asked to revert the AI text on %s %d that nobody changed since.\r\n",
                entity.c_str(), target);
        return false;
    }

    if (word == "help" || word == "?") {
        ch->send_to("{Wgen{x [hint] - at 'done', ask the AI for the empty text fields of this entry, the hint steers it\r\n"
                    "{Wgen auto on{x|{Woff{x - ask for them by itself at 'done' (on by default)\r\n"
                    "{Wgen revert{x [vnum] - remove the AI text nobody changed since\r\n");
        return false;
    }

    if (!OLCState::can_edit(ch, vnum)) {
        ptc(ch, "{CAI:{x you cannot build in %s %d.\r\n", entity.c_str(), vnum);
        return false;
    }

    // Untrusted text: it goes to the worker as data and into the prompt only.
    if (args.size() > 300)
        args = args.substr(0, 300);

    // Sent at 'done', after the commit: the editor may hold a copy the index
    // does not have yet (a new mob or object exists only after 'done').
    state->aiAsked = true;
    state->aiHint = args;
    ch->send_to("{CAI:{x noted. The request goes out at {Wdone{x, for the fields still empty then.\r\n");
    return false;
}
