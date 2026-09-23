/* Passwordless web entry for the account layer -- see ACCOUNTS_NANNY_ROADMAP.md
 * Phase 5.
 *
 * The "account proven, character not chosen" state lives in dreamland_web (a
 * signed cookie), never in the engine: the engine only ever knows characters.
 * The bridge from that web state into the game is a one-use ENTRY TOKEN, a
 * sibling of the resume token (resume.cpp). The web broker authenticates an
 * identity (email code / Telegram widget / Discord OAuth), resolves the account,
 * and -- on a character click -- calls /account/enter to mint a token bound to
 * that (account, character). mudjs presents it as the character-less WS command
 * `account_enter <token>` (beside `resume` in wsHandlePayload), so the token
 * never rides console_in and cannot land in the command log.
 *
 * Where this differs from resume: resume re-attaches a body ALREADY in the world
 * (the phone-suspend case); entry cold-loads a character that is NOT in the world
 * (the roster-click case). When the chosen character happens to be linkdead in
 * the world, entry takes it over exactly as resume does rather than cold-loading
 * a duplicate.
 *
 * Ships dark: nothing mints an entry token until the dreamland_web broker (a
 * later Phase 5 step) exists, so `account_enter` always misses.
 */
#include <stdio.h>
#include <time.h>
#include <map>
#include <jsoncpp/json/json.h>

#include "entrytoken.h"
#include "resume.h"
#include "interprethandler.h"
#include "defaultbufferhandler.h"
#include "descriptorstatemanager.h"
#include "descriptor.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "pcmemoryinterface.h"
#include "accountmanager.h"
#include "accountaudit.h"
#include "loadsave.h"
#include "logstream.h"
#include "interp.h"
#include "room.h"
#include "wiznet.h"
#include "merc.h"
#include "vnum.h"
#include "def.h"

/* Credential-grade and short: the browser has just proven the identity, so this
 * only has to survive the hop from the /account/enter POST to the client's
 * `account_enter` command a page-transition later. Matches resume's 180s: a slow
 * roster page or a phone paused between the mint and the click was expiring the
 * token inside 90s, which the client could only report as a bare "could not enter". */
static const int ENTRY_TTL = 180;

struct EntryEntry {
    DLString account;
    DLString name;
    time_t   expires;
};

/* In memory on purpose: after a reboot every character has left the world, so a
 * token that survived it could only ever resolve to nothing. */
typedef std::map<DLString, EntryEntry> TokenMap;   // token -> {account, char}
static TokenMap tokens;

static void entry_forget_token(TokenMap::iterator t)
{
    tokens.erase(t);
}

static void entry_purge()
{
    time_t now = time(0);

    for (TokenMap::iterator i = tokens.begin(); i != tokens.end(); ) {
        if (i->second.expires <= now)
            tokens.erase(i++);
        else
            i++;
    }
}

/* 128 bits out of the kernel, the same reasoning as resume_random: number_range()
 * is a seeded game die reused for loot and has no business minting something that
 * stands in for a password. */
static DLString entry_random()
{
    unsigned char raw[16];
    FILE *f = fopen("/dev/urandom", "rb");

    if (!f || fread(raw, 1, sizeof(raw), f) != sizeof(raw)) {
        if (f)
            fclose(f);
        LogStream::sendError() << "Entry token: cannot read /dev/urandom, tokens disabled" << endl;
        return DLString::emptyString;
    }
    fclose(f);

    DLString hex;
    for (unsigned int i = 0; i < sizeof(raw); i++) {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02x", raw[i]);
        hex << buf;
    }

    return hex;
}

DLString entry_token_issue(const DLString &accountId, const DLString &charName)
{
    if (accountId.empty() || charName.empty())
        return DLString::emptyString;

    entry_purge();

    // Several live tokens per account are fine, and deliberately so: a double-tap
    // or a client re-render that POSTs /account/enter twice used to evict the first
    // token and leave the client holding a key that opened nothing. Each token is
    // 128-bit, single-use, account-bound and TTL-bounded, and any one of them opens
    // the same roster, so keeping them all costs nothing.

    DLString token = entry_random();
    if (token.empty())
        return token;

    EntryEntry entry;
    entry.account = accountId;
    entry.name = charName;
    entry.expires = time(0) + ENTRY_TTL;
    tokens[token] = entry;

    return token;
}

