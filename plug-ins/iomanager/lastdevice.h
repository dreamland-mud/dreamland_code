/* Dream Land, 2026.
 */
#ifndef LASTDEVICE_H
#define LASTDEVICE_H

#include "xmlmap.h"
#include "xmlinteger.h"
#include "xmllong.h"
#include "xmlvariablecontainer.h"

#include "descriptorstatelistener.h"
#include "playerattributes.h"

/**
 * Web client device ids this player logged in from (the client_device RPC),
 * mirroring XMLAttributeLastHost: id -> use count and last-seen date, kept
 * across remorts.
 * The referral checks compare it between characters.
 */
class XMLAttributeDevices : public RemortAttribute, public XMLVariableContainer {
XML_OBJECT;
public:
        typedef ::Pointer<XMLAttributeDevices> Pointer;
        typedef XMLMapBase<XMLInteger> Devices;
        typedef XMLMapBase<XMLLong> Dates;

        void addDevice(const DLString &device, time_t now);
        void touchDevice(const DLString &device, time_t now);
        bool hasDevice(const DLString &device) const;
        void showDevices(ostringstream &buf) const;
        const Devices & getDevices() const { return devices; }

        /** Same rule as XMLAttributeLastHost::prune. */
        bool prune(time_t seed, time_t cutoff, bool apply, int &seeded, int &dropped);

private:
        /** Map between device id and how many times it was used. */
        XML_VARIABLE Devices devices;

        /** Map between device id and when it was last used (unix time). */
        XML_VARIABLE Dates seen;
};

class XMLAttributeDevicesListenerPlugin : public DescriptorStateListener {
public:
        typedef ::Pointer<XMLAttributeDevicesListenerPlugin> Pointer;

        virtual void run( int, int, Descriptor * );
};
#endif
