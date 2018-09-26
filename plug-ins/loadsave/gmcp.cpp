#include "json/json.h"
#include "logstream.h"
#include "gmcp.h"
#include "telnet.h"
#include "descriptor.h"
#include "character.h"
#include "room.h"
#include "loadsave.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

const char *dir_name[] = {"n","e","s","w","u","d"};

const char C_IAC = static_cast<char>(IAC);
const char C_SB = static_cast<char>(SB);
const char C_GMCP  = static_cast<char>(GMCP);
const char C_SE = static_cast<char>(SE);

void GMCPHandler::sendVersion(Descriptor *d)
{
    if (IS_SET(d->oob_proto, OOB_GMCP)) {
        LogStream::sendNotice() << "telnet: sending GMCP version" << endl;
        send(d, "Client", "GUI", "0.3\nhttps://dreamland.rocks/img/dl.zip");
    }
}


void GMCPHandler::sendRoom(Descriptor *d)
{
    if (!d)
        return;

    if (!IS_SET(d->oob_proto, OOB_GMCP))
        return;

    Character *ch = d->character;
    if (!ch || !ch->in_room)
        return;

    if (eyes_blinded( ch )) 
        return;

    Json::Value data;
    data["num"] = ch->in_room->vnum;
    data["name"] = DLString(ch->in_room->name).colourStrip();
    data["area"] = DLString(ch->in_room->area->name).colourStrip();
    data["map"] = DLString("https://dreamland.rocks/maps/") + ch->in_room->area->area_file->file_name + ".html";
    
    for (int door = 0; door < DIR_SOMEWHERE; door++) {
        EXIT_DATA *pexit;
        Room *room;

        if (!( pexit = ch->in_room->exit[door] ))
            continue;
        if (!( room = pexit->u1.to_room ))
            continue;
        if (!ch->can_see( room ))
            continue;

        data["exits"][DLString(dir_name[door])] = room->vnum;
    }

    Json::FastWriter fast;
    string str = fast.write(data);

    send(d, "Room", "Info", str);
}

void GMCPHandler::send(Descriptor *d, const string &package, const string &message, const string &data)
{
    ostringstream buf;

    buf << C_IAC << C_SB << C_GMCP
        << package << "." << message << " " << data
        << C_IAC << C_SE;

    string str = buf.str();
    LogStream::sendNotice() << "Sending GMCP data " << data << endl;
    d->writeRaw((const unsigned char *)str.c_str(), str.size());
}

