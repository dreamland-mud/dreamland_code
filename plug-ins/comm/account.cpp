#include <jsoncpp/json/json.h>

#include "pcharacter.h"
#include "commandtemplate.h"
#include "wiznet.h"
#include "infonet.h"
#include "messengers.h"
#include "player_account.h"
#include "accountmanager.h"
#include "linkingcode.h"
#include "accountaudit.h"
#include "commonattributes.h"
#include "json_utils.h"
#include "arg_utils.h"
#include "act.h"
#include "def.h"
#include "l10n.h"

void password_set( PCMemoryInterface *pci, const DLString &plainText );
bool password_check( PCMemoryInterface *pci, const DLString &plainText );

CMDRUN( password )
{
    DLString args(constArguments);
    DLString argOld = args.getOneArgument();
    DLString argNew = args.getOneArgument();
    
    if (argOld.empty() || argNew.empty())
    {
        ch->pecho(_("Синтаксис: пароль <старый> <новый>."));
        return;
    }
    
    if (!password_check( ch->getPC( ), argOld ))
    {
        ch->setWait(40 );
        ch->pecho(_("Неверный пароль. Подожди 10 секунд."));
        return;
    }

    if (argNew.length() < 5)
    {
        ch->pecho(_("Новый пароль должен содержать более пяти символов."));
        return;
    }

    password_set( ch->getPC( ), argNew );
    ch->getPC( )->save( );
    ch->pecho("Ok.");
}


/* RT code to delete yourself */

CMDRUNP( delete )
{
    PCharacter *pch;
    
    if ( ch->is_npc() )
           return;
    
    pch = ch->getPC( );

    if (pch->getAttributes().isAvailable("nodelete")) {
        pch->pecho(_("Твой персонаж могут удалить только Боги."));
        return;
    }

    if (pch->confirm_delete)
    {
        if (!password_check( pch, argument ))
        {
            pch->pecho(_("Попытка суицида отменена -- неверный пароль."));
            pch->confirm_delete = false;
            return;
        }
        else
        {
            wiznet( WIZ_SECURE, 0, pch->get_trust( ),
                   "%1$^C1 превращает себя в помехи в проводах.", pch );
            // In-game info channel per viewer; Discord in English (players asked
            // for the Discord relay explicitly).
            infonet(pch, 0, _("{CТихий голос из $o2: {1{C%1$^C1 идет по пути Арханта и совершает суицид, навсегда покидая этот мир."), pch);
            send_to_discord_stream(":ghost: " + fmtLang(LANG_EN, _("{1{C%1$^C1 идет по пути Арханта и совершает суицид, навсегда покидая этот мир."), pch));
            
            Player::quitAndDelete( pch );
            return;
        }
    }

    pch->pecho(_("{RВНИМАНИЕ: {WЭТО НЕОБРАТИМОЕ ДЕЙСТВИЕ, ТВОЙ ПЕРСОНАЖ БУДЕТ УДАЛЕН НАСОВСЕМ!{x"));
    pch->pecho(_("Введи {yудалить <твой пароль>{x для подтверждения команды."));
    pch->pecho(_("Чтобы отменить попытку суицида, введи {yудалить без пароля."));
    pch->getPC( )->confirm_delete = true;
    wiznet( WIZ_SECURE, 0, pch->get_trust( ),
            "%^C1 собирается удалить своего персонажа.", pch );
}


/* The passwordless account layer -- player-facing surface.
 * See ACCOUNTS_NANNY_ROADMAP.md / Trello 2zFpQBoW. Ships dark: `account link`
 * minting is gated off (LinkingCode::mintingEnabled) until the Phase 3 redeem
 * bots exist, so this command shows only status and a "coming soon" line.
 */
// Bot handles for the linking deep-links. Permanent ids -> compile-time constants
// (replace here + rebuild only if a bot is ever swapped). Telegram uses a start-payload
// deep-link; Discord opens the bot's DM (works when the player shares the community
// server with Valkyrie).
static const char *TELEGRAM_BOT   = "dreamland_mud_bot";      // t.me/<this>?start=CODE
static const char *DISCORD_BOT_ID = "659914892941328423";    // discord.com/users/<this>

