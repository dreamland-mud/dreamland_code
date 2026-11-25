/* Dream Land, 2026.
 */
#ifndef NETLISTS_H
#define NETLISTS_H

#include "dlstring.h"

/**
 * What kind of network an IP address belongs to, from local copies of public
 * lists in db/netlists/ that netlists-refresh.py (dreamland_tools) downloads
 * daily: X4BNet lists_vpn (VPN and datacenter ranges), the Tor exit list and
 * iptoasn.com ip2asn-combined. No address is ever sent anywhere.
 *
 * Tables load lazily on the first lookup and reload when a file's mtime
 * changes (checked at most once a minute). A missing or broken file is an
 * empty table, never an error. Flags only: nothing blocks on them.
 *
 * All calls take one address or a lasthost-style chain "a, b" (a proxy's
 * X-Forwarded-For): tag answers the strongest tag of any link, org joins the
 * provider of every link with " / " (provider names contain commas), lastHop
 * reads only the last link.
 */
namespace NetLists {
    // "tor", "vpn", "dc" or "" (precedence tor > vpn > dc).
    DLString tag(const DLString &ip);
    // "AS<n> <description>" or "".
    DLString org(const DLString &ip);
    // The last link of a chain as a plain address, or "" when it is not one.
    // An IPv4-mapped IPv6 address comes back as dotted IPv4.
    DLString lastHop(const DLString &chain);
}

#endif
