#include <jsoncpp/json/json.h>

#include "pcharacter.h"
#include "commandtemplate.h"
#include "wiznet.h"
#include "infonet.h"
#include "messengers.h"
#include "player_account.h"
#include "accountmanager.h"
#include "linkingcode.h"
#include "emailcode.h"
#include "accountaudit.h"
#include "commonattributes.h"
#include "pcharactermanager.h"
#include "npcharacter.h"
#include "descriptor.h"
#include "descriptorstatemanager.h"
#include "interprethandler.h"
#include "rpccommandmanager.h"
#include "resume.h"
#include "entrytoken.h"
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
// Defined in accountservlet.cpp: {name, level, class:{en,ru,ua}} for one character.
Json::Value account_roster_entry( const DLString &name );

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

// Adopt a char's bot-VERIFIED discord.id straight into an account (no code, no round-
// trip: the /link servlet already proved the id). Three cases, all folded through
// AccountManager::setMessengerIdentity so the id mirrors to every member character:
//   - unlinked char, id unknown  -> create an account from it, attach;
//   - unlinked char, id on an account -> join that account;
//   - linked char -> add the verified id to the char's own account as an identity.
// Returns false only on a real create/attach failure.
static bool account_adopt_discord(PCharacter *ch, const DLString &discordId, const DLString &username)
{
    DLString current = AccountManager::accountOf(ch->getName());
    bool wasLinked = !current.empty();
    bool created = false;
    DLString id;

    if (!wasLinked) {
        id = AccountManager::findByIdentity("discord", discordId);
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
    } else {
        id = current;
    }

    // Fold the verified id onto the account and mirror it to every member character.
    AccountManager::setMessengerIdentity(id, "discord", discordId, username);

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
    else if (!wasLinked)
        ch->pecho(_("Персонаж добавлен к аккаунту {W%1$s{x."), title.c_str());
    else
        ch->pecho(_("Discord {W%1$s{x привязан к аккаунту {W%2$s{x."),
                  AccountManager::echoSafe(username).c_str(), title.c_str());
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

    // Auto-adopt any channel already VERIFIED on this character (Discord today), no
    // code: possession of the bot-set discord attribute is the proof. Telegram has no
    // verified id on an unlinked char (config telegram is a self-typed handle), so it
    // stays code-only below.
    bool adopted = false;
    DLString discordId, discordUser;
    if (char_verified_discord(ch, discordId, discordUser)) {
        DLString before = AccountManager::accountOf(ch->getName());
        DLString owner = AccountManager::findByIdentity("discord", discordId);
        // Unlinked -> create/join; linked -> add only if the id is unclaimed anywhere.
        // Never a silent cross-account move on a bare link.
        if (before.empty() || owner.empty())
            adopted = account_adopt_discord(ch, discordId, discordUser);
    }

    DLString current = AccountManager::accountOf(ch->getName());
    DLString code = LinkingCode::mint(ch->getName(), false);
    Json::Value f;
    f["char"] = ch->getName();
    AccountAudit::record("code_mint", f);

    // A code to add the OTHER channels (Telegram, or a Discord on a different account).
    if (adopted)
        ch->pecho(_("Чтобы добавить ещё один способ входа, твой код: {W%1$s{x"), code.c_str());
    else if (!current.empty())
        ch->pecho(_("Добавить способ входа к аккаунту {W%1$s{x. Твой код: {W%2$s{x"),
                  AccountManager::titleOf(current).c_str(), code.c_str());
    else
        ch->pecho(_("Твой код привязки: {W%1$s{x"), code.c_str());
    ch->pecho(_("Он одноразовый и действует 10 минут. Привяжи его в любом из ботов:"));
    ch->pecho(_("  Telegram {W@%1$s{x -- {IWоткрой {Ix{hlhttps://t.me/%1$s?start=%2$s{x, команда {W/attach %2$s{x"),
              TELEGRAM_BOT, code.c_str());
    ch->pecho(_("  Discord Валькирия -- {IWв привате {Ix{hlhttps://discord.com/users/%1$s{x, команда {W/link %2$s{x"),
              DISCORD_BOT_ID, code.c_str());
    if (!current.empty())
        ch->pecho(_("Никому не показывай код: кто его введет, получит доступ к твоему аккаунту и всем персонажам."));
    else
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
    DLString current = AccountManager::accountOf(ch->getName());
    DLString code = LinkingCode::mint(ch->getName(), false);
    Json::Value f;
    f["char"] = ch->getName();
    f["channel"] = "telegram";
    AccountAudit::record("code_mint", f);

    if (!current.empty())
        ch->pecho(_("Добавить способ входа к аккаунту {W%1$s{x. Твой код: {W%2$s{x"),
                  AccountManager::titleOf(current).c_str(), code.c_str());
    else
        ch->pecho(_("Твой код привязки: {W%1$s{x"), code.c_str());
    ch->pecho(_("  {WA{x) {IWоткрой {Ix{hlhttps://t.me/%1$s?start=%2$s{x -- бот привяжет этот аккаунт."),
              TELEGRAM_BOT, code.c_str());
    ch->pecho(_("  {WB{x) с другого Telegram -- напиши боту {W@%1$s{x команду {W/attach %2$s{x."),
              TELEGRAM_BOT, code.c_str());
    if (!current.empty())
        ch->pecho(_("Никому не показывай код: кто его введет, получит доступ к твоему аккаунту и всем персонажам."));
    else
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
    DLString current = AccountManager::accountOf(ch->getName());

    // Unlinked + a bot-verified Discord id: adopt it into a fresh account in one
    // command, no code (choice A). A char already on an account instead mints a code
    // below, so redeeming it adds Discord as another way in.
    if (current.empty()) {
        DLString discordId, username;
        if (char_verified_discord(ch, discordId, username)) {
            account_adopt_discord(ch, discordId, username);
            return;
        }
    }

    DLString code = LinkingCode::mint(ch->getName(), false);
    Json::Value f;
    f["char"] = ch->getName();
    f["channel"] = "discord";
    AccountAudit::record("code_mint", f);

    if (!current.empty())
        ch->pecho(_("Добавить способ входа к аккаунту {W%1$s{x. Твой код: {W%2$s{x"),
                  AccountManager::titleOf(current).c_str(), code.c_str());
    else
        ch->pecho(_("Твой код привязки: {W%1$s{x"), code.c_str());
    ch->pecho(_("Напиши боту Валькирия {IWв привате {Ix{hlhttps://discord.com/users/%1$s{x команду {W/link %2$s{x."),
              DISCORD_BOT_ID, code.c_str());
    if (!current.empty())
        ch->pecho(_("Никому не показывай код: кто его введет, получит доступ к твоему аккаунту и всем персонажам."));
    else
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

    // Load the alt onto this descriptor and into the world. The cold-load half is
    // shared with the web entry token (account_enter_char, entrytoken.cpp): it does
    // create -> world -> associate -> the CON_READ_MOTD->CON_PLAYING transition
    // (which fires account config-apply + last-host) -> look, mirroring the backdoor
    // fresh-load path. This char (ch) is already out of the world above.
    account_enter_char(d, altName);
}

