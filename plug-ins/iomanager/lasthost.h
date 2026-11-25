#ifndef LASTHOST_H
#define LASTHOST_H

#include "xmlmap.h"
#include "xmlinteger.h"
#include "xmllong.h"
#include "xmlstring.h"
#include "xmlvariablecontainer.h"

#include "descriptorstatelistener.h"
#include "playerattributes.h"

class PCMemoryInterface;

/**
 * Keep track of IP addresses this player uses.
 */
class XMLAttributeLastHost : public RemortAttribute, public XMLVariableContainer {
XML_OBJECT;
public:
        typedef ::Pointer<XMLAttributeLastHost> Pointer;
        typedef XMLMapBase<XMLInteger> Hosts;
        typedef XMLMapBase<XMLLong> Dates;

        void addHost(const DLString &host, time_t now);
        void touchHost(const DLString &host, time_t now);
        bool hasHost(const DLString &host) const;
        void showHosts(ostringstream &buf) const;
        const Hosts & getHosts() const { return hosts; }
        const DLString &getMatchingHost(const DLString &hostPrefix) const;

        /** Undated rows get `seed`, rows last seen before `cutoff` go. True if anything changed. */
        bool prune(time_t seed, time_t cutoff, int &seeded, int &dropped);

        /** Check if this player has a unique ip address */
        static bool isUnique(const DLString &playerName, const DLString &host);

        /** The prune rule for any count map with a last-seen map beside it (hosts, web devices). */
        static bool pruneRows(Hosts &counts, Dates &dates, time_t seed, time_t cutoff, int &seeded, int &dropped);

        /** "YYYY-MM-DD" of a row's last-seen date, "-" when undated. */
        static DLString seenAsString(const Dates &dates, const DLString &key);

private:
        /** Map between IP address and how many times it was used. */
        XML_VARIABLE Hosts hosts;

        /** Map between IP address and when it was last used (unix time). */
        XML_VARIABLE Dates seen;
};

class XMLAttributeLastHostListenerPlugin : public DescriptorStateListener {
public:
        typedef ::Pointer<XMLAttributeLastHostListenerPlugin> Pointer;

        virtual void run( int, int, Descriptor * );
};
#endif
