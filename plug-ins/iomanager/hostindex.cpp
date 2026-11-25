/* Dream Land, 2026.
 */
#include <set>
#include "hostindex.h"
#include "lasthost.h"
#include "lastdevice.h"
#include "pcharactermanager.h"
#include "pcmemoryinterface.h"
#include "dreamland.h"

typedef std::set<DLString, MapCompareIgnoreCase> NameSet;
typedef std::map<DLString, NameSet> Index;

static Index hostIndex, deviceIndex;
static bool built = false;
static time_t builtAt = 0;

/** The profile filed under this exact player-list key. PCharacterManager::find
 *  would walk the whole list for a name it takes for Cyrillic. */
static PCMemoryInterface * profile(const DLString &name)
{
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM();
    PCharacterMemoryList::const_iterator p = pcm.find(name);
    return p == pcm.end() ? NULL : p->second;
}

/** How many times name logged in from host; -1 when the row is gone. */
static int host_logins(const DLString &name, const DLString &host)
{
    PCMemoryInterface *pci = profile(name);
    if (!pci)
        return -1;

    XMLAttributeLastHost::Pointer attr = pci->getAttributes().findAttr<XMLAttributeLastHost>("lasthost");
    if (!attr)
        return -1;

    auto row = attr->getHosts().find(host);
    return row == attr->getHosts().end() ? -1 : row->second.getValue();
}

static int device_logins(const DLString &name, const DLString &device)
{
    PCMemoryInterface *pci = profile(name);
    if (!pci)
        return -1;

    XMLAttributeDevices::Pointer attr = pci->getAttributes().findAttr<XMLAttributeDevices>("devices");
    if (!attr)
        return -1;

    auto row = attr->getDevices().find(device);
    return row == attr->getDevices().end() ? -1 : row->second.getValue();
}

static void build()
{
    hostIndex.clear();
    deviceIndex.clear();

    for (auto &p: PCharacterManager::getPCM()) {
        XMLAttributeLastHost::Pointer hosts = p.second->getAttributes().findAttr<XMLAttributeLastHost>("lasthost");
        if (hosts)
            for (auto &row: hosts->getHosts())
                hostIndex[row.first].insert(p.first);

        XMLAttributeDevices::Pointer devices = p.second->getAttributes().findAttr<XMLAttributeDevices>("devices");
        if (devices)
            for (auto &row: devices->getDevices())
                deviceIndex[row.first].insert(p.first);
    }

    built = true;
    builtAt = dreamland->getCurrentTime();
}

static void refresh()
{
    if (!built || dreamland->getCurrentTime() - builtAt >= HostIndex::REBUILD_SECONDS)
        build();
}

/** Names filed under i that still hold the row, with their login counts.
 *  The rest leave the index, and so does the key once nobody holds it. */
static HostIndex::Logins verified(Index &index, Index::iterator i, int (*logins)(const DLString &, const DLString &))
{
    HostIndex::Logins result;

    for (auto n = i->second.begin(); n != i->second.end(); ) {
        int count = logins(*n, i->first);

        if (count >= 0) {
            result.push_back(HostIndex::Login(*n, count));
            n++;
        } else {
            n = i->second.erase(n);
        }
    }

    if (i->second.empty())
        index.erase(i);

    return result;
}

static HostIndex::Names names_of(const HostIndex::Logins &logins)
{
    HostIndex::Names names;

    for (auto &l: logins)
        names.push_back(l.first);

    return names;
}

HostIndex::Logins HostIndex::hostLogins(const DLString &host)
{
    refresh();

    Index::iterator i = hostIndex.find(host);
    if (i == hostIndex.end())
        return Logins();

    return verified(hostIndex, i, host_logins);
}

HostIndex::Names HostIndex::hostOwners(const DLString &host)
{
    return names_of(hostLogins(host));
}

HostIndex::Matches HostIndex::hostPrefixOwners(const DLString &prefix)
{
    Matches matches;

    refresh();

    // Keys starting with prefix sit together from lower_bound(prefix) on, the
    // exact key first: the address the old per-char scan picked too, since a
    // char's own map runs in the same order.
    Index::iterator i = hostIndex.lower_bound(prefix);
    while (i != hostIndex.end() && i->first.compare(0, prefix.size(), prefix) == 0) {
        Index::iterator next = i;
        next++;

        DLString host = i->first;
        for (auto &l: verified(hostIndex, i, host_logins))
            if (matches.find(l.first) == matches.end())
                matches[l.first] = host;

        i = next;
    }

    return matches;
}

HostIndex::Names HostIndex::deviceOwners(const DLString &device)
{
    refresh();

    Index::iterator i = deviceIndex.find(device);
    if (i == deviceIndex.end())
        return Names();

    return names_of(verified(deviceIndex, i, device_logins));
}

bool HostIndex::hostUnique(const DLString &playerName, const DLString &host)
{
    for (auto &name: hostOwners(host))
        if (!(name ^ playerName))
            return false;

    return true;
}

void HostIndex::addHost(const DLString &name, const DLString &host)
{
    // Not built yet: the first read takes every row from the profiles anyway.
    if (built)
        hostIndex[host].insert(name);
}

void HostIndex::addDevice(const DLString &name, const DLString &device)
{
    if (built)
        deviceIndex[device].insert(name);
}