// A mailable address must be pure ASCII (send_email drops `to` into the queue file
// unconverted, so a KOI8/high byte there corrupts the UTF-8 JSON and the drainer
// chokes -- N3) and shaped like an address. '{' and '}' are rejected too: no real
// address carries them, and it keeps a verified address safe to echo through the
// mudtag renderer without a separate escaper. Deliberately loose otherwise -- the
// real proof of a good address is that the code arrives. Non-static: the web
// email servlet (accountservlet.cpp) shares this one validator.
bool account_is_ascii_email(const DLString &s)
{
    if (s.empty() || s.size() > 254)
        return false;

    int at = -1;
    for (int i = 0; i < (int)s.size(); i++) {
        unsigned char c = (unsigned char)s[i];
        if (c < 0x21 || c > 0x7e)      // no controls, no space, no high/KOI8 bytes
            return false;
        if (c == '{' || c == '}')      // never in an address; keeps echoes tag-safe
            return false;
        if (c == '@') {
            if (at >= 0)               // exactly one '@'
                return false;
            at = i;
        }
    }

    if (at <= 0 || at == (int)s.size() - 1)          // '@' not first or last
        return false;
    if (s.find('.', at) == DLString::npos)           // a dot in the domain half
        return false;
    return true;
}

// Attach a just-verified email address to this character's account as an identity,
// creating the account if the char has none. Mirrors account_redeem's create-or-
// attach, including the one-account-per-identity conflict refusal. email has no
// `config` mirror (unlike telegram/discord), so there is no auto-bind step.
static void account_email_attach(PCharacter *ch, const DLString &email)
{
    DLString current = AccountManager::accountOf(ch->getName());
    DLString id = AccountManager::findByIdentity("email", email);

    // An identity belongs to at most one account: refuse to move this email away
    // from an account it already sits on, or the char away from its own account.
    if (!current.empty() && !id.empty() && current != id) {
        ch->pecho(_("Эта почта уже привязана к другому аккаунту."));
        return;
    }

    bool created = false;
    bool identityAdded = false;
    bool charJoined = false;

    if (current.empty()) {
        if (id.empty()) {
            id = AccountManager::create("email", email, "");
            if (id.empty()) {
                ch->pecho(_("Не удалось создать аккаунт. Попробуй позже."));
                return;
            }
            created = true;
        } else {
            // The address already owns an account; the unlinked char joins it (it
            // just proved the address). Distinct from the no-op below, which writes
            // nothing.
            charJoined = true;
        }
        if (!AccountManager::attachChar(id, ch->getName())) {
            ch->pecho(_("Не удалось создать аккаунт. Попробуй позже."));
            return;
        }
    } else {
        id = current;
        if (AccountManager::findByIdentity("email", email).empty()) {
            if (!AccountManager::addIdentity(id, "email", email, "")) {
                ch->pecho(_("Не удалось добавить способ входа. Попробуй позже."));
                return;
            }
            identityAdded = true;
        }
    }

    Json::Value f;
    f["char"] = ch->getName();
    f["account"] = id;
    f["identity"] = DLString("email:") + email;
    f["created"] = created;
    f["identity_added"] = identityAdded;
    f["char_joined"] = charJoined;
    AccountAudit::record("email_verify", f);

    DLString title = AccountManager::titleOf(id);
    if (created)
        ch->pecho(_("Аккаунт {W%1$s{x создан, почта {W%2$s{x подтверждена и привязана."),
                  title.c_str(), email.c_str());
    else if (charJoined)
        ch->pecho(_("Персонаж добавлен к аккаунту {W%1$s{x."), title.c_str());
    else if (identityAdded)
        ch->pecho(_("Почта {W%1$s{x подтверждена и привязана к аккаунту {W%2$s{x."),
                  email.c_str(), title.c_str());
    else
        ch->pecho(_("Эта почта уже привязана к твоему аккаунту {W%1$s{x."), title.c_str());
}

