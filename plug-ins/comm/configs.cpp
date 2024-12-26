/* $Id: configs.cpp,v 1.1.2.9.6.8 2009/11/08 17:46:27 rufina Exp $
 *
 * ruffina, 2005
 * command syntax and messages from DreamLand 2.0
 */

#include "configs.h"
#include "jsoncpp/json/json.h"

#include "player_utils.h"
#include "servlet.h"
#include "commandmanager.h"
#include "commandtemplate.h"
#include "commonattributes.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "dreamland.h"
#include "merc.h"
#include "msgformatter.h"
#include "interp.h"
#include "comm.h"
#include "descriptor.h"
#include "servlet_utils.h"
#include "math_utils.h"
#include "act.h"
#include "arg_utils.h"
#include "def.h"

#define MILD(ch)     (IS_SET((ch)->comm, COMM_MILDCOLOR))

#define CLR_HEADER(ch)  (MILD(ch) ? "w" : "W")
#define CLR_NAME(ch)    (MILD(ch) ? "c" : "C")
#define CLR_YES(ch)     (MILD(ch) ? "g" : "R")
#define CLR_NO(ch)      (MILD(ch) ? "G" : "G")

static void config_scroll(PCharacter *ch, const DLString &constArguments);
static void config_scroll_print(PCharacter *ch);
static void config_telegram(PCharacter *ch, const DLString &constArguments);
static void config_telegram_print(PCharacter *ch);
static void config_discord(PCharacter *ch, const DLString &constArguments);
static void config_discord_print(PCharacter *ch);
static void config_lang(PCharacter *ch, const DLString &constArguments);
static void config_lang_print(PCharacter *ch);

list<PCMemoryInterface *> who_find_offline(PCharacter *looker);

/*-------------------------------------------------------------------------
 * ConfigElement
 *------------------------------------------------------------------------*/
const DLString & ConfigElement::getName( ) const
{
    return name;
}

const DLString & ConfigElement::getRussianName( ) const
{
    return rname;
}

bool ConfigElement::available(PCharacter *ch) const
{
    if (level > ch->get_trust())
        return false;

    return true;
}

bool ConfigElement::handleArgument( PCharacter *ch, const DLString &arg ) const
{
    if (arg.empty( )) {
        bool yes = isSetBit(ch);
        printLine( ch );
        ch->pecho("\nИспользуй команду {hc{yрежим %s %s{x для изменения.",
                      rname.c_str(), yes ? "нет" : "да", name.c_str(), yes ? "no" : "yes");
        return true;
    }
    
    Flags &field = getField( ch );

    if (arg_is_yes(arg) || arg_is_switch_on(arg))
        field.setBit( bit.getValue( ) );
    else if (arg_is_no(arg) || arg_is_switch_off(arg))
        field.removeBit( bit.getValue( ) );
    else if (arg_is(arg, "toggle"))
        field.toggleBit( bit.getValue( ) );
    else 
        return false;
    
    if (!printText( ch ))
        printLine( ch );

    return true;
}

bool ConfigElement::isSetBit( PCharacter *ch ) const
{
    return getField( ch ).isSet( bit );
}

bool ConfigElement::printText( PCharacter *ch ) const
{
    const DLString &msg = (isSetBit( ch ) ? msgOn.getValue( ) : msgOff.getValue( ) );

    if (!msg.empty( )) {
        ch->pecho( msg );
        return true;
    }

    return false;
}

void ConfigElement::printRow( PCharacter *ch ) const
{
    bool yes = isSetBit( ch );
    bool rus = ch->getConfig( ).rucommands;

    ch->pecho( "| {%s%-14s {x|  {%s%-7s {x|", 
                      CLR_NAME(ch), 
                      rus ? rname.getValue( ).c_str( ) : name.getValue( ).c_str( ), 
                      yes ? CLR_YES(ch) : CLR_NO(ch),
                      yes ? "ВКЛ." : "ВЫКЛ." );
}


