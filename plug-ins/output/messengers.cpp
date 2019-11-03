#include "json/json.h"
#include "logstream.h"
#include "messengers.h"
#include "iconvmap.h"
#include "dlfileop.h"
#include "dlfilestream.h"
#include "dldirectory.h"
#include "pcharacter.h"
#include "dreamland.h"
#include "act.h"
#include "merc.h"

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

void send_discord_gquest(const DLString &gqName, const DLString &msg)
{
    static const DLString USERNAME = "Глобальные квесты";

    Json::Value body;
    body["username"] = koi2utf(USERNAME);
    body["embeds"][0]["description"] = koi2utf(msg.colourStrip());
    body["embeds"][0]["color"] = "16640598";
    body["embeds"][0]["author"]["name"] = koi2utf(gqName);

    send_discord(body);
}

static const DLString ORB_USERNAME = "Хрустальный шар";

void send_discord_level(PCharacter *ch)
{   
    Json::Value body;
    DLString msg;

    if (ch->getLevel() == LEVEL_MORTAL)
        msg = fmt(0, "%1$^C1 достиг%1$Gло||ла уровня героя!", ch);
    else
        msg = fmt(0, "%1$^C1 достиг%1$Gло||ла следующей ступени мастерства.", ch);

    body["username"] = koi2utf(ORB_USERNAME);
    body["embeds"][0]["description"] = koi2utf(msg);
    body["embeds"][0]["color"] = "14132165";

    send_discord(body);
}

void send_discord_bonus(const DLString &msg)
{
    static const DLString USERNAME = "Календарь";

    Json::Value body;
    body["username"] = koi2utf(USERNAME);
    body["embeds"][0]["description"] = koi2utf(msg.colourStrip());
    body["embeds"][0]["color"] = "4485139";

    send_discord(body);
}

void send_discord_death(PCharacter *ch, Character *killer)
{
    DLString msg;
    if (killer)
        msg = fmt(0, "%1$C1 па%1$Gло|л|ла от руки %2$C2.", ch, killer);
    else
        msg = fmt(0, "%1$C1 погиб%1$Gло||ла своей смертью.", ch);

    Json::Value body;
    body["username"] = koi2utf(ORB_USERNAME);
    body["embeds"][0]["description"] = koi2utf(msg);
    body["embeds"][0]["color"] = "14824462";

    send_discord(body);
}

void send_discord_news(const DLString &author, const DLString &title, const DLString &description)
{
    static const DLString USERNAME = "Новости и изменения";

    Json::Value body;
    body["username"] = koi2utf(USERNAME);
    body["embeds"][0]["title"] = koi2utf(title.colourStrip());
    body["embeds"][0]["description"] = koi2utf(description.colourStrip());
    body["embeds"][0]["url"] = "https://dreamland.rocks/news.html";
    body["embeds"][0]["color"] = "4514034";
    body["embeds"][0]["author"]["name"] = koi2utf(author.colourStrip());

    send_discord(body);
}