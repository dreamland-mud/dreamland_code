#include "json/json.h"
#include "logstream.h"
#include "messengers.h"
#include "iconvmap.h"
#include "dlfileop.h"
#include "dlfilestream.h"
#include "dldirectory.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "dreamland.h"
#include "act.h"
#include "mudtags.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

static IconvMap koi2utf("koi8-r", "utf-8");

void send_telegram(const DLString &content)
{
    try {
        Json::Value body;
        body["chat_id"] = "@dreamland_rocks";
        body["parse_mode"] = "Markdown";
        body["text"] = koi2utf(content.colourStrip());
        
        Json::FastWriter writer;
        DLDirectory dir( dreamland->getMiscDir( ), "telegram" );
        DLFileStream( dir.tempEntry( ) ).fromString( 
            writer.write(body)
        );

    } catch (const Exception &e) {
        LogStream::sendError() << "Send to telegram: " << e.what() << endl;
    }
}

static DLString discord_string(const DLString &source)
{
    ostringstream outBuf;

    vistags_convert(source.c_str(), outBuf, 0);
    DLString colored = outBuf.str();
    DLString utf = koi2utf(colored.colourStrip());

    if (utf.length() > 2000) {
        utf.cutSize(1995);
        utf << "\n...";
    }
    return utf;
}

static void send_discord(Json::Value &body)
{
    try {

        DLDirectory dir( dreamland->getMiscDir( ), "discord" );

        Json::FastWriter writer;
        DLFileStream( dir.tempEntry( ) ).fromString( 
            writer.write(body)
        );

    } catch (const Exception &e) {
        LogStream::sendError() << "Send to discord: " << e.what() << endl;
    }
}

void send_discord(const DLString &content)
{
    Json::Value body;
    body["content"] = discord_string(content);
    send_discord(body);
}

/**
 * Send out-of-character channel messages with real usernames.
 */
void send_discord_ooc(Character *ch, const DLString &format, const DLString &msg)
{
    DLString description = fmt(0, format.c_str(), ch, msg.c_str(), 0);
    send_discord(":speech_left: " + description);
}

/** 
 * Send in-char channel message, with speaker name seen from p.o.v. of someone
 * w/o any detects. 
 */
void send_discord_ic(Character *ch, const DLString &format, const DLString &msg)
{
    // Create a pseudo-player, with just enough parameters in order not to crash.
    PCharacter vict;
    vict.in_room = get_room_index(2);
    vict.config.setBit(CONFIG_RUNAMES);

    DLString description = fmt(&vict, format.c_str(), ch, msg.c_str(), 0);
    send_discord(":speech_left: " + description);
}

void send_discord_note_notify(const DLString &thread, const DLString &from, const DLString &subj)
{
    send_discord(":envelope: " + thread.upperFirstCharacter() + " от " + from + " на тему: " + subj);
}

void send_discord_orb(const DLString &msg)
{
    send_discord(":arrow_right: " + msg);
}

void send_discord_clan(const DLString &msg)
{
    send_discord(":crossed_swords: " + msg);
}

void send_discord_gquest(const DLString &gqName, const DLString &msg)
{
    send_discord(":gem: **" + gqName + "** " + msg);
}

// TODO use it for sub-prof
void send_discord_level(PCharacter *ch)
{   
    DLString msg;

    if (ch->getLevel() == LEVEL_MORTAL)
        msg = fmt(0, "%1$^C1 достиг%1$Gло||ла уровня героя!", ch);
    else
        msg = fmt(0, "%1$^C1 достиг%1$Gло||ла следующей ступени мастерства.", ch);

    send_discord(":zap: " + msg);
}

void send_discord_bonus(const DLString &msg)
{
    send_discord(":calendar_spiral: " + msg);
}

void send_discord_death(PCharacter *ch, Character *killer)
{
    DLString msg;
    if (!killer || killer == ch)
        msg = fmt(0, "%1$C1 погиб%1$Gло||ла своей смертью.", ch);
    else
        msg = fmt(0, "%1$C1 па%1$Gло|л|ла от руки %2$C2.", ch, killer);

    send_discord(":grave: " + msg);
}

static const DLString COLOR_PINK = "14132165";
static const DLString COLOR_CYAN = "2088924";
static const DLString COLOR_GOLDEN = "16640598";
static const DLString COLOR_GREEN = "4485139";
static const DLString COLOR_CRIMSON = "14824462";
static const DLString COLOR_BLUE = "4514034";

void send_discord_note(const DLString &thread, const DLString &author, const DLString &title, const DLString &description)
{
    Json::Value body;
    body["username"] = koi2utf(thread.upperFirstCharacter());
    body["embeds"][0]["title"] = discord_string(title);
    body["embeds"][0]["description"] = discord_string(description);
    body["embeds"][0]["color"] = COLOR_GREEN;
    body["embeds"][0]["author"]["name"] = discord_string(author);

    send_discord(body);
}

void send_discord_news(const DLString &thread, const DLString &author, const DLString &title, const DLString &description)
{
    Json::Value body;
    body["username"] = koi2utf(thread.upperFirstCharacter());
    body["embeds"][0]["title"] = discord_string(title);
    body["embeds"][0]["description"] = discord_string(description);
    body["embeds"][0]["url"] = "https://dreamland.rocks/news.html";
    body["embeds"][0]["color"] = COLOR_BLUE;
    body["embeds"][0]["author"]["name"] = discord_string(author);

    send_discord(body);
}

