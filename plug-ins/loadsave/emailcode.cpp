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

map<DLString, EmailCode::Entry> EmailCode::codes;

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

DLString EmailCode::issue(const DLString &key, const DLString &email)
{
    purgeExpired();

    Entry e;
    e.email = email;
    e.code = DLString(create_secure_digits(CODE_DIGITS));
    e.mintedAt = now();
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
