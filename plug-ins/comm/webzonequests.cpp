/*
 * The character's own zone quest progress, for a web client's game guide.
 *
 * zonequests_get -> zonequests_data
 *
 * Only a client that asks gets it. Each zone quest the character has a record
 * for comes back with what the area help already says about it (its place in
 * the help's quest list, the limits) and the character's own counts: whether
 * it is under way, how many times it was done this life and in all, and when
 * the once-a-day limit lets it start again. Nothing about the steps: the
 * guide shows done and in progress, not where the player is in the quest.
 * Quests the area help leaves out stay out here too: hidden quests, and
 * quests in areas the help world index does not list.
 *
 * {"command": "zonequests_data", "args": [{"now": <unix time>, "quests": [
 *     {"vnum": 123, "help": 456, "n": 2, "title": "...", "active": false,
 *      "life": 1, "total": 3, "limit": 1, "daily": true, "again": <unix time>}
 * ]}]}
 *
 * help is the id of the area's help article (-1 when it has none, as from
 * area_helpid) and n the quest's place in that article's quest list, counting
 * from 1, so a guide built from the help can find the row. title is the
 * quest's title, or its description for older quests without one, as the
 * help names a quest. limit is the times per life (0: no limit), daily marks
 * a once-a-day quest, again is when the daily limit lets it start again (0:
 * the daily limit does not hold it now; a quest done limit times this life
 * has again 0 too, it is closed for this life). Tries left is limit - life at
 * most: some quests add conditions of their own.
 */
#include <sstream>
#include "jsoncpp/json/json.h"
#include "rpccommandmanager.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "areaquest.h"
#include "areaquestutils.h"
#include "xmlattributeareaquest.h"
#include "areahelp.h"
#include "helpworldindex.h"
#include "webblob.h"
#include "mudtags.h"
#include "player_utils.h"
#include "area.h"
#include "dreamland.h"
#include "merc.h"
#include "def.h"

/** Place of q among the quests its area help lists, counting from 1, or 0. */
static int zonequest_place(AreaQuest *q)
{
    int n = 0;
    for (auto &other: q->pAreaIndex->quests) {
        if (other->flags.isSet(AQUEST_HIDDEN))
            continue;
        n++;
        if (*other == q)
            return n;
    }
    return 0;
}

/** The quest's name as the area help gives it, without colours or link tags. */
static DLString zonequest_title(AreaQuest *q, lang_t lang)
{
    DLString name = q->title.getForLang(lang);
    name.stripWhiteSpace();
    if (name.empty())
        name = q->description.getForLang(lang);

    std::ostringstream buf;
    mudtags_convert(name.c_str(), buf,
                    TAGS_CONVERT_VIS|TAGS_ENFORCE_RAW|TAGS_CONVERT_COLOR|TAGS_ENFORCE_NOCOLOR);
    DLString out = buf.str();
    out.stripWhiteSpace();
    return out;
}

RPCRUN(zonequests_get)
{
    if (!web_blob_caller_ok(ch))
        return;

    PCharacter *pch = ch->getPC();
    lang_t lang = Player::displayLang(ch);
    Json::Value list(Json::arrayValue);

    // Read the records as they are: aquest_data() would add an empty one for
    // every quest it is asked about.
    auto attr = pch->getAttributes().findAttr<XMLAttributeAreaQuest>("areaquest");
    if (attr) {
        for (auto &rec: **attr) {
            AreaQuest *q = get_area_quest(rec.first);
            if (!q || q->flags.isSet(AQUEST_HIDDEN))
                continue;
            if (!q->pAreaIndex || !help_world_area_listed(q->pAreaIndex))
                continue;

            const AreaQuestData &qdata = rec.second;
            if (!qdata.questActive() && qdata.thisLife <= 0 && qdata.total <= 0)
                continue;

            int limit = q->limitPerLife.getValue() > 0 ? q->limitPerLife.getValue() : 0;
            bool closedForLife = limit > 0 && qdata.thisLife >= limit;

            Json::Value row;
            row["vnum"] = q->vnum.getValue();
            row["help"] = area_helpid(q->pAreaIndex);
            row["n"] = zonequest_place(q);
            row["title"] = zonequest_title(q, lang);
            row["active"] = qdata.questActive();
            row["life"] = qdata.thisLife.getValue();
            row["total"] = qdata.total.getValue();
            row["limit"] = limit;
            row["daily"] = q->oncePerDay.getValue();
            row["again"] = (Json::Int64)(closedForLife ? 0 : aquest_daily_open_at(q, qdata));
            list.append(row);
        }
    }

    Json::Value frame;
    frame["command"] = "zonequests_data";
    frame["args"][0]["now"] = (Json::Int64)dreamland->getCurrentTime();
    frame["args"][0]["quests"] = list;
    ch->desc->writeWSCommand(frame);
}
