#include "json/json.h"
#include "logstream.h"
#include "grammar_entities_impl.h"
#include "string_utils.h"

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

#include "def.h"

static IconvMap koi2utf("koi8-u", "utf-8");

/** Strip mud-tags from the string. Escape characters that are part of Markdown syntax. */
static DLString telegram_string(const DLString &source)
{
    ostringstream outBuf;

    mudtags_convert(source.c_str(), outBuf, TAGS_CONVERT_VIS);
    DLString dest = outBuf.str();
    dest.colourstrip();
    
    dest.replaces("_", "\\_");
    dest.replaces("*", "\\*");
    dest.replaces("`", "\\`");
    dest.replaces("[", "\\[");

    return dest;
}

/** Save one file with Telegram JSON markup to disk. Assume all strings are already escaped and converted to UTF-8. */
static void send_to_telegram(const DLString &content)
{
    try {
        Json::Value body;
        body["chat_id"] = "@dreamland_rocks";
        body["parse_mode"] = "Markdown";
        body["text"] = content;
        
        Json::FastWriter writer;
        DLDirectory dir( dreamland->getMiscDir( ), "telegram" );
        DLFileStream( dir.tempEntry( ) ).fromString( 
            writer.write(body)
        );

    } catch (const Exception &e) {
        LogStream::sendError() << "Send to telegram: " << e.what() << endl;
    }
}

/** Paste news or note to Telegram. */
void send_telegram_note(const DLString &thread, const DLString &author, const DLString &title, const DLString &text)
{
    const int max_content_size = 4096; // Message limit imposed by Telegram API.
    const int text_chunk_size = 2000; // How to split the text block (in koi8-u) if message exceeds the max.

    ostringstream header_koi;
    header_koi  
        << "*" << telegram_string(thread).upperFirstCharacter() << "* " << endl
        << "*Автор*: " << telegram_string(author) << endl
        << "*Тема*: " << telegram_string(title);

    DLString header_utf = koi2utf(header_koi.str());
    DLString text_koi = telegram_string(text);
    DLString text_utf = koi2utf(text_koi);
    DLString content_utf = header_utf + "\n\n" + text_utf;

    // Message will go through as a whole, just send it.    
    if (content_utf.size() <= max_content_size) {
        send_to_telegram(content_utf);
        return;
    }

    // Split the text into 2k chunks, add "[part 1]" remark to the header.
    // Standard string operations don't work well with utf strings, so will split koi8 original instead.

    size_t current_size = 0;
    int part_num = 1;
    size_t last_pos = text_koi.size() - 1;
    
    while (current_size < last_pos) {
        size_t part_begins = current_size;
        size_t part_could_end = part_begins + text_chunk_size;
        // Unless it's the last part, don't cut mid-word, but look for the nearest space.
        size_t part_ends = part_could_end >= last_pos ? part_could_end : text_koi.find_last_of(' ', part_could_end);
        DLString part_koi = text_koi.substr(part_begins, part_ends - part_begins);
        
        DLString header_ext_koi = " [часть ";
        header_ext_koi << part_num << "]\n\n";
        
        DLString chunk_koi = header_koi.str() + header_ext_koi + part_koi;
        DLString chunk_utf = koi2utf(chunk_koi);
        send_to_telegram(chunk_utf);

        part_num++;
        current_size = part_ends;
    }
}

void send_telegram_level(PCharacter *ch)
{   
    if (ch->getLevel() == LEVEL_MORTAL)
        send_to_telegram(koi2utf( fmt(0, "%1$^C1 достиг%1$Gло||ла уровня героя!", ch) ));
}

void send_telegram_gquest(const DLString &gqName, const DLString &msg)
{
    DLString gqName_koi = telegram_string(gqName);
    DLString msg_koi = telegram_string(msg);
    
    send_to_telegram("*" + koi2utf(gqName_koi) + ":* " + koi2utf(msg_koi));
}

/** Send arbitrary string as Telegram message. Convert to UTF8 and escape. */
void send_telegram(const DLString &content)
{
    DLString escaped = telegram_string(content);
    send_to_telegram(koi2utf(escaped));
}

/** Send arbitrary string as Telegram message. Convert to UTF8, no escaping for special characters. */
void send_telegram_no_escape(const DLString &content)
{
    send_to_telegram(koi2utf(content));
}

/** Prepare string to use inside Discord JSON field. Strip tags and colors and trim to size. */
static DLString discord_string(const DLString &source)
{
    ostringstream outBuf;

    mudtags_convert(source.c_str(), outBuf, TAGS_CONVERT_VIS);
    DLString dest = outBuf.str();
    dest.colourstrip();

    // Discord 'description' field has a limit of 2000 characters (in UTF encoding).
    if (dest.size() > 1000) {
        String::truncate(dest, 995);
        dest << "\n...";
    }

    return koi2utf(dest);
}

/** Save one file with Discord JSON markup to disk. Assume all strings are already trimmed and converted to UTF-8. */
static void send_to_discord(Json::Value &body, const DLString &dirName)
{
    try {
        DLDirectory dir( dreamland->getMiscDir( ), dirName );

        Json::FastWriter writer;
        DLFileStream( dir.tempEntry( ) ).fromString( 
            writer.write(body)
        );

    } catch (const Exception &e) {
        LogStream::sendError() << "Send to discord: " << e.what() << endl;
    }
}