static void print_line(PCharacter *ch, const DLString &name, const DLString &rname, bool yes, const DLString &msgYes, const DLString &msgNo)
{
    if (ch->getConfig( ).rucommands)
        ch->pecho( "  {%s%-14s {%s%5s {x%s",
                        CLR_NAME(ch),
                        rname.c_str(),
                        yes ? CLR_YES(ch) : CLR_NO(ch),
                        yes ? "ДА" : "НЕТ",
                        yes ? msgYes.c_str() : msgNo.c_str() );
    else
        ch->pecho( "  {%s%-12s {%s%5s {x%s",
                        CLR_NAME(ch),
                        name.c_str( ),
                        yes ? CLR_YES(ch) : CLR_NO(ch),
                        yes ? "YES" : "NO",
                        yes ? msgYes.c_str() : msgNo.c_str() );
}

void ConfigElement::printLine( PCharacter *ch ) const
{
    bool yes = isSetBit( ch );
    print_line(ch, name, rname, yes, msgOn, msgOff);
}

Flags & ConfigElement::getField( PCharacter *ch ) const
{
    static Flags zero;
    const FlagTable *table = bit.getTable( );

    if (table == &config_flags)
        return ch->config;
    
    if (table == &comm_flags)
        return ch->comm;
    
    if (table == &plr_flags)
        return ch->act;

    if (table == &add_comm_flags)
        return ch->add_comm;
    
    return zero;
}

/*-------------------------------------------------------------------------
 * ConfigGroup
 *------------------------------------------------------------------------*/
void ConfigGroup::printHeader( PCharacter *ch ) const
{
    ch->pecho( "\r\n{%s%s{x", 
                    CLR_HEADER(ch),
                    name.getValue( ).c_str( ) );
}

/*-------------------------------------------------------------------------
 * ConfigCommand
 *------------------------------------------------------------------------*/
ConfigCommand * ConfigCommand::thisClass = NULL;

void ConfigCommand::initialization( )
{
    thisClass = this;
    Class::regMoc<ConfigElement>( );
    Class::regMoc<ConfigGroup>( );
    Class::regMoc<ConfigCommand>( );
    CommandPlugin::initialization( );
}

void ConfigCommand::destruction( )
{
    CommandPlugin::destruction( );
    Class::unregMoc<ConfigCommand>( );
    Class::unregMoc<ConfigGroup>( );
    Class::unregMoc<ConfigElement>( );
    thisClass = NULL;
}

COMMAND(ConfigCommand, "config")
{
    PCharacter *pch;
    DLString arguments, arg1, arg2;
    Groups::iterator g;
    ConfigGroup::iterator c;

    if (ch->is_npc( ))
        return;
    
    pch = ch->getPC( );

    arguments = constArguments;
    arg1 = arguments.getOneArgument( );
    arg2 = arguments.getOneArgument( );

    if (arg1.empty( )) {
        for (g = groups.begin( ); g != groups.end( ); g++) {
            g->printHeader( pch );
            
            for (c = g->begin( ); c != g->end( ); c++) 
                if ((*c)->available(pch))
                    (*c)->printLine( pch );
        }

        config_scroll_print(pch);
        config_telegram_print(pch);
        config_discord_print(pch);
        config_lang_print(pch);

        ch->pecho("\r\nПодробнее смотри по команде {yрежим {Dнастройка{w.");
        return;
    }

    if (arg_is(arg1, "lang")) {
        config_lang(pch, arg2);
        return; 
    }

    if (arg_is(arg1, "scroll")) {
        config_scroll(pch, arg2);
        return; 
    }

    if (arg_is(arg1, "telegram")) {
        config_telegram(pch, arg2);
        return;
    }

    if (arg_is(arg1, "discord")) {
        config_discord(pch, arg2);
        return;
    }

    for (g = groups.begin( ); g != groups.end( ); g++) 
        for (c = g->begin( ); c != g->end( ); c++) 
            if ((*c)->available(pch))
                if (arg1.strPrefix((*c)->getName()) || arg1.strPrefix((*c)->getRussianName()))
                {
                    if (!(*c)->handleArgument( pch, arg2 ))
                        pch->pecho("Неправильный переключатель. См. {W? режим{x.");

                    return;
                }

    
    pch->pecho("Опция не найдена. Используй {hc{yрежим{x для списка.");
}


