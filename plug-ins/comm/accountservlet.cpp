/* accountservlet -- redeem/info/resetpw endpoints for the passwordless account
 * layer. See ACCOUNTS_NANNY_ROADMAP.md / Trello 2zFpQBoW.
 *
 * Called by the Telegram/Discord redeem bots (Phase 3); a web caller lands in
 * Phase 5. Auth reuses servlet_auth_bot through the servlet_auth_account() seam
 * below: the shared dreamland_bot.token already gates /api/exec (arbitrary code
 * execution on live), so folding account mutations onto it adds no blast radius
 * over what a token holder can already do. The seam is where Phase 5 introduces a
 * separate `web` token without touching the three handlers.
 *
 * 5.1 HARD BLOCKER: dreamland_bot.token must NEVER be placed in the dreamland_web
 * config -- see the roadmap. Web talks to these endpoints with its own token,
 * added at the seam, kept server-side.
 *
 * Endpoints (all POST, JSON body {token, bottype, args:{...}}):
 *   /account/redeem  {code, identityType, value, display?} -> attach-or-create
 *   /account/info    {identityType, value}                 -> account + chars
 *   /account/resetpw {identityType, value, char}           -> temp password
 *
 * Ships dark: with minting gated off (LinkingCode::mintingEnabled) no live codes
 * exist, so /account/redeem always misses and no account is ever created here
 * until Phase 3 flips the gate.
 */
#include <string>

#include "servlet.h"
#include "servlet_utils.h"
#include "accountmanager.h"
#include "linkingcode.h"
#include "emailcode.h"
#include "accountaudit.h"
#include "entrytoken.h"
#include "messengers.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "pcmemoryinterface.h"
#include "commonattributes.h"
#include "math_utils.h"
#include "json_utils.h"
#include "logstream.h"
#include "l10n.h"
#include "def.h"

using namespace std;

// Defined in loadsave/pcharactermanager.cpp; salted hash, same as `password`.
void password_set(PCMemoryInterface *pci, const DLString &plainText);

// Defined in account.cpp: the one ASCII/shape validator for a mailable address,
// shared so the in-game and web email paths cannot disagree on what they mail to.
bool account_is_ascii_email(const DLString &s);

// ---- helpers ---------------------------------------------------------------

// The auth seam. Phase 5 swaps this for a per-surface check without touching the
// handlers below. Today it is the shared bot token, honest about its scope.
static bool servlet_auth_account(Json::Value &params, HttpResponse &response)
{
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

// After a verified redeem, mirror the identity onto the char's `config` attr so account
// linking and `config telegram`/`config discord` stop being two separate steps: linking
// an account via the bot also fills in the char's messenger. Telegram stores the handle
// label (display); Discord follows /link's one-char-per-id rule (clear the id off every
// other char first) because the who-list Discord bridge is per-id. email has no config
// equivalent, so it is skipped. Called only on the bot-redeem path -- the in-game
// `account discord` adopt already reads an existing config discord.
static void account_bind_char_config(PCMemoryInterface *pc, const DLString &type,
                                     const DLString &value, const DLString &display)
{
    if (type == "telegram") {
        DLString handle = display.empty() ? value : display;
        pc->getAttributes().getAttr<XMLStringAttribute>("telegram")->setValue(handle);
        PCharacterManager::saveMemory(pc);
    } else if (type == "discord") {
        for (PCMemoryInterface *alt : find_players_by_json_attribute("discord", "id", value)) {
            if (alt != pc) {
                alt->getAttributes().eraseAttribute("discord");
                PCharacterManager::saveMemory(alt);
            }
        }
        Json::Value d;
        d["id"] = value;
        d["username"] = display;
        set_json_attribute(pc, "discord", d);
        PCharacterManager::saveMemory(pc);
    }
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

    // PR-B only handles a real, saved character. attach-at-creation (a pending
    // code, char not yet on disk) is Phase 4 and rewrites this path.
    if (entry.pendingCreation) {
        Json::Value a;
        a["result"] = "pending_unsupported";
        a["char"] = entry.charName;
        AccountAudit::record("code_redeem", a);
        servlet_response_400(response, "Pending-creation codes are not supported yet");
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

    // Auto-bind: mirror the just-verified identity onto the char's config attr so the
    // player didn't need a separate `config telegram`/`config discord` step. cname was
    // capitalized by the char-exists check above, so this find reuses that exact key.
    PCMemoryInterface *boundPc = PCharacterManager::find(cname);
    if (boundPc)
        account_bind_char_config(boundPc, type, value, display);

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
    }

    Json::Value body;
    body["account"] = id;
    body["title"] = AccountManager::titleOf(id);   // the bot shows the title, not the id
    body["char"] = entry.charName;
    body["created"] = created;
    body["identity_added"] = identityAdded;
    servlet_response_200_json(response, body);
}

// ---- /account/info ---------------------------------------------------------

static void account_info(HttpRequest &request, HttpResponse &response)
{
    Json::Value params;
    if (!servlet_parse_params(request, response, params))
        return;
    if (!servlet_auth_account(params, response))
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
    for (const DLString &name : AccountManager::charsOf(id))
        body["chars"].append(name);

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
// via entry_token_issue. The token is a password-equivalent for ~90s: it goes back
// to the broker (server-to-server, holding the web token) and is NEVER logged. The
// browser never sees it; the broker hands the client only the moment-to-moment
// `account_enter <token>` command. Ships dark until the broker exists.

static void account_enter(HttpRequest &request, HttpResponse &response)
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
// The body is English for now -- the broker passes the viewer's language at 5.1,
// where the per-surface token also replaces the god-token seam.

static void account_emailcode(HttpRequest &request, HttpResponse &response)
{
    Json::Value params;
    if (!servlet_parse_params(request, response, params))
        return;
    if (!servlet_auth_account(params, response))
        return;

    DLString rawEmail;
    if (!servlet_get_arg(params, response, "email", rawEmail))
        return;

    DLString email = account_canon_value("email", rawEmail);   // trim + lower-case
    if (!account_is_ascii_email(email)) {
        servlet_response_400(response, "Invalid email address");
        return;
    }

    // Rate-limited per address (and per key -- here key == email). Over a cap mints
    // and mails nothing, so the endpoint cannot be turned into a mail relay.
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
    send_email(email, subject, body);

    // Audit the request, NEVER the code.
    Json::Value a;
    a["result"] = "sent";
    a["email"] = email;
    AccountAudit::record("email_request", a);

    Json::Value out;
    out["sent"] = true;
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
    if (!servlet_auth_account(params, response))
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
        for (const DLString &name : AccountManager::charsOf(id))
            out["chars"].append(name);
    }
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