/** Send one message to the main discussion channel. */
static void send_to_discord_chat(Json::Value &body)
{
    send_to_discord(body, "discord");
}

/** Send one message to the stream channel. */
void send_to_discord_stream(const DLString &content)
{
    Json::Value body;
    body["content"] = discord_string(content);
    send_to_discord(body, "discord-stream");
}

/** Send one message to the SCREAM Discord private channel. */
static void send_to_discord_scream(Json::Value &body)
{
    send_to_discord(body, "discord-scream");
}

void send_discord_wiznet(const DLString &content)
{
    Json::Value body;
    body["content"] = discord_string(content);
    send_to_discord_scream(body);
}

/**
 * Send out-of-character channel messages with real usernames.
 */
void send_discord_ooc(Character *ch, const DLString &format, const DLString &msg)
{
    DLString description = fmt(0, format.c_str(), ch, msg.c_str(), 0);
    send_to_discord_stream(":speech_left: `" + description + "`");
}

/** 
 * Send in-char channel message, with speaker name seen from p.o.v. of someone
 * w/o any detects. 
 */
void send_discord_ic(Character *ch, const DLString &format, const DLString &msg)
{
    // Create a pseudo-player, with just enough parameters in order not to crash.
    PCharacter vict;
    vict.in_room = get_room_instance(2);
    vict.config.setBit(CONFIG_RUNAMES);

    DLString description = fmt(&vict, format.c_str(), ch, msg.c_str(), 0);
    send_to_discord_stream(":speech_left: `" + description + "`");
}

void send_discord_note_notify(const DLString &thread, const DLString &from, const DLString &subj)
{
    send_to_discord_stream(":envelope: " + thread.upperFirstCharacter() + " от " + from + " на тему: " + subj);
}

// Removed emoji from here to make other messages stand out more amidst in/out spam
void send_discord_orb(const DLString &msg)
{
    send_to_discord_stream(msg);
}

void send_discord_clan(const DLString &msg)
{
    send_to_discord_stream(":crossed_swords: " + msg);
}

void send_discord_gquest(const DLString &gqName, const DLString &msg)
{
    send_to_discord_stream(":gem: **" + gqName + "** " + msg);
}

// TODO use it for sub-prof
void send_discord_level(PCharacter *ch)
{   
    DLString msg;

    if (ch->getLevel() == LEVEL_MORTAL)
        msg = fmt(0, "%1$^C1 достиг%1$Gло||ла уровня героя!", ch);
    else
        msg = fmt(0, "%1$^C1 достиг%1$Gло||ла следующей ступени мастерства.", ch);

    send_to_discord_stream(":zap: " + msg);
}

void send_discord_bonus(const DLString &msg)
{
    send_to_discord_stream(":calendar_spiral: " + msg);
}

void send_discord_death(const DLString &msg)
{
    send_to_discord_stream(":skull_crossbones: " + msg);
}

static const DLString COLOR_PINK = "14132165";
static const DLString COLOR_CYAN = "2088924";
static const DLString COLOR_GOLDEN = "16640598";
static const DLString COLOR_GREEN = "4485139";
static const DLString COLOR_CRIMSON = "14824462";
static const DLString COLOR_BLUE = "4514034";

/** Send Fenia exception notifications, similar to 'Fenia orb'. */
void send_discord_fenia(const DLString &header, const DLString &exception)
{
    Json::Value body;
    body["username"] = koi2utf("Хрустальный Шар Фенера");
    body["embeds"][0]["title"] = discord_string(header);
    body["embeds"][0]["description"] = discord_string(exception);
    body["embeds"][0]["color"] = COLOR_CRIMSON;

    send_to_discord_scream(body);
}

/** Notify imms about character description changes. */
void send_discord_confirm(PCharacter *ch)
{
    ostringstream d;

    d << ch->getName( ) << ", " << ch->getRussianName().decline('1') << endl
        << "level " << ch->getLevel( ) << ", "
        << sex_table.name( ch->getSex( ) ) << " "
        << ch->getRace( )->getName( ) << " "
        << ch->getProfession( )->getName( )
        << ", clan " << ch->getClan( )->getShortName( )
        << endl
        << ch->getDescription() << endl;

    Json::Value body;
    body["username"] = koi2utf("Всевидящее Око Руфины");
    body["embeds"][0]["title"] = discord_string("Описание изменилось");
    body["embeds"][0]["description"] = discord_string(d.str());
    body["embeds"][0]["color"] = COLOR_GREEN;
    body["embeds"][0]["author"]["name"] = discord_string(ch->getName());

    send_to_discord_scream(body);
}


void send_discord_note(const DLString &thread, const DLString &author, const DLString &title, const DLString &description)
{
    Json::Value body;
    body["username"] = koi2utf(thread.upperFirstCharacter());
    body["embeds"][0]["title"] = discord_string(title);
    body["embeds"][0]["description"] = discord_string(description);
    body["embeds"][0]["color"] = COLOR_GREEN;
    body["embeds"][0]["author"]["name"] = discord_string(author);

    send_to_discord_chat(body);
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

    send_to_discord_chat(body);
}

