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
#include "pcharactermanager.h"
#include "npcharacter.h"
#include "descriptor.h"
#include "descriptorstatemanager.h"
#include "interprethandler.h"
#include "resume.h"
#include "fight_extract.h"
#include "clanreference.h"
#include "skillreference.h"
#include "loadsave.h"
#include "auction.h"
#include "room.h"
#include "vnum.h"
#include "merc.h"
#include "json_utils.h"
#include "arg_utils.h"
#include "act.h"
#include "interp.h"
#include "def.h"
#include "l10n.h"

void password_set( PCMemoryInterface *pci, const DLString &plainText );
bool password_check( PCMemoryInterface *pci, const DLString &plainText );

// Per-TU static references the account-switch guards need, same as quit.cpp:97-100
// (GSN/CLAN create file-scope statics; including the headers gives only the macros).
CLAN(invader);
CLAN(none);
GSN(evil_spirit);
GSN(suspect);

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

    ch->pecho(_("Персонажи аккаунта (кликни, чтобы войти):"));
    for (const DLString &name : AccountManager::charsOf(id)) {
        if (name == ch->getName())
            ch->pecho(_("  %1$s {D(сейчас){x"), name.c_str());
        else
            // {hc'command'label{x -- shows the name, sends the switch on click; the
            // typed `account switch <name>` is the keyboard/screen-reader path.
            ch->pecho("  {hc'account switch %1$s'%1$s{x", name.c_str());
    }
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
            // value is an externally-supplied identity string (bot username, email) --
            // escape it before the mudtag renderer, same as the player-facing status.
            DLString value = (*i)["value"].asString();
            ch->pecho("  identity %1$s: %2$s", (*i)["type"].asString().c_str(),
                      AccountManager::echoSafe(value).c_str());
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

