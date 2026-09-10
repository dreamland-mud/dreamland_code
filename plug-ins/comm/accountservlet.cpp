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
        LogStream::sendNotice() << "Accounts: redeem miss (invalid/expired) for "
            << type << ":" << value << "." << endl;
        servlet_response_400(response, "Invalid or expired code");
        return;
    }

    // PR-B only handles a real, saved character. attach-at-creation (a pending
    // code, char not yet on disk) is Phase 4 and rewrites this path.
    if (entry.pendingCreation) {
        LogStream::sendNotice() << "Accounts: redeem of a pending-creation code for "
            << entry.charName << " (not supported until Phase 4)." << endl;
        servlet_response_400(response, "Pending-creation codes are not supported yet");
        return;
    }

    // find-before-create: an identity belongs to at most one account.
    DLString id = AccountManager::findByIdentity(type, value);
    bool created = false;
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

    // Refuse to steal a character already linked to a DIFFERENT account.
    DLString current = AccountManager::accountOf(entry.charName);
    if (!current.empty() && current != id) {
        LogStream::sendWarning() << "Accounts: redeem refused, " << entry.charName
            << " already on account " << current << " (code offered " << id << ")." << endl;
        servlet_response_400(response, "Character is already linked to another account");
        return;
    }

    if (current != id) {
        if (!AccountManager::attachChar(id, entry.charName)) {
            servlet_response_404(response, "Character not found: " + entry.charName);
            return;
        }
    }

    LogStream::sendNotice() << "Accounts: redeem ok -- " << entry.charName
        << (created ? " created+attached " : " attached ") << id
        << " via " << type << "." << endl;

    // Minter echo: the code is a bearer credential, so tell the (still-online)
    // minter their code was just consumed and by which identity, so a misdirected
    // paste is caught immediately instead of silently handing the char away.
    PCharacter *online = PCharacterManager::findPlayer(entry.charName);
    if (online) {
        DLString who = display.empty() ? value : display;
        online->pecho(_("Твой код привязки использован (%1$s: %2$s). Если это не ты -- сразу смени пароль командой {yпароль{x."),
                      type.c_str(), who.c_str());
    }

    Json::Value body;
    body["account"] = id;
    body["char"] = entry.charName;
    body["created"] = created;
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

    LogStream::sendWarning() << "Accounts: password reset for " << pc->getName()
        << " (account " << id << ", via " << type << ")." << endl;

    Json::Value body;
    body["char"] = pc->getName();
    body["password"] = temp;
    servlet_response_200_json(response, body);
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
