/* $Id$
 *
 * ruffina, 2018
 */
#include <fstream>
#include <sstream>

#include "gmcpcommand.h"
#include "webprompt.h"
#include "follow_utils.h"
#include "lang.h"
#include "logstream.h"
#include "json/json.h"
#include "descriptor.h"

#include "bitstring.h"
#include "affect.h"
#include "room.h"
#include "pcharacter.h"
#include "directions.h"
#include "door_utils.h"
#include "stats_apply.h"
#include "loadsave.h"
#include "telnet.h"
#include "merc.h"

#include "so.h"
#include "def.h"

static const char C_IAC = static_cast<char>(IAC);
static const char C_SB = static_cast<char>(SB);
static const char C_GMCP  = static_cast<char>(GMCP);
static const char C_SE = static_cast<char>(SE);
static const char *PROTO_NAME = "GMCP";
// Fallback only: the live version comes from GUI_VERSION_FILE, which a cron job
// fetches from the gh-pages "downloads/version" of dreamland_mudlet.
static const DLString GUI_VERSION = "6";
static const char *GUI_VERSION_FILE = "var/run/mudlet_version";
static const DLString DISCORD_GAME = "DreamLand";
static const DLString DISCORD_INVITE = "https://discord.gg/Ds46EpdKaF";
static const DLString GUI_URL = "https://dreamland-mud.github.io/dreamland_mudlet/downloads/Dreamland.zip";
static const DLString MAP_URL = "https://dreamland-mud.github.io/dreamland_mudlet/downloads/Dreamland-mapping.dat";
static const DLString ASCII_MAPS = "https://dreamland.rocks/maps/";

static string json_to_string( const Json::Value &value )
{
    Json::FastWriter writer;
    return writer.write( value );
}    

bool GMCPCommand::isSupported(Descriptor *d) const 
{
    return d && IS_SET(d->oob_proto, OOB_GMCP);
}

void GMCPCommand::checkSupport(Descriptor *d, const string &proto) const {

    if (d && proto == PROTO_NAME)
        SET_BIT(d->oob_proto, OOB_GMCP);
}

void GMCPCommand::send(Descriptor *d, const string &package, const string &message, const string &data)
{
    ostringstream buf;
    DLString translatedData = d->buffer_handler->convert(data.c_str());

    buf << C_IAC << C_SB << C_GMCP
        << package << "." << message << " " << translatedData
        << C_IAC << C_SE;

    string str = buf.str();
//    LogStream::sendNotice() << "Sending GMCP data " << str << endl;
    d->writeRaw((const unsigned char *)str.c_str(), str.size());
}

/* Mudlet re-downloads the package whenever this string changes. */
static DLString gui_version( )
{
    std::ifstream in( GUI_VERSION_FILE );
    std::string line;

    if (in && std::getline( in, line )) {
        DLString version( line );
        version.stripWhiteSpace( );
        if (!version.empty( ))
            return version;
    }

    return GUI_VERSION;
}

GMCPCOMMAND_RUN(protoInit)
{
    const ProtoInitArgs &myArgs = static_cast<const ProtoInitArgs &>( args );
    checkSupport(myArgs.d, myArgs.proto.c_str());
    if (isSupported(myArgs.d)) {
        send(myArgs.d, "Client", "GUI", gui_version( ) + "\n" + GUI_URL);

        Json::Value data;
        data["url"] = MAP_URL;
        send(myArgs.d, "Client", "Map", json_to_string(data));

        // No applicationid: Mudlet shows the game under its own Discord application.
        Json::Value discord;
        discord["inviteurl"] = DISCORD_INVITE;
        send(myArgs.d, "External.Discord", "Info", json_to_string(discord));
    }
}

/*
 * Discord Rich Presence, as Mudlet reads it from External.Discord.Status.
 * Level, class, zone, combat and group -- never the character name: the status is
 * shown to the player's Discord friends, who may not know the character.
 */
