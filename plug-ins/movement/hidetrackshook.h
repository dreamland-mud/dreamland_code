#ifndef HIDETRACKSHOOK_H
#define HIDETRACKSHOOK_H

class Character;

/**
 * A Beastmaster's hidden trail, for code that can't link the craft plugin.
 * The craft plugin sets the hook on load and must reset it to NULL on unload.
 * True: ch leaves no footprint on this step.
 */
typedef bool (*HideTracksHook)( Character * );
extern HideTracksHook hide_tracks_hook;

#endif
