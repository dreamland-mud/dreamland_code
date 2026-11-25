#include <jsoncpp/json/json.h>
#include "configurable.h"
#include "lasthost.h"
#include "lastdevice.h"
#include "hostindex.h"
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

bool XMLAttributeLastHost::prune(time_t seed, time_t cutoff, bool apply, int &seeded, int &dropped)
{
    if (apply)
        return pruneRows(hosts, seen, seed, cutoff, seeded, dropped);

    Hosts counts = hosts;
    Dates dates = seen;
    return pruneRows(counts, dates, seed, cutoff, seeded, dropped);
}

time_t XMLAttributeLastHost::seenOf(const DLString &host, time_t fallback) const
{
    auto d = seen.find(host);
    return d == seen.end() ? fallback : d->second.getValue();
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
    HostIndex::addHost(pch->getName(), host);
    pch->save();
}

/*
 * Retention: addresses and web device ids are personal data, kept
 * keep_days (config/host_retention.json) after last seen, then deleted; the
 * owner's last address goes with them. Runs once per boot, right after the
 * playerbase loads, never on a plugin reload: the first pass dates and saves
 * every pfile, and that must not happen with players online. With "apply"
 * off it only counts and logs, so a new rule can be checked on live data first.
 */
static const int MIN_KEEP_DAYS = 180;
static Json::Value retentionConfig;

CONFIGURABLE_LOADED(config, host_retention)
{
    retentionConfig = value;
}

class HostRetentionTask : public SchedulerTaskRoundPlugin {
public:
    typedef ::Pointer<HostRetentionTask> Pointer;

    virtual void run( );

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

private:
    void sweep( bool apply, int keepDays );
    void sweepOne( PCMemoryInterface *pci, bool apply, time_t cutoff, int &chars, int &undated,
                   int &hostsDropped, int &devicesDropped, int &lastCleared, int &saved );
};

void HostRetentionTask::run( )
{
    // A missing or broken config must never delete anything, nor stop the boot:
    // operator[] throws on a root that is not an object.
    try {
        const Json::Value &cfg = retentionConfig;
        bool apply = false;
        int keepDays = 0;

        if (cfg.isObject( )) {
            apply = cfg["apply"].isBool( ) && cfg["apply"].asBool( );
            keepDays = cfg["keep_days"].isInt( ) ? cfg["keep_days"].asInt( ) : 0;
        }

        if (keepDays < MIN_KEEP_DAYS) {
            LogStream::sendError( ) << "Host retention: keep_days " << keepDays << " is below "
                                    << MIN_KEEP_DAYS << " or config/host_retention.json is unreadable, "
                                    << "counting with 365 and writing nothing" << endl;
            keepDays = 365;
            apply = false;
        }

        sweep( apply, keepDays );
    } catch (const std::exception &e) {
        LogStream::sendError( ) << "Host retention: " << e.what( ) << endl;
    }
}

void HostRetentionTask::sweepOne( PCMemoryInterface *pci, bool apply, time_t cutoff, int &chars, int &undated,
                                  int &hostsDropped, int &devicesDropped, int &lastCleared, int &saved )
{
    time_t seed = pci->getLastAccessTime( ).getTime( );
    bool changed = false;

    XMLAttributeLastHost::Pointer hosts = pci->getAttributes( ).findAttr<XMLAttributeLastHost>("lasthost");
    XMLAttributeDevices::Pointer devices = pci->getAttributes( ).findAttr<XMLAttributeDevices>("devices");

    // The last address is as old as its own row (owner's last login when undated).
    if (!pci->getLastAccessHost( ).empty( )) {
        time_t lastSeen = hosts ? hosts->seenOf( pci->getLastAccessHost( ), seed ) : seed;
        if (lastSeen < cutoff) {
            lastCleared++;
            changed = true;
            if (apply)
                pci->setLastAccessHost( DLString::emptyString );
        }
    }

    if (hosts) {
        chars++;
        changed = hosts->prune( seed, cutoff, apply, undated, hostsDropped ) || changed;
    }

    if (devices)
        changed = devices->prune( seed, cutoff, apply, undated, devicesDropped ) || changed;

    if (changed) {
        if (apply)
            PCharacterManager::saveMemory( pci );
        saved++;
    }
}

void HostRetentionTask::sweep( bool apply, int keepDays )
{
    time_t cutoff = dreamland->getCurrentTime( ) - keepDays * (time_t)Date::SECOND_IN_DAY;
    int chars = 0, undated = 0, hostsDropped = 0, devicesDropped = 0, lastCleared = 0, saved = 0, failed = 0;
    clock_t started = clock( );

    for (auto &p: PCharacterManager::getPCM( )) {
        // One bad profile must not stop retention for everyone after it.
        try {
            sweepOne( p.second, apply, cutoff, chars, undated, hostsDropped, devicesDropped, lastCleared, saved );
        } catch (const std::exception &e) {
            failed++;
            LogStream::sendError( ) << "Host retention: " << p.first << ": " << e.what( ) << endl;
        }
    }

    LogStream::sendNotice( )
        << "Host retention (" << (apply ? "applied" : "dry run, nothing written") << ", keep "
        << keepDays << " days, cutoff " << Date::getTimeAsString( cutoff, "%Y-%m-%d" ) << "): "
        << chars << " players with addresses, " << undated << " undated rows, "
        << hostsDropped << " addresses and " << devicesDropped << " devices dropped, "
        << lastCleared << " last addresses cleared, " << saved << " pfiles " << (apply ? "saved" : "to save")
        << ", " << failed << " failed, " << (clock( ) - started) * 1000 / CLOCKS_PER_SEC << " ms cpu" << endl;
}

PluginInitializer<HostRetentionTask> initHostRetention;