// Switch to another character on the SAME account without a password. Account
// ownership is the auth: logging in one char (with its own password) authorizes
// any char the account owns -- so no password is asked and none is ever stored.
// The alt-login half mirrors backdoorhandler.cpp (the proven load path) and the
// leave-current half is extract_char (the proven extraction), so this reuses the
// real login/extract machinery rather than hand-rolling the descriptor lifecycle.
static void account_switch(PCharacter *ch, DLString &args)
{
    DLString id = AccountManager::accountOf(ch->getName());
    if (id.empty()) {
        ch->pecho(_("Ты не привязан к аккаунту."));
        return;
    }

    DLString target = args.getOneArgument();
    if (target.empty()) {
        ch->pecho(_("Кого загрузить? Набери {yаккаунт персонаж <имя>{x или кликни имя в {hh5106аккаунт{x."));
        return;
    }
    target.capitalize();

    if (target == ch->getName()) {
        ch->pecho(_("Ты уже играешь этим персонажем."));
        return;
    }

    // Must be a real character on the SAME account (ownership = the authorization).
    PCMemoryInterface *pcm = PCharacterManager::find(target);
    if (pcm == 0 || AccountManager::accountOf(pcm->getName()) != id) {
        ch->pecho(_("Этот персонаж не на твоем аккаунте."));
        return;
    }

    // Already in the world on another connection -- refuse (no takeover in v1).
    if (pcm->getPlayer() != 0) {
        ch->pecho(_("Этот персонаж уже в игре."));
        return;
    }

    // Leave-cleanly guards, mirroring quit.cpp (a switch is never "forced").
    if (ch->position == POS_FIGHTING || ch->fighting) {
        ch->pecho(_("Не сейчас -- сначала закончи бой."));
        return;
    }
    if (!ch->is_immortal()) {
        if (ch->position < POS_STUNNED) {   // dying/incapacitated -- no death-escape
            ch->pecho(_("Ты при смерти -- сейчас не переключиться."));
            return;
        }
        if (IS_VIOLENT(ch)) {
            ch->pecho(_("У тебя слишком много адреналина в крови."));
            return;
        }
        if (IS_SLAIN(ch)) {
            ch->pecho(_("Правда о твоем поражении еще не забыта."));
            return;
        }
        if (IS_KILLER(ch)) {
            ch->pecho(_("Боги еще помнят убийство, совершенное тобой."));
            return;
        }
    }

    if (IS_CHARMED(ch)) {
        ch->pecho(_("Сейчас ты не можешь оставить своего хозяина."));
        return;
    }

    if (auction->item != 0 && (ch == auction->buyer || ch == auction->seller)) {
        ch->pecho(_("Подожди, пока вещь с аукциона будет продана или возвращена."));
        return;
    }

    // Full parity with quit's leave-world guards (Kit's rule: block switch wherever
    // quit is blocked). A switch is never "forced", so each is a plain refusal --
    // the exact conditions and messages from quit.cpp:209-267. 🛑 Kept as a direct
    // mirror rather than a shared predicate to avoid refactoring the leave-world
    // command; if quit's guards change, update these in lockstep (or extract a shared
    // can-leave predicate then).
    if (!ch->is_immortal()) {
        if (IS_SET(ch->act, PLR_NO_EXP)) {
            ch->pecho(_("Ты не можешь покинуть этот мир! Твой дух во власти противника."));
            return;
        }
        if (IS_ROOM_AFFECTED(ch->in_room, AFF_ROOM_ESPIRIT)) {
            ch->pecho(_("Злые духи в этой зоне не отпускают тебя."));
            return;
        }
        if (ch->getClan() != clan_invader && ch->isAffected(gsn_evil_spirit)) {
            ch->pecho(_("Злые духи, овладевшие тобой, не позволяют тебе покинуть этот мир."));
            return;
        }
        if (ch->isAffected(gsn_suspect)) {
            ch->pecho(_("Ты не можешь этого сделать -- тебя ждет Суд!"));
            return;
        }
        if (ch->death_ground_delay > 0 && ch->trap.isSet(TF_NO_MOVE)) {
            ch->pecho(_("Сначала выберись из ловушки, а потом можно и покинуть этот мир."));
            return;
        }
        if (ch->in_room->pIndexData->clan != clan_none
            && ch->getClan() != ch->in_room->pIndexData->clan) {
            ch->pecho(_("Ты не можешь этого сделать -- здесь не твоя территория!"));
            return;
        }
    }

    Descriptor *d = ch->desc;
    if (d == 0)
        return;

    // Refuse from inside an OLC editor / pager / any layered handler: the login below
    // clears handle_input, which would free the executing handler under the input pump
    // (only the plain interpreter is safe to switch from). Precedent: CharacterWrapper
    // isInInterpret.
    if (d->handle_input.empty()
        || d->handle_input.front()->getType() != "InterpretHandler") {
        ch->pecho(_("Нельзя переключиться отсюда -- сначала выйди из редактора."));
        return;
    }

    DLString altName = pcm->getName();

    Json::Value f;
    f["char"] = altName;
    f["from"] = ch->getName();
    f["account"] = id;
    AccountAudit::record("account_switch", f);

    // Leave current: save first (persists pfile, inventory and current room, so a
    // later switch back lands where we left), then take it out of the world.
    // 🛑 PCharacterManager::quit snapshots allList[name] -> a memory interface BEFORE
    // extract_char, which does NOT free the PC -- it pools/recycles the shell. Without
    // quit(), create(alt) below pops that SAME shell and allList[current]/allList[alt]
    // collide (switch-back refused, identity map corrupted, resume-token hijack).
    // Mirrors quit.cpp order. resume_token_clear: this char leaves the descriptor for
    // good, so a stale web tab can never resume it. After this block ch is invalid --
    // never touch it again; use d/alt/altName/id.
    ch->save();
    PCharacterManager::quit(ch);
    resume_token_clear(ch);
    extract_char(ch, false);

    // Log the alt in -- mirrors backdoorhandler.cpp:108-135.
    PCharacter *alt = PCharacterManager::create(altName);
    PCharacterManager::update(alt);
    char_to_list(alt, &char_list);

    Room *start_room = get_room_instance(alt->getStartRoom());
    if (!start_room)
        start_room = get_room_instance(ROOM_VNUM_TEMPLE);
    char_to_room(alt, start_room);

    if (alt->pet) {
        if (alt->pet->in_room)
            char_to_room(alt->pet, alt->pet->in_room);
        else
            char_to_room(alt->pet, alt->in_room);
    }

    d->associate(alt);
    InterpretHandler::init(d);
    // oldState != CON_PLAYING so the transition fires the CON_PLAYING listeners --
    // account config-apply (AccountConfigLoginListener) and last-host -- same as the
    // backdoor's fresh-load path.
    DescriptorStateManager::getThis()->handle(CON_READ_MOTD, CON_PLAYING, d);

    interpret_raw(alt, "look");
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

    // Passwordless switch to another owned char (the clickable list targets this).
    if (arg_oneof(cmd, "switch", "персонаж", "персонажі", "перемкнути")) {
        account_switch(ch->getPC(), args);
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
