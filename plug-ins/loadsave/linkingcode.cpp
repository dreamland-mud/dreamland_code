/* Dream Land, passwordless account layer, 2026.
 *
 * See ACCOUNTS_NANNY_ROADMAP.md / Trello 2zFpQBoW.
 */
#include <ctime>

#include "linkingcode.h"
#include "math_utils.h"

using namespace std;

// 10-minute TTL. Short enough that a leaked code is a narrow window, long enough
// for a player to switch to a bot/browser and paste it.
const int LinkingCode::TTL_SECONDS = 600;

// Go-public flip: the redeem surfaces are live (Telegram /attach, Discord /link,
// web OAuth), so minting is enabled. `account link` and the nanny V2 account step
// now mint real codes for everyone. (The account.cpp !is_immortal() minting-gate
// bypasses are dead code once this is true -- harmless, left for a cleanup pass.)
static const bool ACCOUNTS_MINTING_ENABLED = true;

map<DLString, LinkingCode::Entry> LinkingCode::codes;

long LinkingCode::now()
{
    return (long)time(0);
}

bool LinkingCode::expired(const Entry &e, long nowT)
{
    return nowT - e.mintedAt >= TTL_SECONDS;
}

DLString LinkingCode::normalize(const DLString &code)
{
    DLString c = code;
    c.toUpper();
    return c;
}

void LinkingCode::purgeExpired()
{
    long nowT = now();
    for (map<DLString, Entry>::iterator i = codes.begin(); i != codes.end(); ) {
        if (expired(i->second, nowT))
            codes.erase(i++);
        else
            ++i;
    }
}

bool LinkingCode::mintingEnabled()
{
    return ACCOUNTS_MINTING_ENABLED;
}

DLString LinkingCode::mint(const DLString &charName, bool pendingCreation)
{
    purgeExpired();

    // One active code per char: drop any prior code this char still holds.
    for (map<DLString, Entry>::iterator i = codes.begin(); i != codes.end(); ) {
        if (i->second.charName == charName)
            codes.erase(i++);
        else
            ++i;
    }

    // Unambiguous CSPRNG alphabet (no 0/O/1/I/L), upper-case -- normalize() is a
    // no-op on it, but redeem still upper-cases player input.
    DLString code;
    for (;;) {
        code = DLString("DL-") + DLString(create_secure_nonce(5));
        if (codes.find(code) == codes.end())
            break;
    }

    Entry e;
    e.charName = charName;
    e.mintedAt = now();
    e.pendingCreation = pendingCreation;
    codes[code] = e;

    return code;
}

bool LinkingCode::peek(const DLString &code, Entry &out)
{
    map<DLString, Entry>::iterator i = codes.find(normalize(code));
    if (i == codes.end())
        return false;

    if (expired(i->second, now())) {
        codes.erase(i);
        return false;
    }

    out = i->second;
    return true;
}

bool LinkingCode::redeem(const DLString &code, Entry &out)
{
    map<DLString, Entry>::iterator i = codes.find(normalize(code));
    if (i == codes.end())
        return false;

    bool dead = expired(i->second, now());
    if (!dead)
        out = i->second;

    codes.erase(i);     // single-use: consume whether live or expired
    return !dead;
}
