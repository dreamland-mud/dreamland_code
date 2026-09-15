/* Dream Land, passwordless account layer, 2026.
 *
 * See ACCOUNTS_NANNY_ROADMAP.md (Phase 5.2a) / Trello 2zFpQBoW.
 */
#ifndef EMAILCODE_H
#define EMAILCODE_H

#include <map>
#include <vector>
#include "dlstring.h"

/**
 * The email-verification code -- proves a player controls an address, so the
 * address can become an account identity ({type:"email"}), the same footing as a
 * telegram/discord id. Sibling of LinkingCode, but the code travels the other way:
 * the game mints it and mails it, the player reads their inbox and types it back.
 *
 * Keyed by an opaque principal string, so one primitive serves two surfaces whose
 * keyspaces never collide (a character login is Latin letters, an email has '@'):
 *   - in-game telnet:  key = character name  (the char that will own the identity)
 *   - web login:       key = the email itself (no character in the browser yet)
 *
 * In-RAM only, like LinkingCode: a reboot clears every pending code. Fine at a
 * 10-minute TTL. The code is a bearer secret of the address, so it is short-lived,
 * single-use on success, and guess-capped (MAX_ATTEMPTS) to blunt a ~20-bit brute
 * force (N6). It is NEVER logged and NEVER echoed back -- it lives only in the
 * mailed message and the player's own reply.
 */
class EmailCode {
public:
    enum Result {
        OK,        // code matched: `outEmail` filled, entry consumed
        BADCODE,   // wrong code, entry still live unless `attemptsLeft` hit 0
        NONE       // nothing pending for this key (never issued, or expired)
    };

    // Mint a fresh 6-digit code for `key`, mailing-address `email`, dropping any
    // prior pending code this key held (one active per key). Returns the code so
    // the caller can hand it to send_email(). Lazy-purges expired entries.
    //
    // When `enforceLimits` is true the send is rate-limited: at most a few codes
    // per address per hour and per key per day, so the command cannot flood a
    // mailbox or burn the outbound mail quota. Over a limit -> returns "" and mints
    // nothing (the caller reports "too many requests"). Immortals pass false so
    // testing is not throttled. Passing sends are recorded; refused ones are not.
    static DLString issue(const DLString &key, const DLString &email, bool enforceLimits);

    // Consume: on a live entry whose code matches, fill `outEmail`, erase the
    // entry, return OK. A wrong code increments the attempt counter and returns
    // BADCODE with `attemptsLeft` set (the entry is erased when it reaches 0).
    // No live entry -> NONE.
    static Result verify(const DLString &key, const DLString &code,
                         DLString &outEmail, int &attemptsLeft);

    // Is a live (unexpired) code pending for this key? Fills `outEmail` if so.
    // Lets a surface tell "you have no request" from "wrong code".
    static bool pending(const DLString &key, DLString &outEmail);

private:
    struct Entry {
        DLString email;     // canonicalized address the code was mailed to
        DLString code;      // 6 digits from create_secure_digits
        long     mintedAt;  // time() at issue
        int      attempts;  // wrong tries so far
    };

    static bool expired(const Entry &e, long nowT);
    static void purgeExpired();
    static long now();

    // Rate-limit bookkeeping: timestamps of recent sends, per address, per key, and
    // one global list that mirrors the single shared resource (the outbound mail
    // quota) -- the per-principal caps alone do not bound a caller who re-keys.
    static void purgeSends(long nowT);
    static int  countRecentVec(const std::vector<long> &v, long nowT, long window);
    static int  countRecent(std::map<DLString, std::vector<long> > &hist,
                            const DLString &k, long nowT, long window);

    static std::map<DLString, Entry> codes;   // principal key -> pending entry
    static std::map<DLString, std::vector<long> > sendsByEmail;  // address -> send times
    static std::map<DLString, std::vector<long> > sendsByKey;    // key -> send times
    static std::vector<long> sendsGlobal;                        // all limited sends

    static const int TTL_SECONDS;
    static const int MAX_ATTEMPTS;
    static const int CODE_DIGITS;
    static const int ADDR_MAX_PER_WINDOW;    // sends to one address per ADDR_WINDOW
    static const int ADDR_WINDOW_SECONDS;
    static const int KEY_MAX_PER_WINDOW;     // sends from one key per KEY_WINDOW
    static const int KEY_WINDOW_SECONDS;
    static const int GLOBAL_MAX_PER_WINDOW;  // total sends per GLOBAL_WINDOW
    static const int GLOBAL_WINDOW_SECONDS;
};

#endif
