#ifndef LASTHOST_H
#define LASTHOST_H

#include "xmlmap.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlvariablecontainer.h"

#include "descriptorstatelistener.h"
#include "playerattributes.h"

/**
 * Keep track of IP addresses this player uses.
 */
class XMLAttributeLastHost : public RemortAttribute, public XMLVariableContainer {
XML_OBJECT;
public:
        typedef ::Pointer<XMLAttributeLastHost> Pointer;

        void addHost(const DLString &host);
        bool hasHost(const DLString &host) const;
        void showHosts(ostringstream &buf) const;

private:
        /** Map between IP address and how many times it was used. */
        XML_VARIABLE XMLMapBase<XMLInteger> hosts;        

};        

class XMLAttributeLastHostListenerPlugin : public DescriptorStateListener {
public:
        typedef ::Pointer<XMLAttributeLastHostListenerPlugin> Pointer;

        virtual void run( int, int, Descriptor * );        
};
#endif