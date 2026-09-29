#include "pvp.h"
#include "xmlpvpattribute.h"
#include "lasthost.h"
#include "accountmanager.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "dreamland.h"
#include "merc.h"

static bool pvp_same_account(PCMemoryInterface *a, PCMemoryInterface *b)
{
    DLString account = AccountManager::accountOf(a->getName());
    return !account.empty() && account == AccountManager::accountOf(b->getName());
}

static bool pvp_eligible(PCMemoryInterface *killer, PCMemoryInterface *victim)
{
    if (killer->getName() == victim->getName())
        return false;

    if (killer->get_trust() >= LEVEL_IMMORTAL || victim->get_trust() >= LEVEL_IMMORTAL)
        return false;

    return !pvp_same_account(killer, victim);
}

static bool pvp_record(PCMemoryInterface *killer, PCMemoryInterface *victim, time_t when)
{
    // NULL when a script already holds the name with another attribute type.
    XMLPvpAttribute::Pointer attr = killer->getAttributes().getAttr<XMLPvpAttribute>("pvp");
    return attr && attr->record(victim->getName(), when);
}

static bool pvp_hosts_shared(PCMemoryInterface *a, PCMemoryInterface *b)
{
    XMLAttributeLastHost::Pointer hostsA = a->getAttributes().findAttr<XMLAttributeLastHost>("lasthost");
    XMLAttributeLastHost::Pointer hostsB = b->getAttributes().findAttr<XMLAttributeLastHost>("lasthost");

    if (!hostsA || !hostsB)
        return false;

    for (auto &h: hostsA->getHosts())
        if (hostsB->hasHost(h.first))
            return true;

    return false;
}

bool pvp_count_kill(PCharacter *killer, PCharacter *victim)
{
    if (!pvp_eligible(killer, victim))
        return false;

    // A linkdead victim or two chars on one machine is how alts get farmed.
    if (!killer->desc || !victim->desc)
        return false;

    // The current host can be forged from the client side, so also refuse
    // pairs that ever played from the same address.
    if (DLString(killer->desc->getRealHost()) == victim->desc->getRealHost()
            || pvp_hosts_shared(killer, victim))
        return false;

    return pvp_record(killer, victim, dreamland->getCurrentTime());
}

bool pvp_seed_kill(PCMemoryInterface *killer, PCMemoryInterface *victim, time_t when)
{
    if (!pvp_eligible(killer, victim) || pvp_hosts_shared(killer, victim))
        return false;

    return pvp_record(killer, victim, when);
}
