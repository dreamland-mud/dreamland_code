/* Dream Land, 2026.
 */
#include "lastdevice.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "act.h"

void XMLAttributeDevices::addDevice(const DLString &device)
{
    devices[device]++;
}

bool XMLAttributeDevices::hasDevice(const DLString &device) const
{
    return devices.isAvailable(device);
}

/**
 *  Outputs device/usage pairs sorted by usage in descending order.
 */
void XMLAttributeDevices::showDevices(ostringstream &buf) const
{
    Devices::ValueList list = devices.toSortedList();

    for (Devices::ValueList::const_iterator v = list.begin(); v != list.end(); v++)
        buf << fmt(0, "%s %4d", v->first.c_str(), v->second) << endl;
}

void XMLAttributeDevicesListenerPlugin::run(int oldState, int newState, Descriptor *d)
{
    Character *ch = d->character;

    if (!ch || newState != CON_PLAYING || d->client_device.empty())
        return;

    PCharacter *pch = ch->getPC();
    if (!pch)
        return;

    // Same exemption as lasthost: a link-dead reconnect or a resumed web
    // session from a device already on record is not a new use.
    if (oldState == CON_BREAK_CONNECT || oldState == CON_RESUME) {
        XMLAttributeDevices::Pointer known = pch->getAttributes().findAttr<XMLAttributeDevices>("devices");
        if (known && known->hasDevice(d->client_device))
            return;
    }

    XMLAttributeDevices::Pointer attr = pch->getAttributes().getAttr<XMLAttributeDevices>("devices");
    attr->addDevice(d->client_device);
    pch->save();
}
