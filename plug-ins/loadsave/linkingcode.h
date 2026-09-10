/* Dream Land, passwordless account layer, 2026.
 *
 * See ACCOUNTS_NANNY_ROADMAP.md / Trello 2zFpQBoW.
 */
#ifndef LINKINGCODE_H
#define LINKINGCODE_H

#include <map>
#include "dlstring.h"

/**
 * The cross-channel linking code -- the one primitive of the account layer.
 *
 * A character mints a short code in-game (`account link`); the player redeems it
 * on a bot or the web page, and the redeem surface attaches (or creates) the
 * account. The code is a bearer credential: whoever redeems it captures the
 * minting character, so the redeem surface must echo the minter and the code must
 * be short-lived, single-use, and one-at-a-time per character.
 *
 * In-RAM only: a reboot clears every code. That is fine at a 10-minute TTL (a
 * mid-flight code is simply re-minted). Not persisting also means a redeem can
 * never resurrect a code the game already forgot.
 */
class LinkingCode {
public:
    struct Entry {
        DLString charName;      // capitalize()d Latin login name of the minter
        long     mintedAt;      // time() at mint
        bool     pendingCreation; // char not saved yet (attach-at-creation, Phase 4)
    };

    // Mint a fresh code for a character, dropping any prior active code it held
    // (one active code per char). Returns "DL-XXXXX". Lazy-purges expired codes.
    static DLString mint(const DLString &charName, bool pendingCreation);

    // Look up without consuming. true + fills `out` for a live (unexpired) code.
    static bool peek(const DLString &code, Entry &out);

    // Consume: on a live code fill `out`, erase it, return true. Expired/missing
    // -> false (an expired code is erased on the way out).
    static bool redeem(const DLString &code, Entry &out);

    // Phase 3 gate. Minting stays OFF until the redeem bots exist, so `account
    // link` never hands a player a code for a flow that cannot complete yet --
    // the account layer keeps its "ships dark" promise through PR-B. Flip to true
    // in the PR that lands the Telegram/Discord redeem surfaces (Phase 3).
    static bool mintingEnabled();

private:
    static DLString normalize(const DLString &code);   // upper-case
    static bool     expired(const Entry &e, long nowT);
    static void     purgeExpired();
    static long     now();

    static std::map<DLString, Entry> codes;   // normalized code -> entry
    static const int TTL_SECONDS;
};

#endif
