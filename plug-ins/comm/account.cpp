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
static void account_status(PCharacter *ch)
{
    DLString id = AccountManager::accountOf(ch->getName());

    if (id.empty()) {
        ch->pecho(_("Твой персонаж не привязан к аккаунту."));
        ch->pecho(_("Аккаунт связывает твоих персонажей и дает способ восстановить доступ. Набери {yаккаунт связать{x, чтобы начать."));
        return;
    }

    ch->pecho(_("Аккаунт {W%1$s{x."), id.c_str());

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
            ch->pecho("  {W%1$s{x: %2$s", type.c_str(), display.c_str());
        }
    }

    ch->pecho(_("Персонажи аккаунта:"));
    for (const DLString &name : AccountManager::charsOf(id))
        ch->pecho("  %1$s", name.c_str());
}

static void account_link(PCharacter *ch)
{
    // Non-public rollout: immortals can mint now so the whole loop (mint ->
    // bot redeem -> account create/attach -> reset) is testable on Taiphoen
    // while public minting stays gated off. Remove the is_immortal bypass (or
    // flip ACCOUNTS_MINTING_ENABLED) for the public launch.
    if (!LinkingCode::mintingEnabled() && !ch->is_immortal()) {
        ch->pecho(_("Привязка аккаунтов скоро откроется. Немного терпения."));
        return;
    }

    DLString code = LinkingCode::mint(ch->getName(), false);

    Json::Value f;
    f["char"] = ch->getName();
    AccountAudit::record("code_mint", f);

    ch->pecho(_("Твой код привязки: {W%1$s{x"), code.c_str());
    ch->pecho(_("Он одноразовый и действует 10 минут. Введи его в боте (Telegram или Discord) или на сайте, чтобы привязать этого персонажа к аккаунту."));
    ch->pecho(_("Никому не показывай этот код: кто его введет, привяжет персонажа к своему аккаунту."));
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
        account_link(ch->getPC());
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

    ch->pecho(_("Использование: {yаккаунт{x -- статус, {yаккаунт связать{x -- код привязки."));
}
