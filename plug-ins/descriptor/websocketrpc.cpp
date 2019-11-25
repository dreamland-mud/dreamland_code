#include "websocketrpc.h"
#include "pcharacter.h"
#include "descriptor.h"

/**
 * Create command tag in the form of [cmd=...,see=...,nonce=...],
 * which is going to become a clickable action link in webcilent.
 * The client will compare nonce with the one communicated to it in
 * 'version' command, to ensure the link comes from the server and not
 * from random grigoriy in [ooc].
 */
DLString web_cmd(Character *ch, const DLString &cmd, const DLString &seeFmt)
{
    ostringstream buf;

    if (!is_websock(ch))
        return seeFmt;

    buf << "[cmd=" << cmd << ",see=" << seeFmt << ",nonce=" << ch->desc->websock.nonce << "]";
    return buf.str();
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
    return ch && ch->desc && ch->desc->websock.state == WS_ESTABLISHED;
}