/*-------------------------------------------------------------------------
 * 'config lang'  
 *------------------------------------------------------------------------*/
static void config_lang(PCharacter *ch, const DLString &constArguments)
{
    DLString args = constArguments;
    DLString arg = args.getOneArgument();
    DLString lang; 

    // TODO improve
    if (arg_is(arg, "en"))
        lang = "en";
    else if (arg_is(arg, "ua"))
        lang = "ua";
    else if (arg_is(arg, "ru"))
        lang = "ru";
    else {
        ch->pecho("Укажи язык: англ, укр или рус.");
        return;
    }

    ch->getAttributes().getAttr<XMLStringAttribute>("lang")->setValue(lang);
    ch->pecho("Ок.");
}

static void config_lang_print(PCharacter *ch)
{
    lang_t lang = Player::lang(ch);
    // TODO move to a func
    DLString langname = lang == EN ? "англ" : lang == UA ? "укр" : "рус";

    ch->pecho( "  {%s%-14s {%s%5s {xнабери {y{hcрежим язык{x для установки",
                    CLR_NAME(ch),
                    "язык команд",
                    CLR_YES(ch),
                    langname.c_str());
}

/*-------------------------------------------------------------------------
 * 'config scroll'  
 *------------------------------------------------------------------------*/
static void config_scroll_print(PCharacter *ch)
{
    DLString lines(ch->lines);
    bool yes = ch->lines > 0;
    DLString msgNo = "Ты получаешь длинные сообщения без буферизации.";
    DLString msgYes = fmt(0, "Тебе непрерывно выводится %d лин%s текста.",
                       ch->lines.getValue( ), GET_COUNT(ch->lines.getValue( ), "ия","ии","ий") );

    print_line(ch, "scroll", "буфер", yes, msgYes, msgNo);
}

static void config_scroll(PCharacter *ch, const DLString &constArguments)
{
    int lines;
    DLString arg;

    arg = constArguments;
    arg = arg.getOneArgument( );

    if (arg.empty( ))
    {
        config_scroll_print(ch);
        ch->pecho("Для изменения используй {yрежим буфер{x число.");
        return;
    }

    if (!arg.isNumber( )) {
        ch->pecho("Ты должен ввести количество линий.");
        return;
    }

    try {
        lines = arg.toInt( );
    }
    catch (const ExceptionBadType& ) {
        ch->pecho("Неправильное число.");
        return;
    }

    if (lines == 0)
    {
        ch->pecho("Буферизация вывода отключена.");
        ch->lines = 0;
        return;
    }

    if (lines < 10 || lines > 200)
    {
        ch->pecho("Введи значение между 10 и 200.");
        return;
    }

    ch->lines = lines;
    ch->pecho( "Вывод установлен на %d лин%s.", lines,
                GET_COUNT(lines, "ию","ии","ий") );
}

/**
 * Print Telegram handle.
 */
static void config_telegram_print(PCharacter *ch)
{
    const DLString &user = get_string_attribute(ch, "telegram");
    bool yes = !user.empty();
    DLString msgYes = fmt(0, "Пользователь Telegram {C%s{x.", user.c_str());
    DLString msgNo = "Твой персонаж не связан с пользователями Telegram, набери {y{hcрежим телеграм{x.";
    print_line(ch, "telegram", "телеграм", yes, msgYes, msgNo);
}

/**
 * Link your character to a Telegram handle.
 */
