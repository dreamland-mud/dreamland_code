/* accountservlet -- redeem/info/resetpw endpoints for the passwordless account
 * layer. See ACCOUNTS_NANNY_ROADMAP.md / Trello 2zFpQBoW.
 *
 * Called by the Telegram/Discord redeem bots (Phase 3) and the dreamland_web
 * account broker (Phase 5). Two auth seams below:
 *   servlet_auth_account     -- bot token only (redeem, resetpw). The bots hold
 *     dreamland_bot.token for these. Code-running endpoints (/api/exec, /api/eval)
 *     take the separate dreamland_ops.token, so a bot token leak never reaches them.
 *   servlet_auth_account_web -- bot token OR the scoped web token (emailcode,
 *     emailverify, info, enter, kofi, linknew). None of these mint a password,
 *     and only linknew attaches a character: a hero a nanny session is still
 *     creating, holding a code minted for that creation, to an account the
 *     caller's identity already owns. So a leak stays far below god. kofi writes state too: a leaked web
 *     token could forge Ko-fi payments and grant supporter time. Harmless while
 *     supporter is only a date -- revisit before it buys anything.
 *
 * 5.1 HARD BLOCKER: dreamland_bot.token must NEVER be placed in the dreamland_web
 * config -- see the roadmap. The broker holds its own account_web.token, matched at
 * the web seam and kept server-side.
 *
 * Endpoints (all POST, JSON body {token, bottype, args:{...}}):
 *   /account/redeem      {code, identityType, value, display?} -> attach-or-create
 *   /account/info        {identityType, value}                 -> account + chars
 *   /account/resetpw     {identityType, value, char}           -> temp password
 *   /account/enter       {identityType, value, char}           -> one-use entry token
 *   /account/linknew     {code, identityType, value}           -> link a hero in creation
 *   /account/emailcode   {email}                               -> mail a login code
 *   /account/emailverify {email, code}                         -> verify + resolve
 *   /account/kofi        {txn, type, amount, currency, ...}    -> Ko-fi payment ledger
 *
 * Ships dark: with minting gated off (LinkingCode::mintingEnabled) no live codes
 * exist, so /account/redeem always misses and no account is ever created here
 * until Phase 3 flips the gate.
 */
#include <string>

#include <sstream>
#include <map>
#include <ctime>
#include <vector>
#include <algorithm>

#include "servlet.h"
#include "servlet_utils.h"
#include "accountmanager.h"
#include "linkingcode.h"
#include "emailcode.h"
#include "accountaudit.h"
#include "kofiledger.h"
#include "entrytoken.h"
#include "messengers.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "pcharactermanager.h"
#include "pcmemoryinterface.h"
#include "profession.h"
#include "commonattributes.h"
#include "math_utils.h"
#include "json_utils.h"
#include "logstream.h"
#include "l10n.h"
#include "dlfilestream.h"
#include "dreamland.h"
#include "def.h"

using namespace std;

// Defined in loadsave/pcharactermanager.cpp; salted hash, same as `password`.
void password_set(PCMemoryInterface *pci, const DLString &plainText);

// Defined in account.cpp: the one ASCII/shape validator for a mailable address,
// shared so the in-game and web email paths cannot disagree on what they mail to.
bool account_is_ascii_email(const DLString &s);

// Defined in account.cpp: the bank-ceiling notice every link path prints.
void account_bank_cap_notice(PCharacter *ch, const DLString &id);

// ---- helpers ---------------------------------------------------------------

// A per-surface web token authenticates the dreamland_web /account broker without
// the god bot token. It is scoped here on purpose: the broker reaches account
// endpoints only, never /api/exec (execservlet gates on servlet_auth_ops,
// which rejects bottype "web"). The token lives in var/misc/account_web.token,
// created out of band like dreamland_bot.token. A missing/empty file fails closed,
// so accounts stay dark on a host that never configured a broker.
static DLString read_web_token()
{
    DLString token;

    try {
        ostringstream buf;
        DLFileStream tokenFile(dreamland->getMiscDir(), "account_web.token");
        tokenFile.toStream(buf);
        token = buf.str();
        token.replaces("\n", "");
        token.replaces("\r", "");   // a CRLF-edited file would otherwise never match

    } catch (const std::exception &) {
        // No web token file -> no web auth. This is the dark default.
    }

    return token;
}

static bool servlet_auth_web(Json::Value &params, HttpResponse &response)
{
    if (!params["token"].isString()) {
        servlet_response_400(response, "token must be a string");
        return false;
    }

    DLString myToken = params["token"].asString();
    DLString token = read_web_token();

    if (myToken.empty() || token.empty() || myToken != token) {
        LogStream::sendError() << "Account web auth failed: invalid or missing token" << endl;
        response.status = 403;
        response.message = "Unauthorized";
        response.body = "Invalid token";
        return false;
    }

    return true;
}