/* Drop whatever login-window state this descriptor still carries before it may
 * claim a character. Closes the WHOLE handler stack -- the same sweep
 * Descriptor::close runs on a dropped link -- not just the front handler:
 * NannyHandler::close, wherever it sits in the stack, extracts a pre-login
 * newbie shell or a half-logged-in character from newbie_list, frees it and
 * nulls d->character; handlers with nothing to detach (a pager on top, the base
 * no-op close) are unaffected. If a character somehow survives the sweep,
 * refuse: associate() over it would abandon it with a stale desc pointer, the
 * exact dangle the redeem guard exists to prevent. */
static bool entry_detach_login(Descriptor *d)
{
    for (handle_input_t::iterator h = d->handle_input.begin(); h != d->handle_input.end(); h++)
        (*h)->close(d);

    if (d->character) {
        LogStream::sendError() << "Entry token: " << d->host
                               << " still holds a character after closing its login handlers, refusing" << endl;
        return false;
    }

    return true;
}

bool entry_token_redeem(Descriptor *d, const DLString &token)
{
    entry_purge();

    if (!d || token.empty())
        return false;

    // A descriptor whose character is in the world (CON_PLAYING) keeps it:
    // stepping into another character over a live session is `account switch`'s
    // job, and evicting the current one here would leave it dangling. Anything
    // ELSE d->character can be is pre-login nanny state: the web client answers
    // the codepage menu the moment the socket opens, which runs
    // NannyHandler::doPlace and pins a throwaway newbie shell on the descriptor
    // under the /newui login overlay -- so a fresh web descriptor is NOT
    // character-less by the time the roster click sends `account_enter`. That
    // shell (or a half-finished login/remort on this same socket) is
    // newbie_list state whose own handler disposes of it on close() exactly as
    // a dropped link would -- entry_detach_login below, after the token checks.
    if (d->character && d->connected == CON_PLAYING) {
        LogStream::sendWarning() << "Entry token: " << d->host
                                 << " sent a token from a descriptor that is already playing" << endl;
        return false;
    }

    TokenMap::iterator t = tokens.find(token);
    if (t == tokens.end()) {
        // The one redeem failure that used to be silent. A token not on file is
        // the ordinary end of an expired, already-spent or superseded one -- but
        // with no line here, a client's "could not enter" had nothing behind it.
        LogStream::sendNotice() << "Entry token: " << d->host
                                << " sent a token not on file (expired or already used)" << endl;
        return false;
    }

    // Copy what the entry needs out of the map entry now: everything below may
    // refuse (token kept) or burn (token gone), and after a burn `t` is dead.
    DLString name = t->second.name;
    DLString mintedAccount = t->second.account;
    PCharacter *twin = PCharacterManager::findPlayer(name);

    // The target is already in the world. Mirror resume_attach's refusal for the
    // cases the login flow (not this path) must own -- checked BEFORE spending the
    // token so it stays single-use, and BEFORE touching the descriptor so a refusal
    // leaves it at the login prompt:
    //   - an immortal switched into a mob;
    //   - a still-attached descriptor that is NOT CON_PLAYING (a player mid-login /
    //     mid-remort on a nanny descriptor, whose close() would delete the char --
    //     evicting it here and reattaching would be a use-after-free).
    if (twin && (twin->switchedTo
                 || (twin->desc && twin->desc->connected != CON_PLAYING))) {
        LogStream::sendNotice() << "Entry token: " << d->host << " has a token for "
                                << name << ", who is still connected -- token kept for a retry" << endl;
        return false;
    }

    // The account binding the token was minted against must still hold: within the
    // TTL the character could have been detached (`account admin detach`) or moved
    // to another account, which would make this a stranger's key. Token kept -- it
    // is simply stale now and dies by its own TTL.
    if (AccountManager::accountOf(name) != mintedAccount) {
        LogStream::sendNotice() << "Entry token: " << d->host << " has a token for "
                                << name << ", no longer on account " << mintedAccount << endl;
        return false;
    }

    // Same-account simultaneous-login block (mortals only), the invariant every
    // other login door enforces (nannyhandler.cpp, backdoorhandler.cpp): one
    // account, one character in the world at a time. conflictingOnlineChar returns a
    // DIFFERENT online character on this account (never the target itself), so it
    // catches both a cold-load and a linkdead take-over that would add a second.
    // Trust is read off find(), not a loaded char, exactly as nanny/backdoor do.
    // Token kept: the player quits the other character and the same click still
    // works within the TTL.
    PCMemoryInterface *pci = PCharacterManager::find(name);
    if (pci != 0 && pci->get_trust() < LEVEL_IMMORTAL) {
        DLString conflict = AccountManager::conflictingOnlineChar(name);
        if (!conflict.empty()) {
            Json::Value fields;
            fields["char"] = name;
            fields["conflict"] = conflict;
            fields["channel"] = "entry";
            AccountAudit::record("login_block_conflict", fields);
            LogStream::sendNotice() << "Entry token: " << d->host << " for " << name
                << " blocked -- " << conflict << " (same account) is already online" << endl;
            return false;
        }
    }

    // Every outcome from here on is final, so the token is finished.
    entry_forget_token(t);

    if (twin) {
        // The chosen character is already in the world: take over its body rather
        // than cold-load a duplicate, the way resume_attach and the backdoor's
        // front-door takeover do. The refuse guard above proved twin->desc, if any,
        // is CON_PLAYING -- which may be a live session on another device, not only
        // a dead-but-unreaped socket; taking it over (this is the same player,
        // proven by an owner-authenticated token) is deliberate, resume's semantics.
        // Either way close() runs only InterpretHandler-family handlers that detach
        // the character without freeing it (CON_PLAYING keeps NannyHandler off it).
        // Clear the char's own resume token first, like the backdoor does, so a
        // stale web tab cannot later `resume` back in and evict this fresh session.
        resume_token_clear(twin);
        d->buffer_handler = new DefaultBufferHandler(0);   // koi8-r, what the web client decodes
        if (!entry_detach_login(d))
            return false;
        if (twin->desc)
            twin->desc->close();
        d->associate(twin);
        InterpretHandler::init(d);
        // CON_RESUME rather than CON_READ_MOTD: the CON_PLAYING listeners still fire,
        // but the ones that greet an arriving player can tell a take-over from a fresh
        // login and stay quiet -- this is the player's own body coming back.
        DescriptorStateManager::getThis()->handle(CON_RESUME, CON_PLAYING, d);
        twin->timer = 0;
        LogStream::sendNotice() << "Entry token: " << d->host << " entered "
                                << name << " (took over the existing session)" << endl;

        // The web client lifts its login overlay on the first prompt, and a prompt
        // only goes out with output. The quiet take-over produces none, so without
        // this the client times out and reports a failed entry over a session that
        // is actually live. Same closing look as account_enter_char/account_reconnect_char.
        interpret_raw(twin, "look");
        return true;
    }

    // The chosen character is not in the world. Confirm the pfile still exists
    // (deleted between mint and redeem) BEFORE touching the descriptor, so a stale
    // token leaves it at the login prompt instead of cold-loading a blank shell.
    if (PCharacterManager::find(name) == 0) {
        LogStream::sendNotice() << "Entry token: " << d->host << " has a token for "
                                << name << ", who no longer exists" << endl;
        return false;
    }

    // Drop the login state this fresh web descriptor carries -- its handler and,
    // since the web client has already walked the nanny past the codepage step,
    // the newbie shell pinned on it -- then cold-load the character.
    d->buffer_handler = new DefaultBufferHandler(0);
    if (!entry_detach_login(d))
        return false;

    PCharacter *ch = account_enter_char(d, name);
    if (!ch) {
        LogStream::sendError() << "Entry token: " << d->host
                               << " could not cold-load " << name << endl;
        return false;
    }

    LogStream::sendNotice() << "Entry token: " << d->host << " entered " << name << endl;
    return true;
}

