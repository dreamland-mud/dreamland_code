/* Passwordless web entry for the account layer -- see entrytoken.cpp and
 * ACCOUNTS_NANNY_ROADMAP.md Phase 5. */
#ifndef ENTRYTOKEN_H
#define ENTRYTOKEN_H

#include "dlstring.h"

class Descriptor;
class PCharacter;

/**
 * Load an OWNED character fresh onto descriptor `d` and drop it into the world:
 * the cold-load half shared by `account switch` and the web entry token. The
 * CALLER owns the pre-step -- `account switch` first leaves its current character
 * (save/quit/resume_token_clear/extract_char), the entry-token redeem first drops
 * the login handler -- so this only does create -> world -> associate -> the
 * CON_READ_MOTD->CON_PLAYING transition (which fires the account config-apply and
 * last-host listeners, exactly like a login) -> look. Mirrors backdoorhandler's
 * fresh-load path (backdoorhandler.cpp:108-133). Returns the new character, or 0
 * on a create failure -- defensive only: PCharacterManager::create currently
 * always returns a shell (a missing pfile yields a blank char, not 0), so the
 * REAL "does this character exist" gate is the caller's find()!=0 check before it
 * hands a name here.
 */
PCharacter * account_enter_char(Descriptor *d, const DLString &charName);

/**
 * Reconnect descriptor `d` into an OWNED character's LINKDEAD body already in the
 * world, instead of cold-loading a duplicate: the take-over sibling of
 * account_enter_char that `account switch` uses when the chosen character is
 * lostlink. Same caller contract (the current character is already left the world),
 * same quiet CON_RESUME take-over the entry token and web resume do. Returns the
 * reconnected character, or 0 on a null argument.
 */
PCharacter * account_reconnect_char(Descriptor *d, PCharacter *twin);

/**
 * Mint a one-use entry token for (accountId, charName). Credential-grade like a
 * resume token: 128 bits from /dev/urandom, 180s TTL, single-use, never logged (a
 * re-mint no longer evicts the previous token, so several can be live at once).
 * Called by the /account/enter servlet AFTER it has
 * confirmed the character is on the account; the browser never sees the token --
 * the web broker (holding the web token) keeps it server-side and hands the
 * client only the moment-to-moment `account_enter` command.
 */
DLString entry_token_issue(const DLString &accountId, const DLString &charName);

/**
 * Redeem a token on descriptor `d`: cold-load the owned character, or take over
 * its linkdead body if it is already in the world. Any pre-login nanny state on
 * `d` -- the throwaway newbie shell the web client's codepage answer pins on
 * every fresh descriptor -- is disposed of first, the way a dropped link would
 * be; a descriptor whose character is in the world (CON_PLAYING) is refused,
 * that being `account switch`'s job. Burns the token on any final outcome.
 * Returns false and leaves `d` at the login prompt for an unknown/expired/spent
 * token, a playing descriptor, a target mid-login on another connection, or a
 * character that no longer exists.
 */
bool entry_token_redeem(Descriptor *d, const DLString &token);

#endif
