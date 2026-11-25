/* Dream Land, 2026.
 */
#ifndef LASTDEVICE_H
#define LASTDEVICE_H

#include "xmlmap.h"
#include "xmlinteger.h"
#include "xmlvariablecontainer.h"

#include "descriptorstatelistener.h"
#include "playerattributes.h"

/**
 * Web client device ids this player logged in from (the client_device RPC),
 * mirroring XMLAttributeLastHost: id -> use count, kept across remorts.
 * The referral checks compare it between characters.
 */
class XMLAttributeDevices : public RemortAttribute, public XMLVariableContainer {
XML_OBJECT;
public:
        typedef ::Pointer<XMLAttributeDevices> Pointer;
        typedef XMLMapBase<XMLInteger> Devices;

        void addDevice(const DLString &device);
        bool hasDevice(const DLString &device) const;
        void showDevices(ostringstream &buf) const;
        const Devices & getDevices() const { return devices; }

private:
        /** Map between device id and how many times it was used. */
        XML_VARIABLE Devices devices;
};

class XMLAttributeDevicesListenerPlugin : public DescriptorStateListener {
public:
        typedef ::Pointer<XMLAttributeDevicesListenerPlugin> Pointer;

        virtual void run( int, int, Descriptor * );
};
#endif
