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
 * last-host listeners, exactly like a login) -> look. Returns the new character,
 * or 0 if it could not be created. Mirrors backdoorhandler's fresh-load path
 * (backdoorhandler.cpp:108-133).
 */
PCharacter * account_enter_char(Descriptor *d, const DLString &charName);

/**
 * Mint a one-use entry token for (accountId, charName). Credential-grade like a
 * resume token: 128 bits from /dev/urandom, ~90s TTL, single-use, one live token
 * per account, never logged. Called by the /account/enter servlet AFTER it has
 * confirmed the character is on the account; the browser never sees the token --
 * the web broker (holding the web token) keeps it server-side and hands the
 * client only the moment-to-moment `account_enter` command.
 */
DLString entry_token_issue(const DLString &accountId, const DLString &charName);

/**
 * Redeem a token on the character-less descriptor `d`: cold-load the owned
 * character, or take over its linkdead body if it is already in the world. Burns
 * the token on any final outcome. Returns false and leaves `d` untouched at the
 * login prompt for an unknown/expired/spent token, a descriptor that already has
 * a character, a target being played on a live connection, or a character that no
 * longer exists.
 */
bool entry_token_redeem(Descriptor *d, const DLString &token);

#endif