// `account email <addr>`: mint a 6-digit code, mail it, and wait for `account code`.
// Gated dark like `account link` (immortals bypass) so it ships without offering a
// flow the playerbase cannot finish yet.
static void account_email_request(PCharacter *ch, const DLString &rawAddr)
{
    if (!LinkingCode::mintingEnabled() && !ch->is_immortal()) {
        ch->pecho(_("Привязка аккаунтов скоро откроется. Немного терпения."));
        return;
    }

    if (rawAddr.empty()) {
        ch->pecho(_("Укажи адрес почты, например: {yаккаунт почта name@example.com{x."));
        return;
    }

    DLString email = rawAddr;
    email.toLower();
    if (!account_is_ascii_email(email)) {
        ch->pecho(_("Это не похоже на адрес почты. Только латиницей, например: {Wname@example.com{x."));
        return;
    }

    // Each request queues a real email; slow a mortal down so the command can't be
    // used to flood an inbox. Immortals test freely.
    if (!ch->is_immortal())
        ch->setWait(24);

    // Rate-limited per address and per character (mortals only); over a cap mints
    // and mails nothing, so neither a mailbox nor the send quota can be flooded.
    DLString code = EmailCode::issue(ch->getName(), email, !ch->is_immortal());
    if (code.empty()) {
        Json::Value rf;
        rf["char"] = ch->getName();
        rf["email"] = email;
        rf["result"] = "rate_limited";
        AccountAudit::record("email_request", rf);
        ch->pecho(_("Слишком много запросов на подтверждение почты. Попробуй позже."));
        return;
    }

    // send_email does not strip markup (N2), so keep subject and body plain -- no
    // colour codes, nothing that would leak a tag into the message. l() resolves to
    // the character's language now (send_email converts KOI8 -> UTF-8 for the mail).
    DLString subject(l(ch, "Dream Land: подтверждение почты аккаунта"));
    DLString body(l(ch, "Код подтверждения твоего аккаунта в Dream Land:"));
    body += " ";
    body += code;
    body += "\n\n";
    body += l(ch, "Введи его в игре: аккаунт код <шесть цифр>. Код действует 10 минут.");
    body += "\n";
    body += l(ch, "Если это письмо пришло по ошибке, просто удали его.");
    send_email(email, subject, body);

    // Audit the request, NEVER the code.
    Json::Value f;
    f["char"] = ch->getName();
    f["email"] = email;
    f["result"] = "sent";
    AccountAudit::record("email_request", f);

    ch->pecho(_("Код подтверждения отправлен на {W%1$s{x. Он живет 10 минут -- введи {yаккаунт код{x <шесть цифр>."),
              email.c_str());
}