PCharacter * account_enter_char(Descriptor *d, const DLString &charName)
{
    if (!d)
        return 0;

    // Load and link to the world anew -- the fresh-load half of backdoorhandler
    // (backdoorhandler.cpp:108-133). The caller has already taken any current
    // character out of the world (switch) or dropped the login handler (entry).
    PCharacter *ch = PCharacterManager::create(charName);
    if (!ch)
        return 0;

    PCharacterManager::update(ch);
    char_to_list(ch, &char_list);

    Room *start_room = get_room_instance(ch->getStartRoom());
    if (!start_room)
        start_room = get_room_instance(ROOM_VNUM_TEMPLE);
    char_to_room(ch, start_room);

    if (ch->pet) {
        // If the pet's room was set in fread_pet, place it there; else use master's.
        if (ch->pet->in_room)
            char_to_room(ch->pet, ch->pet->in_room);
        else
            char_to_room(ch->pet, ch->in_room);
    }

    d->associate(ch);
    InterpretHandler::init(d);
    // oldState != CON_PLAYING so the transition fires the CON_PLAYING listeners --
    // account config-apply (AccountConfigLoginListener) and last-host -- the same as
    // the backdoor's and switch's fresh-load path.
    DescriptorStateManager::getThis()->handle(CON_READ_MOTD, CON_PLAYING, d);

    interpret_raw(ch, "look");
    return ch;
}

