/* Session resume for web clients -- see resume.h for what this is for. */
#include <stdio.h>
#include <time.h>
#include <map>

#include "resume.h"
#include "interprethandler.h"
#include "defaultbufferhandler.h"
#include "descriptorstatemanager.h"
#include "descriptor.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "logstream.h"
#include "merc.h"
#include "def.h"

/* A token outlives the socket by this much: long enough to walk back from
 * another app, short enough that a stolen one is worth little -- and short
 * because the linkdead body has to stand in the world for the whole of it
 * (see resume.h), able to be attacked while nobody is driving. */
static const int RESUME_TTL = 90;

struct ResumeEntry {
    DLString name;
    time_t   expires;
};

/* In memory on purpose: after a reboot every character has left the world, so
 * a token that survived it could only ever resolve to nothing. */
typedef std::map<DLString, ResumeEntry> TokenMap;   // token -> who
typedef std::map<DLString, DLString> NameMap;       // who   -> token
static TokenMap tokens;
static NameMap byName;

static void resume_forget(const DLString &name)
{
    NameMap::iterator n = byName.find(name);

    if (n == byName.end())
        return;

    tokens.erase(n->second);
    byName.erase(n);
}

static void resume_purge()
{
    time_t now = time(0);

    for (TokenMap::iterator i = tokens.begin(); i != tokens.end(); ) {
        if (i->second.expires <= now) {
            byName.erase(i->second.name);
            tokens.erase(i++);
        } else {
            i++;
        }
    }
}

/** 128 bits out of the kernel. The MUD's own number_range() is a game die --
 *  seeded, predictable, and reused for loot; it has no business minting
 *  something that stands in for a password. */
static DLString resume_random()
{
    unsigned char raw[16];
    FILE *f = fopen("/dev/urandom", "rb");

    if (!f || fread(raw, 1, sizeof(raw), f) != sizeof(raw)) {
        if (f)
            fclose(f);
        LogStream::sendError() << "Resume: cannot read /dev/urandom, tokens disabled" << endl;
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

DLString resume_token_issue(PCharacter *ch)
{
    if (!ch)
        return DLString::emptyString;

    resume_purge();

    const DLString &name = ch->getName();
    NameMap::iterator n = byName.find(name);

    // Refresh rather than rotate: the client stores whatever it last saw, and
    // handing it a new secret on every prompt would just widen the window in
    // which the one it holds is already dead.
    if (n != byName.end()) {
        TokenMap::iterator t = tokens.find(n->second);
        if (t != tokens.end()) {
            t->second.expires = time(0) + RESUME_TTL;
            return t->first;
        }
        byName.erase(n);
    }

    DLString token = resume_random();
    if (token.empty())
        return token;

    ResumeEntry entry;
    entry.name = name;
    entry.expires = time(0) + RESUME_TTL;
    tokens[token] = entry;
    byName[name] = token;

    return token;
}

bool resume_pending(PCharacter *ch)
{
    if (!ch)
        return false;

    NameMap::iterator n = byName.find(ch->getName());
    if (n == byName.end())
        return false;

    TokenMap::iterator t = tokens.find(n->second);
    if (t == tokens.end())
        return false;

    /* The token is refreshed on every prompt, so its expiry is really "last
     * seen + TTL": once the socket dies the refreshes stop and this runs out
     * on its own, without anything having to notice the disconnect. */
    return t->second.expires > time(0);
}

void resume_token_clear(PCharacter *ch)
{
    if (ch)
        resume_forget(ch->getName());
}

bool resume_attach(Descriptor *d, const DLString &token)
{
    resume_purge();

    if (!d || token.empty())
        return false;

    /* Only a descriptor that has not got a character yet may claim one. Without
     * this, a client that sent `resume` mid-session would have associate()
     * point a second character at this descriptor while the first still held a
     * pointer back to it -- a dangling desc on the abandoned character, and a
     * way to step into someone else's session from a live one. */
    if (d->character) {
        LogStream::sendWarning() << "Resume: " << d->host
                                 << " sent a token from a descriptor that is already playing" << endl;
        return false;
    }

    TokenMap::iterator t = tokens.find(token);
    if (t == tokens.end())
        return false;

    DLString name = t->second.name;
    PCharacter *twin = PCharacterManager::findPlayer(name);

    /* Refuse (token kept, dies of its own TTL) for the "still connected" cases
     * this path must NOT take over -- both are the login flow's job. Checked
     * BEFORE the token is spent, so it stays single-use:
     *   - an immortal switched into a mob;
     *   - a still-attached descriptor that is NOT CON_PLAYING. That is a player
     *     mid-login or mid-remort sitting on a CON_NANNY descriptor, whose
     *     close() runs NannyHandler::close -> extractNewbie -> delete on the
     *     character; closing it here and reattaching below would be a
     *     use-after-free.
     * Only a dead-but-still-CON_PLAYING descriptor -- the phone-suspend case
     * this change exists for -- is safe to evict: its handlers (InterpretHandler,
     * OLC, Pager) detach the character without freeing it. */
    if (twin && (twin->switchedTo
                 || (twin->desc && twin->desc->connected != CON_PLAYING))) {
        LogStream::sendNotice() << "Resume: " << d->host << " has a token for "
                                << name << ", who is still connected -- token kept for a retry" << endl;
        return false;
    }

    // Every outcome from here on is final, so the token is finished.
    resume_forget(name);

    if (!twin) {
        LogStream::sendNotice() << "Resume: " << d->host << " has a token for "
                                << name << ", who is no longer in the world" << endl;
        return false;
    }

    /* The returning player's own previous descriptor is usually still attached:
     * a phone that suspends drops the link without a FIN, so the character keeps
     * that descriptor until a write to the dead socket finally fails -- which on
     * an idle character can be minutes away, long enough that the player gives
     * up and logs in by hand. A valid token proves this is the same player
     * coming back to their own body, so close the stale descriptor now rather
     * than refusing and waiting for it to die. The guard above guarantees it is
     * CON_PLAYING, so close() runs only InterpretHandler-family handlers that
     * detach the character (null twin->desc) without freeing it; the main loop
     * then reaps the CON_CLOSED descriptor. Desktop is unaffected: its socket
     * closes cleanly on reconnect, so twin->desc is already null here and this
     * branch never runs. */
    if (twin->desc)
        twin->desc->close();

    /* The take-over itself, in the order nanny.reconnect uses: drop the login
     * handler this descriptor was born with, hand it the character, then let
     * the interpreter take over the input. */
    d->buffer_handler = new DefaultBufferHandler(0);   // koi8-r, what the web client decodes
    // comm.cpp installs the login handler at accept time, so there is always
    // one here -- but this runs on input from the network, and an empty list
    // would take the whole server down rather than one session.
    if (!d->handle_input.empty() && d->handle_input.front())
        d->handle_input.front()->close(d);
    d->associate(twin);
    InterpretHandler::init(d);
    /* CON_RESUME rather than CON_BREAK_CONNECT: listeners keyed on
     * newState == CON_PLAYING still fire, but the ones that greet an arriving
     * player can tell this apart from a login and keep quiet. */
    DescriptorStateManager::getThis()->handle(CON_RESUME, CON_PLAYING, d);

    // Deliberately quiet: no room echo, no wiznet. This fires every time a
    // phone locks its screen, and "%C1 restored their link" fifty times an
    // evening is noise, not information.
    twin->timer = 0;

    LogStream::sendNotice() << "Resume: " << d->host << " resumed the session of "
                            << name << endl;
    return true;
}