// `account code <NNNNNN>`: check the code and, on success, attach the verified
// address to this character's account.
static void account_email_verify_cmd(PCharacter *ch, const DLString &rawCode)
{
    if (!LinkingCode::mintingEnabled() && !ch->is_immortal()) {
        ch->pecho(_("Привязка аккаунтов скоро откроется. Немного терпения."));
        return;
    }

    DLString code = rawCode;
    if (code.empty()) {
        ch->pecho(_("Введи код из письма: {yаккаунт код{x <шесть цифр>."));
        return;
    }

    DLString email;
    int attemptsLeft = 0;
    EmailCode::Result r = EmailCode::verify(ch->getName(), code, email, attemptsLeft);

    if (r == EmailCode::NONE) {
        ch->pecho(_("Нет активного запроса. Сначала: {yаккаунт почта{x <адрес>."));
        return;
    }
    if (r == EmailCode::BADCODE) {
        if (!ch->is_immortal())
            ch->setWait(12);   // slow a guesser between tries
        if (attemptsLeft > 0)
            ch->pecho(_("Неверный код. Осталось попыток: %1$d."), attemptsLeft);
        else
            ch->pecho(_("Неверный код, попытки исчерпаны. Запроси новый: {yаккаунт почта{x <адрес>."));
        return;
    }

    account_email_attach(ch, email);
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

    // Email path (scenario 7, blind/telnet-native, and the web login primitive):
    // `account email <addr>` mails a code, `account code <NNNNNN>` verifies it.
    if (arg_oneof(cmd, "email", "почта", "пошта")) {
        account_email_request(ch->getPC(), args.getOneArgument());
        return;
    }
    if (arg_oneof(cmd, "code", "код")) {
        account_email_verify_cmd(ch->getPC(), args.getOneArgument());
        return;
    }

    ch->pecho(_("Использование: {yаккаунт{x -- статус, {yаккаунт связать{x -- код в любой бот, {yаккаунт дискорд{x / {yаккаунт телеграм{x -- по каналу, {yаккаунт почта{x <адрес> -- по почте. Подробнее: {hh5106аккаунт{x."));
}

/*-----------------------------------------------------------------------------
 * account_chars rpc: hand the web client the account's characters so the
 * settings window can draw a roster and switch by click. Read-only, and mirrors
 * do_account/account_status: current character = the one in the world, chars =
 * charsOf, no online detection (the `account switch` command's own guard refuses
 * a same-account character that is already online, same as at the keyboard).
 * Reply shape (mudjs AccountPage.jsx): { current, account, title,
 * chars:[{name, level, class:{en,ru,ua}}] } -- same roster entry as the login
 * panel, so the settings roster can draw the class badge too.
 *---------------------------------------------------------------------------*/
RPCRUN(account_chars)
{
    if (ch == 0 || ch->getPC( ) == 0 || ch->desc == 0)
        return;

    // Refuse before login completes. In the nanny the character's name is only
    // attacker-typed, not an authenticated identity (the nanny sets ch.name as
    // soon as an existing name is entered, before the password), so keying the
    // account registry by it would hand any connecting socket a stranger's alt
    // roster and account title. Only a character actually in the world may ask.
    if (ch->desc->connected != CON_PLAYING)
        return;

    PCharacter *pch = ch->getPC( );

    Json::Value msg;
    msg["command"] = "account_chars";
    Json::Value &data = msg["args"][0];

    DLString id = AccountManager::accountOf(pch->getName( ));
    data["current"] = pch->getName( ).c_str( );
    data["account"] = !id.empty( );
    data["title"] = id.empty( ) ? "" : AccountManager::titleOf(id).c_str( );
    data["identities"] = Json::Value(Json::arrayValue);
    data["chars"] = Json::Value(Json::arrayValue);

    if (!id.empty( )) {
        // Login methods, same source as account_status. display is an
        // externally-supplied identity string, but it rides out as a JSON string
        // value (FastWriter-escaped) and the client renders it as a text node --
        // never through the mudtag/pecho renderer -- so no echo-injection here.
        Json::Value acc = AccountManager::get(id);
        const Json::Value &identities = acc["identities"];
        for (Json::Value::const_iterator i = identities.begin(); i != identities.end(); ++i) {
            if (!(*i).isObject( ))   // a hand-corrupted account file must not crash the rpc
                continue;
            Json::Value ident;
            ident["type"] = (*i)["type"].asString( );
            DLString display = (*i)["display"].asString( );
            if (display.empty( ))
                display = (*i)["value"].asString( );
            ident["display"] = display.c_str( );
            data["identities"].append(ident);
        }

        for (const DLString &name : AccountManager::charsOf(id))
            data["chars"].append(account_roster_entry(name));
    }

    ch->desc->writeWSCommand(msg);
}
