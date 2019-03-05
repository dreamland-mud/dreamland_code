#include "lasthost.h"
#include "logstream.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "act.h"

void XMLAttributeLastHost::addHost(const DLString &host)
{
    hosts[host]++;
}

bool XMLAttributeLastHost::hasHost(const DLString &host) const
{
    return hosts.find(host) != hosts.end();
}

static bool compare_values(const pair<DLString, XMLInteger> &a, const pair<DLString, XMLInteger> &b)
{
    return a.second >= b.second;
}

/**
 *  Outputs host/usage pairs sorted by usage in descending order.
 */
void XMLAttributeLastHost::showHosts(ostringstream &buf) const
{
    // TODO to base class or utility exension
    typedef list<pair<DLString, XMLInteger> > SortedList;
    SortedList sorted(hosts.begin(), hosts.end());
    sorted.sort(compare_values);

    for (SortedList::const_iterator s = sorted.begin(); s != sorted.end(); s++) 
        buf << dlprintf("%16s %4d", s->first.c_str(), s->second) << endl;
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

    // Don't increment usage counter if reconnecting with the same IP address.
    if (pch->getLastAccessHost() == host && oldState == CON_BREAK_CONNECT)
        return;
    
    // Remember current host and increment usage counter.
    pch->setLastAccessHost(host);
    XMLAttributeLastHost::Pointer attr = pch->getAttributes( ).getAttr<XMLAttributeLastHost>("lasthost");
    attr->addHost(host);
    pch->save();
}