// Bot-only seam: the god token, bottype discord/telegram. This gates the two
// endpoints a leaked web token must never reach -- /account/redeem (attach-or-create)
// and /account/resetpw (mints a working temp password). Default-deny: a new account
// endpoint added on this seam is bot-only until it explicitly opts into web below.
static bool servlet_auth_account(Json::Value &params, HttpResponse &response)
{
    return servlet_auth_bot(params, response);
}

// Web-capable seam: the endpoints the dreamland_web broker drives (emailcode,
// emailverify, info, enter, kofi, linknew). Accepts the scoped web token as well as
// the god token. None of these mint a password; linknew attaches only a hero in
// creation to the caller's own account, so the web token stays far below god scope
// even if it leaks; kofi can grant supporter time (see the file header).
static bool servlet_auth_account_web(Json::Value &params, HttpResponse &response)
{
    if (!params["bottype"].isString()) {
        servlet_response_400(response, "bottype must be a string");
        return false;
    }

    DLString botType = params["bottype"].asString();
    botType.toLower();

    if (botType == "web")
        return servlet_auth_web(params, response);

    return servlet_auth_bot(params, response);
}

static const char *ACCOUNT_IDENTITY_TYPES[] = { "email", "telegram", "discord", 0 };

static bool account_valid_type(const DLString &type)
{
    for (int i = 0; ACCOUNT_IDENTITY_TYPES[i]; i++)
        if (type == ACCOUNT_IDENTITY_TYPES[i])
            return true;
    return false;
}

static DLString account_norm_type(const DLString &raw)
{
    DLString t = raw;
    t.toLower();
    if (t == "tg")
        return "telegram";
    return t;
}

static DLString account_strip_ws(const DLString &s)
{
    string::size_type a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos)
        return DLString::emptyString;
    string::size_type b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// Canonicalize the identity VALUE so the same account never splits in two.
// identityKey is case-sensitive, so 'Kit@Ukr.net' and 'kit@ukr.net' must fold
// together. Emails -> trimmed lower-case; telegram/discord ids are numeric.
static DLString account_canon_value(const DLString &type, const DLString &value)
{
    if (type == "email") {
        DLString v = account_strip_ws(value);
        v.toLower();
        return v;
    }
    return value;
}

// The mudtag-safe echo escaper now lives on AccountManager (AccountManager::echoSafe),
// shared with the in-game adopt surface so there is exactly one escaper.

