#include "lasthost.h"
#include "lastdevice.h"
#include "netlists.h"
#include "logstream.h"
#include "pcharactermanager.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "schedulertaskroundplugin.h"
#include "plugininitializer.h"
#include "dlscheduler.h"
#include "dreamland.h"
#include "date.h"
#include "act.h"

void XMLAttributeLastHost::addHost(const DLString &host, time_t now)
{
    hosts[host]++;
    seen[host] = now;
}

void XMLAttributeLastHost::touchHost(const DLString &host, time_t now)
{
    if (hosts.isAvailable(host))
        seen[host] = now;
}

bool XMLAttributeLastHost::hasHost(const DLString &host) const
{
    return hosts.isAvailable(host);
}

const DLString &XMLAttributeLastHost::getMatchingHost(const DLString &hostPrefix) const
{
    Hosts::const_iterator h;
    for (h = hosts.begin(); h != hosts.end(); h++)
        if (hostPrefix.strPrefix(h->first))
            return h->first;

    return DLString::emptyString;
}

bool XMLAttributeLastHost::isUnique(const DLString &playerName, const DLString &host)
{
    for (auto &p: PCharacterManager::getPCM()) {
        if (playerName != p.first) {
            XMLAttributeLastHost::Pointer attr = p.second->getAttributes().findAttr<XMLAttributeLastHost>("lasthost");
            if (attr && attr->hasHost(host))
                return false;
        }
    }

    return true;
}

bool XMLAttributeLastHost::prune(time_t seed, time_t cutoff, int &seeded, int &dropped)
{
    return pruneRows(hosts, seen, seed, cutoff, seeded, dropped);
}

bool XMLAttributeLastHost::pruneRows(Hosts &counts, Dates &dates, time_t seed, time_t cutoff, int &seeded, int &dropped)
{
    bool changed = false;

    for (auto c = counts.begin(); c != counts.end(); ) {
        auto d = dates.find(c->first);

        // Rows written before dates existed: the owner's last login is the
        // latest the address can have been used.
        if (d == dates.end()) {
            d = dates.insert(Dates::value_type(c->first, XMLLong(seed))).first;
            seeded++;
            changed = true;
        }

        if (d->second.getValue() < cutoff) {
            dates.erase(d);
            c = counts.erase(c);
            dropped++;
            changed = true;
        } else {
            c++;
        }
    }

    // A date with no count row beside it means nothing.
    for (auto d = dates.begin(); d != dates.end(); )
        if (!counts.isAvailable(d->first)) {
            d = dates.erase(d);
            changed = true;
        } else {
            d++;
        }

    return changed;
}

DLString XMLAttributeLastHost::seenAsString(const Dates &dates, const DLString &key)
{
    auto d = dates.find(key);
    if (d == dates.end())
        return "-";

    return Date::getTimeAsString(d->second.getValue(), "%Y-%m-%d");
}

/**
 *  Outputs host/usage pairs sorted by usage in descending order.
 */
void XMLAttributeLastHost::showHosts(ostringstream &buf) const
{
    Hosts::ValueList list = hosts.toSortedList();

    for (Hosts::ValueList::const_iterator v = list.begin(); v != list.end(); v++) {
        buf << fmt(0, "%16s %4d %10s", v->first.c_str(), v->second.getValue(),
                   seenAsString(seen, v->first).c_str());

        DLString tag = NetLists::tag(v->first);
        DLString org = NetLists::org(v->first);
        if (!tag.empty())
            buf << " {R" << tag << "{x";
        if (!org.empty())
            buf << " " << org;
        buf << endl;
    }
}

void XMLAttributeLastHostListenerPlugin::run(int oldState, int newState, Descriptor *d)
{
    Character *ch = d->character;

    if (!ch)
        return;

    if (newState != CON_PLAYING)
        return;

    PCharacter *pch = ch->getPC();
    DLString host = d->getRealHost();
    time_t now = dreamland->getCurrentTime();

    // Don't increment usage counter if reconnecting with the same IP address.
    // A resumed web session counts as reconnecting -- otherwise every screen
    // lock would bump the counter and write the pfile. The date still moves;
    // it reaches the pfile with the next save.
    if (pch->getLastAccessHost() == host
            && (oldState == CON_BREAK_CONNECT || oldState == CON_RESUME)) {
        XMLAttributeLastHost::Pointer known = pch->getAttributes().findAttr<XMLAttributeLastHost>("lasthost");
        if (known)
            known->touchHost(host, now);
        return;
    }

    // Remember current host and increment usage counter.
    pch->setLastAccessHost(host);
    XMLAttributeLastHost::Pointer attr = pch->getAttributes( ).getAttr<XMLAttributeLastHost>("lasthost");
    attr->addHost(host, now);
    pch->save();
}

/*
 * Retention: addresses and web device ids are kept 12 months after last seen
 * (multing tools, decision 3). Runs at boot, right after the playerbase
 * loads, then daily. A plugin reload drops the daily run until the next boot:
 * the first pass dates and saves every pfile, and that must not happen with
 * players online.
 */
static const time_t HOST_KEEP = 365 * (time_t)Date::SECOND_IN_DAY;

class HostRetentionTask : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<HostRetentionTask> Pointer;

    virtual void run( );

    virtual void after( )
    {
        DLScheduler::getThis( )->putTaskInSecond( Date::SECOND_IN_DAY, Pointer( this ) );
    }

    // After PlayerLoadTask (SCDP_BOOT + 10) has filled the player list at boot.
    virtual int getPriority( ) const
    {
        return SCDP_AUTO + 22;
    }

protected:
    virtual void initialization( )
    {
        if (DLScheduler::getThis( )->getCurrentTick( ) == 0)
            DLScheduler::getThis( )->putTaskNOW( Pointer( this ) );
    }
};

void HostRetentionTask::run( )
{
    time_t now = dreamland->getCurrentTime( );
    time_t cutoff = now - HOST_KEEP;
    int chars = 0, seeded = 0, hostsDropped = 0, devicesDropped = 0, saved = 0;
    clock_t started = clock( );

    for (auto &p: PCharacterManager::getPCM( )) {
        PCMemoryInterface *pci = p.second;
        time_t seed = pci->getLastAccessTime( ).getTime( );
        bool changed = false;

        XMLAttributeLastHost::Pointer hosts = pci->getAttributes( ).findAttr<XMLAttributeLastHost>("lasthost");
        if (hosts) {
            chars++;
            changed = hosts->prune( seed, cutoff, seeded, hostsDropped ) || changed;
        }

        XMLAttributeDevices::Pointer devices = pci->getAttributes( ).findAttr<XMLAttributeDevices>("devices");
        if (devices)
            changed = devices->prune( seed, cutoff, seeded, devicesDropped ) || changed;

        if (changed) {
            PCharacterManager::saveMemory( pci );
            saved++;
        }
    }

    LogStream::sendNotice( )
        << "Host retention: " << chars << " players with addresses, " << seeded << " rows dated, "
        << hostsDropped << " addresses and " << devicesDropped << " devices older than "
        << Date::getTimeAsString( cutoff, "%Y-%m-%d" ) << " dropped, " << saved << " saved in "
        << (clock( ) - started) * 1000 / CLOCKS_PER_SEC << " ms cpu" << endl;
}

PluginInitializer<HostRetentionTask> initHostRetention;