static void config_telegram(PCharacter *ch, const DLString &constArguments)
{
    DLString arg = constArguments;
    XMLStringAttribute::Pointer user = ch->getAttributes().getAttr<XMLStringAttribute>("telegram");

    if (arg.empty()) {
        if (!user->empty())
            ch->pecho("Твой персонаж связан с пользователем Telegram {C%s{x."
                       "Используй {hc{yрежим телеграм очистить{x для очистки.\r\n", 
                       user->getValue().c_str());
        else
            ch->pecho("Твой персонаж не связан с пользователями Telegram.\r\n"
                        "Используй {yрежим телеграм @имя{x для установки.");        
        return;
    }

    if (arg_is_clear(arg)) {
        if (user->empty()) 
            ch->pecho("Нечего очищать.");
        else {
            ch->pecho("Связь с пользователем {C%s{x очищена.", user->getValue().c_str());
            user->clear();
            PCharacterManager::save(ch);
        }
        return;
    }

    if (arg.at(0) != '@') {
        ch->pecho("Задай имя пользователя начиная с @.");
        return;
    }

    user->setValue(arg);
    PCharacterManager::save(ch);
    ch->pecho("Теперь {C%s{x привязан к твоему персонажу.", arg.c_str());
}

/**
 *  Print Discord username and status.
 */
static void config_discord_print(PCharacter *ch)
{
    Json::Value discord;
    get_json_attribute(ch, "discord", discord);
    bool yes = !discord["id"].asString().empty();

    ostringstream msgYes;
    msgYes << "Пользователь Discord {C" << discord["username"].asString() 
           << "{w ({C" << discord["id"].asString() << "{w), статус " << discord["status"];            

    DLString msgNo = "Твой персонаж не связан с пользователем Discord, набери {y{hcрежим дискорд{x";
    print_line(ch, "discord", "дискорд", yes, msgYes.str(), msgNo);
}

/**
 * Print Discord linking instructions, regenerate token, clear user.
 */
static void config_discord(PCharacter *ch, const DLString &constArguments)
{
    DLString arg = constArguments;
    ostringstream buf;
    Json::Value discord;
    get_json_attribute(ch, "discord", discord);

    if (arg_is_clear(arg)) {
        // Clear out all user data and regenerate token.
        bool linked = !discord["id"].asString().empty();
        discord.clear();
        discord["token"] = create_nonce(6);
        set_json_attribute(ch, "discord", discord);
        PCharacterManager::save(ch);

        if (linked)
            ch->pecho("Связь с пользователем Discord очищена. ");

        ch->pecho("Новое секретное слово: {W%s{x.",
                   discord["token"].asString().c_str());
        return;
    }

    // Generate secret token for the first time.
    if (discord["token"].asString().empty()) {
        discord["token"] = create_nonce(6);
        set_json_attribute(ch, "discord", discord);
        PCharacterManager::save(ch);
    }

    if (discord["id"].asString().empty()) {
        buf << "Для связи этого персонажа с пользователем Discord: " << endl
            << "  {W*{x зайди на сервер {hlhttps://discord.gg/fVtaeePyey{x" << endl
            << "  {W*{x зайди на канал {W#дрим-чат{x или открой приват с ботом {WВалькирия{x" << endl
            << "  {W*{x набери {W/link " << discord["token"].asString() << "{x" << endl
            << "Для смены секретного слова набери {hc{yрежим дискорд очистить{x." << endl;
    } else {
        buf << "Твой персонаж связан с пользователем {C" << discord["username"].asString() << "{w ({C" 
            << discord["id"].asString() << "{w), статус " << discord["status"].asString() << endl;
        buf << "Для повторной линковки набери {W/link " << discord["token"].asString() <<"{x в привате с ботом {WВалькирия{x" << endl
            << "Для очистки и смены секретного слова набери {hc{yрежим дискорд очистить{x."
            << endl;
    }

    ch->send_to(buf);
}


/**
 * Discord: /link <6-symbol-token>
 * Auth: bottype=discord, token=<discord secret>
 * Args: link, id, username, status
 */
