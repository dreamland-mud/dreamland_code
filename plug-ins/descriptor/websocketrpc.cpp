#include "json/json.h"
#include "websocketrpc.h"
#include "stringlist.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "merc.h"
#include "def.h"

/**
 * Create command tag in the form of [cmd=...,see=...,nonce=...],
 * which is going to become a clickable action link in webcilent.
 * The client will compare nonce with the one communicated to it in
 * 'version' command, to ensure the link comes from the server and not
 * from random grigoriy in [ooc].
 */
DLString web_cmd(Descriptor *d, const DLString &cmd, const DLString &seeFmt)
{
    ostringstream buf;

    if (!is_websock(d))
        return seeFmt;

    buf << "[cmd=" << cmd.colourStrip() << ",see=" << seeFmt.colourStrip() << ",nonce=" << d->websock.nonce << "]";
    return buf.str();
}

/** 
 * Prepares a command tag but without a concrete nonce. The nonce will be
 * inserted later when the target player is known. Can be used to prepare output to many
 * users at once. Example output:
 * {Iw[cmd=shrug,see=shrug your shoulders,nonce=NONCE]{IWshrug your shoulders{Ix
 */
DLString web_cmd_placeholder(const DLString &_cmd, const DLString &_seeFmt, const DLString &placeholder)
{
    ostringstream buf;
    DLString cmd = _cmd.colourStrip();
    DLString seeFmt = _seeFmt.colourStrip();
    
    buf << "{Iw[cmd=" << cmd << ",see=" << seeFmt << ",nonce=" << placeholder << "]"
        << "{IW" << seeFmt << "{Ix";

    return buf.str();
}

DLString web_cmd(Character *ch, const DLString &cmd, const DLString &seeFmt)
{
    return web_cmd(ch->desc, cmd, seeFmt);
}

DLString web_edit_button(Character *ch, const DLString &editor, const DLString &args)
{
    if (!is_websock(ch))    
        return DLString::emptyString;

    if (ch->is_npc() || ch->getPC()->getSecurity() <= 0)
        return DLString::emptyString;

    ostringstream buf;
    buf << "[cmd=" << editor << " " << args << ",see=edit,nonce=" << ch->desc->websock.nonce << "]";
    return buf.str();
}

bool is_websock( Character *ch )
{
    return ch && is_websock(ch->desc);
}

bool is_websock( Descriptor *d )
{
    return d && d->websock.state == WS_ESTABLISHED;
}

DLString web_menu(const StringList &commands, const DLString &id, const DLString &label)
{
    ostringstream buf;

    buf << "{Iw<m";

    if (!id.empty())
        buf << " i='" << id << "'";

    buf << " c='" << commands.join(", ") << "'>{Ix"
        << label
        << "{Iw</m>{Ix";

    return buf.str();
}

WebList::WebList(Character *ch, const DLString &kind, const DLString &title)
    : ch(ch), list(0)
{
    if (!is_websock(ch) || !IS_SET(ch->desc->oob_proto, OOB_LIST))
        return;

    list = new Json::Value;
    (*list)["kind"] = kind;
    (*list)["title"] = title.colourStrip();
    (*list)["rows"] = Json::Value(Json::arrayValue);
}

WebList::~WebList()
{
    delete list;
}

int WebList::row(const DLString &label, const DLString &id, const DLString &section)
{
    if (!list)
        return -1;

    Json::Value r;
    r["label"] = label.colourStrip();
    if (!id.empty())
        r["id"] = id;
    if (!section.empty())
        r["section"] = section.colourStrip();
    r["actions"] = Json::Value(Json::arrayValue);
    (*list)["rows"].append(r);
    return (*list)["rows"].size() - 1;
}

void WebList::action(int row, const DLString &verb, const DLString &cmd)
{
    if (!list || row < 0)
        return;

    Json::Value a;
    a["verb"] = verb;
    a["cmd"] = cmd;
    (*list)["rows"][row]["actions"].append(a);
}

void WebList::data(int row, const DLString &key, int value)
{
    if (list && row >= 0)
        (*list)["rows"][row]["data"][key] = value;
}

void WebList::data(int row, const DLString &key, bool value)
{
    if (list && row >= 0)
        (*list)["rows"][row]["data"][key] = value;
}

void WebList::data(int row, const DLString &key, const DLString &value)
{
    if (list && row >= 0)
        (*list)["rows"][row]["data"][key] = value;
}

void WebList::send()
{
    if (!list)
        return;

    if (ch->desc) {
        Json::Value frame;
        frame["command"] = "list";
        frame["args"][0] = *list;
        ch->desc->writeWSCommand(frame);
    }

    delete list;
    list = 0;
}
