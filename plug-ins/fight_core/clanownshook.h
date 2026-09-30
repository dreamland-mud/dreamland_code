#ifndef CLANOWNSHOOK_H
#define CLANOWNSHOOK_H

class Character;
class DLString;

/**
 * Clan catalog purchases for code that can't link the clan plugin (fight, skills).
 * The clan plugin sets the hook on load and must reset it to NULL on unload.
 */
typedef bool (*ClanOwnsHook)( Character *, const DLString & );
extern ClanOwnsHook clan_owns_hook;

/** True if ch's clan is reformed and owns catalog item id. False while the clan plugin is unloaded. */
bool clan_char_owns( Character *ch, const DLString &id );

typedef int (*ClanPowerHook)( Character * );
extern ClanPowerHook clan_power_hook;

/** rankPower of ch's reformed clan at ch's rank, in percent. -1 while the clan plugin is unloaded. */
int clan_char_power( Character *ch );

#endif
