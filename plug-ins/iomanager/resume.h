/* Session resume for web clients.
 *
 * A phone suspends the browser tab a second or two after the player switches
 * apps, and WebKit tears the WebSocket down with it -- the engine finds out
 * only when its next write returns EPIPE (1445 "Broken pipe" across the log
 * archive, against zero timeouts: the close always comes from the peer). No
 * server or nginx setting can hold that socket open, so the connection is
 * treated as expendable and the SESSION is made to survive it instead.
 *
 * Each web session carries a token, refreshed on every prompt and handed to
 * the client inside the web prompt. When the client comes back it presents the
 * token instead of a name and password, and the new descriptor is attached to
 * the character it left linkdead -- the same take-over `nanny.reconnect` does
 * after a manual re-login, minus the login and minus the announcement.
 *
 * The token is credential-grade for as long as it lives, so: 90 seconds, one
 * use (burned on presentation, valid or not), one live token per player, never
 * written to a log, and useless unless that character is actually in the world
 * with no descriptor on it.
 *
 * The window is 90 seconds and not longer because of what has to be true for a
 * resume to land: the body must still be in the world. char_update_lostlink()
 * runs every pulse and, with `lostlink: 0`, quits a descriptor-less player out
 * a quarter of a second after the socket dies -- so resume_pending() holds that
 * sweep off while a session can still come back, and every second of that is a
 * second the character stands there able to be attacked.
 */
#ifndef RESUME_H
#define RESUME_H

#include "dlstring.h"

class Descriptor;
class PCharacter;

/** This player's token, minted on first call and refreshed thereafter. */
DLString resume_token_issue(PCharacter *ch);

/** Forget this player's token (they quit, or it has just been spent). */
void resume_token_clear(PCharacter *ch);

/**
 * Attach `d` to the linkdead character the token was issued to.
 * Returns false -- and leaves the descriptor untouched, still at the login
 * prompt -- for any token that is unknown, expired, already spent, or whose
 * character is gone, playing elsewhere, or switched into a mob.
 */
bool resume_attach(Descriptor *d, const DLString &token);

/**
 * True while this player has a live token and lost their link recently enough
 * to use it -- the window in which char_update_lostlink() must leave the body
 * alone, or there will be nothing left to resume into.
 */
bool resume_pending(PCharacter *ch);

#endif
