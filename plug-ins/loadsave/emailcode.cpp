/* Dream Land, passwordless account layer, 2026.
 *
 * See ACCOUNTS_NANNY_ROADMAP.md (Phase 5.2a) / Trello 2zFpQBoW.
 */
#include <ctime>

#include "emailcode.h"
#include "math_utils.h"

using namespace std;

// 10-minute TTL: long enough to switch to a mail client and read the code, short
// enough that a leaked or intercepted code is a narrow window. Matches LinkingCode.
const int EmailCode::TTL_SECONDS = 600;

// A 6-digit code is ~20 bits; three tries makes a blind guess 3/10^6, and the code
// dies on the third miss so a run of guesses cannot walk the space (N6).
const int EmailCode::MAX_ATTEMPTS = 3;

const int EmailCode::CODE_DIGITS = 6;

// Rate limits: bound both a single mailbox (harassment / bounce storm) and a
// single principal spraying many addresses (quota burn). Small enough to protect
// the shared Gmail send quota, loose enough for a real recovery: 3 per address per
// hour, 5 per key per day.
const int EmailCode::ADDR_MAX_PER_WINDOW = 3;
const int EmailCode::ADDR_WINDOW_SECONDS = 3600;
const int EmailCode::KEY_MAX_PER_WINDOW = 5;
const int EmailCode::KEY_WINDOW_SECONDS = 86400;

// The one counter that mirrors the protected resource: a griefer who re-keys (many
// fresh characters, or many distinct addresses) walks past the per-principal caps,
// but every send still counts here, so the shared mail quota is bounded outright.
const int EmailCode::GLOBAL_MAX_PER_WINDOW = 200;
const int EmailCode::GLOBAL_WINDOW_SECONDS = 86400;

map<DLString, EmailCode::Entry> EmailCode::codes;
map<DLString, vector<long> > EmailCode::sendsByEmail;
map<DLString, vector<long> > EmailCode::sendsByKey;
vector<long> EmailCode::sendsGlobal;

long EmailCode::now()
{
    return (long)time(0);
}

bool EmailCode::expired(const Entry &e, long nowT)
{
    return nowT - e.mintedAt >= TTL_SECONDS;
}

void EmailCode::purgeExpired()
{
    long nowT = now();
    for (map<DLString, Entry>::iterator i = codes.begin(); i != codes.end(); ) {
        if (expired(i->second, nowT))
            codes.erase(i++);
        else
            ++i;
    }
}

// Drop send timestamps older than the widest window, and any list left empty, so
// the history maps do not grow without bound.
// Drop send timestamps older than the widest window (a day), and any list left
// empty, so the history maps and the global list do not grow without bound. All
// three windows are <= a day, and countRecent(Vec) re-filters to the exact window,
// so one horizon here is correct.
static void purge_prefix(std::vector<long> &v, long oldest)
{
    std::vector<long>::iterator w = v.begin();
    while (w != v.end() && *w < oldest)
        ++w;
    v.erase(v.begin(), w);
}

void EmailCode::purgeSends(long nowT)
{
    int widest = ADDR_WINDOW_SECONDS;
    if (KEY_WINDOW_SECONDS > widest)    widest = KEY_WINDOW_SECONDS;
    if (GLOBAL_WINDOW_SECONDS > widest) widest = GLOBAL_WINDOW_SECONDS;
    long oldest = nowT - widest;

    std::map<DLString, std::vector<long> > *maps[2] = { &sendsByEmail, &sendsByKey };
    for (int m = 0; m < 2; m++) {
        for (std::map<DLString, std::vector<long> >::iterator i = maps[m]->begin();
             i != maps[m]->end(); ) {
            purge_prefix(i->second, oldest);
            if (i->second.empty())
                maps[m]->erase(i++);
            else
                ++i;
        }
    }
    purge_prefix(sendsGlobal, oldest);
}

int EmailCode::countRecentVec(const std::vector<long> &v, long nowT, long window)
{
    int n = 0;
    for (std::vector<long>::const_iterator w = v.begin(); w != v.end(); ++w)
        if (*w >= nowT - window)
            n++;
    return n;
}

int EmailCode::countRecent(std::map<DLString, std::vector<long> > &hist,
                           const DLString &k, long nowT, long window)
{
    std::map<DLString, std::vector<long> >::iterator i = hist.find(k);
    if (i == hist.end())
        return 0;
    return countRecentVec(i->second, nowT, window);
}

DLString EmailCode::issue(const DLString &key, const DLString &email, bool enforceLimits)
{
    purgeExpired();

    long t = now();
    if (enforceLimits) {
        purgeSends(t);
        // Global cap first -- it is the one that mirrors the shared mail quota and
        // bounds a caller who re-keys past the per-address and per-key caps.
        if (countRecentVec(sendsGlobal, t, GLOBAL_WINDOW_SECONDS) >= GLOBAL_MAX_PER_WINDOW)
            return DLString::emptyString;
        if (countRecent(sendsByEmail, email, t, ADDR_WINDOW_SECONDS) >= ADDR_MAX_PER_WINDOW)
            return DLString::emptyString;
        if (countRecent(sendsByKey, key, t, KEY_WINDOW_SECONDS) >= KEY_MAX_PER_WINDOW)
            return DLString::emptyString;
        sendsByEmail[email].push_back(t);
        sendsByKey[key].push_back(t);
        sendsGlobal.push_back(t);
    }

    Entry e;
    e.email = email;
    e.code = DLString(create_secure_digits(CODE_DIGITS));
    e.mintedAt = t;
    e.attempts = 0;
    codes[key] = e;   // one active per key: a re-request overwrites the old one

    return e.code;
}

bool EmailCode::pending(const DLString &key, DLString &outEmail)
{
    map<DLString, Entry>::iterator i = codes.find(key);
    if (i == codes.end())
        return false;

    if (expired(i->second, now())) {
        codes.erase(i);
        return false;
    }

    outEmail = i->second.email;
    return true;
}

EmailCode::Result EmailCode::verify(const DLString &key, const DLString &code,
                                    DLString &outEmail, int &attemptsLeft)
{
    map<DLString, Entry>::iterator i = codes.find(key);
    if (i == codes.end())
        return NONE;

    if (expired(i->second, now())) {
        codes.erase(i);
        return NONE;
    }

    if (i->second.code == code) {
        outEmail = i->second.email;
        codes.erase(i);   // single-use
        return OK;
    }

    // Wrong: burn one attempt, and drop the whole entry once the budget is spent
    // so the next try starts from NONE (request a fresh code) rather than guessing on.
    i->second.attempts++;
    attemptsLeft = MAX_ATTEMPTS - i->second.attempts;
    if (attemptsLeft <= 0) {
        attemptsLeft = 0;
        codes.erase(i);
    }
    return BADCODE;
}
