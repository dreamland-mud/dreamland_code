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

#include "entrytoken.h"
#include "interprethandler.h"
#include "defaultbufferhandler.h"
#include "descriptorstatemanager.h"
#include "descriptor.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "loadsave.h"
#include "logstream.h"
#include "interp.h"
#include "room.h"
#include "merc.h"
#include "vnum.h"
#include "def.h"

/* Credential-grade and short: the browser has just proven the identity, so this
 * only has to survive the hop from the /account/enter POST to the client's
 * `account_enter` command a page-transition later. Shorter than resume's 180s
 * because nothing linkdead is standing in the world waiting on it. */
static const int ENTRY_TTL = 90;

struct EntryEntry {
    DLString account;
    DLString name;
    time_t   expires;
};

/* In memory on purpose: after a reboot every character has left the world, so a
 * token that survived it could only ever resolve to nothing. */
typedef std::map<DLString, EntryEntry> TokenMap;   // token   -> {account, char}
typedef std::map<DLString, DLString> AccountMap;   // account -> token (one live)
static TokenMap tokens;
static AccountMap byAccount;

static void entry_forget_account(const DLString &account)
{
    AccountMap::iterator a = byAccount.find(account);

    if (a == byAccount.end())
        return;

    tokens.erase(a->second);
    byAccount.erase(a);
}

static void entry_forget_token(TokenMap::iterator t)
{
    byAccount.erase(t->second.account);
    tokens.erase(t);
}

static void entry_purge()
{
    time_t now = time(0);

    for (TokenMap::iterator i = tokens.begin(); i != tokens.end(); ) {
        if (i->second.expires <= now) {
            byAccount.erase(i->second.account);
            tokens.erase(i++);
        } else {
            i++;
        }
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

    // One live token per account: a fresh mint replaces (and so invalidates) any
    // outstanding one. Only the click that mints this token is meant to be usable.
    entry_forget_account(accountId);

    DLString token = entry_random();
    if (token.empty())
        return token;

    EntryEntry entry;
    entry.account = accountId;
    entry.name = charName;
    entry.expires = time(0) + ENTRY_TTL;
    tokens[token] = entry;
    byAccount[accountId] = token;

    return token;
}

bool entry_token_redeem(Descriptor *d, const DLString &token)
{
    entry_purge();

    if (!d || token.empty())
        return false;

    // Only a descriptor that has not got a character yet may claim one -- the same
    // reason as resume_attach: otherwise associate() would point a second character
    // at a descriptor a first still references, dangling the abandoned one.
    if (d->character) {
        LogStream::sendWarning() << "Entry token: " << d->host
                                 << " sent a token from a descriptor that is already playing" << endl;
        return false;
    }

    TokenMap::iterator t = tokens.find(token);
    if (t == tokens.end())
        return false;

    DLString name = t->second.name;
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

    // Every outcome from here on is final, so the token is finished.
    entry_forget_token(t);

    if (twin) {
        // The chosen character is linkdead in the world: take over its body rather
        // than cold-load a duplicate. Identical to resume_attach's take-over tail;
        // the guard above proved twin->desc, if any, is a dead CON_PLAYING socket,
        // so close() runs only the InterpretHandler-family handlers that detach the
        // character without freeing it.
        d->buffer_handler = new DefaultBufferHandler(0);   // koi8-r, what the web client decodes
        if (!d->handle_input.empty() && d->handle_input.front())
            d->handle_input.front()->close(d);
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
                                << name << " (took over a linkdead body)" << endl;
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

    // Drop the login handler this fresh web descriptor was born with (resume_attach
    // does the same before its take-over), then cold-load the character.
    d->buffer_handler = new DefaultBufferHandler(0);
    if (!d->handle_input.empty() && d->handle_input.front())
        d->handle_input.front()->close(d);

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