static Json::Value discord_status( PCharacter *ch )
{
    Json::Value status;
    DLString area;

    if (ch->in_room)
        area = ch->in_room->areaName( LANG_EN ).colourStrip( );

    status["game"] = DISCORD_GAME;
    ostringstream details;
    details << "Level " << ch->getRealLevel( ) << " " << ch->getProfession( )->getName( );
    status["details"] = details.str( );

    if (ch->fighting)
        status["state"] = "Fighting in " + area;
    else
        status["state"] = area;

    status["starttime"] = Json::Int64( ch->age.getLogon( ).getTime( ) );

    int party = 0;
    for (Descriptor *d = descriptor_list; d; d = d->next)
        if (d->connected == CON_PLAYING && d->character && is_same_group( d->character, ch ))
            party++;

    if (party > 1)
        status["partysize"] = party;

    return status;
}

/*
 * The structured prompt the web client gets, for Mudlet and other GMCP clients:
 * Char.Vitals with the conventional keys, so a bare client can draw gauges,
 * Dreamland.Prompt with everything else, and the Discord status when it changes.
 */
GMCPCOMMAND_RUN(prompt)
{
    if (!isSupported(args.d))
        return;

    const PromptArgs &myArgs = static_cast<const PromptArgs &>( args );
    const Json::Value &prompt = myArgs.prompt;

    Json::Value vitals;
    vitals["hp"] = prompt["hit"];
    vitals["maxhp"] = prompt["max_hit"];
    vitals["mp"] = prompt["mana"];
    vitals["maxmp"] = prompt["max_mana"];
    vitals["mv"] = prompt["move"];
    vitals["maxmv"] = prompt["max_move"];
    send(args.d, "Char", "Vitals", json_to_string(vitals));

    send(args.d, "Dreamland", "Prompt", json_to_string(prompt));

    Character *ch = args.d->character;
    if (!ch || !ch->getPC( ))
        return;

    // Same delta store as the web prompt fields: cleared on entering the game,
    // so the first prompt after login always carries the status.
    WebPromptAttribute::Pointer attr = ch->getPC( )->getAttributes( ).getAttr<WebPromptAttribute>( "webprompt" );
    Json::Value changed;
    attr->updateIfNew( "discord", discord_status( ch->getPC( ) ), changed );

    if (changed.isMember( "discord" ))
        send(args.d, "External.Discord", "Status", json_to_string(changed["discord"]));
}

/*
 * One chat message, as Comm.Channel.Text: channel, talker and text are the
 * conventional keys; the rest of the web chat frame rides along (see chatframe.h).
 */
GMCPCOMMAND_RUN(chat)
{
    if (!isSupported(args.d))
        return;

    const ChatArgs &myArgs = static_cast<const ChatArgs &>( args );
    Json::Value data = myArgs.body;

    data["channel"] = data["id"];
    if (data.isMember( "peer" ) && data["peer"].isMember( "name" ))
        data["talker"] = data["peer"]["name"];

    send(args.d, "Comm.Channel", "Text", json_to_string(data));
}

GMCPCOMMAND_RUN(charToRoom)
{
    if (!isSupported(args.d))
        return;

    Character *ch = args.d->character;
    if (!ch || !ch->in_room)
        return;

    Json::Value data;
    data["num"] = ch->in_room->vnum;
    data["name"] = DLString(ch->in_room->getName()).colourStrip();
    data["area"] = ch->in_room->areaName().colourStrip();
    DLString areaName = ch->in_room->areaIndex()->area_file->file_name;
    data["map"] = ASCII_MAPS + areaName.replaces(".are", ".html");
    
    for (int door = 0; door < DIR_SOMEWHERE; door++) {
        Room *room = direction_target(ch->in_room, door);
        if (room && ch->can_see(room)) 
            data["exits"][DLString(dir_name_small[door])] = room->vnum;
    }

    send(args.d, "Room", "Info", json_to_string(data));
}

/*-------------------------------------------------------------------------
 * initialize_gmcp
 *------------------------------------------------------------------------*/
extern "C"
{
    SO::PluginList initialize_gmcp( )
    {
        SO::PluginList ppl;
        return ppl;
    }
}

