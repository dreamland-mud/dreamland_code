#include <list>
#include <jsoncpp/json/json.h>

#include "mssp.h"
#include "configurable.h"
#include "descriptor.h"
#include "character.h"
#include "dreamland.h"
#include "telnet.h"

using std::list;
using std::make_pair;
using std::pair;
using std::string;

static Json::Value msspConfig;

CONFIGURABLE_LOADED(config, mssp)
{
    msspConfig = value;
}

/** Players in the game, without invisible immortals. */
static int mssp_players()
{
    int count = 0;

    for (Descriptor *d = descriptor_list; d; d = d->next)
        if (d->connected == CON_PLAYING && d->character && d->character->invis_level <= 0)
            count++;

    return count;
}

/** Variable/value pairs in send order. A JSON array value becomes several values of one variable. */
static list<pair<DLString, DLString> > mssp_fields()
{
    list<pair<DLString, DLString> > fields;

    fields.push_back(make_pair(DLString("NAME"), msspConfig.get("NAME", "DreamLand").asString()));
    fields.push_back(make_pair(DLString("PLAYERS"), DLString(mssp_players())));
    fields.push_back(make_pair(DLString("UPTIME"), DLString((long)dreamland->getBootTime())));

    for (auto &key: msspConfig.getMemberNames()) {
        if (key == "NAME")
            continue;

        const Json::Value &v = msspConfig[key];
        if (v.isArray()) {
            for (auto &item: v)
                fields.push_back(make_pair(DLString(key), DLString(item.asString())));
        } else {
            fields.push_back(make_pair(DLString(key), DLString(v.asString())));
        }
    }

    return fields;
}

/** Drop bytes that would break the telnet framing: IAC, MSSP_VAR, MSSP_VAL, NUL. */
static string mssp_clean(const DLString &s)
{
    string out;

    for (unsigned char c: s)
        if (c != IAC && c != MSSP_VAR && c != MSSP_VAL && c != 0 && c != '\r' && c != '\n')
            out += c;

    return out;
}

void mssp_send_telnet(Descriptor *d)
{
    string buf;

    buf += (char)IAC;
    buf += (char)SB;
    buf += (char)TELOPT_MSSP;

    for (auto &f: mssp_fields()) {
        buf += (char)MSSP_VAR;
        buf += mssp_clean(f.first);
        buf += (char)MSSP_VAL;
        buf += mssp_clean(f.second);
    }

    buf += (char)IAC;
    buf += (char)SE;

    d->writeRaw((const unsigned char *)buf.data(), buf.size());
}

void mssp_send_plain(Descriptor *d)
{
    string buf = "\r\nMSSP-REPLY-START\r\n";

    for (auto &f: mssp_fields())
        buf += mssp_clean(f.first) + "\t" + mssp_clean(f.second) + "\r\n";

    buf += "MSSP-REPLY-END\r\n";

    d->writeRaw((const unsigned char *)buf.data(), buf.size());
}

bool mssp_is_request(const char *arg)
{
    DLString line(arg);
    line.stripWhiteSpace();
    return line == "MSSP-REQUEST";
}