SERVLET_HANDLE(cmd_link, "/link")
{
    Json::Value params;
    if (!servlet_parse_params(request, response, params))
        return;

    if (!servlet_auth_bot(params, response)) 
        return;

    DLString linkToken, discordId, discordUsername, discordStatus;
    if (!servlet_get_arg(params, response, "link", linkToken)
        || !servlet_get_arg(params, response, "id", discordId)
        || !servlet_get_arg(params, response, "username", discordUsername)
        || !servlet_get_arg(params, response, "status", discordStatus))
        return;

    list<PCMemoryInterface *> players = find_players_by_json_attribute("discord", "token", linkToken);
    if (players.empty()) {
        servlet_response_404(response, "Player with this token not found");
        return;
    }

    PCMemoryInterface *player = players.front();
    Json::Value discord;
    get_json_attribute(player, "discord", discord);
    DLString currentId = discord["id"].asString();
    if (currentId == discordId) {
        servlet_response_200(response, "Already linked");
        return;
    }

    if (!currentId.empty()) {
        notice("Change Discord id from %s(%s) to %s(%s) for player %s.",
                currentId.c_str(), discord["username"].asString().c_str(),
                discordId.c_str(), discordUsername.c_str(), player->getName().c_str());
    } else {
        notice("Set Discord id to %s(%s) for player %s.",
                discordId.c_str(), discordUsername.c_str(), player->getName().c_str());
    }

    // Clean up alt players linked to the same id.
    for (const auto &alt: find_players_by_json_attribute("discord", "id", discordId)) {
        alt->getAttributes().eraseAttribute("discord");
        notice("Clear Discord info from %s for id %s.", alt->getName().c_str(), discordId.c_str());
    }

    discord["id"] = discordId;
    discord["username"] = discordUsername;
    discord["status"] = discordStatus;
    discord["token"] = DLString::emptyString;
    set_json_attribute(player, "discord", discord);
    PCharacterManager::saveMemory(player);

    servlet_response_200(response, "Link successful");
}


/**
 * Update player Discord status on bot logon.
 * Auth: bottype=discord, token=<discord secret>
 * Args: array of {id, username, status}
 */
SERVLET_HANDLE(cmd_update_all, "/update/all")
{
    Json::Value params;
    if (!servlet_parse_params(request, response, params))
        return;

    if (!servlet_auth_bot(params, response)) 
        return;

    DLString botType = params["bottype"].asString();
    botType.toLower();

    // Transform request payload into a map by id for quicker access.
    map<DLString, Json::Value> statuses;
    for (const auto &status: params["args"]) {
        DLString id = status["id"].asString();
        if (!id.empty()) 
            statuses[id] = status;
    }

    // Update all players linked to Discord. If their status is not mentioned
    // in the payload, assume they're offline.
    for (const auto &pcm: PCharacterManager::getPCM()) {
        Json::Value discord;

        if (get_json_attribute(pcm.second, botType, discord)) {
            DLString myDiscordId = discord["id"].asString();
            const auto &d = statuses.find(myDiscordId);
            if (d != statuses.end()) {
                discord["username"] = d->second["username"];
                discord["status"] = d->second["status"];
            } else {
                discord["status"] = "offline";
            }

            set_json_attribute(pcm.second, botType, discord);
            PCharacterManager::saveMemory(pcm.second);
            LogStream::sendNotice() << "Discord: update " << pcm.first << " status to " << discord["status"] << endl;
        }
    }

    servlet_response_200(response, "Success");

    Descriptor::updateMaxOffline(who_find_offline(0).size());
}

/**
 * Update player status when their presence in Discord changes.
 * Auth: bottype=discord, token=<discord secret>
 * Args: id, username, status
 */
SERVLET_HANDLE(cmd_update_one, "/update/one")
{
   Json::Value params;
    if (!servlet_parse_params(request, response, params))
        return;

    if (!servlet_auth_bot(params, response)) 
        return;

    PCMemoryInterface *player = servlet_find_player(params, response);
    if (!player)
        return;

    DLString discordUsername, discordStatus;
    if (!servlet_get_arg(params, response, "username", discordUsername)
        || !servlet_get_arg(params, response, "status", discordStatus))
        return;

    Json::Value discord;
    DLString botType = DLString(params["bottype"].asString()).toLower();
    get_json_attribute(player, botType, discord);

    discord["username"] = discordUsername;
    discord["status"] = discordStatus;

    set_json_attribute(player, botType, discord);
    PCharacterManager::saveMemory(player);
    LogStream::sendNotice() << "Discord: update " << player->getName() << " status to " << discord["status"] << endl;    

    servlet_response_200(response, "Success");

    Descriptor::updateMaxOffline(who_find_offline(0).size());
}
