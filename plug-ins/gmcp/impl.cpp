/* $Id$
 *
 * ruffina, 2018
 */
#include "gmcpcommand.h"
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
#include "mercdb.h"
#include "so.h"
#include "def.h"

static const char C_IAC = static_cast<char>(IAC);
static const char C_SB = static_cast<char>(SB);
static const char C_GMCP  = static_cast<char>(GMCP);
static const char C_SE = static_cast<char>(SE);
static const char *PROTO_NAME = "GMCP";
static const DLString GUI_VERSION = "6";
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

void GMCPCommand::checkSupport(Descriptor *d, const char *proto) const {

    if (d && !str_cmp(PROTO_NAME, proto))
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

GMCPCOMMAND_RUN(protoInit)
{
    const ProtoInitArgs &myArgs = static_cast<const ProtoInitArgs &>( args );
    checkSupport(myArgs.d, myArgs.proto.c_str());
    if (isSupported(myArgs.d)) {
        send(myArgs.d, "Client", "GUI", GUI_VERSION + "\n" + GUI_URL);

        Json::Value data;
        data["url"] = MAP_URL;
        send(myArgs.d, "Client", "Map", json_to_string(data));
    }
}

GMCPCOMMAND_RUN(charToRoom)
{
    if (!isSupported(args.d))
        return;

    Character *ch = args.d->character;
    if (!ch || !ch->in_room)
        return;

    if (eyes_blinded( ch )) 
        return;

    Json::Value data;
    data["num"] = ch->in_room->vnum;
    data["name"] = DLString(ch->in_room->getName()).colourStrip();
    data["area"] = DLString(ch->in_room->areaName()).colourStrip();
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

