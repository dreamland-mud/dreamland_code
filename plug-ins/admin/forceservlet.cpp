#include "servlet.h"
#include "servlet_utils.h"
#include "defaultbufferhandler.h"
#include "iconvmap.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "descriptor.h"
#include "interp.h"
#include <jsoncpp/json/json.h>

static IconvMap koi2utf("koi8-u", "utf-8");

/** Find the DefaultBufferHandler attached to a player's descriptor, sending error response on failure. */
static DefaultBufferHandler * servlet_find_buffer(PCharacter *player, HttpResponse &response)
{
    DefaultBufferHandler *buf = 0;
    if (player->desc && player->desc->buffer_handler)
        buf = dynamic_cast<DefaultBufferHandler *>(player->desc->buffer_handler.getPointer());

    if (!buf) {
        servlet_response_404(response, "Player has no output buffer");
        return 0;
    }

    return buf;
}

/** Look up an online player by name from servlet params, sending error response on failure. */
static PCharacter * servlet_find_online_player(const Json::Value &params, HttpResponse &response)
{
    DLString playerName;
    if (!servlet_get_arg(params, response, "player", playerName))
        return 0;

    PCharacter *player = PCharacterManager::findPlayer(playerName);
    if (!player) {
        servlet_response_404(response, "Player " + playerName + " is not online");
        return 0;
    }

    return player;
}

/** Build JSON response with output entries having seq >= startSeq, plus current_seq. */
static void servlet_output_response(HttpResponse &response, DefaultBufferHandler *buf, long long startSeq)
{
    Json::Value body;
    Json::Value outputArr(Json::arrayValue);

    for (const auto &entry : buf->outputLog) {
        if (entry.seq.getValue() >= startSeq) {
            Json::Value item;
            item["seq"] = (Json::Int64)entry.seq.getValue();
            item["text"] = entry.text.getValue();
            outputArr.append(item);
        }
    }

    body["output"] = outputArr;
    body["current_seq"] = (Json::Int64)buf->getCurrentSeq();

    servlet_response_200_json(response, body);

}

/**
 * Servlet for /api/force endpoint.
 * Executes a command on behalf of an online player and returns
 * any immediate output captured by the buffer handler.
 *
 * Auth: bottype=telegram|discord, token=<secret>
 * Args: player (character name), cmd (command string)
 *
 * Example:
 *   curl -s 'http://localhost:1235/api/force' \
 *     -d '{"token":"<secret>","bottype":"telegram","args":{"player":"Ruffina","cmd":"look prophet"}}'
 *   Response (200):
 *     {"output":[{"seq":5001,"text":"Ты видишь ..."}],"current_seq":5001}
 */
SERVLET_HANDLE(api_force, "/force")
{
    Json::Value params;

    if (!servlet_parse_params(request, response, params))
        return;

    if (!servlet_auth_bot(params, response))
        return;

    PCharacter *player = servlet_find_online_player(params, response);
    if (!player)
        return;

    DLString cmd;
    if (!servlet_get_arg(params, response, "cmd", cmd))
        return;

    DefaultBufferHandler *buf = servlet_find_buffer(player, response);
    if (!buf)
        return;

    long long seqBefore = buf->getCurrentSeq();

    interpret(player, cmd.c_str());

    servlet_output_response(response, buf, seqBefore + 1);
}

/**
 * Servlet for /api/snoop endpoint.
 * Returns captured output for an online player since a given sequence number.
 * Useful for observing delayed or asynchronous output (combat, rituals, traps).
 *
 * Auth: bottype=telegram|discord, token=<secret>
 * Args: player (character name), start_seq (sequence number)
 *
 * Example:
 *   curl -s 'http://localhost:1235/api/snoop' \
 *     -d '{"token":"<secret>","bottype":"telegram","args":{"player":"Ruffina","start_seq":"5000"}}'
 *   Response (200):
 *     {"output":[{"seq":5001,"text":"..."},{"seq":5002,"text":"..."}],"current_seq":5003}
 */
SERVLET_HANDLE(api_snoop, "/snoop")
{
    Json::Value params;

    if (!servlet_parse_params(request, response, params))
        return;

    if (!servlet_auth_bot(params, response))
        return;

    PCharacter *player = servlet_find_online_player(params, response);
    if (!player)
        return;

    DLString startSeqStr;
    if (!servlet_get_arg(params, response, "start_seq", startSeqStr))
        return;

    if (!startSeqStr.isNumber()) {
        servlet_response_400(response, "Parameter start_seq must be a number");
        return;
    }

    DefaultBufferHandler *buf = servlet_find_buffer(player, response);
    if (!buf)
        return;

    servlet_output_response(response, buf, startSeqStr.toLongLong());
}