// A character's bot-VERIFIED Discord identity, or false. The `discord` attribute is
// set only by the /link servlet after the Discord bot POSTs the confirmed numeric id,
// so a non-empty id here is trustworthy -- the in-game adopt can create/attach an
// account from it with no linking code. Probe with findAttr (NOT get_json_attribute)
// so an unlinked char never gets an empty `discord` attr churned into its pfile.
static bool char_verified_discord(PCharacter *ch, DLString &discordId, DLString &username)
{
    XMLStringAttribute::Pointer attr = ch->getAttributes().findAttr<XMLStringAttribute>("discord");
    if (!attr)
        return false;

    Json::Value d;
    JsonUtils::fromString(attr->getValue(), d);
    if (!d.isObject())
        return false;

    discordId = d["id"].asString();
    username = d["username"].asString();
    return !discordId.empty();
}

static void account_status(PCharacter *ch)
{
    DLString id = AccountManager::accountOf(ch->getName());

    if (id.empty()) {
        ch->pecho(_("Твой персонаж не привязан к аккаунту. Аккаунт вернет доступ, если потеряешь пароль, и соберет всех персонажей под одним входом; скоро -- перенос qp между своими и перки."));
        ch->pecho(_("Набери {yаккаунт связать{x, чтобы начать. Подробнее: {hh5106аккаунт{x."));

        // A large slice of the playerbase already carries a bot-verified Discord id
        // from the old /link flow -- offer the one-command adopt, no code dance.
        DLString discordId, username;
        if (char_verified_discord(ch, discordId, username))
            ch->pecho(_("Твой Discord уже подтвержден ({W%1$s{x). Набери {yаккаунт дискорд{x, чтобы привязать аккаунт сразу, без кода."),
                      AccountManager::echoSafe(username).c_str());
        return;
    }

    ch->pecho(_("Аккаунт {W%1$s{x."), AccountManager::titleOf(id).c_str());

    Json::Value acc = AccountManager::get(id);
    const Json::Value &identities = acc["identities"];
    if (!identities.empty()) {
        ch->pecho(_("Способы входа:"));
        for (Json::Value::const_iterator i = identities.begin(); i != identities.end(); ++i) {
            if (!(*i).isObject())   // a hand-corrupted account file must not crash a player command
                continue;
            DLString type = (*i)["type"].asString();
            DLString display = (*i)["display"].asString();
            if (display.empty())
                display = (*i)["value"].asString();
            // display is an externally-supplied identity string (a bot username, an
            // email) -- escape it before it goes through the mudtag renderer.
            ch->pecho("  {W%1$s{x: %2$s", type.c_str(), AccountManager::echoSafe(display).c_str());
        }
    }

    ch->pecho(_("Персонажи аккаунта:"));
    for (const DLString &name : AccountManager::charsOf(id))
        ch->pecho("  %1$s", name.c_str());
}

// A char already on an account can't mint -- one account per char, no stealing.
static bool account_already_linked(PCharacter *ch)
{
    DLString current = AccountManager::accountOf(ch->getName());
    if (current.empty())
        return false;
    ch->pecho(_("Ты уже привязан к аккаунту {W%1$s{x."), AccountManager::titleOf(current).c_str());
    return true;
}

// Adopt a char's bot-VERIFIED discord.id straight into an account (choice A of the
// Discord flow): no code, no round-trip, because the /link servlet already proved the
// id. Returns false only on a real create/attach failure (already-linked is handled by
// the caller). Shared by `account discord` and the `account link discord` alias.
static bool account_adopt_discord(PCharacter *ch, const DLString &discordId, const DLString &username)
{
    bool created = false;
    DLString id = AccountManager::findByIdentity("discord", discordId);
    if (id.empty()) {
        id = AccountManager::create("discord", discordId, username);
        if (id.empty()) {
            ch->pecho(_("Не удалось создать аккаунт. Попробуй позже."));
            return false;
        }
        created = true;
    }

    if (!AccountManager::attachChar(id, ch->getName())) {
        ch->pecho(_("Не удалось создать аккаунт. Попробуй позже."));
        return false;
    }

    Json::Value f;
    f["char"] = ch->getName();
    f["account"] = id;
    f["identity"] = DLString("discord:") + discordId;
    f["created"] = created;
    AccountAudit::record("account_adopt", f);

    DLString title = AccountManager::titleOf(id);
    if (created)
        ch->pecho(_("Аккаунт {W%1$s{x создан по твоему Discord ({W%2$s{x)."),
                  title.c_str(), AccountManager::echoSafe(username).c_str());
    else
        ch->pecho(_("Персонаж добавлен к аккаунту {W%1$s{x."), title.c_str());
    return true;
}

