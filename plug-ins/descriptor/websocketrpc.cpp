#include "websocketrpc.h"
#include "stringlist.h"
#include "pcharacter.h"
#include "descriptor.h"

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