/**
 * Reconnect descriptor `d` into the LINKDEAD body `twin` already in the world,
 * instead of cold-loading a duplicate: the take-over sibling of account_enter_char
 * that `account switch` uses when the chosen character is lostlink (Player present,
 * its own descriptor gone). The CALLER owns the same pre-step -- leave the current
 * character (save/quit/resume_token_clear/extract_char) -- so `d` already carries a
 * live game buffer and a spent InterpretHandler here: no login shell to detach, no
 * buffer swap, unlike the entry token's fresh web descriptor. Mirrors the
 * entry-token/resume take-over (entrytoken.cpp:249-276, resume.cpp:213-235): drop any
 * stale resume token on the body, close whatever dead descriptor still clings to it,
 * hand `d` the body, then fire CON_RESUME->CON_PLAYING (config-apply + last-host still
 * run, arriving-player greeters stay quiet -- this is a reconnect, not a login).
 */
PCharacter * account_reconnect_char(Descriptor *d, PCharacter *twin)
{
    if (!d || !twin)
        return 0;

    // A stale web resume token on the body would let a suspended tab `resume` back in
    // and evict this session -- drop it first, like the backdoor and entry take-over do.
    resume_token_clear(twin);

    // A descriptor still on the body is a dead-but-unreaped socket. account_switch's
    // guard already refused a live connection, so this never fires from there -- but
    // this is a public entry point, and closing a non-CON_PLAYING (nanny-stage)
    // descriptor would run NannyHandler::close -> extractNewbie -> delete the body out
    // from under us. Only a CON_PLAYING descriptor closes cleanly (InterpretHandler
    // handlers detach without freeing); refuse anything else, the way resume.cpp:213 and
    // entry_token_redeem do.
    if (twin->desc != 0) {
        if (twin->desc->connected != CON_PLAYING)
            return 0;
        twin->desc->close();
    }

    d->associate(twin);
    InterpretHandler::init(d);
    DescriptorStateManager::getThis()->handle(CON_RESUME, CON_PLAYING, d);
    twin->timer = 0;

    // Answer the net-death close this reconnect undoes (interprethandler.cpp:586): a
    // wiznet line and a forensics notice so admins and the log see the link come back.
    // NOT quiet like web-resume -- a switch is a one-off, not a phone locking its screen
    // fifty times an evening. No room echo: this TU carries no l10n catalog, and a
    // player-facing "restored the link" belongs translated, not RU-only.
    wiznet(WIZ_LINKS, 0, twin->get_trust(), "%C1 has restored the link.", twin);
    LogStream::sendNotice() << "account switch: " << d->host << " reconnected into "
                            << twin->getName() << endl;

    interpret_raw(twin, "look");
    return twin;
}