// Generic: mint a code and offer BOTH bots. {hl<url>{x is itself client-aware -- web
// renders it clickable, telnet shows the raw URL -- so the link needs no invis wrapping;
// a {IW telnet-only verb ("открой"/"в привате") sits before it, and the code + typed
// command stay outside every invis span so every client keeps them.
static void account_link(PCharacter *ch)
{
    if (!LinkingCode::mintingEnabled() && !ch->is_immortal()) {
        ch->pecho(_("Привязка аккаунтов скоро откроется. Немного терпения."));
        return;
    }
    if (account_already_linked(ch))
        return;

    DLString code = LinkingCode::mint(ch->getName(), false);
    Json::Value f;
    f["char"] = ch->getName();
    AccountAudit::record("code_mint", f);

    ch->pecho(_("Твой код привязки: {W%1$s{x"), code.c_str());
    ch->pecho(_("Он одноразовый и действует 10 минут. Привяжи его в любом из ботов:"));
    ch->pecho(_("  Telegram {W@%1$s{x -- {IWоткрой {Ix{hlhttps://t.me/%1$s?start=%2$s{x, команда {W/attach %2$s{x"),
              TELEGRAM_BOT, code.c_str());
    ch->pecho(_("  Discord Валькирия -- {IWв привате {Ix{hlhttps://discord.com/users/%1$s{x, команда {W/link %2$s{x"),
              DISCORD_BOT_ID, code.c_str());
    ch->pecho(_("Никому не показывай код: кто его введет, привяжет этого персонажа к своему аккаунту."));
}

// Telegram: config telegram is a self-typed handle (UNVERIFIED), so there is no adopt --
// always the bot round-trip. Two choices: tap the deep-link (opens Hassan as your
// Telegram), or message from any other Telegram.
static void account_telegram(PCharacter *ch)
{
    if (!LinkingCode::mintingEnabled() && !ch->is_immortal()) {
        ch->pecho(_("Привязка аккаунтов скоро откроется. Немного терпения."));
        return;
    }
    if (account_already_linked(ch))
        return;

    DLString code = LinkingCode::mint(ch->getName(), false);
    Json::Value f;
    f["char"] = ch->getName();
    f["channel"] = "telegram";
    AccountAudit::record("code_mint", f);

    ch->pecho(_("Твой код привязки: {W%1$s{x"), code.c_str());
    ch->pecho(_("  {WA{x) {IWоткрой {Ix{hlhttps://t.me/%1$s?start=%2$s{x -- бот привяжет этот аккаунт."),
              TELEGRAM_BOT, code.c_str());
    ch->pecho(_("  {WB{x) с другого Telegram -- напиши боту {W@%1$s{x команду {W/attach %2$s{x."),
              TELEGRAM_BOT, code.c_str());
    ch->pecho(_("Никому не показывай код: кто его введет, привяжет этого персонажа к своему аккаунту."));
}

// Discord: config discord.id is bot-VERIFIED, so a char that carries it adopts in one
// command, no code (choice A). Otherwise mint a code and message Valkyrie (choice B).
static void account_discord(PCharacter *ch)
{
    if (!LinkingCode::mintingEnabled() && !ch->is_immortal()) {
        ch->pecho(_("Привязка аккаунтов скоро откроется. Немного терпения."));
        return;
    }
    if (account_already_linked(ch))
        return;

    DLString discordId, username;
    if (char_verified_discord(ch, discordId, username)) {
        account_adopt_discord(ch, discordId, username);
        return;
    }

    DLString code = LinkingCode::mint(ch->getName(), false);
    Json::Value f;
    f["char"] = ch->getName();
    f["channel"] = "discord";
    AccountAudit::record("code_mint", f);

    ch->pecho(_("Твой код привязки: {W%1$s{x"), code.c_str());
    ch->pecho(_("Напиши боту Валькирия {IWв привате {Ix{hlhttps://discord.com/users/%1$s{x команду {W/link %2$s{x."),
              DISCORD_BOT_ID, code.c_str());
    ch->pecho(_("Никому не показывай код: кто его введет, привяжет этого персонажа к своему аккаунту."));
}

/* Immortal-only backstop -- the human vibe-check with real hands (roadmap 2.9).
 * Every mutation is audited with the acting immortal as `actor`. */
