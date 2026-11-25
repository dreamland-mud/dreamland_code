/* Dream Land, 2026.
 */
#ifndef HOSTINDEX_H
#define HOSTINDEX_H

#include <list>
#include <map>
#include "dlstring.h"
#include "pcharactermemorylist.h"

/**
 * Who used an address or a web client device id, read from the in-RAM
 * playerbase, so one question costs a map lookup instead of a walk over
 * every profile.
 *
 * Built on first use and fed by the connect listeners. Every read checks
 * each name against its profile, so a deleted char or a pruned row drops out
 * on its own. Renames and undeletes happen in loadsave, out of sight, and a
 * read during a remort (the char is briefly out of the player list) drops
 * the name, so the index rebuilds itself once it is older than REBUILD_SECONDS.
 */
class HostIndex {
public:
    typedef std::list<DLString> Names;
    /** Char name and how many times it logged in from the address. */
    typedef std::pair<DLString, int> Login;
    typedef std::list<Login> Logins;
    /** Char name -> the address it matched, ordered like the player list. */
    typedef std::map<DLString, DLString, MapCompareIgnoreCase> Matches;

    /** Chars that ever used exactly this address. */
    static Names hostOwners(const DLString &host);
    /** The same, each with its login count from that address. */
    static Logins hostLogins(const DLString &host);

    /** Chars that used an address starting with prefix, each with the first
     *  such address (the exact one when it has it). */
    static Matches hostPrefixOwners(const DLString &prefix);

    /** Chars that ever logged in from this web client device id. */
    static Names deviceOwners(const DLString &device);

    /** True when no char but playerName ever used host. */
    static bool hostUnique(const DLString &playerName, const DLString &host);

    /** A connect listener recorded a new row for name. */
    static void addHost(const DLString &name, const DLString &host);
    static void addDevice(const DLString &name, const DLString &device);

    static const int REBUILD_SECONDS = 600;
};

#endif