// Account character keys are always the Latin login name. Reject anything else so
// PCharacterManager::find (which fuzzy-matches declined Cyrillic names) can never
// resolve a free-typed RU/UA name onto the wrong character.
static bool account_is_latin_name(const DLString &name)
{
    if (name.empty())
        return false;
    for (int i = 0; i < (int)name.size(); i++) {
        char c = name[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')))
            return false;
    }
    return true;
}

// Pull {identityType(normalized+validated), value(canonicalized)} out of args.
// Returns false (with the response already filled) on a missing/invalid field.
static bool account_read_identity(const Json::Value &params, HttpResponse &response,
                                  DLString &type, DLString &value)
{
    DLString rawType, rawValue;
    if (!servlet_get_arg(params, response, "identityType", rawType))
        return false;
    if (!servlet_get_arg(params, response, "value", rawValue))
        return false;

    type = account_norm_type(rawType);
    if (!account_valid_type(type)) {
        servlet_response_400(response, "Invalid identityType, expecting email/telegram/discord");
        return false;
    }

    value = account_canon_value(type, rawValue);
    if (value.empty()) {
        servlet_response_400(response, "Empty identity value");
        return false;
    }
    return true;
}

// A pendingCreation code outlives its creation: an abandoned session drops the
// parked identity (extractNewbie) but not the code. Park only while a nanny session
// still holds this unsaved name, or a later redeem would park it for the next
// player who picks the name, and their hero would join the redeemer's account.
static bool account_creation_alive(const DLString &charName)
{
    DLString name = charName;
    name.capitalize();

    if (PCharacterManager::find(name))
        return false;

    for (Descriptor *d = descriptor_list; d != 0; d = d->next) {
        if (d->connected != CON_NANNY || d->character == 0)
            continue;

        PCharacter *pc = d->character->getPC();
        if (pc == 0)
            continue;

        DLString held = pc->getName();
        held.capitalize();
        if (held == name)
            return true;
    }
    return false;
}

// ---- /account/redeem -------------------------------------------------------

static void account_redeem(HttpRequest &request, HttpResponse &response)
{
    Json::Value params;
    if (!servlet_parse_params(request, response, params))
        return;
    if (!servlet_auth_account(params, response))
        return;

    DLString code;
    if (!servlet_get_arg(params, response, "code", code))
        return;

    DLString type, value;
    if (!account_read_identity(params, response, type, value))
        return;

    DLString display;
    servlet_get_arg(params, "display", display);   // optional

    LinkingCode::Entry entry;
    if (!LinkingCode::redeem(code, entry)) {
        Json::Value a;
        a["result"] = "invalid_or_expired";
        a["identity"] = type + ":" + value;
        AccountAudit::record("code_redeem", a);
        servlet_response_400(response, "Invalid or expired code");
        return;
    }

    // attach-at-creation (Phase 4): the code was minted for a character still in the
    // nanny, so it is not on disk and attachChar cannot run yet. Park the verified
    // identity against the creating name; the nanny commits it once the character is
    // saved. No account is created here, so an abandoned creation leaves nothing.
    if (entry.pendingCreation) {
        if (!account_creation_alive(entry.charName)) {
            Json::Value a;
            a["result"] = "creation_gone";
            a["char"] = entry.charName;
            a["identity"] = type + ":" + value;
            AccountAudit::record("code_redeem", a);
            servlet_response_400(response, "That character is no longer being created");
            return;
        }
        AccountManager::recordPendingAttach(entry.charName, type, value, display);
        Json::Value a;
        a["result"] = "pending_recorded";
        a["char"] = entry.charName;
        a["identity"] = type + ":" + value;
        AccountAudit::record("code_redeem", a);

        Json::Value body;
        body["char"] = entry.charName;
        body["pending"] = true;
        servlet_response_200_json(response, body);
        return;
    }

    // find-before-create: an identity belongs to at most one account. Resolve the
    // target (may be "" -> a new account is minted below), but run EVERY rejection
    // BEFORE create() so a refused or failed redeem never leaves an orphan account
    // persisted for the identity.
    DLString id = AccountManager::findByIdentity(type, value);

    // Resolve the character's current account. Three shapes from here:
    //  * char unlinked             -> create-or-join the identity's account (attach)
    //  * char linked, identity new -> ADD the identity to the char's OWN account
    //  * char linked, identity on a DIFFERENT account -> refuse (no cross-account move)
    // Only the last is a conflict: an identity belongs to at most one account, so a
    // redeem that would move it away from the account it is already on is refused.
    DLString current = AccountManager::accountOf(entry.charName);
    if (!current.empty() && !id.empty() && current != id) {
        LogStream::sendWarning() << "Accounts: redeem refused, " << entry.charName
            << " on account " << current << ", identity belongs to " << id << "." << endl;
        Json::Value a;
        a["result"] = "refused_other_account";
        a["char"] = entry.charName;
        a["account"] = current;
        AccountAudit::record("code_redeem", a);
        servlet_response_400(response, "This login method belongs to a different account");
        return;
    }

    // The character must still exist (deleted/renamed between mint and redeem).
    // entry.charName is the Latin login minted in-game, so this is an exact lookup.
    DLString cname = entry.charName;
    if (PCharacterManager::find(cname.capitalize()) == 0) {
        Json::Value a;
        a["result"] = "char_gone";
        a["char"] = entry.charName;
        AccountAudit::record("code_redeem", a);
        servlet_response_404(response, "Character not found: " + entry.charName);
        return;
    }

    // All rejections passed -- apply the change.
    bool created = false;
    bool identityAdded = false;

    if (current.empty()) {
        // Char has no account yet: mint one for the identity if it has none, then attach.
        if (id.empty()) {
            id = AccountManager::create(type, value, display);
            if (id.empty()) {
                response.status = 500;
                response.message = "Command failed";
                response.body = "Account creation failed";
                return;
            }
            created = true;
        }
        if (!AccountManager::attachChar(id, entry.charName)) {
            // Unreachable in practice (the char-exists check above uses the same
            // lookup), but audit for symmetry if it ever fires as a safety net.
            Json::Value a;
            a["result"] = "attach_failed";
            a["char"] = entry.charName;
            a["account"] = id;
            AccountAudit::record("code_redeem", a);
            servlet_response_404(response, "Character not found: " + entry.charName);
            return;
        }
    } else {
        // Char is already on this account (the conflicting-account case was refused
        // above). Add the identity as another way in, unless it is already present.
        bool identityWasNew = id.empty();
        id = current;
        if (identityWasNew) {
            if (!AccountManager::addIdentity(id, type, value, display)) {
                response.status = 500;
                response.message = "Command failed";
                response.body = "Could not add the login method";
                return;
            }
            identityAdded = true;
        }
    }

    // Fold: the verified identity now lives on the account and mirrors down to EVERY
    // member character (the shape the who-list / bot bridge read), so the player never
    // needs a separate `config telegram`/`config discord`. Messenger types only; email
    // has no char-side mirror. Also enforces one id per account (clears the id off any
    // foreign character), replacing the old per-char account_bind_char_config.
    if (type == "telegram" || type == "discord")
        AccountManager::setMessengerIdentity(id, type, value, display);

    Json::Value a;
    a["result"] = "ok";
    a["char"] = entry.charName;
    a["account"] = id;
    a["identity_type"] = type;
    a["created"] = created;
    a["identity_added"] = identityAdded;
    AccountAudit::record("code_redeem", a);

    // Minter echo: the code is a bearer credential, so tell the (still-online)
    // minter their code was just consumed and by which identity, so a misdirected
    // paste is caught immediately instead of silently handing the char away.
    PCharacter *online = PCharacterManager::findPlayer(entry.charName);
    if (online) {
        // `display` (and value) come from the REDEEMER via the bot -- escape them
        // for the mudtag renderer so they can't paint the minter's screen, forge a
        // line, or hide the warning tail with an invis tag.
        DLString who = AccountManager::echoSafe(display.empty() ? value : display);
        online->pecho(_("Твой код привязки использован (%1$s: %2$s). Если это не ты -- сразу смени пароль командой {yпароль{x."),
                      type.c_str(), who.c_str());
        if (current.empty())
            account_bank_cap_notice(online, id);
    }

    Json::Value body;
    body["account"] = id;
    body["title"] = AccountManager::titleOf(id);   // the bot shows the title, not the id
    body["char"] = entry.charName;
    body["created"] = created;
    body["identity_added"] = identityAdded;
    servlet_response_200_json(response, body);
}

// A roster entry for the login UI: the character's name plus its level, remort count and class,
// read from the offline memory index (no pfile load -- allList is in RAM from
// boot). Class ships as {en,ru,ua} so the language-switchable login panel can label
// it in the player's chosen tongue. A listed name with no loadable memory (a
// deleted or unreadable profile) degrades to the bare name. Shared with account.cpp
// (the account_chars rpc) so the in-game settings roster matches the login roster.
Json::Value account_roster_entry(const DLString &name)
{
    Json::Value entry;
    entry["name"] = name.c_str();

    PCMemoryInterface *pci = PCharacterManager::find(name);
    if (pci != 0) {
        entry["level"] = pci->getLevel();
        entry["remorts"] = (int)pci->getRemorts().size();

        // getRusName/getUaName are Flexer declension pads ("паладин||а|у|а|ом|і"),
        // not display strings -- resolve the nominative (ruscase '1' parses the pad
        // for RU and UA alike) so the roster never leaks the raw pipe-pad. The EN
        // name has no pad. UA falls back to the RU nominative for a profession that
        // carries no UA name (the engine's own display convention).
        auto prof = pci->getProfession();
        DLString ru = prof->getRusName().ruscase('1');
        DLString ua = prof->getUaName().empty() ? ru : prof->getUaName().ruscase('1');
        Json::Value cls;
        cls["en"] = prof->getName().c_str();
        cls["ru"] = ru.c_str();
        cls["ua"] = ua.c_str();
        entry["class"] = cls;
    }
    return entry;
}

// The account's characters, most played first (the main on top), ties by name.
// A name with no loadable memory sinks to the bottom. Shared with account.cpp so
// the login roster and the in-game settings roster list in the same order.
list<DLString> account_chars_by_played(const DLString &id)
{
    vector<pair<int, DLString> > ranked;
    for (const DLString &name : AccountManager::charsOf(id)) {
        PCMemoryInterface *pci = PCharacterManager::find(name);
        ranked.push_back(make_pair(pci != 0 ? pc_total_hours(pci) : -1, name));
    }
    sort(ranked.begin(), ranked.end(),
         [](const pair<int, DLString> &a, const pair<int, DLString> &b) {
             if (a.first != b.first)
                 return a.first > b.first;
             return a.second < b.second;
         });

    list<DLString> names;
    for (auto &r : ranked)
        names.push_back(r.second);
    return names;
}

// ---- /account/info ---------------------------------------------------------

static void account_info(HttpRequest &request, HttpResponse &response)
{
    Json::Value params;
    if (!servlet_parse_params(request, response, params))
        return;
    if (!servlet_auth_account_web(params, response))
        return;

    DLString type, value;
    if (!account_read_identity(params, response, type, value))
        return;

    DLString id = AccountManager::findByIdentity(type, value);
    if (id.empty()) {
        servlet_response_404(response, "No account for this identity");
        return;
    }

    Json::Value acc = AccountManager::get(id);
    Json::Value body;
    body["account"] = id;
    body["title"] = AccountManager::titleOf(id);
    body["identities"] = acc["identities"];
    for (const DLString &name : account_chars_by_played(id))
        body["chars"].append(account_roster_entry(name));

    servlet_response_200_json(response, body);
}

// ---- /account/resetpw ------------------------------------------------------

static void account_resetpw(HttpRequest &request, HttpResponse &response)
{
    Json::Value params;
    if (!servlet_parse_params(request, response, params))
        return;
    if (!servlet_auth_account(params, response))
        return;

    DLString type, value;
    if (!account_read_identity(params, response, type, value))
        return;

    DLString charName;
    if (!servlet_get_arg(params, response, "char", charName))
        return;

    if (!account_is_latin_name(charName)) {
        servlet_response_400(response, "Character name must be Latin letters (the login name)");
        return;
    }

    // The identity must own the account the character belongs to.
    DLString id = AccountManager::findByIdentity(type, value);
    if (id.empty()) {
        servlet_response_404(response, "No account for this identity");
        return;
    }

    DLString charAccount = AccountManager::accountOf(charName);
    if (charAccount.empty() || charAccount != id) {
        LogStream::sendWarning() << "Accounts: resetpw refused, " << charName
            << " is not on account " << id << "." << endl;
        Json::Value a;
        a["result"] = "refused_not_on_account";
        a["char"] = charName;
        a["account"] = id;
        AccountAudit::record("pw_reset", a);
        servlet_response_400(response, "Character is not on this account");
        return;
    }

    DLString name = charName;
    PCMemoryInterface *pc = PCharacterManager::find(name.capitalize());
    if (pc == 0) {
        servlet_response_404(response, "Character not found: " + charName);
        return;
    }

    // Set the forced-change marker first, then password_set persists both it and
    // the new password in its single saveMemory (Phase 3.2 reads pwreset).
    Json::Value flag;
    flag["forced"] = true;
    set_json_attribute(pc, "pwreset", flag);

    // Fresh CSPRNG password. Never logged. Until the forced-change nanny step
    // reads pwreset, this temp password is a full working password -- acceptable
    // while only token-holding bots can call this.
    DLString temp = create_secure_nonce(8);
    password_set(pc, temp);

    // Audit the event, NEVER the password.
    Json::Value a;
    a["result"] = "ok";
    a["char"] = pc->getName();
    a["account"] = id;
    a["identity_type"] = type;
    AccountAudit::record("pw_reset", a);

    Json::Value body;
    body["char"] = pc->getName();
    body["password"] = temp;
    servlet_response_200_json(response, body);
}

// ---- /account/enter --------------------------------------------------------
//
// Mint a one-use entry token for a web client that has proven an identity on the
// site (Phase 5). The broker (holding the web token) posts {identityType, value,
// char}; this re-resolves the account from the identity server-side, confirms the
// character is on it -- the same ownership gate as resetpw -- and mints the token
// via entry_token_issue. The token is a password-equivalent for 180s: it goes back
// to the broker (server-to-server, holding the web token) and is NEVER logged. The
// browser never sees it; the broker hands the client only the moment-to-moment
// `account_enter <token>` command. Ships dark until the broker exists.

static void account_enter(HttpRequest &request, HttpResponse &response)
{
    Json::Value params;
    if (!servlet_parse_params(request, response, params))
        return;
    if (!servlet_auth_account_web(params, response))
        return;

    DLString type, value;
    if (!account_read_identity(params, response, type, value))
        return;

    DLString charName;
    if (!servlet_get_arg(params, response, "char", charName))
        return;

    // Account character keys are the Latin login name; reject anything else so
    // PCharacterManager::find can't fuzzy-match a declined RU/UA name onto the
    // wrong character (same guard as resetpw).
    if (!account_is_latin_name(charName)) {
        servlet_response_400(response, "Character name must be Latin letters (the login name)");
        return;
    }

    // The identity must own the account the character belongs to.
    DLString id = AccountManager::findByIdentity(type, value);
    if (id.empty()) {
        servlet_response_404(response, "No account for this identity");
        return;
    }

    DLString charAccount = AccountManager::accountOf(charName);
    if (charAccount.empty() || charAccount != id) {
        LogStream::sendWarning() << "Accounts: enter refused, " << charName
            << " is not on account " << id << "." << endl;
        Json::Value a;
        a["result"] = "refused_not_on_account";
        a["char"] = charName;
        a["account"] = id;
        AccountAudit::record("entry_token", a);
        servlet_response_400(response, "Character is not on this account");
        return;
    }

    // The character must still exist (deleted/renamed after the account link).
    DLString cname = charName;
    if (PCharacterManager::find(cname.capitalize()) == 0) {
        servlet_response_404(response, "Character not found: " + charName);
        return;
    }

    DLString token = entry_token_issue(id, cname);
    if (token.empty()) {
        response.status = 500;
        response.message = "Command failed";
        response.body = "Could not mint an entry token";
        return;
    }

    // Audit the mint, NEVER the token.
    Json::Value a;
    a["result"] = "ok";
    a["char"] = cname.c_str();
    a["account"] = id;
    a["identity_type"] = type;
    AccountAudit::record("entry_token", a);

    Json::Value body;
    body["char"] = cname.c_str();
    body["token"] = token.c_str();   // server-to-server; the broker keeps it, not the browser
    servlet_response_200_json(response, body);
}

// ---- /account/emailcode ----------------------------------------------------
//
// Mail a 6-digit code to an address so the web login can prove the visitor owns
// it (Phase 5.2a). Keyed by the email itself (the browser has no character yet);
// the in-game path keys the same primitive by character name. Shares EmailCode and
// account_is_ascii_email with `account email`, so the two surfaces cannot drift.
// The body is English for now -- the broker will pass the viewer's language later.
// Auth is the per-surface web token via servlet_auth_account_web (5.1).

static void account_emailcode(HttpRequest &request, HttpResponse &response)
{
    Json::Value params;
    if (!servlet_parse_params(request, response, params))
        return;
    if (!servlet_auth_account_web(params, response))
        return;

    DLString rawEmail;
    if (!servlet_get_arg(params, response, "email", rawEmail))
        return;

    DLString email = account_canon_value("email", rawEmail);   // trim + lower-case
    if (!account_is_ascii_email(email)) {
        servlet_response_400(response, "Invalid email address");
        return;
    }

    // Rate-limited per address (and per key -- here key == email), under a global
    // daily cap. Over any cap it mints and mails nothing, so one address takes at
    // most a few codes an hour and the global cap bounds the total send volume.
    DLString code = EmailCode::issue(email, email, true);
    if (code.empty()) {
        servlet_response_400(response, "Too many requests for this address");
        return;
    }

    // send_email does not strip markup (N2); the body is plain text, no tags. It
    // does not name the in-game path -- a web-issued code lives under key=email and
    // the in-game `account code` looks up key=character, so it would not resolve.
    DLString subject = "Dream Land: email verification";
    DLString body = DLString("Your Dream Land verification code: ") + code
        + "\n\nEnter it where the site asked for it. It expires in 10 minutes."
        + "\nIf you did not request this, just delete this message.";
    // Report what actually happened: the code is minted, but if the spool write
    // failed the visitor never got it, so do not claim "sent". The stale code just
    // expires on its 10-minute TTL.
    bool queued = send_email(email, subject, body);

    // Audit the request, NEVER the code.
    Json::Value a;
    a["result"] = queued ? "sent" : "send_failed";
    a["email"] = email;
    AccountAudit::record("email_request", a);

    Json::Value out;
    out["sent"] = queued;
    servlet_response_200_json(response, out);
}

// ---- /account/emailverify --------------------------------------------------
//
// Check a mailed code and resolve the account behind the address. The web path is
// LOGIN only: an account is created solely in-game, where a character can own it,
// so a verified-but-unknown address returns account:null (the broker then offers
// to link it in-game) rather than minting an ownerless account here.

static void account_emailverify(HttpRequest &request, HttpResponse &response)
{
    Json::Value params;
    if (!servlet_parse_params(request, response, params))
        return;
    if (!servlet_auth_account_web(params, response))
        return;

    DLString rawEmail, code;
    if (!servlet_get_arg(params, response, "email", rawEmail))
        return;
    if (!servlet_get_arg(params, response, "code", code))
        return;

    // Validate the key the same way /account/emailcode does, so a caller cannot
    // aim `verify` at a non-email principal -- e.g. a character login name, whose
    // pending in-game code this would otherwise probe and burn.
    DLString email = account_canon_value("email", rawEmail);
    if (!account_is_ascii_email(email)) {
        servlet_response_400(response, "Invalid email address");
        return;
    }

    DLString verified;
    int attemptsLeft = 0;
    EmailCode::Result r = EmailCode::verify(email, code, verified, attemptsLeft);

    if (r == EmailCode::NONE) {
        servlet_response_400(response, "No pending code for this address");
        return;
    }
    if (r == EmailCode::BADCODE) {
        Json::Value a;
        a["result"] = "bad_code";
        a["email"] = email;
        a["attempts_left"] = attemptsLeft;
        AccountAudit::record("email_verify", a);
        servlet_response_400(response, "Wrong or expired code");
        return;
    }

    // OK: ownership proven. Resolve the existing account (login), or report none.
    DLString id = AccountManager::findByIdentity("email", verified);

    Json::Value a;
    a["result"] = "ok";
    a["email"] = verified;
    a["account"] = id;   // "" when the address is on no account yet
    AccountAudit::record("email_verify", a);

    Json::Value out;
    out["verified"] = true;
    out["email"] = verified;
    if (id.empty()) {
        out["account"] = Json::Value(Json::nullValue);
    } else {
        out["account"] = id;
        out["title"] = AccountManager::titleOf(id);
        for (const DLString &name : account_chars_by_played(id))
            out["chars"].append(account_roster_entry(name));
    }
    servlet_response_200_json(response, out);
}

// ---- /account/linknew ------------------------------------------------------
//
// The web roster's "Create character". The nanny hands the creating client a
// pendingCreation linking code over that client's own websocket (account_code),
// and the broker posts it here with the identity its session cookie proved. On the
// web seam, so it is narrower than /account/redeem on purpose:
//  - only a pendingCreation code: a saved character's `account link` code is
//    neither consumed nor attached here;
//  - only an identity that already owns an account: no account is ever created;
//  - it only parks the identity for commitPendingAttach, which the nanny runs once
//    the hero saves;
//  - only while a nanny session still creates that name (account_creation_alive);
//  - wrong codes are capped per identity, so a session cannot hunt for someone
//    else's code.

static const long LINKNEW_MISS_WINDOW = 3600;
static const int LINKNEW_MISSES_PER_IDENTITY = 5;

struct LinknewMisses {
    int count;
    long windowStart;
};
static map<DLString, LinknewMisses> linknewMisses;   // "type:value" -> misses

static bool linknew_capped(LinknewMisses &m, int cap, long nowT)
{
    if (nowT - m.windowStart >= LINKNEW_MISS_WINDOW) {
        m.count = 0;
        m.windowStart = nowT;
    }
    return m.count >= cap;
}

static void linknew_purge(long nowT)
{
    for (map<DLString, LinknewMisses>::iterator i = linknewMisses.begin(); i != linknewMisses.end(); ) {
        if (nowT - i->second.windowStart >= LINKNEW_MISS_WINDOW)
            linknewMisses.erase(i++);
        else
            ++i;
    }
}

// The display the account already stores for this identity, so the commit's
// messenger refresh keeps the Discord/Telegram handle instead of blanking it.
static DLString account_stored_display(const DLString &id, const DLString &type, const DLString &value)
{
    Json::Value account = AccountManager::get(id);
    if (!account.isObject())
        return DLString::emptyString;

    const Json::Value &ids = account["identities"];
    for (Json::Value::const_iterator i = ids.begin(); i != ids.end(); ++i) {
        if ((*i).isObject()
            && (*i)["type"].asString() == type
            && (*i)["value"].asString() == value)
            return (*i)["display"].asString();
    }
    return DLString::emptyString;
}

static void account_linknew(HttpRequest &request, HttpResponse &response)
{
    Json::Value params;
    if (!servlet_parse_params(request, response, params))
        return;
    if (!servlet_auth_account_web(params, response))
        return;

    DLString code;
    if (!servlet_get_arg(params, response, "code", code))
        return;

    DLString type, value;
    if (!account_read_identity(params, response, type, value))
        return;

    DLString id = AccountManager::findByIdentity(type, value);
    if (id.empty()) {
        servlet_response_404(response, "No account for this identity");
        return;
    }

    long nowT = (long)time(0);
    DLString key = type + ":" + value;
    if (linknewMisses.size() > 1000)
        linknew_purge(nowT);
    LinknewMisses &mine = linknewMisses[key];
    if (linknew_capped(mine, LINKNEW_MISSES_PER_IDENTITY, nowT)) {
        Json::Value a;
        a["result"] = "rate_limited";
        a["account"] = id;
        a["identity_type"] = type;
        AccountAudit::record("linknew", a);
        servlet_response_400(response, "Too many wrong codes, try later");
        return;
    }

    // Peek first: a code that is missing, expired, not minted for a hero in
    // creation, or whose creation is gone counts as a miss and stays untouched.
    LinkingCode::Entry entry;
    if (!LinkingCode::peek(code, entry) || !entry.pendingCreation
        || !account_creation_alive(entry.charName))
    {
        mine.count++;
        Json::Value a;
        a["result"] = "invalid_or_expired";
        a["account"] = id;
        a["identity_type"] = type;
        AccountAudit::record("linknew", a);
        servlet_response_400(response, "Invalid or expired code");
        return;
    }

    if (!LinkingCode::redeem(code, entry)) {
        servlet_response_400(response, "Invalid or expired code");
        return;
    }

    AccountManager::recordPendingAttach(entry.charName, type, value,
                                        account_stored_display(id, type, value));

    Json::Value a;
    a["result"] = "pending_recorded";
    a["char"] = entry.charName;
    a["account"] = id;
    a["identity_type"] = type;
    AccountAudit::record("linknew", a);

    Json::Value body;
    body["char"] = entry.charName;
    body["pending"] = true;
    servlet_response_200_json(response, body);
}

// ---- /account/kofi ---------------------------------------------------------
//
// One Ko-fi webhook payment, already verified (verification token) and
// whitelisted by the dreamland_web broker. Web token auth: the endpoint only
// records a payment and extends a supporter date, it never mints access.
// 200 {status, months, duplicate:false} when stored, 200 {duplicate:true} for a
// transaction already in the ledger, 500 when the ledger write fails so the
// broker answers Ko-fi with a 5xx and Ko-fi retries.

static const char *KOFI_STRING_FIELDS[] = {
    "message_id", "timestamp", "type", "amount", "currency", "from_name",
    "message", "email", "tier_name", "discord_username", "discord_userid", 0
};
static const char *KOFI_BOOL_FIELDS[] = {
    "is_public", "is_subscription_payment", "is_first_subscription_payment", 0
};

static void account_kofi(HttpRequest &request, HttpResponse &response)
{
    Json::Value params;
    if (!servlet_parse_params(request, response, params))
        return;
    if (!servlet_auth_account_web(params, response))
        return;

    DLString rawTxn;
    if (!servlet_get_arg(params, response, "txn", rawTxn))
        return;
    DLString txn = KofiLedger::sanitizeTxn(rawTxn);
    if (txn.empty()) {
        servlet_response_400(response, "Invalid txn");
        return;
    }

    DLString type, amount;
    if (!servlet_get_arg(params, response, "type", type))
        return;
    if (!servlet_get_arg(params, response, "amount", amount))
        return;

    Json::Value payment(Json::objectValue);
    payment["txn"] = txn.c_str();
    for (int i = 0; KOFI_STRING_FIELDS[i]; i++) {
        DLString v;
        if (servlet_get_arg(params, KOFI_STRING_FIELDS[i], v))
            payment[KOFI_STRING_FIELDS[i]] = v.substr(0, 1000).c_str();
    }
    const Json::Value &args = params["args"];
    for (int i = 0; KOFI_BOOL_FIELDS[i]; i++)
        if (args.isObject() && args.get(KOFI_BOOL_FIELDS[i], Json::Value()).isBool())
            payment[KOFI_BOOL_FIELDS[i]] = args[KOFI_BOOL_FIELDS[i]].asBool();
    if (args.isObject() && args.get("shop_items", Json::Value()).isArray()) {
        // Shop items are only stored for the immortal who fulfils the order, never
        // echoed in-game; the broker passes them as printable ASCII.
        Json::Value items(Json::arrayValue);
        const Json::Value &raw = args["shop_items"];
        for (Json::Value::ArrayIndex i = 0; i < raw.size() && i < 20; i++) {
            if (!raw[i].isObject())
                continue;
            Json::Value item(Json::objectValue);
            if (raw[i].get("direct_link_code", Json::Value()).isString())
                item["direct_link_code"] = raw[i]["direct_link_code"].asString().substr(0, 100);
            if (raw[i].get("variation_name", Json::Value()).isString())
                item["variation_name"] = raw[i]["variation_name"].asString().substr(0, 100);
            if (raw[i].get("quantity", Json::Value()).isIntegral())
                item["quantity"] = raw[i]["quantity"].asInt();
            items.append(item);
        }
        payment["shop_items"] = items;
    }

    Json::Value entry;
    KofiLedger::RecordResult r = KofiLedger::record(payment, entry);
    Json::Value out;
    if (r == KofiLedger::RECORD_DUPLICATE) {
        out["duplicate"] = true;
        servlet_response_200_json(response, out);
        return;
    }
    if (r == KofiLedger::RECORD_FAILED) {
        response.status = 500;
        response.message = "Ledger write failed";
        response.body = "Ledger write failed";
        return;
    }

    out["duplicate"] = false;
    out["status"] = entry["status"];
    out["match"] = entry["match"];
    out["months"] = entry["months"];
    servlet_response_200_json(response, out);
}

// ---- registration ----------------------------------------------------------

SERVLET_HANDLE(api_account_redeem, "/account/redeem")
{
    account_redeem(request, response);
}

SERVLET_HANDLE(api_account_info, "/account/info")
{
    account_info(request, response);
}

SERVLET_HANDLE(api_account_resetpw, "/account/resetpw")
{
    account_resetpw(request, response);
}

SERVLET_HANDLE(api_account_enter, "/account/enter")
{
    account_enter(request, response);
}

SERVLET_HANDLE(api_account_emailcode, "/account/emailcode")
{
    account_emailcode(request, response);
}

SERVLET_HANDLE(api_account_emailverify, "/account/emailverify")
{
    account_emailverify(request, response);
}

SERVLET_HANDLE(api_account_linknew, "/account/linknew")
{
    account_linknew(request, response);
}

SERVLET_HANDLE(api_account_kofi, "/account/kofi")
{
    account_kofi(request, response);
}
