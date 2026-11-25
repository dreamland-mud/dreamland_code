/* Dream Land, 2026.
 */
#include "lastdevice.h"
#include "lasthost.h"
#include "dreamland.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "act.h"

void XMLAttributeDevices::addDevice(const DLString &device, time_t now)
{
    devices[device]++;
    seen[device] = now;
}

void XMLAttributeDevices::touchDevice(const DLString &device, time_t now)
{
    if (devices.isAvailable(device))
        seen[device] = now;
}

bool XMLAttributeDevices::hasDevice(const DLString &device) const
{
    return devices.isAvailable(device);
}

bool XMLAttributeDevices::prune(time_t seed, time_t cutoff, bool apply, int &seeded, int &dropped)
{
    if (apply)
        return XMLAttributeLastHost::pruneRows(devices, seen, seed, cutoff, seeded, dropped);

    Devices counts = devices;
    Dates dates = seen;
    return XMLAttributeLastHost::pruneRows(counts, dates, seed, cutoff, seeded, dropped);
}

/**
 *  Outputs device/usage pairs sorted by usage in descending order.
 */
void XMLAttributeDevices::showDevices(ostringstream &buf) const
{
    Devices::ValueList list = devices.toSortedList();

    for (Devices::ValueList::const_iterator v = list.begin(); v != list.end(); v++)
        buf << fmt(0, "%s %4d %10s", v->first.c_str(), v->second.getValue(),
                   XMLAttributeLastHost::seenAsString(seen, v->first).c_str()) << endl;
}

void XMLAttributeDevicesListenerPlugin::run(int oldState, int newState, Descriptor *d)
{
    Character *ch = d->character;

    if (!ch || newState != CON_PLAYING || d->client_device.empty())
        return;

    PCharacter *pch = ch->getPC();
    if (!pch)
        return;

    time_t now = dreamland->getCurrentTime();

    // Same exemption as lasthost: a link-dead reconnect or a resumed web
    // session from a device already on record is not a new use. The date
    // still moves and reaches the pfile with the next save.
    if (oldState == CON_BREAK_CONNECT || oldState == CON_RESUME) {
        XMLAttributeDevices::Pointer known = pch->getAttributes().findAttr<XMLAttributeDevices>("devices");
        if (known && known->hasDevice(d->client_device)) {
            known->touchDevice(d->client_device, now);
            return;
        }
    }

    XMLAttributeDevices::Pointer attr = pch->getAttributes().getAttr<XMLAttributeDevices>("devices");
    attr->addDevice(d->client_device, now);
    pch->save();
}