static void account_admin(PCharacter *ch, DLString &args)
{
    DLString sub = args.getOneArgument();

    if (arg_oneof(sub, "info")) {
        DLString charName = args.getOneArgument();
        if (charName.empty()) {
            ch->pecho("Usage: account admin info <char>");
            return;
        }
        DLString id = AccountManager::accountOf(charName);
        if (id.empty()) {
            ch->pecho("%1$s is not linked to any account.", charName.c_str());
            return;
        }
        ch->pecho("Account {W%1$s{x:", id.c_str());
        Json::Value acc = AccountManager::get(id);
        const Json::Value &identities = acc["identities"];
        for (Json::Value::const_iterator i = identities.begin(); i != identities.end(); ++i) {
            if (!(*i).isObject())
                continue;
            ch->pecho("  identity %1$s: %2$s", (*i)["type"].asString().c_str(), (*i)["value"].asString().c_str());
        }
        for (const DLString &n : AccountManager::charsOf(id))
            ch->pecho("  char %1$s", n.c_str());
        return;
    }

    if (arg_oneof(sub, "attach")) {
        DLString charName = args.getOneArgument();
        DLString id = args.getOneArgument();
        if (charName.empty() || id.empty()) {
            ch->pecho("Usage: account admin attach <char> <accountId>");
            return;
        }
        if (!AccountManager::exists(id)) {
            ch->pecho("No such account: %1$s", id.c_str());
            return;
        }
        DLString current = AccountManager::accountOf(charName);
        if (!current.empty() && current != id) {
            ch->pecho("%1$s is already on account %2$s -- detach first.", charName.c_str(), current.c_str());
            return;
        }
        if (!AccountManager::attachChar(id, charName)) {
            ch->pecho("Character not found: %1$s", charName.c_str());
            return;
        }
        Json::Value f;
        f["actor"] = ch->getName();
        f["char"] = charName;
        f["account"] = id;
        AccountAudit::record("admin_attach", f);
        ch->pecho("Attached %1$s to %2$s.", charName.c_str(), id.c_str());
        return;
    }

    if (arg_oneof(sub, "detach")) {
        DLString charName = args.getOneArgument();
        if (charName.empty()) {
            ch->pecho("Usage: account admin detach <char>");
            return;
        }
        DLString current = AccountManager::accountOf(charName);
        if (current.empty()) {
            ch->pecho("%1$s is not linked to any account.", charName.c_str());
            return;
        }
        if (!AccountManager::detachChar(charName)) {
            ch->pecho("Character not found: %1$s", charName.c_str());
            return;
        }
        Json::Value f;
        f["actor"] = ch->getName();
        f["char"] = charName;
        f["account"] = current;
        AccountAudit::record("admin_detach", f);
        ch->pecho("Detached %1$s from %2$s.", charName.c_str(), current.c_str());
        return;
    }

    ch->pecho("Usage: account admin info|attach|detach ...");
}

CMDRUN( account )
{
    if (ch->is_npc())
        return;

    DLString args(constArguments);
    DLString cmd = args.getOneArgument();

    if (cmd.empty()) {
        account_status(ch->getPC());
        return;
    }

    // UA matcher form is apostrophe-less on purpose: KOI8 (the exec charset) has no
    // U+02BC, and a mudjs client strips it from input anyway, so "звязати" is the
    // form that actually arrives. Help shows the orthographic "звʼязати".
    if (arg_oneof(cmd, "link", "связать", "звязати")) {
        DLString sub = args.getOneArgument();
        if (arg_oneof(sub, "discord", "дискорд"))
            account_discord(ch->getPC());
        else if (arg_oneof(sub, "telegram", "телеграм", "телеграмм"))
            account_telegram(ch->getPC());
        else
            account_link(ch->getPC());
        return;
    }

    // Channel subcommands mirror `config telegram`/`config discord` (kept separately).
    if (arg_oneof(cmd, "discord", "дискорд")) {
        account_discord(ch->getPC());
        return;
    }
    if (arg_oneof(cmd, "telegram", "телеграм", "телеграмм")) {
        account_telegram(ch->getPC());
        return;
    }

    if (ch->is_immortal() && arg_oneof(cmd, "admin")) {
        account_admin(ch->getPC(), args);
        return;
    }

    // Email/telnet fallback path is Phase 4 -- acknowledge, do nothing yet.
    if (arg_oneof(cmd, "email", "почта", "пошта")
        || arg_oneof(cmd, "code", "код"))
    {
        ch->pecho(_("Привязка по почте появится позже."));
        return;
    }

    ch->pecho(_("Использование: {yаккаунт{x -- статус, {yаккаунт связать{x -- код в любой бот, {yаккаунт дискорд{x / {yаккаунт телеграм{x -- по каналу. Подробнее: {hh5106аккаунт{x."));
}
